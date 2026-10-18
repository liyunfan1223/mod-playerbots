/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldEncounter_Mimiron.h"

#include "BossAuraTriggers.h"
#include "Creature.h"
#include "EncounterHelpers.h"
#include "Group.h"
#include "Item.h"
#include "LootMgr.h"
#include "Map.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "RaidInstanceState.h"
#include "ServerFacade.h"
#include "SpellAuras.h"
#include "ThreatManager.h"
#include "Timer.h"
#include "UldEncounterGate.h"
#include "UldHardMode.h"
#include "UldScripts.h"
#include "UldTargets.h"
#include "Unit.h"
#include "Vehicle.h"

#include <algorithm>
#include <cmath>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace EncounterHelpers;

const Position ULDUAR_MIMIRON_ROOM_CENTER = Position(2744.65f, 2569.46f, 364.32f);
const Position ULDUAR_MIMIRON_PHASE3_STAGE = Position(2762.65f, 2569.46f, 364.31f);
const Position ULDUAR_MIMIRON_PHASE4_TANK_SPOT = Position(2744.5754f, 2570.8657f, 364.3138f);
const Position ULDUAR_MIMIRON_PHASE1_TANK_SPOT = Position(2691.5762f, 2568.5315f, 364.3138f);
const Position ULDUAR_MIMIRON_PHASE1_STACK_SPOTS[ULDUAR_MIMIRON_PHASE1_STACK_COUNT] = {
    Position(2716.0465f, 2579.9422f, 364.3138f),  // 25 deg off the tank spot
    Position(2717.6562f, 2561.5434f, 364.3138f),  // 345
};

namespace
{
// Where NPC 33576 will be `seconds` from now: it laps the room clockwise, so rotate it about the room
// centre. Radius is read live, since its waypoint path isn't a true circle.
Position MimironOrbitAhead(Position const& now, float seconds)
{
    float const dx = now.GetPositionX() - ULDUAR_MIMIRON_ROOM_CENTER.GetPositionX();
    float const dy = now.GetPositionY() - ULDUAR_MIMIRON_ROOM_CENTER.GetPositionY();
    float const radius = std::sqrt(dx * dx + dy * dy);
    if (radius < 1.0f)
        return now;

    float const turned = -ULDUAR_MIMIRON_DB_TARGET_SPEED * seconds / radius;
    float const c = std::cos(turned);
    float const sn = std::sin(turned);

    return Position(ULDUAR_MIMIRON_ROOM_CENTER.GetPositionX() + dx * c - dy * sn,
                    ULDUAR_MIMIRON_ROOM_CENTER.GetPositionY() + dx * sn + dy * c,
                    now.GetPositionZ());
}
}  // namespace

float GetMimironSpinningUpSeconds(Unit* vx001)
{
    if (!vx001)
        return -1.0f;

    Spell* spinningUp = vx001->FindCurrentSpellBySpellId(SPELL_SPINNING_UP);
    if (!spinningUp)
        return -1.0f;

    // Clamped. The channel timer is decremented before the tick that ends the channel, so the last pass
    // can read a negative, and a negative here would predict the ignition backwards.
    return std::max(0.0f, static_cast<float>(spinningUp->GetCastTimeRemaining()) / IN_MILLISECONDS);
}

namespace
{
// The raw read. The public wrapper below folds it onto the trigger pass: it costs a 250 yd grid
// scan for the DB Target, and four nodes ask for the same cone on the same pass.
MimironBarrageWindow ReadMimironBarrageWindow(Player* bot, Unit* vx001)
{
    MimironBarrageWindow window;
    if (!bot || !vx001)
        return window;

    // Both timers read live, the 4 s Spinning Up channel and then the 10 s aura, so a bot joining
    // mid-cast still gets the right window.
    float fire = ULDUAR_MIMIRON_BARRAGE_FIRE_SECONDS;
    float const spinning = GetMimironSpinningUpSeconds(vx001);
    if (spinning >= 0.0f)
        window.untilLive = spinning;
    else if (Aura* barrage = vx001->GetAura(SPELL_P3WX2_LASER_BARRAGE_AURA_1))
        fire = static_cast<float>(barrage->GetDuration()) / IN_MILLISECONDS;
    else
        return window;

    window.valid = true;

    Creature* dbTarget = bot->FindNearestCreature(NPC_MIMIRON_DB_TARGET, ULDUAR_MIMIRON_DB_TARGET_SEARCH_RANGE);
    if (!dbTarget)
    {
        // Without 33576 FaceBarrageArc returns early and the cone stays frozen, so model a static wedge.
        window.lead = vx001->GetOrientation();
        return window;
    }

    Position const dbNow = dbTarget->GetPosition();
    Position const ignition = MimironOrbitAhead(dbNow, window.untilLive);
    Position const finish = MimironOrbitAhead(dbNow, window.untilLive + fire);

    window.lead = vx001->GetAngle(ignition.GetPositionX(), ignition.GetPositionY());

    float const tail = vx001->GetAngle(finish.GetPositionX(), finish.GetPositionY());
    window.sweep = Position::NormalizeOrientation(window.lead - tail);
    window.rate = fire > 0.0f ? window.sweep / fire : 0.0f;

    return window;
}
}  // namespace

bool IsMimironSpotMineSafe(Player* bot, Position const& dest, float clearance)
{
    if (!bot)
        return true;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
        return true;

    for (ObjectGuid const& guid : botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get())
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || unit->GetEntry() != NPC_PROXIMITY_MINE)
            continue;

        if (dest.GetExactDist2d(unit->GetPositionX(), unit->GetPositionY()) < clearance)
            return false;
    }

    return true;
}

namespace
{
MimironFirefighterHazards ReadMimironFirefighterHazards(PlayerbotAI* botAI)
{
    MimironFirefighterHazards hazards;
    if (!botAI || !IsMimironHardModeActive(botAI))
        return hazards;

    // Only spread flames draw a spray: the fire bot searches for NPC_FLAMES_SPREAD alone.
    std::vector<Position> spread;
    for (ObjectGuid const& guid : botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get())
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        switch (unit->GetEntry())
        {
            case NPC_FLAMES_SPREAD:
                spread.push_back(unit->GetPosition());
                // Only a node with the aura burns. The Flames (Initial) seed never does, and a put-out node drops
                // the aura 2.5 s before it despawns.
                if (unit->HasAura(SPELL_MIMIRON_FLAMES_AURA))
                    hazards.flames.push_back(unit->GetPosition());
                break;
            case NPC_FROST_BOMB:
                hazards.bombs.push_back(unit->GetPosition());
                break;
            case NPC_EMERGENCY_FIRE_BOT:
                hazards.fireBots.push_back(unit->GetPosition());
                break;
            default:
                break;
        }
    }

    // Two lanes per bot: the live line, for a spray already lined up, and the one its next check
    // aims, toward the nearest spread flame from where the walk there stops.
    for (Position const& fireBot : hazards.fireBots)
    {
        hazards.sprayLanes.push_back(fireBot);

        Position const* target = nullptr;
        float targetDist = 0.0f;
        for (Position const& flame : spread)
        {
            float const dist = fireBot.GetExactDist2d(flame.GetPositionX(), flame.GetPositionY());
            if (!target || dist < targetDist)
            {
                target = &flame;
                targetDist = dist;
            }
        }

        if (!target)
            continue;

        float const facing = fireBot.GetAngle(target->GetPositionX(), target->GetPositionY());
        float const walk = std::max(targetDist - ULDUAR_MIMIRON_FIREBOT_SPRAY_REACH, 0.0f);
        hazards.sprayLanes.emplace_back(fireBot.GetPositionX() + walk * std::cos(facing),
                                        fireBot.GetPositionY() + walk * std::sin(facing),
                                        fireBot.GetPositionZ(), facing);
    }

    return hazards;
}
}  // namespace

bool IsMimironSpotFireSafe(MimironFirefighterHazards const& hazards, Position const& dest)
{
    for (Position const& node : hazards.flames)
        if (dest.GetExactDist2d(node.GetPositionX(), node.GetPositionY()) < ULDUAR_MIMIRON_FLAMES_RADIUS)
            return false;

    return true;
}

bool IsMimironSpotBombSafe(MimironFirefighterHazards const& hazards, Position const& dest)
{
    for (Position const& bomb : hazards.bombs)
        if (dest.GetExactDist2d(bomb.GetPositionX(), bomb.GetPositionY()) < ULDUAR_MIMIRON_FROST_BOMB_STAND_RADIUS)
            return false;

    return true;
}

bool IsMimironSpotInFireBotSpray(MimironFirefighterHazards const& hazards, Position const& dest,
                                 float halfWidth)
{
    for (Position const& lane : hazards.sprayLanes)
    {
        float const dx = dest.GetPositionX() - lane.GetPositionX();
        float const dy = dest.GetPositionY() - lane.GetPositionY();

        // The line only reaches forward: HasInLine checks the front half-circle first.
        float const facing = lane.GetOrientation();
        float const ahead = dx * std::cos(facing) + dy * std::sin(facing);
        float const side = dy * std::cos(facing) - dx * std::sin(facing);
        if (ahead >= 0.0f && ahead < ULDUAR_MIMIRON_FIREBOT_SPRAY_LENGTH && std::fabs(side) < halfWidth)
            return true;
    }

    return false;
}

bool IsMimironSpotFireBotSafe(Player* bot, MimironFirefighterHazards const& hazards, Position const& dest,
                              float sirenRadius, float sprayHalfWidth)
{
    if (hazards.fireBots.empty())
        return true;

    bool const silenced = bot && (PlayerbotAI::IsCaster(bot) || PlayerbotAI::IsHeal(bot)) &&
                          bot->GetMap()->Is25ManRaid();

    if (silenced)
        for (Position const& fireBot : hazards.fireBots)
            if (dest.GetExactDist2d(fireBot.GetPositionX(), fireBot.GetPositionY()) < sirenRadius)
                return false;

    return !IsMimironSpotInFireBotSpray(hazards, dest, sprayHalfWidth);
}

bool IsMimironSpotShockSafe(PlayerbotAI* botAI, Position const& dest)
{
    Unit* leviathanMkII = botAI ? GetUldUnitByEntry(botAI, NPC_LEVIATHAN_MKII) : nullptr;
    if (!leviathanMkII || !leviathanMkII->FindCurrentSpellBySpellId(SPELL_SHOCK_BLAST))
        return true;

    return leviathanMkII->GetExactDist2d(dest.GetPositionX(), dest.GetPositionY()) >=
           ULDUAR_MIMIRON_SHOCK_BLAST_SAFE_DIST;
}

namespace
{
// The standing hazards MimironFirefighterHazards leaves out: mines, live Rocket Strike markers and a
// Shock Blast being cast. Gathered once, so screening a batch of candidate spots costs one scan.
struct MimironMarkers
{
    std::vector<Position> mines;
    std::vector<Position> rockets;
    bool shock = false;
    Position shockCentre;
};

MimironMarkers ReadMimironMarkers(PlayerbotAI* botAI)
{
    MimironMarkers markers;
    for (ObjectGuid const& guid : botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get())
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_PROXIMITY_MINE)
            markers.mines.push_back(unit->GetPosition());
        else if (unit->GetEntry() == NPC_ROCKET_STRIKE_N)
            markers.rockets.push_back(unit->GetPosition());
    }

    Unit* leviathanMkII = GetUldUnitByEntry(botAI, NPC_LEVIATHAN_MKII);
    if (leviathanMkII && leviathanMkII->FindCurrentSpellBySpellId(SPELL_SHOCK_BLAST))
    {
        markers.shock = true;
        markers.shockCentre = leviathanMkII->GetPosition();
    }

    return markers;
}

// Hazard reads cached for one trigger pass: four triggers ask, and each read walks the fire field or
// the group. Actions and multipliers may run after the world changed, so they always read live.
struct MimironPassReads
{
    PlayerbotAI* botAI = nullptr;
    uint32 passId = 0;

    bool hazardsRead = false;
    MimironFirefighterHazards hazards;

    bool markersRead = false;
    MimironMarkers markers;

    bool burstRead = false;
    MimironRapidBurstWindow burst;

    bool barrageRead = false;
    MimironBarrageWindow barrage;
};

// thread_local is fine: a whole pass runs on one map thread.
thread_local MimironPassReads mimironPassReads;

// Null outside a trigger pass, meaning read live.
MimironPassReads* MimironPassReadsFor(PlayerbotAI* botAI)
{
    uint32 const passId = EncounterTriggerPassId(botAI);
    if (!passId)
        return nullptr;

    if (mimironPassReads.botAI != botAI || mimironPassReads.passId != passId)
    {
        mimironPassReads = MimironPassReads();
        mimironPassReads.botAI = botAI;
        mimironPassReads.passId = passId;
    }

    return &mimironPassReads;
}

MimironMarkers GetMimironMarkers(PlayerbotAI* botAI)
{
    MimironPassReads* pass = MimironPassReadsFor(botAI);
    if (!pass)
        return ReadMimironMarkers(botAI);

    if (!pass->markersRead)
    {
        pass->markers = ReadMimironMarkers(botAI);
        pass->markersRead = true;
    }

    return pass->markers;
}

bool IsMimironSpotStandable(Player* bot, Position const& dest, MimironMarkers const& markers,
                            MimironFirefighterHazards const& hazards)
{
    for (Position const& mine : markers.mines)
        if (dest.GetExactDist2d(mine.GetPositionX(), mine.GetPositionY()) < ULDUAR_MIMIRON_MINE_CLEARANCE)
            return false;

    for (Position const& rocket : markers.rockets)
        if (dest.GetExactDist2d(rocket.GetPositionX(), rocket.GetPositionY()) < ULDUAR_MIMIRON_ROCKET_CLEARANCE)
            return false;

    if (markers.shock && dest.GetExactDist2d(markers.shockCentre.GetPositionX(),
                                             markers.shockCentre.GetPositionY()) <
                             ULDUAR_MIMIRON_SHOCK_BLAST_SAFE_DIST)
        return false;

    // Fire and bomb too, or the formation pulls a bot straight back into what its dodge just left.
    return IsMimironSpotFireSafe(hazards, dest) && IsMimironSpotBombSafe(hazards, dest) &&
           IsMimironSpotFireBotSafe(bot, hazards, dest, ULDUAR_MIMIRON_FIREBOT_SIREN_STAND,
                                    ULDUAR_MIMIRON_FIREBOT_SPRAY_STAND_HALF_WIDTH);
}

// The straight walk from `from` to `dest` against a set of hazards of one radius. Hazards `from`
// already stands in are left out, since walking out of those is the point.
bool IsMimironLegClearOf(Position const& from, std::vector<Position> const& nodes, float radius,
                         Position const& dest)
{
    if (nodes.empty())
        return true;

    float const fromX = from.GetPositionX();
    float const fromY = from.GetPositionY();
    float const dx = dest.GetPositionX() - fromX;
    float const dy = dest.GetPositionY() - fromY;
    float const lengthSq = dx * dx + dy * dy;
    if (lengthSq <= 0.0f)
        return true;

    for (Position const& node : nodes)
    {
        float const nx = node.GetPositionX() - fromX;
        float const ny = node.GetPositionY() - fromY;
        if (std::sqrt(nx * nx + ny * ny) < radius)
            continue;

        // Closest point of the walk to this node.
        float const t = std::clamp((nx * dx + ny * dy) / lengthSq, 0.0f, 1.0f);
        float const ox = nx - t * dx;
        float const oy = ny - t * dy;
        if (std::sqrt(ox * ox + oy * oy) < radius)
            return false;
    }

    return true;
}

bool IsMimironLegFireSafe(Position const& from, MimironFirefighterHazards const& hazards,
                          Position const& dest)
{
    return IsMimironLegClearOf(from, hazards.flames, ULDUAR_MIMIRON_FLAMES_RADIUS, dest);
}

// Pathing knows nothing about mines, and the slot screen only covers where the walk ends.
bool IsMimironLegMineSafe(Position const& from, MimironMarkers const& markers, Position const& dest)
{
    return IsMimironLegClearOf(from, markers.mines, ULDUAR_MIMIRON_MINE_LEG_CLEARANCE, dest);
}
}  // namespace

MimironBarrageWindow GetMimironBarrageWindow(Player* bot, Unit* vx001)
{
    PlayerbotAI* botAI = bot ? GET_PLAYERBOT_AI(bot) : nullptr;
    MimironPassReads* pass = botAI ? MimironPassReadsFor(botAI) : nullptr;
    if (!pass)
        return ReadMimironBarrageWindow(bot, vx001);

    if (!pass->barrageRead)
    {
        pass->barrage = ReadMimironBarrageWindow(bot, vx001);
        pass->barrageRead = true;
    }

    return pass->barrage;
}

MimironFirefighterHazards GetMimironFirefighterHazards(PlayerbotAI* botAI)
{
    MimironPassReads* pass = MimironPassReadsFor(botAI);
    if (!pass)
        return ReadMimironFirefighterHazards(botAI);

    if (!pass->hazardsRead)
    {
        pass->hazards = ReadMimironFirefighterHazards(botAI);
        pass->hazardsRead = true;
    }

    return pass->hazards;
}

bool IsMimironWalkFireSafe(Player* bot, MimironFirefighterHazards const& hazards, Position const& dest)
{
    return !bot || IsMimironLegFireSafe(bot->GetPosition(), hazards, dest);
}

bool IsMimironSpotSafe(Player* bot, Position const& dest)
{
    PlayerbotAI* botAI = bot ? GET_PLAYERBOT_AI(bot) : nullptr;
    if (!botAI)
        return true;

    return IsMimironSpotStandable(bot, dest, GetMimironMarkers(botAI), GetMimironFirefighterHazards(botAI));
}

bool IsMimironSpotRocketSafe(PlayerbotAI* botAI, Position const& dest)
{
    if (!botAI)
        return true;

    MimironMarkers const markers = GetMimironMarkers(botAI);
    for (Position const& rocket : markers.rockets)
        if (dest.GetExactDist2d(rocket.GetPositionX(), rocket.GetPositionY()) < ULDUAR_MIMIRON_ROCKET_CLEARANCE)
            return false;

    return true;
}

float GetMimironApproachRange(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot)
        return 0.0f;

    return PlayerbotAI::IsMelee(bot) ? sPlayerbotAIConfig.MeleeDistance : botAI->GetRange("spell");
}

bool GetMimironTargetApproach(PlayerbotAI* botAI, Player* bot, Unit* target, float range,
                              Position& out)
{
    if (!botAI || !bot || !target)
        return false;

    if (bot->IsWithinCombatRange(target, range))
        return false;

    MimironFirefighterHazards const hazards = GetMimironFirefighterHazards(botAI);
    if (hazards.flames.empty())
        return false;

    MimironMarkers const markers = GetMimironMarkers(botAI);
    Unit* vx001 = GetUldUnitByEntry(botAI, NPC_VX001);
    MimironRapidBurstWindow const burst =
        vx001 ? GetMimironRapidBurstWindow(botAI, bot, vx001) : MimironRapidBurstWindow();
    auto const standable = [&](Position const& spot)
    {
        return IsMimironSpotStandable(bot, spot, markers, hazards) &&
               IsMimironSpotRapidBurstSafe(vx001, burst, spot);
    };

    float const stop = range + bot->GetCombatReach() + target->GetCombatReach();
    float const radius = std::max(1.0f, stop - ULDUAR_MIMIRON_APPROACH_INSET);

    // Bearing toward the bot first, which is where the generic reach nodes would stop, then out
    // to either side. Step 0 clear means there is nothing here to fix.
    float const towardBot = target->GetAngle(bot->GetPositionX(), bot->GetPositionY());
    for (uint32 step = 0; step < ULDUAR_MIMIRON_STAND_IN_BEARINGS; ++step)
    {
        float const turn = static_cast<float>((step + 1) / 2) * ULDUAR_MIMIRON_STAND_IN_BEARING_STEP;
        float const bearing = towardBot + (step % 2 ? turn : -turn);
        Position const candidate(target->GetPositionX() + radius * std::cos(bearing),
                                 target->GetPositionY() + radius * std::sin(bearing),
                                 bot->GetPositionZ());
        if (!standable(candidate))
            continue;

        if (step == 0)
            return false;

        out = candidate;
        return true;
    }

    return false;
}

std::vector<MimironApproach> GetMimironSlotApproaches(PlayerbotAI* botAI, Player* bot, Position const& slot,
                                                      MimironFirefighterHazards const& hazards)
{
    std::vector<MimironApproach> approaches;
    if (!botAI || !bot)
        return approaches;

    MimironMarkers const markers = GetMimironMarkers(botAI);
    Unit* vx001 = GetUldUnitByEntry(botAI, NPC_VX001);
    MimironRapidBurstWindow const burst =
        vx001 ? GetMimironRapidBurstWindow(botAI, bot, vx001) : MimironRapidBurstWindow();
    MimironBarrageWindow const barrage =
        vx001 ? GetMimironBarrageWindow(bot, vx001) : MimironBarrageWindow();

    // Cone judged at arrival: these are the fight's longest legs, and the cone turns ~10.6 degrees a second.
    float const speed = bot->GetSpeed(MOVE_RUN);
    auto const coneClear = [&](Position const& spot)
    {
        float const travel = speed > 0.0f
                                 ? bot->GetExactDist2d(spot.GetPositionX(), spot.GetPositionY()) / speed
                                 : 0.0f;
        return IsMimironSpotBarrageSafe(vx001, barrage, spot, travel);
    };

    auto const standable = [&](Position const& spot)
    {
        return IsMimironSpotStandable(bot, spot, markers, hazards) &&
               IsMimironSpotRapidBurstSafe(vx001, burst, spot) && coneClear(spot);
    };

    Position goal = slot;
    if (!IsMimironSpotStandable(bot, slot, markers, hazards) || !coneClear(slot))
    {
        // Already on clear ground beside it: stay. A substitute moves every time the fire grows, and
        // chasing it is a walk every tick.
        if (bot->GetExactDist2d(slot.GetPositionX(), slot.GetPositionY()) <=
                ULDUAR_MIMIRON_SLOT_SUBSTITUTE_RADIUS &&
            IsMimironSpotStandable(bot, bot->GetPosition(), markers, hazards))
            return approaches;

        // Phase 1 keeps its slots. Its camp moves off its own fire by switching anchors, and a stand-in
        // squeezed between 6 yd rows is how two bots end up under one Napalm Shell.
        if (!vx001 && GetUldUnitByEntry(botAI, NPC_LEVIATHAN_MKII) &&
            !GetUldUnitByEntry(botAI, NPC_AERIAL_COMMAND_UNIT))
            return approaches;

        // Hard-mode phase 2 melee: a stand-in must reach VX-001 without going under it or into the wedge's
        // line, and skips the Napalm spacing test, which the tightly packed ring always fails.
        bool const meleeRing = vx001 && !PlayerbotAI::IsRanged(bot) && IsMimironPhase2(botAI);
        MimironBurstSector const sector =
            meleeRing ? GetMimironOtherGroupSector(botAI, bot, vx001) : MimironBurstSector();
        float const meleeReach =
            vx001 ? vx001->GetCombatReach() + ULDUAR_MIMIRON_PHASE2_MELEE_REACH_MARGIN : 0.0f;

        std::vector<Position> others;
        if (Group* group = meleeRing ? nullptr : bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member != bot && member->IsAlive() && member->IsInMap(bot))
                    others.push_back(member->GetPosition());
            }
        }

        // Nearest ring first, and on each ring the bearing toward the bot first, so the stand-in is on
        // the near side of the slot.
        float const towardBot = slot.GetAngle(bot->GetPositionX(), bot->GetPositionY());
        bool found = false;
        // Phase 1 has already returned above, so this is the wide ring.
        for (float radius = ULDUAR_MIMIRON_STAND_IN_RING_STEP;
             radius <= ULDUAR_MIMIRON_SLOT_SUBSTITUTE_RADIUS_WIDE + 0.01f && !found;
             radius += ULDUAR_MIMIRON_STAND_IN_RING_STEP)
        {
            for (uint32 step = 0; step < ULDUAR_MIMIRON_STAND_IN_BEARINGS && !found; ++step)
            {
                float const turn = static_cast<float>((step + 1) / 2) * ULDUAR_MIMIRON_STAND_IN_BEARING_STEP;
                float const bearing = towardBot + (step % 2 ? turn : -turn);
                Position const candidate(slot.GetPositionX() + radius * std::cos(bearing),
                                         slot.GetPositionY() + radius * std::sin(bearing),
                                         slot.GetPositionZ());
                if (!standable(candidate))
                    continue;

                if (meleeRing)
                {
                    float const reach = candidate.GetExactDist2d(vx001);
                    if (reach < ULDUAR_MIMIRON_BARRAGE_MELEE_RING_MIN || reach > meleeReach ||
                        IsMimironSpotInSector(vx001, sector, candidate))
                        continue;
                }

                bool crowded = false;
                for (Position const& other : others)
                {
                    if (candidate.GetExactDist2d(other.GetPositionX(), other.GetPositionY()) <
                        ULDUAR_MIMIRON_DISPERSE_DISTANCE)
                    {
                        crowded = true;
                        break;
                    }
                }

                if (!crowded)
                {
                    goal = candidate;
                    found = true;
                }
            }
        }

        // An empty answer stands the formation down.
        if (!found)
            return approaches;
    }

    Position const from = bot->GetPosition();
    bool const directFireSafe = IsMimironLegFireSafe(from, hazards, goal);
    if (directFireSafe && IsMimironLegMineSafe(from, markers, goal))
    {
        approaches.push_back({goal});
        return approaches;
    }

    auto const legSafe = [&](Position const& start, Position const& end)
    { return IsMimironLegFireSafe(start, hazards, end) && IsMimironLegMineSafe(start, markers, end); };

    // Round the fire and the mines instead: one waypoint over the walk's midpoint, whose legs both
    // miss them.
    float const distance = from.GetExactDist2d(goal.GetPositionX(), goal.GetPositionY());
    float const bearing = from.GetAngle(goal.GetPositionX(), goal.GetPositionY());
    for (float degrees : ULDUAR_MIMIRON_DETOUR_TURNS_DEG)
    {
        float const turn = degrees * static_cast<float>(M_PI) / 180.0f;
        float const leg = 0.5f * distance / std::cos(turn);
        for (float sign : {1.0f, -1.0f})
        {
            Position const waypoint(from.GetPositionX() + leg * std::cos(bearing + sign * turn),
                                    from.GetPositionY() + leg * std::sin(bearing + sign * turn),
                                    from.GetPositionZ());
            if (standable(waypoint) && legSafe(from, waypoint) && legSafe(waypoint, goal))
                approaches.push_back({waypoint});
        }
    }

    // Mines rank last: a blast is a healable 9000, a bot stuck off its slot costs the formation. So
    // the straight walk is still the last resort whenever the fire leaves it open.
    if (directFireSafe)
        approaches.push_back({goal});

    return approaches;
}

bool IsMimironSpotBarrageSafe(Unit* vx001, MimironBarrageWindow const& window, Position const& dest,
                              float travelSeconds)
{
    if (!vx001 || !window.valid)
        return true;

    if (vx001->GetExactDist2d(dest.GetPositionX(), dest.GetPositionY()) < ULDUAR_MIMIRON_BARRAGE_BOUNDARY)
        return false;

    float const clearance = ULDUAR_MIMIRON_BARRAGE_HALF_ANGLE + ULDUAR_MIMIRON_BARRAGE_MARGIN;
    float const twoPi = 2.0f * static_cast<float>(M_PI);

    // Grow the band's trailing edge by the sweep during the walk. Not while spinning up: `sweep` already
    // spans the whole fire. The barrage dodge zeroes its rate the same way, and the two must agree.
    float const rate = window.untilLive > 0.0f ? 0.0f : window.rate;
    float const grown =
        std::min(window.sweep + rate * std::max(travelSeconds, 0.0f), twoPi - 2.0f * clearance);

    float const cw = Position::NormalizeOrientation(
        window.lead - vx001->GetAngle(dest.GetPositionX(), dest.GetPositionY()));

    return cw > grown + clearance && cw < twoPi - clearance;
}

std::string GetMimironBombBotSnare(Player* bot)
{
    if (!bot)
        return "";

    switch (bot->getClass())
    {
        case CLASS_HUNTER:  return "concussive shot";
        case CLASS_SHAMAN:  return "frost shock";
        case CLASS_WARLOCK: return "curse of exhaustion";
        default:            return "";
    }
}

Unit* GetMimironBombBotChasing(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot)
        return nullptr;

    for (ObjectGuid const& guid : botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get())
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || unit->GetEntry() != NPC_BOMB_BOT)
            continue;

        if (ServerFacade::instance().GetChaseTarget(unit) == bot)
            return unit;
    }

    return nullptr;
}

float GetMimironBombBotApproach(Player* bot, Unit* bombBot)
{
    if (!bot || !bombBot)
        return 0.0f;

    if (Unit* victim = ServerFacade::instance().GetChaseTarget(bombBot))
        return bombBot->GetExactDist2d(victim);

    return bombBot->GetExactDist2d(bot);
}

bool IsMimironAcuGrounded(PlayerbotAI* botAI)
{
    if (!botAI)
        return false;

    Unit* aerialCommandUnit = GetUldUnitByEntry(botAI, NPC_AERIAL_COMMAND_UNIT);
    return aerialCommandUnit && aerialCommandUnit->HasAura(SPELL_MIMIRON_MAGNETIC_CORE_AURA);
}

bool IsMimironAcuAirborne(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot)
        return false;

    // Phase 4 has it attackable too, sitting on the chassis.
    return GetUldUnitByEntry(botAI, NPC_AERIAL_COMMAND_UNIT) && !IsMimironPhase4(bot) &&
           !IsMimironAcuGrounded(botAI);
}

Unit* GetMimironRingFocus(PlayerbotAI* botAI)
{
    if (!botAI)
        return nullptr;

    if (Unit* vx001 = GetUldUnitByEntry(botAI, NPC_VX001))
        return vx001;

    if (Unit* leviathanMkII = GetUldUnitByEntry(botAI, NPC_LEVIATHAN_MKII))
        return leviathanMkII;

    return GetUldUnitByEntry(botAI, NPC_AERIAL_COMMAND_UNIT);
}

bool IsNearMimironRoom(Player* bot)
{
    return bot && bot->GetMapId() == ULDUAR_MAP_ID &&
           bot->GetExactDist2d(ULDUAR_MIMIRON_ROOM_CENTER) <= ULDUAR_MIMIRON_STAGING_SEARCH_RANGE;
}

Unit* GetMimironStagingFocus(Player* bot)
{
    if (!bot)
        return nullptr;

    Creature* vx001 = bot->FindNearestCreature(NPC_VX001, ULDUAR_MIMIRON_STAGING_SEARCH_RANGE);
    Creature* aerialCommandUnit =
        bot->FindNearestCreature(NPC_AERIAL_COMMAND_UNIT, ULDUAR_MIMIRON_STAGING_SEARCH_RANGE);

    // VX-001 only rides anything in phase 4, so once it boards, the phase 4 spots are worth walking to.
    if (vx001 && vx001->GetVehicleBase())
        return vx001;

    if (aerialCommandUnit)
        return aerialCommandUnit;

    // Nothing before the pull or after a wipe: the MK II is NOT_SELECTABLE until it is pulled, and
    // evade despawns VX-001 and the Aerial Command Unit outright.
    return vx001;
}

namespace
{
// Throttles the wipe check in the tick, per instance.
struct MimironTickState
{
    uint32 scanMs = 0;
};

RaidInstanceState<MimironTickState> mimironTickStates;

// Raid-wide answers, the same for every bot in the instance: a per-bot tank latch would start the
// formation walking while the tank still builds threat, and a per-bot stack anchor is no stack.
struct MimironFightState
{
    // One-way per pull. Set when the main tank's threat lead on the MK II is real, or when the hold
    // times out; until then he has no anchor and stands on the boss.
    bool tankDragReady = false;
    uint32 tankHoldStartedMs = 0;

    uint8 stackAnchor = 0;
    uint32 stackPickedMs = 0;
    uint32 stackScanMs = 0;

    // Extra turn on the phase 1 camp toward the room, radians, so every slot keeps sight of the MK II.
    float campTurn = 0.0f;
    uint32 campTurnScanMs = 0;
    uint32 campTurnRaisedMs = 0;

    ObjectGuid plasmaClaimedBy;
    uint32 plasmaWindowMs = 0;

    // Assault Bot corpses that already gave their Magnetic Core.
    std::unordered_set<ObjectGuid> coreCorpses;

    // When the last Magnetic Core went in, so the next one waits its arming out.
    uint32 coreSpentMs = 0;

    std::vector<ObjectGuid> keptFireBots;
    uint32 fireBotScanMs = 0;

    // Where the Firefighter wedge points once a barrage has moved it off the east centreline. A
    // bearing of zero is a real one, so wedgeAimedMs is what says whether it has been aimed.
    float wedgeBearing = 0.0f;
    uint32 wedgeAimedMs = 0;

    // The wedge slide in force, and the focus it was worked out against.
    uint32 shiftFocusEntry = 0;
    float shiftX = 0.0f;
    float shiftY = 0.0f;

    // Where the phase 4 tank holds the MK II under Firefighter. Zero picked means the fixed spot.
    Position tankSpot;
    uint32 tankSpotPickedMs = 0;
    uint32 tankSpotScanMs = 0;
};

RaidInstanceState<MimironFightState> mimironFightStates;

// Each bot's last fire dodge. Kept out of MimironFightState, which resets on every handover scan,
// right when the raid follows its master through the fire and most needs the hold.
RaidInstanceState<std::unordered_map<ObjectGuid, uint32>> mimironFireDodges;

std::unordered_map<ObjectGuid, uint32>& MimironFireDodgesFor(Player* bot)
{
    return mimironFireDodges.For(bot->GetInstanceId());
}

MimironFightState& MimironFightStateFor(Player* bot) { return mimironFightStates.For(bot->GetInstanceId()); }

void TickMimiron(PlayerbotAI* botAI, Player* bot)
{
    if (!bot || bot->GetMapId() != ULDUAR_MAP_ID)
        return;

    MimironTickState& state = mimironTickStates.For(bot->GetInstanceId());
    if (state.scanMs && GetMSTimeDiffToNow(state.scanMs) < ULDUAR_MIMIRON_SCAN_INTERVAL_MS)
        return;

    state.scanMs = getMSTime();

    // Off the constructs, never off the calling bot's combat state: one bot dropping combat is not a wipe.
    if (!GetUldUnitByEntry(botAI, NPC_LEVIATHAN_MKII) && !GetUldUnitByEntry(botAI, NPC_VX001) &&
        !GetUldUnitByEntry(botAI, NPC_AERIAL_COMMAND_UNIT))
        ResetMimironFightState(bot);
}
}  // namespace

void MimironTick(PlayerbotAI* botAI) { TickMimiron(botAI, botAI->GetBot()); }

bool IsMimironEngaged(PlayerbotAI* botAI)
{
    // Any construct in combat. Mid-handover the outgoing one is passive and unselectable before the next
    // calls SetInCombatWithZone, so there's nothing to target then anyway.
    Unit* leviathanMkII = GetUldUnitByEntry(botAI, NPC_LEVIATHAN_MKII);
    Unit* vx001 = GetUldUnitByEntry(botAI, NPC_VX001);
    Unit* aerialCommandUnit = GetUldUnitByEntry(botAI, NPC_AERIAL_COMMAND_UNIT);

    for (Unit* construct : {leviathanMkII, vx001, aerialCommandUnit})
        if (construct && construct->IsInCombat())
            return true;

    return false;
}

bool IsMimironPhase4(Player* bot)
{
    PlayerbotAI* botAI = bot ? GET_PLAYERBOT_AI(bot) : nullptr;
    if (!botAI)
        return false;

    // Cached lookups only. This is asked several times per bot per tick, from the target list, the
    // tank node and the pet node, so a grid sweep here would cost the whole raid every phase.

    // VX-001 and the Aerial Command Unit both ride something from phase 4 on, and neither does before
    // it, so either one answers on its own while it is still attackable.
    if (Unit* vx001 = GetUldUnitByEntry(botAI, NPC_VX001))
        return vx001->GetVehicleBase() != nullptr;

    if (Unit* aerialCommandUnit = GetUldUnitByEntry(botAI, NPC_AERIAL_COMMAND_UNIT))
        return aerialCommandUnit->GetVehicleBase() != nullptr;

    // Both under 15000 and NON_ATTACKABLE, so only the chassis is left. Its seat 3 holds the cannon in
    // phase 1 and VX-001 from phase 4 on, readable whatever flags the passengers carry.
    if (Unit* leviathanMkII = GetUldUnitByEntry(botAI, NPC_LEVIATHAN_MKII))
        if (Vehicle* kit = leviathanMkII->GetVehicleKit())
            if (Unit* seated = kit->GetPassenger(3))
                return seated->GetEntry() == NPC_VX001;

    return false;
}

bool MimironPhase1Active(PlayerbotAI* botAI)
{
    return !IsMimironPhase4(botAI->GetBot()) && GetUldUnitByEntry(botAI, NPC_LEVIATHAN_MKII) &&
           !GetUldUnitByEntry(botAI, NPC_VX001) && !GetUldUnitByEntry(botAI, NPC_AERIAL_COMMAND_UNIT);
}

bool IsMimironPhase2(PlayerbotAI* botAI)
{
    if (!botAI || GetUldUnitByEntry(botAI, NPC_LEVIATHAN_MKII))
        return false;

    Unit* vx001 = GetUldUnitByEntry(botAI, NPC_VX001);
    return vx001 && !vx001->GetVehicleBase();
}

bool IsMimironPhase3(PlayerbotAI* botAI)
{
    return botAI && GetUldUnitByEntry(botAI, NPC_AERIAL_COMMAND_UNIT) &&
           !GetUldUnitByEntry(botAI, NPC_LEVIATHAN_MKII) && !GetUldUnitByEntry(botAI, NPC_VX001);
}

Player* GetMimironFrostResistancePaladin(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot || !IsMimironHardModeActive(botAI) || !IsMimironPhase3(botAI))
        return nullptr;

    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    // The fire carrier is whoever holds "rfire", dead or not, so frost doesn't hop when that pick dies.
    // Nobody holding it means the node hasn't run yet, and it'll pick the first alive paladin.
    Player* fire = nullptr;
    Player* firstAlive = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->getClass() != CLASS_PALADIN)
            continue;

        if (!firstAlive && member->IsAlive())
            firstAlive = member;

        PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
        if (!fire && memberAI && memberAI->HasStrategy("rfire", BotState::BOT_STATE_COMBAT))
            fire = member;
    }

    if (!fire)
        fire = firstAlive;

    static uint32 const ranks[] = {SPELL_FROST_RESISTANCE_AURA_RANK_5, SPELL_FROST_RESISTANCE_AURA_RANK_4,
                                   SPELL_FROST_RESISTANCE_AURA_RANK_3, SPELL_FROST_RESISTANCE_AURA_RANK_2,
                                   SPELL_FROST_RESISTANCE_AURA_RANK_1};

    Player* tank = nullptr;
    Player* dps = nullptr;
    Player* healer = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == fire || !member->IsAlive() || member->getClass() != CLASS_PALADIN ||
            member->GetMapId() != bot->GetMapId() || !GET_PLAYERBOT_AI(member))
            continue;

        bool knows = false;
        for (uint32 rank : ranks)
            if (member->HasActiveSpell(rank))
            {
                knows = true;
                break;
            }

        if (!knows)
            continue;

        if (PlayerbotAI::IsTank(member))
        {
            if (!tank)
                tank = member;
        }
        else if (PlayerbotAI::IsHeal(member))
        {
            if (!healer)
                healer = member;
        }
        else if (!dps)
        {
            dps = member;
        }
    }

    if (tank)
        return tank;

    return dps ? dps : healer;
}

Unit* GetMimironPhase4Focus(PlayerbotAI* botAI, Player* bot, bool melee)
{
    if (!botAI || !bot || !IsMimironPhase4(bot))
        return nullptr;

    std::vector<Unit*> parts;
    for (uint32 entry : {NPC_LEVIATHAN_MKII, NPC_VX001, NPC_AERIAL_COMMAND_UNIT})
        if (Unit* part = GetUldUnitByEntry(botAI, entry))
            parts.push_back(part);

    if (parts.empty())
        return nullptr;

    // Banded, ties to the fixed entry order: the band stops the ground pair trading the lead, and a pure
    // function of world state keeps the tank node, this node and the pets on the same part.
    auto const highest = [](std::vector<Unit*> const& candidates) -> Unit*
    {
        auto const band = [](Unit* unit)
        { return static_cast<int32>(unit->GetHealthPct() / ULDUAR_MIMIRON_PHASE4_FOCUS_BAND_PCT); };

        Unit* best = nullptr;
        for (Unit* candidate : candidates)
            if (!best || band(candidate) > band(best))
                best = candidate;

        return best;
    };

    // Fewer than three attackable: one is already in Self Repair and its 15 s clock runs, so every
    // restriction comes off, melee on the Aerial Command Unit included.
    if (parts.size() < 3)
        return highest(parts);

    std::vector<Unit*> allowed;
    for (Unit* part : parts)
    {
        // Ranged DPS own the Aerial Command Unit, melee only get it below. IsRangedDps, not IsRanged, so a
        // healer is never steered onto it or into the hold.
        if (part->GetEntry() == NPC_AERIAL_COMMAND_UNIT && (melee || !PlayerbotAI::IsRangedDps(bot)))
            continue;

        allowed.push_back(part);
    }

    if (allowed.empty())
        return nullptr;

    // All three levelled out, so stop holding and push them under together.
    bool levelled = true;
    for (Unit* part : parts)
        if (part->GetHealthPct() > ULDUAR_MIMIRON_PHASE4_HOLD_PCT)
            levelled = false;

    if (levelled)
        return highest(allowed);

    // Floor paced to the part furthest from death, not a fixed percentage: the Aerial Command Unit always
    // arrives last, since most of the AoE meant for it lands on the other two.
    float leader = 0.0f;
    for (Unit* part : parts)
        leader = std::max(leader, part->GetHealthPct());

    float const floorPct =
        std::max(ULDUAR_MIMIRON_PHASE4_HOLD_PCT, leader - ULDUAR_MIMIRON_PHASE4_CONVERGE_PCT);

    std::vector<Unit*> aboveFloor;
    for (Unit* part : allowed)
        if (part->GetHealthPct() > floorPct)
            aboveFloor.push_back(part);

    if (!aboveFloor.empty())
        return highest(aboveFloor);

    // Both ground mechs at the floor: melee and pets hit the leading unit to close the gap, but only
    // within two converge steps, since cleave still splashes the pair. Past that, hold.
    if (melee)
    {
        Unit* aerialCommandUnit = nullptr;
        float pairLow = 100.0f;
        for (Unit* part : parts)
        {
            if (part->GetEntry() == NPC_AERIAL_COMMAND_UNIT)
                aerialCommandUnit = part;
            else
                pairLow = std::min(pairLow, part->GetHealthPct());
        }

        if (aerialCommandUnit && aerialCommandUnit->GetHealthPct() > floorPct &&
            pairLow >= leader - 2.0f * ULDUAR_MIMIRON_PHASE4_CONVERGE_PCT)
            return aerialCommandUnit;
    }

    return nullptr;
}

Unit* GetMimironPhase4TankFocus(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot || !IsMimironPhase4(bot))
        return nullptr;

    if (Unit* leviathanMkII = GetUldUnitByEntry(botAI, NPC_LEVIATHAN_MKII))
        return leviathanMkII;

    return GetMimironPhase4Focus(botAI, bot, true);
}

Spell* GetMimironPlasmaBlastCast(Unit* cannon)
{
    if (!cannon)
        return nullptr;

    if (Spell* spell = cannon->FindCurrentSpellBySpellId(SPELL_MIMIRON_PLASMA_BLAST))
        return spell;

    return cannon->FindCurrentSpellBySpellId(SPELL_MIMIRON_PLASMA_BLAST_25);
}

bool IsMimironTankAnchorSlot(PlayerbotAI* botAI, Player* bot)
{
    // Phases 1 and 4 both park the MK II, and both are the phases it lays mines in.
    return botAI && bot && PlayerbotAI::IsMainTank(bot) &&
           GetUldUnitByEntry(botAI, NPC_LEVIATHAN_MKII) != nullptr;
}

float GetMimironPhase1DisperseDistance(PlayerbotAI* /*botAI*/)
{
    // Both modes. The Firefighter camp deals its own 6 yd slots, and a threshold equal to that would
    // have the unstacker judge every bot on its slot too close and shove it off.
    return ULDUAR_MIMIRON_DISPERSE_DISTANCE;
}

void ResetMimironFightState(Player* bot)
{
    if (!bot)
        return;

    MimironFightState& state = MimironFightStateFor(bot);
    state = MimironFightState();
}

void NoteMimironFireDodge(Player* bot)
{
    if (!bot)
        return;

    MimironFireDodgesFor(bot)[bot->GetGUID()] = getMSTime();
}

bool IsMimironFireHoldActive(Player* bot)
{
    if (!bot)
        return false;

    std::unordered_map<ObjectGuid, uint32> const& dodges = MimironFireDodgesFor(bot);
    auto const it = dodges.find(bot->GetGUID());
    return it != dodges.end() && GetMSTimeDiffToNow(it->second) < ULDUAR_MIMIRON_FLAMES_HOLD_MS;
}

bool ClaimMimironPlasmaWindow(Player* bot)
{
    if (!bot)
        return false;

    MimironFightState& state = MimironFightStateFor(bot);
    if (state.plasmaWindowMs &&
        GetMSTimeDiffToNow(state.plasmaWindowMs) < ULDUAR_MIMIRON_PLASMA_WINDOW_MS)
        return state.plasmaClaimedBy == bot->GetGUID();

    state.plasmaWindowMs = getMSTime();
    state.plasmaClaimedBy = bot->GetGUID();
    return true;
}

bool IsMimironTankDragReady(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot)
        return false;

    MimironFightState& state = MimironFightStateFor(bot);
    if (state.tankDragReady)
        return true;

    Unit* leviathanMkII = GetUldUnitByEntry(botAI, NPC_LEVIATHAN_MKII);
    if (!leviathanMkII)
        return false;

    if (!state.tankHoldStartedMs)
        state.tankHoldStartedMs = getMSTime();
    else if (GetMSTimeDiffToNow(state.tankHoldStartedMs) >= ULDUAR_MIMIRON_TANK_HOLD_MAX_MS)
    {
        // Nobody is going to win this threat table. Walking a boss somebody else is holding is still
        // better than standing at the pull spot burning the middle of the room down.
        state.tankDragReady = true;
        return true;
    }

    if (leviathanMkII->GetVictim() != bot)
        return false;

    ThreatManager& mgr = leviathanMkII->GetThreatMgr();
    float const tankThreat = mgr.GetThreat(bot);
    if (tankThreat <= 0.0f)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Against the raid rather than against the runner-up tank: an off-tank above the lead is fine,
    // it is a dps crossing the line that stops the boss dead halfway through the drag.
    float highest = 0.0f;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() || PlayerbotAI::IsTank(member))
            continue;

        highest = std::max(highest, mgr.GetThreat(member));
    }

    if (tankThreat < highest * ULDUAR_MIMIRON_TANK_THREAT_LEAD)
        return false;

    state.tankDragReady = true;
    return true;
}

Position const& GetMimironPhase1StackAnchor(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot)
        return ULDUAR_MIMIRON_PHASE1_STACK_SPOTS[0];

    MimironFightState& state = MimironFightStateFor(bot);

    // Gathering the field costs a scan of every nearby npc, and twenty-five bots ask for this every
    // tick, so the answer is folded on the same interval the observability pass uses.
    if (state.stackScanMs &&
        GetMSTimeDiffToNow(state.stackScanMs) < ULDUAR_MIMIRON_SCAN_INTERVAL_MS)
        return ULDUAR_MIMIRON_PHASE1_STACK_SPOTS[state.stackAnchor];

    state.stackScanMs = getMSTime();

    if (state.stackPickedMs && GetMSTimeDiffToNow(state.stackPickedMs) < ULDUAR_MIMIRON_STACK_HOLD_MS)
        return ULDUAR_MIMIRON_PHASE1_STACK_SPOTS[state.stackAnchor];

    MimironFirefighterHazards const hazards = GetMimironFirefighterHazards(botAI);

    uint32 counts[ULDUAR_MIMIRON_PHASE1_STACK_COUNT] = {};
    for (uint8 i = 0; i < ULDUAR_MIMIRON_PHASE1_STACK_COUNT; ++i)
    {
        Position const& spot = ULDUAR_MIMIRON_PHASE1_STACK_SPOTS[i];
        for (Position const& flame : hazards.flames)
        {
            if (spot.GetExactDist2d(flame.GetPositionX(), flame.GetPositionY()) <=
                ULDUAR_MIMIRON_STACK_FIRE_RADIUS)
                ++counts[i];
        }
    }

    uint8 const live = state.stackAnchor;
    if (counts[live] <= ULDUAR_MIMIRON_STACK_FIRE_LIMIT)
        return ULDUAR_MIMIRON_PHASE1_STACK_SPOTS[live];

    uint8 best = live;
    for (uint8 i = 0; i < ULDUAR_MIMIRON_PHASE1_STACK_COUNT; ++i)
        if (counts[i] + ULDUAR_MIMIRON_STACK_FIRE_MARGIN <= counts[best])
            best = i;

    if (best == live)
        return ULDUAR_MIMIRON_PHASE1_STACK_SPOTS[live];

    state.stackAnchor = best;
    state.stackPickedMs = getMSTime();
    return ULDUAR_MIMIRON_PHASE1_STACK_SPOTS[best];
}

namespace
{
MimironRapidBurstWindow ReadMimironRapidBurstWindow(PlayerbotAI* botAI, Player* bot, Unit* vx001)
{
    MimironRapidBurstWindow window;
    if (!botAI || !bot || !vx001 || !vx001->IsAlive())
        return window;

    Group* group = bot->GetGroup();
    if (!group)
        return window;

    Player* carrier = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref && !carrier; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->HasAura(SPELL_MIMIRON_RAPID_BURST))
            carrier = member;
    }

    if (!carrier)
        return window;

    window.valid = true;
    window.centreline = vx001->GetAngle(carrier->GetPositionX(), carrier->GetPositionY());
    return window;
}
}  // namespace

MimironRapidBurstWindow GetMimironRapidBurstWindow(PlayerbotAI* botAI, Player* bot, Unit* vx001)
{
    MimironPassReads* pass = MimironPassReadsFor(botAI);
    if (!pass)
        return ReadMimironRapidBurstWindow(botAI, bot, vx001);

    if (!pass->burstRead)
    {
        pass->burst = ReadMimironRapidBurstWindow(botAI, bot, vx001);
        pass->burstRead = true;
    }

    return pass->burst;
}

bool IsMimironSpotRapidBurstSafe(Unit* vx001, MimironRapidBurstWindow const& window,
                                 Position const& dest)
{
    if (!vx001 || !window.valid)
        return true;

    float const bearing = vx001->GetAngle(dest.GetPositionX(), dest.GetPositionY());
    float off = Position::NormalizeOrientation(bearing - window.centreline);
    if (off > static_cast<float>(M_PI))
        off -= 2.0f * static_cast<float>(M_PI);

    return std::fabs(off) > ULDUAR_MIMIRON_RAPID_BURST_HALF_ANGLE;
}

Player* GetMimironCoreCarrier(PlayerbotAI* botAI)
{
    if (!botAI)
        return nullptr;

    Group* group = botAI->GetBot()->GetGroup();
    if (!group)
        return nullptr;

    Player* fallback = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member))
            continue;

        if (!fallback)
            fallback = member;

        if (!PlayerbotAI::IsRanged(member) && !PlayerbotAI::IsTank(member))
            return member;
    }

    return fallback;
}

namespace
{
enum : int32
{
    MIMIRON_CORE_NOT_IN_LOOT = -1,  // loot never filled for this corpse
    MIMIRON_CORE_ALREADY_TAKEN = -2
};

// The loot slot holding this corpse's Magnetic Core, or one of the two codes above.
int32 MimironCoreLootSlot(Creature* corpse)
{
    int32 result = MIMIRON_CORE_NOT_IN_LOOT;
    for (size_t i = 0; i < corpse->loot.items.size(); ++i)
    {
        LootItem const& item = corpse->loot.items[i];
        if (item.itemid != ITEM_MIMIRON_MAGNETIC_CORE)
            continue;

        if (!item.is_looted)
            return static_cast<int32>(i);

        result = MIMIRON_CORE_ALREADY_TAKEN;
    }

    return result;
}
}  // namespace

Creature* GetMimironCoreCorpse(Player* bot)
{
    if (!bot)
        return nullptr;

    MimironFightState& state = MimironFightStateFor(bot);

    std::list<Creature*> assaultBots;
    bot->GetCreatureListWithEntryInGrid(assaultBots, NPC_ASSAULT_BOT, ULDUAR_MIMIRON_CORE_SEARCH_RANGE);

    Creature* nearest = nullptr;
    float nearestDist = 0.0f;
    for (Creature* corpse : assaultBots)
    {
        if (!corpse || corpse->IsAlive() || state.coreCorpses.count(corpse->GetGUID()) ||
            MimironCoreLootSlot(corpse) == MIMIRON_CORE_ALREADY_TAKEN)
            continue;

        float const dist = bot->GetExactDist2d(corpse);
        if (!nearest || dist < nearestDist)
        {
            nearest = corpse;
            nearestDist = dist;
        }
    }

    return nearest;
}

bool IsMimironCoreFetchDue(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot || GetMimironCoreCarrier(botAI) != bot)
        return false;

    return bot->GetItemCount(ITEM_MIMIRON_MAGNETIC_CORE, false) < ULDUAR_MIMIRON_CORE_BANK &&
           GetMimironCoreCorpse(bot);
}

bool TakeMimironCore(Player* bot, Creature* corpse)
{
    if (!bot || !corpse)
        return false;

    ItemPosCountVec dest;
    if (bot->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, ITEM_MIMIRON_MAGNETIC_CORE, 1) != EQUIP_ERR_OK)
        return false;

    MimironFightStateFor(bot).coreCorpses.insert(corpse->GetGUID());

    // Same bookkeeping as Player::StoreLootItem and the creature branch of DoLootRelease, so the
    // corpse shows nothing left to a player who opens it afterwards.
    int32 const slot = MimironCoreLootSlot(corpse);
    if (slot >= 0)
    {
        Loot& loot = corpse->loot;
        loot.items[slot].is_looted = true;
        --loot.unlootedCount;
        loot.NotifyItemRemoved(static_cast<uint8>(slot));

        if (loot.isLooted())
        {
            corpse->AllLootRemovedFromCorpse();
            corpse->RemoveDynamicFlag(UNIT_DYNFLAG_LOOTABLE);
            loot.clear();
        }
    }

    bot->StoreNewItem(dest, ITEM_MIMIRON_MAGNETIC_CORE, true,
                      Item::GenerateItemRandomPropertyId(ITEM_MIMIRON_MAGNETIC_CORE));
    return true;
}

bool IsMimironCoreUseReady(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot)
        return false;

    Unit* aerialCommandUnit = GetUldUnitByEntry(botAI, NPC_AERIAL_COMMAND_UNIT);
    if (!aerialCommandUnit || IsMimironAcuGrounded(botAI) ||
        !aerialCommandUnit->HasUnitMovementFlag(MOVEMENTFLAG_HOVER))
        return false;

    // Hover stays set through a core's arming and comes back while an aura is still on, and the placed
    // core isn't findable for its first few hundred ms, so each spend is stamped too.
    MimironFightState const& state = MimironFightStateFor(bot);
    if (state.coreSpentMs && GetMSTimeDiffToNow(state.coreSpentMs) < ULDUAR_MIMIRON_CORE_ARM_MS)
        return false;

    return !bot->FindNearestCreature(NPC_MAGNETIC_CORE, ULDUAR_MIMIRON_CORE_PENDING_RANGE);
}

void NoteMimironCoreSpent(Player* bot)
{
    if (bot)
        MimironFightStateFor(bot).coreSpentMs = getMSTime();
}

std::vector<ObjectGuid> GetMimironKeptFireBots(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot || !IsMimironHardModeActive(botAI))
        return {};

    Unit* aerialCommandUnit = GetUldUnitByEntry(botAI, NPC_AERIAL_COMMAND_UNIT);
    if (!aerialCommandUnit || IsMimironPhase4(bot) ||
        aerialCommandUnit->GetHealthPct() <= ULDUAR_MIMIRON_FIREBOT_CLEANUP_PCT)
        return {};

    // A grid scan, since "possible targets" is per bot and the kept pair has to be the same for
    // everyone. Folded on the observability interval, like the stack anchor.
    MimironFightState& state = MimironFightStateFor(bot);
    if (!state.fireBotScanMs ||
        GetMSTimeDiffToNow(state.fireBotScanMs) >= ULDUAR_MIMIRON_SCAN_INTERVAL_MS)
    {
        state.fireBotScanMs = getMSTime();

        std::list<Creature*> fireBots;
        bot->GetCreatureListWithEntryInGrid(fireBots, NPC_EMERGENCY_FIRE_BOT,
                                            ULDUAR_MIMIRON_STAGING_SEARCH_RANGE);

        // Every one: with no flame within 150 yd a fire bot stops and never sprays, so a cleared field quiets them.
        std::vector<ObjectGuid> alive;
        for (Creature* fireBot : fireBots)
            if (fireBot && fireBot->IsAlive())
                alive.push_back(fireBot->GetGUID());

        std::sort(alive.begin(), alive.end());
        state.keptFireBots = alive;
    }

    return state.keptFireBots;
}

bool IsMimironFireBotProtected(PlayerbotAI* botAI, Player* bot, Unit* fireBot)
{
    if (!fireBot || fireBot->GetEntry() != NPC_EMERGENCY_FIRE_BOT)
        return false;

    std::vector<ObjectGuid> const kept = GetMimironKeptFireBots(botAI, bot);
    return std::find(kept.begin(), kept.end(), fireBot->GetGUID()) != kept.end();
}

namespace
{
// Keeps a formation anchor on the floor. Moves the anchor, never a single slot: per-slot clamping
// deforms the formation into the clumps the spread exists to prevent.
Position ClampMimironAnchorToRoom(Position anchor)
{
    float const z = ULDUAR_MIMIRON_ROOM_CENTER.GetPositionZ();

    float const fromCentre =
        ULDUAR_MIMIRON_ROOM_CENTER.GetExactDist2d(anchor.GetPositionX(), anchor.GetPositionY());
    if (fromCentre > ULDUAR_MIMIRON_ROOM_RADIUS)
    {
        float const bearing =
            ULDUAR_MIMIRON_ROOM_CENTER.GetAngle(anchor.GetPositionX(), anchor.GetPositionY());
        anchor = Position(
            ULDUAR_MIMIRON_ROOM_CENTER.GetPositionX() + ULDUAR_MIMIRON_ROOM_RADIUS * cos(bearing),
            ULDUAR_MIMIRON_ROOM_CENTER.GetPositionY() + ULDUAR_MIMIRON_ROOM_RADIUS * sin(bearing), z);
    }

    return anchor;
}

// How many rows `count` bots need, capped at `maxRows`. Past that they pack tighter, since giving up
// spacing is cheaper than giving up range.
uint32 MimironWedgeRows(float firstRow, uint32 maxRows, uint32 count, float halfAngle, float spacing)
{
    uint32 held = 0;
    for (uint32 rows = 1; rows <= maxRows; ++rows)
    {
        float const radius = firstRow + (rows - 1) * spacing;
        held += static_cast<uint32>(2.0f * halfAngle * radius / spacing);
        if (held >= count)
            return rows;
    }

    return maxRows;
}

// Slot `index` of `count`, dealt row by row from the inside out and then spread edge to edge along
// whichever row it landed in.
void MimironWedgeSlot(float firstRow, uint32 rows, uint32 index, uint32 count, float halfAngle,
                      float spacing, float& outRadius, float& outOffset)
{
    uint32 const base = count / rows;
    uint32 const extra = count % rows;  // the first `extra` rows carry one more

    uint32 row = 0;
    uint32 filled = 0;
    for (; row + 1 < rows; ++row)
    {
        uint32 const size = base + (row < extra ? 1 : 0);
        if (index < filled + size)
            break;

        filled += size;
    }

    uint32 const size = base + (row < extra ? 1 : 0);
    uint32 const slot = index - filled;

    outRadius = firstRow + row * spacing;
    outOffset = size <= 1 ? 0.0f : -halfAngle + 2.0f * halfAngle * slot / (size - 1);
}

// Casting reach to `focus` as IsWithinCombatRange measures it in 3D, with a default player reach
// rather than the asking bot's own, so a gnome and a tauren build the same wedge.
float MimironWedgeReach(Unit* focus)
{
    return std::max(sPlayerbotAIConfig.SpellDistance + focus->GetCombatReach() + DEFAULT_COMBAT_REACH -
                        ULDUAR_MIMIRON_SPREAD_RANGE_MARGIN,
                    1.0f);
}

// How far past `reach` of `focus` a slot sits once slid, on the floor. Negative inside it.
float MimironSlotExcess(Position const& slot, Unit* focus, float reach, float shiftX, float shiftY)
{
    float const dz = focus->GetPositionZ() - slot.GetPositionZ();
    float const floor = std::sqrt(std::max(reach * reach - dz * dz, 0.0f));
    return focus->GetExactDist2d(slot.GetPositionX() + shiftX, slot.GetPositionY() + shiftY) - floor;
}

// `inset` pulls the limit in, so the farthest slot lands that far inside it.
void MimironShiftIntoRange(std::vector<Position> const& slots, Unit* focus, float& shiftX, float& shiftY,
                           float inset = 0.0f)
{
    shiftX = 0.0f;
    shiftY = 0.0f;
    if (!focus)
        return;

    float const reach = std::max(MimironWedgeReach(focus) - inset, 1.0f);

    // One pass puts the farthest slot on the limit, but a rigid move can leave a second one just past
    // it at a different bearing, so settle it a couple more times.
    for (uint32 pass = 0; pass < 3; ++pass)
    {
        float worst = 0.0f;
        float worstX = 0.0f;
        float worstY = 0.0f;
        for (Position const& slot : slots)
        {
            float const excess = MimironSlotExcess(slot, focus, reach, shiftX, shiftY);
            if (excess > worst)
            {
                worst = excess;
                worstX = slot.GetPositionX() + shiftX;
                worstY = slot.GetPositionY() + shiftY;
            }
        }

        if (worst <= 0.0f)
            break;

        float const bearing = std::atan2(focus->GetPositionY() - worstY, focus->GetPositionX() - worstX);
        shiftX += worst * std::cos(bearing);
        shiftY += worst * std::sin(bearing);
    }
}

// MimironShiftIntoRange held raid-wide: computed SLACK inside the limit, kept until a slot leaves
// range or the fresh answer moves SLACK away, so the wedge doesn't walk after every yard of drift.
void MimironStickyShiftIntoRange(Player* bot, std::vector<Position> const& slots, Unit* focus, float& shiftX,
                                 float& shiftY)
{
    MimironShiftIntoRange(slots, focus, shiftX, shiftY, ULDUAR_MIMIRON_SHIFT_SLACK);
    if (!bot || !focus)
        return;

    MimironFightState& state = MimironFightStateFor(bot);
    if (state.shiftFocusEntry == focus->GetEntry())
    {
        float const reach = MimironWedgeReach(focus);
        bool const inReach = std::all_of(slots.begin(), slots.end(),
                                         [&](Position const& slot)
                                         {
                                             return MimironSlotExcess(slot, focus, reach, state.shiftX,
                                                                      state.shiftY) <= 0.0f;
                                         });
        if (inReach && std::hypot(state.shiftX - shiftX, state.shiftY - shiftY) <= ULDUAR_MIMIRON_SHIFT_SLACK)
        {
            shiftX = state.shiftX;
            shiftY = state.shiftY;
            return;
        }
    }

    state.shiftFocusEntry = focus->GetEntry();
    state.shiftX = shiftX;
    state.shiftY = shiftY;
}

// Every slot of the Firefighter phase 1 camp: a wedge on the tank spot through the live stack anchor,
// turned `turn` toward the room. Fixed so the fire gathers in one place, sliding only to stay in range.
std::vector<Position> MimironPhase1CampSlots(Position const& anchor, Unit* focus, uint32 count, float turn)
{
    Position const& hub = ULDUAR_MIMIRON_PHASE1_TANK_SPOT;
    float centreline = hub.GetAngle(anchor.GetPositionX(), anchor.GetPositionY());

    // Never past the room's own bearing, or the two anchors cross over.
    float toRoom = Position::NormalizeOrientation(
        hub.GetAngle(ULDUAR_MIMIRON_ROOM_CENTER.GetPositionX(), ULDUAR_MIMIRON_ROOM_CENTER.GetPositionY()) -
        centreline);
    if (toRoom > static_cast<float>(M_PI))
        toRoom -= 2.0f * static_cast<float>(M_PI);
    centreline += (toRoom >= 0.0f ? 1.0f : -1.0f) * std::min(turn, std::fabs(toRoom));

    uint32 const rows =
        MimironWedgeRows(ULDUAR_MIMIRON_PHASE1_CAMP_FIRST_ROW, ULDUAR_MIMIRON_PHASE1_CAMP_ROWS, count,
                         ULDUAR_MIMIRON_PHASE1_CAMP_HALF_ANGLE, ULDUAR_MIMIRON_PHASE1_CAMP_SPACING);

    // Every slot, not just this one: the shift is set by whichever is farthest out, and every bot has
    // to derive the same shift or the wedge deforms into the lopsided blob the slots exist to prevent.
    std::vector<Position> slots;
    slots.reserve(count);
    for (uint32 i = 0; i < count; ++i)
    {
        float radius = 0.0f;
        float offset = 0.0f;
        MimironWedgeSlot(ULDUAR_MIMIRON_PHASE1_CAMP_FIRST_ROW, rows, i, count,
                         ULDUAR_MIMIRON_PHASE1_CAMP_HALF_ANGLE, ULDUAR_MIMIRON_PHASE1_CAMP_SPACING,
                         radius, offset);

        float const bearing = Position::NormalizeOrientation(centreline + offset);
        slots.emplace_back(hub.GetPositionX() + radius * std::cos(bearing),
                           hub.GetPositionY() + radius * std::sin(bearing), hub.GetPositionZ());
    }

    float shiftX = 0.0f;
    float shiftY = 0.0f;
    MimironShiftIntoRange(slots, focus, shiftX, shiftY);

    for (Position& slot : slots)
        slot.Relocate(slot.GetPositionX() + shiftX, slot.GetPositionY() + shiftY, slot.GetPositionZ());

    return slots;
}

// Whether a player standing on `slot` sees `focus`. Same ray IsWithinLOSInMap casts from a player: its
// eye to the creature's hit sphere point. Out of sight is an invalid target and the bot drops it.
bool MimironSlotSeesFocus(Position const& slot, Unit* focus)
{
    Position const eye(slot.GetPositionX(), slot.GetPositionY(),
                       slot.GetPositionZ() + ULDUAR_MIMIRON_CAMP_SIGHT_EYE);
    Position const hit = focus->GetHitSpherePointFor(eye);
    return focus->GetMap()->isInLineOfSight(eye.GetPositionX(), eye.GetPositionY(), eye.GetPositionZ(),
                                            hit.GetPositionX(), hit.GetPositionY(), hit.GetPositionZ(),
                                            focus->GetPhaseMask(), LINEOFSIGHT_ALL_CHECKS,
                                            VMAP::ModelIgnoreFlags::Nothing);
}

// Smallest turn at which every camp slot sees the MK II, raid-wide so every bot builds the same camp.
// Rises at once, falls only after ULDUAR_MIMIRON_CAMP_SIGHT_HOLD_MS, holds when no step clears.
float MimironPhase1CampTurn(Player* bot, Position const& anchor, Unit* focus, uint32 count)
{
    MimironFightState& state = MimironFightStateFor(bot);
    if (!focus || (state.campTurnScanMs &&
                   GetMSTimeDiffToNow(state.campTurnScanMs) < ULDUAR_MIMIRON_SCAN_INTERVAL_MS))
        return state.campTurn;

    state.campTurnScanMs = getMSTime();

    float wanted = -1.0f;
    for (float turn = 0.0f; turn <= ULDUAR_MIMIRON_CAMP_SIGHT_MAX + 0.001f;
         turn += ULDUAR_MIMIRON_CAMP_SIGHT_STEP)
    {
        std::vector<Position> const slots = MimironPhase1CampSlots(anchor, focus, count, turn);
        if (std::all_of(slots.begin(), slots.end(),
                        [focus](Position const& slot) { return MimironSlotSeesFocus(slot, focus); }))
        {
            wanted = turn;
            break;
        }
    }

    if (wanted < 0.0f)
        return state.campTurn;

    if (wanted > state.campTurn)
    {
        state.campTurn = wanted;
        state.campTurnRaisedMs = getMSTime();
    }
    else if (wanted < state.campTurn &&
             GetMSTimeDiffToNow(state.campTurnRaisedMs) >= ULDUAR_MIMIRON_CAMP_SIGHT_HOLD_MS)
    {
        state.campTurn = wanted;
    }

    return state.campTurn;
}

Position MimironPhase1CampSlot(Player* bot, Position const& anchor, Unit* focus, uint32 index,
                               uint32 count)
{
    std::vector<Position> const slots =
        MimironPhase1CampSlots(anchor, focus, count, MimironPhase1CampTurn(bot, anchor, focus, count));
    return slots[std::min(index, count - 1)];
}

// Phase 3 east wedge: the add pads sit on west, north-east and south-east arms, so a ring would put
// lone ranged bots in an add's path.
bool GetMimironPhase3Slot(Player* bot, Group* group, Unit* focus, Position& out, uint32& index,
                          uint32& count)
{
    // No melee slots: adds walk in from outside the wedge, and a slot at ACTION_RAID would beat the chase
    // at ACTION_HIGH every tick.
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    index = 0;
    count = 0;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !PlayerbotAI::IsRanged(member))
            continue;

        if (member == bot)
            index = count;

        ++count;
    }

    if (count == 0)
        return false;

    // Built to fit casting range: rows stop there and pack tighter past it.
    float const rangedDepth = std::max(sPlayerbotAIConfig.SpellDistance -
                                           ULDUAR_MIMIRON_SPREAD_RANGE_MARGIN -
                                           ULDUAR_MIMIRON_PHASE3_MIN_RADIUS,
                                       0.0f);
    uint32 const rows = MimironWedgeRows(
        ULDUAR_MIMIRON_PHASE3_MIN_RADIUS,
        1u + static_cast<uint32>(rangedDepth / ULDUAR_MIMIRON_PHASE3_SPACING), count,
        ULDUAR_MIMIRON_PHASE3_WEDGE_HALF_ANGLE, ULDUAR_MIMIRON_PHASE3_SPACING);

    // Anchored on the room centre, so a Bomb Bot spawning on the unit has ~30 yd to cross. Centreline on
    // the staging point, the gap between the east arms that nothing walks in from.
    Position const& anchor = ULDUAR_MIMIRON_ROOM_CENTER;
    float const centreline = ULDUAR_MIMIRON_ROOM_CENTER.GetAngle(
        ULDUAR_MIMIRON_PHASE3_STAGE.GetPositionX(), ULDUAR_MIMIRON_PHASE3_STAGE.GetPositionY());

    std::vector<Position> slots;
    slots.reserve(count);
    for (uint32 i = 0; i < count; ++i)
    {
        float radius = 0.0f;
        float offset = 0.0f;
        MimironWedgeSlot(ULDUAR_MIMIRON_PHASE3_MIN_RADIUS, rows, i, count,
                         ULDUAR_MIMIRON_PHASE3_WEDGE_HALF_ANGLE, ULDUAR_MIMIRON_PHASE3_SPACING, radius,
                         offset);

        float const bearing = Position::NormalizeOrientation(centreline + offset);
        slots.emplace_back(anchor.GetPositionX() + radius * std::cos(bearing),
                           anchor.GetPositionY() + radius * std::sin(bearing),
                           ULDUAR_MIMIRON_ROOM_CENTER.GetPositionZ());
    }

    // Shifted toward the unit only as far as casting range needs: it hovers ~16 yd up, so floor distance
    // understates range. Never onto it, or raid and unit circle the room together.
    float shiftX = 0.0f;
    float shiftY = 0.0f;
    MimironStickyShiftIntoRange(bot, slots, focus, shiftX, shiftY);

    Position const& mine = slots[std::min(index, count - 1)];
    out = Position(mine.GetPositionX() + shiftX, mine.GetPositionY() + shiftY, mine.GetPositionZ());
    return true;
}

// Firefighter wedge centreline: the east gap, re-aimed onto a barrage's safe sector before it ignites.
// That sector's bisector is lead + pi - sweep/2, and it's about the wedge's own width, so aim exactly.
float GetMimironWedgeCentreline(PlayerbotAI* botAI, Player* bot)
{
    float const east = ULDUAR_MIMIRON_ROOM_CENTER.GetAngle(
        ULDUAR_MIMIRON_PHASE3_STAGE.GetPositionX(), ULDUAR_MIMIRON_PHASE3_STAGE.GetPositionY());

    MimironFightState& state = MimironFightStateFor(bot);
    float const current = state.wedgeAimedMs ? state.wedgeBearing : east;

    Unit* vx001 = GetUldUnitByEntry(botAI, NPC_VX001);
    if (!vx001)
        return current;

    // Only while spinning up: once lit, lead and sweep describe what's left of the cast and the bisector
    // moves every tick. One re-aim a barrage, latched for everyone.
    MimironBarrageWindow const window = GetMimironBarrageWindow(bot, vx001);
    if (!window.valid || window.untilLive <= 0.0f)
        return current;

    if (state.wedgeAimedMs && GetMSTimeDiffToNow(state.wedgeAimedMs) < ULDUAR_MIMIRON_WEDGE_REAIM_MS)
        return current;

    // A cone that misses the wedge is worth sitting through: moving costs 25 bots a walk each, and
    // the fire convergence the wedge exists for is paid for in where it sits.
    auto const onWedge = [&](float bearing)
    {
        return Position(ULDUAR_MIMIRON_ROOM_CENTER.GetPositionX() +
                            ULDUAR_MIMIRON_SPREAD_RADIUS * std::cos(bearing),
                        ULDUAR_MIMIRON_ROOM_CENTER.GetPositionY() +
                            ULDUAR_MIMIRON_SPREAD_RADIUS * std::sin(bearing),
                        ULDUAR_MIMIRON_ROOM_CENTER.GetPositionZ());
    };

    bool clear = true;
    for (float edge :
         {-ULDUAR_MIMIRON_PHASE3_WEDGE_HALF_ANGLE, 0.0f, ULDUAR_MIMIRON_PHASE3_WEDGE_HALF_ANGLE})
        clear = clear && IsMimironSpotBarrageSafe(vx001, window, onWedge(current + edge), 0.0f);

    if (clear)
        return current;

    state.wedgeBearing =
        Position::NormalizeOrientation(window.lead + static_cast<float>(M_PI) - 0.5f * window.sweep);
    state.wedgeAimedMs = getMSTime();

    return state.wedgeBearing;
}

// Phase 4 Firefighter tank spot: the fixed one until fire gathers, then the ring point with least fire.
// Raid-wide on the observability interval, since trigger, action and formation guard must agree.
Position GetMimironPhase4TankSpot(PlayerbotAI* botAI, Player* bot)
{
    MimironFightState& state = MimironFightStateFor(bot);
    Position const live = state.tankSpotPickedMs ? state.tankSpot : ULDUAR_MIMIRON_PHASE4_TANK_SPOT;

    if (state.tankSpotScanMs &&
        GetMSTimeDiffToNow(state.tankSpotScanMs) < ULDUAR_MIMIRON_SCAN_INTERVAL_MS)
        return live;

    state.tankSpotScanMs = getMSTime();

    if (state.tankSpotPickedMs &&
        GetMSTimeDiffToNow(state.tankSpotPickedMs) < ULDUAR_MIMIRON_PHASE4_TANK_HOLD_MS)
        return live;

    // Never mid-barrage: VX-001 rides the chassis and the cone radiates from wherever it stands.
    if (Unit* vx001 = GetUldUnitByEntry(botAI, NPC_VX001))
    {
        if (GetMimironSpinningUpSeconds(vx001) >= 0.0f ||
            vx001->HasAura(SPELL_P3WX2_LASER_BARRAGE_AURA_1) ||
            vx001->HasAura(SPELL_P3WX2_LASER_BARRAGE_AURA_2))
            return live;
    }

    MimironFirefighterHazards const hazards = GetMimironFirefighterHazards(botAI);
    auto const fireNear = [&hazards](Position const& spot)
    {
        uint32 count = 0;
        for (Position const& flame : hazards.flames)
            if (spot.GetExactDist2d(flame.GetPositionX(), flame.GetPositionY()) <=
                ULDUAR_MIMIRON_PHASE4_TANK_CLEAR_RADIUS)
                ++count;

        return count;
    };

    uint32 const liveFire = fireNear(live);
    if (liveFire <= ULDUAR_MIMIRON_PHASE4_TANK_FIRE_LIMIT)
        return live;

    // Ties go to the shorter drag.
    Position best = live;
    uint32 bestFire = liveFire;
    auto const consider = [&](Position const& candidate)
    {
        uint32 const fire = fireNear(candidate);
        if (fire < bestFire ||
            (fire == bestFire && live.GetExactDist2d(candidate) < live.GetExactDist2d(best)))
        {
            best = candidate;
            bestFire = fire;
        }
    };

    consider(ULDUAR_MIMIRON_PHASE4_TANK_SPOT);
    for (float ring : ULDUAR_MIMIRON_PHASE4_TANK_RINGS)
    {
        for (uint32 i = 0; i < ULDUAR_MIMIRON_PHASE4_TANK_BEARINGS; ++i)
        {
            float const bearing = 2.0f * static_cast<float>(M_PI) * static_cast<float>(i) /
                                  static_cast<float>(ULDUAR_MIMIRON_PHASE4_TANK_BEARINGS);
            consider(Position(ULDUAR_MIMIRON_ROOM_CENTER.GetPositionX() + ring * std::cos(bearing),
                              ULDUAR_MIMIRON_ROOM_CENTER.GetPositionY() + ring * std::sin(bearing),
                              ULDUAR_MIMIRON_ROOM_CENTER.GetPositionZ()));
        }
    }

    if (bestFire + ULDUAR_MIMIRON_PHASE4_TANK_FIRE_MARGIN > liveFire)
        return live;

    state.tankSpot = best;
    state.tankSpotPickedMs = getMSTime();

    return best;
}

bool DeriveMimironSpreadSlot(PlayerbotAI* botAI, Player* bot, Position& out, uint32& index, uint32& count)
{
    index = 0;
    count = 0;

    if (!botAI || !bot)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Nothing attackable means a handover, with every mech NOT_SELECTABLE. The staging focus finds them
    // anyway, for the one spot handed out then.
    Unit* focus = GetMimironRingFocus(botAI);
    bool const staging = focus == nullptr;
    if (staging)
        focus = GetMimironStagingFocus(bot);

    if (!focus)
        return false;

    // The phase 4 tank holds the chassis still, since it carries the barrage apex. Phase 4 by the vehicle
    // seat: a part in Self Repair is NON_ATTACKABLE and would fail a three-attackable test.
    bool const phase4 = staging ? focus->GetEntry() == NPC_VX001 && focus->GetVehicleBase() != nullptr
                                : IsMimironPhase4(bot);

    if (PlayerbotAI::IsMainTank(bot) && phase4)
    {
        // Only while the MK II is his. The chassis walks after whoever it is hitting, so a spot would
        // keep him away from it; with none, reach melee and his taunt take him back.
        if (!staging)
        {
            Unit* leviathanMkII = GetUldUnitByEntry(botAI, NPC_LEVIATHAN_MKII);
            if (leviathanMkII && leviathanMkII->GetVictim() && leviathanMkII->GetVictim() != bot)
                return false;
        }

        // Under Firefighter the spot moves off the fire, so the melee on the chassis stand clear.
        // During the handover it stays fixed: that is where the chassis arrives.
        out = staging || !IsMimironHardModeActive(botAI) ? ULDUAR_MIMIRON_PHASE4_TANK_SPOT
                                                         : GetMimironPhase4TankSpot(botAI, bot);
        return true;
    }

    // Nobody else gets a spot in a handover: refusing hands the tick to follow, and a raid walking with its
    // master stays ahead of the fire. After p4tank on purpose, since that spot holds the boss.
    if (staging)
        return false;

    // Phase 3 tank spot, for the Magnetic Core: the unit holds 30 yd from its tank and the core grounds it
    // where it is, so the tank's spot decides where the raid spends the 20 s window.
    if (PlayerbotAI::IsMainTank(bot) && focus->GetEntry() == NPC_AERIAL_COMMAND_UNIT)
    {
        out = ULDUAR_MIMIRON_ROOM_CENTER;
        return true;
    }

    if (focus->GetEntry() == NPC_AERIAL_COMMAND_UNIT)
        return GetMimironPhase3Slot(bot, group, focus, out, index, count);

    // Phase 1 tank spot: the tank flees every Shock Blast and the boss follows, so nothing else brings it
    // back. Firefighter holds it 53 yd west, keeping the fire off the ground VX-001 is summoned onto.
    bool const phase1 = !phase4 && focus->GetEntry() == NPC_LEVIATHAN_MKII;
    bool const firefighter = IsMimironHardModeActive(botAI);

    if (PlayerbotAI::IsMainTank(bot) && phase1)
    {
        // No anchor until the boss is his, so "reach melee" keeps him on it building threat. Firefighter
        // only: normal mode's anchor is the centre, where the MK II already stands.
        if (firefighter && !IsMimironTankDragReady(botAI, bot))
            return false;

        out = firefighter ? ULDUAR_MIMIRON_PHASE1_TANK_SPOT : ULDUAR_MIMIRON_ROOM_CENTER;
        return true;
    }

    // Hard-mode melee in phases 2 and 4: a sector opposite the ranged wedge, out of its burst and pulse
    // cones, on the barrage dodge's ring round VX-001. The phase 4 main tank holds the MK II instead.
    if (firefighter && focus->GetEntry() == NPC_VX001 && (phase4 || !focus->GetVehicleBase()) &&
        !PlayerbotAI::IsRanged(bot))
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || PlayerbotAI::IsRanged(member) ||
                (phase4 && PlayerbotAI::IsMainTank(member)))
                continue;

            if (member == bot)
                index = count;

            ++count;
        }

        if (count == 0)
            return false;

        float const half = ULDUAR_MIMIRON_PHASE2_MELEE_HALF_ANGLE;
        float const offset = -half + 2.0f * half * (static_cast<float>(index) + 0.5f) / static_cast<float>(count);
        float const bearing = Position::NormalizeOrientation(GetMimironWedgeCentreline(botAI, bot) +
                                                             static_cast<float>(M_PI) + offset);
        float const radius = focus->GetCombatReach() + ULDUAR_MIMIRON_BARRAGE_MELEE_RING_MARGIN;
        out = Position(focus->GetPositionX() + radius * std::cos(bearing),
                       focus->GetPositionY() + radius * std::sin(bearing),
                       ULDUAR_MIMIRON_ROOM_CENTER.GetPositionZ());
        return true;
    }

    // Melee stand on whatever they are hitting, so only ranged and healers get a slot from here on.
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !PlayerbotAI::IsRanged(member) ||
            PlayerbotAI::IsMainTank(member))
            continue;

        if (member == bot)
            index = count;

        ++count;
    }

    if (count == 0)
        return false;

    // One camp rather than the ring, so every chain grows toward the same place. The anchor it points at
    // is raid-wide and moves off the fire.
    if (phase1 && firefighter)
    {
        out = MimironPhase1CampSlot(bot, GetMimironPhase1StackAnchor(botAI, bot), focus, index, count);
        return true;
    }

    // Centred on the mech, which its tank drags about. A slot past range deadlocks rather than correcting:
    // "reach spell" is ACTION_HIGH and this ring ACTION_RAID.
    Position const anchor = ClampMimironAnchorToRoom(Position(
        focus->GetPositionX(), focus->GetPositionY(), ULDUAR_MIMIRON_ROOM_CENTER.GetPositionZ()));

    // Firefighter, mid-phase: a wedge rather than a ring, so the chains converge on one sector and the
    // Frost Bomb, which only summons on a burning node, lands there and clears it.
    if (firefighter)
    {
        float const rangedDepth = std::max(sPlayerbotAIConfig.SpellDistance -
                                               ULDUAR_MIMIRON_SPREAD_RANGE_MARGIN -
                                               ULDUAR_MIMIRON_PHASE3_MIN_RADIUS,
                                           0.0f);
        uint32 const rows = MimironWedgeRows(
            ULDUAR_MIMIRON_PHASE3_MIN_RADIUS,
            1u + static_cast<uint32>(rangedDepth / ULDUAR_MIMIRON_PHASE3_SPACING), count,
            ULDUAR_MIMIRON_PHASE3_WEDGE_HALF_ANGLE, ULDUAR_MIMIRON_PHASE3_SPACING);

        float radius = 0.0f;
        float offset = 0.0f;
        MimironWedgeSlot(ULDUAR_MIMIRON_PHASE3_MIN_RADIUS, rows, index, count,
                         ULDUAR_MIMIRON_PHASE3_WEDGE_HALF_ANGLE, ULDUAR_MIMIRON_PHASE3_SPACING, radius,
                         offset);

        // Centreline from the room centre, not the anchor, so the sector stays put for the fire to pile up
        // in. Only a barrage re-aim moves it.
        float const bearing =
            Position::NormalizeOrientation(GetMimironWedgeCentreline(botAI, bot) + offset);

        out = Position(anchor.GetPositionX() + radius * cos(bearing),
                       anchor.GetPositionY() + radius * sin(bearing),
                       ULDUAR_MIMIRON_ROOM_CENTER.GetPositionZ());
        return true;
    }

    float const angle = 2.0f * static_cast<float>(M_PI) * index / count;
    out = Position(anchor.GetPositionX() + ULDUAR_MIMIRON_SPREAD_RADIUS * cos(angle),
                   anchor.GetPositionY() + ULDUAR_MIMIRON_SPREAD_RADIUS * sin(angle),
                   ULDUAR_MIMIRON_ROOM_CENTER.GetPositionZ());
    return true;
}
}  // namespace

MimironBurstSector GetMimironOtherGroupSector(PlayerbotAI* botAI, Player* bot, Unit* vx001)
{
    MimironBurstSector sector;
    if (!botAI || !bot || !vx001 || !IsMimironHardModeActive(botAI) || !IsMimironPhase2(botAI))
        return sector;

    bool const ranged = PlayerbotAI::IsRanged(bot);
    sector.valid = true;
    sector.centre = Position::NormalizeOrientation(GetMimironWedgeCentreline(botAI, bot) +
                                                   (ranged ? static_cast<float>(M_PI) : 0.0f));
    sector.halfWidth =
        (ranged ? ULDUAR_MIMIRON_PHASE2_MELEE_HALF_ANGLE : ULDUAR_MIMIRON_PHASE3_WEDGE_HALF_ANGLE) +
        ULDUAR_MIMIRON_RAPID_BURST_HALF_ANGLE;
    return sector;
}

bool IsMimironSpotInSector(Unit* vx001, MimironBurstSector const& sector, Position const& dest)
{
    if (!vx001 || !sector.valid)
        return false;

    float const bearing = vx001->GetAngle(dest.GetPositionX(), dest.GetPositionY());
    float off = Position::NormalizeOrientation(bearing - sector.centre);
    if (off > static_cast<float>(M_PI))
        off -= 2.0f * static_cast<float>(M_PI);

    return std::fabs(off) < sector.halfWidth;
}

bool GetMimironSpreadSlot(PlayerbotAI* botAI, Player* bot, Position& out)
{
    // Ahead of the derivation. Every caller here is registered for the whole raid, and the staging
    // fallback inside is two 200 yd grid scans.
    if (!IsNearMimironRoom(bot))
        return false;

    uint32 index = 0;
    uint32 count = 0;
    return DeriveMimironSpreadSlot(botAI, bot, out, index, count);
}
