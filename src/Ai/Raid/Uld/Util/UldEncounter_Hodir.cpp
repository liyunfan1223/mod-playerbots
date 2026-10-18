/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldEncounter_Hodir.h"

#include "BossAuraTriggers.h"
#include "Creature.h"
#include "EncounterHelpers.h"
#include "Group.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RaidInstanceState.h"
#include "SpellAuras.h"
#include "SpellMgr.h"
#include "TemporarySummon.h"
#include "Timer.h"
#include "UldScripts.h"
#include "UldTargets.h"
#include "Unit.h"

#include <algorithm>
#include <cmath>
#include <list>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace EncounterHelpers;

// Hold point when no fire qualifies. Every helper spawns inside its AttackStartCaster stand-off from
// here, so none walks to reach him and early zones land near their spawns.
const Position ULDUAR_HODIR_CENTRE = Position(1998.0f, -235.5f, 432.687f);

// Tank bearing when Hodir already stands on the hold point and the bearing to him means nothing.
// North-east into open floor, away from the chamfered south-west corner.
constexpr float ULDUAR_HODIR_HOLD_FALLBACK_BEARING = 1.0845f;

Unit* GetHodir(PlayerbotAI* botAI) { return GetUldUnitByEntry(botAI, NPC_HODIR); }

bool IsHodirEngaged(PlayerbotAI* botAI)
{
    Unit* boss = GetHodir(botAI);
    return boss && boss->IsInCombat();
}

bool IsHodirFlashFreezeIncoming(PlayerbotAI* botAI)
{
    Unit* boss = GetHodir(botAI);

    // Every cast slot, not CURRENT_GENERIC_SPELL: which slot a scripted boss cast lands in is the
    // script's business, and guessing wrong here silently opens the window on nothing.
    return boss && boss->HasUnitState(UNIT_STATE_CASTING) &&
           boss->FindCurrentSpellBySpellId(SPELL_FLASH_FREEZE) != nullptr;
}

bool IsHodirLustHeld(PlayerbotAI* botAI)
{
    Unit* boss = GetHodir(botAI);
    if (!boss || !boss->IsInCombat())
        return false;

    // Gone after 3:00, or never cast: nothing to time the hold by, so don't hold.
    Aura const* timer = boss->GetAura(SPELL_HODIR_SHATTER_CHEST_TIMER);
    if (!timer ||
        timer->GetMaxDuration() - timer->GetDuration() >= static_cast<int32>(ULDUAR_HODIR_LUST_FALLBACK_MS))
        return false;

    return !botAI->GetBot()->FindNearestCreature(NPC_TOASTY_FIRE, ULDUAR_HODIR_ROOM_SEARCH_RADIUS, true);
}

bool HodirFrozenBlowsActive(PlayerbotAI* botAI, Player* bot)
{
    Unit* boss = GetHodir(botAI);
    return boss && bot && boss->HasAura(sSpellMgr->GetSpellIdForDifficulty(SPELL_HODIR_FROZEN_BLOWS, bot));
}

bool HodirTauntWouldBeSuicide(PlayerbotAI* botAI, Player* bot)
{
    if (!bot || bot->GetHealthPct() >= ULDUAR_HODIR_TAUNT_HEALTH_FLOOR)
        return false;

    if (!HodirFrozenBlowsActive(botAI, bot))
        return false;

    // Only while another player holds him: with nobody on him the taunt is the rescue whatever the
    // bot's health, and a taunt is how he comes off a pet.
    Unit* boss = GetHodir(botAI);
    Unit* victim = boss ? boss->GetVictim() : nullptr;
    return victim && victim != bot && victim->IsPlayer() && victim->IsAlive();
}

float GetHodirShelterPark(Creature* shelter)
{
    return shelter && shelter->GetEntry() == NPC_HODIR_ICICLE_DRIFT ? ULDUAR_HODIR_BIG_SHARDS_CLEAR
                                                                    : ULDUAR_HODIR_SAFE_AREA_TOLERANCE;
}

float GetHodirShelterRelease(Creature* shelter)
{
    return GetHodirShelterPark(shelter) +
           (ULDUAR_HODIR_SAFE_AREA_RELEASE - ULDUAR_HODIR_SAFE_AREA_TOLERANCE);
}

// Both per-bot latches, keyed by instance on the way in so the inner maps are only ever touched by the
// one thread ticking that map.
struct HodirStarlightLatch
{
    Position zone;
    Position stand;
    uint32 rejectedSince = 0;  // when the stand started failing the band, 0 while it passes
};

// The fire a bot stands in, latched the same way and for the same reason: what makes a stand usable is
// measured against a boss who moves, so a stateless sweep swaps fires whenever he drifts.
struct HodirFireLatch
{
    ObjectGuid fire;
    Position stand;
};

// One per carry, keyed on the carrier, not the reader: carrier and receivers must gather on one point.
// applied is the aura's apply time, which tells two carries apart; stack counts repeat across them.
struct HodirStormRally
{
    time_t applied = 0;
    Position rally;
};

// Where Hodir is held, one per instance so both tanks agree. fire is empty on the centre; bearing runs
// from where he stood at adoption to the point, so the tank stands past it and he trails onto it.
struct HodirHoldLatch
{
    bool set = false;
    ObjectGuid fire;
    Position point;
    float bearing = 0.0f;
    uint32 offSince = 0;  // when he parked off the point, 0 while on it
};

struct HodirBotLatches
{
    std::unordered_map<ObjectGuid, ObjectGuid> shelter;
    std::unordered_map<ObjectGuid, HodirStarlightLatch> starlight;
    std::unordered_map<ObjectGuid, HodirFireLatch> fireStand;
    std::unordered_map<ObjectGuid, HodirStormRally> stormRally;

    // Where each ice-breaking candidate stood when the first block of a wave went up. Cleared once no
    // block is left, so the next wave reads fresh positions.
    std::unordered_map<ObjectGuid, Position> blockRank;

    HodirHoldLatch hold;
};

static RaidInstanceState<HodirBotLatches> hodirLatches;

static HodirBotLatches& HodirLatchesFor(Player* bot) { return hodirLatches.For(bot->GetInstanceId()); }

Creature* GetHodirShelter(PlayerbotAI* botAI, Player* bot)
{
    if (!bot || !GetHodir(botAI))
        return nullptr;

    std::unordered_map<ObjectGuid, ObjectGuid>& latched = HodirLatchesFor(bot).shelter;

    std::vector<Creature*> candidates;
    std::list<Creature*> found;

    bot->GetCreatureListWithEntryInGrid(found, NPC_SNOWPACKED_ICICLE, ULDUAR_HODIR_ROOM_SEARCH_RADIUS);
    for (Creature* target : found)
        if (target && target->IsAlive())
            candidates.push_back(target);

    // A drift stands where its target will spawn and is down from the cast's first tick, the target
    // only at 3.9s. Only while the cast is up: otherwise a drift is something to dodge.
    if (IsHodirFlashFreezeIncoming(botAI))
    {
        found.clear();
        bot->GetCreatureListWithEntryInGrid(found, NPC_HODIR_ICICLE_DRIFT, ULDUAR_HODIR_ROOM_SEARCH_RADIUS);
        for (Creature* drift : found)
            if (IsHodirIcicleLethal(drift))
                candidates.push_back(drift);
    }

    if (candidates.empty())
    {
        latched.erase(bot->GetGUID());

        return nullptr;
    }

    Creature* best = nullptr;

    // Latched, since a sideways dodge can carry a bot past the midpoint between two. A landed drift
    // drops out, and the sweep picks the target that spawned where the bot already stands.
    if (auto held = latched.find(bot->GetGUID()); held != latched.end())
        for (Creature* candidate : candidates)
            if (candidate->GetGUID() == held->second)
            {
                best = candidate;
                break;
            }

    if (!best)
    {
        // Nearest the bot, not one shared centre: all three targets grant the Safe Area. Trigger and
        // action both call this, so they never disagree.
        float bestDist = 0.0f;
        for (Creature* candidate : candidates)
        {
            float const dist = bot->GetExactDist2d(candidate);
            if (!best || dist < bestDist)
            {
                best = candidate;
                bestDist = dist;
            }
        }

        latched[bot->GetGUID()] = best->GetGUID();
    }

    return best;
}

bool IsHodirInLandedShelter(PlayerbotAI* botAI, Player* bot)
{
    Creature* shelter = GetHodirShelter(botAI, bot);
    return shelter && shelter->GetEntry() == NPC_SNOWPACKED_ICICLE &&
           bot->GetExactDist2d(shelter) <= GetHodirShelterPark(shelter);
}

// The live fire nearest the centre within ULDUAR_HODIR_HOLD_FIRE_LEASH, else the centre. On the fire,
// not beside it: melee wrap round him. Only Flash Freeze puts a fire out, so the pick holds till it dies.
static HodirHoldLatch const* DeriveHodirHold(PlayerbotAI* botAI, Player* bot)
{
    Unit* hodir = bot ? GetHodir(botAI) : nullptr;
    if (!hodir)
        return nullptr;

    HodirHoldLatch& hold = HodirLatchesFor(bot).hold;

    std::list<Creature*> found;
    bot->GetCreatureListWithEntryInGrid(found, NPC_TOASTY_FIRE, ULDUAR_HODIR_ROOM_SEARCH_RADIUS);

    Creature* pick = nullptr;
    if (!hold.fire.IsEmpty())
        for (Creature* fire : found)
            if (fire && fire->IsAlive() && fire->GetGUID() == hold.fire)
            {
                pick = fire;
                break;
            }

    if (!pick)
    {
        float bestDist = 0.0f;
        for (Creature* fire : found)
        {
            if (!fire || !fire->IsAlive())
                continue;

            float const dist = fire->GetExactDist2d(&ULDUAR_HODIR_CENTRE);
            if (dist > ULDUAR_HODIR_HOLD_FIRE_LEASH)
                continue;

            if (!pick || dist < bestDist)
            {
                pick = fire;
                bestDist = dist;
            }
        }
    }

    ObjectGuid const fire = pick ? pick->GetGUID() : ObjectGuid::Empty;

    // Out of combat the bearing is retaken every tick. Latches on engage; a wipe drops it.
    bool const engaged = hodir->IsInCombat();
    if (!engaged || !hold.set || hold.fire != fire)
    {
        hold.set = engaged;
        hold.fire = fire;
        hold.point = pick ? pick->GetPosition() : ULDUAR_HODIR_CENTRE;
        hold.offSince = 0;

        // He trails the tank, so the tank stands past the point on the side away from him and he stops
        // on it. Right on top of the point there is no side, so it falls back to a fixed bearing.
        hold.bearing = hodir->GetExactDist2d(&hold.point) > ULDUAR_HODIR_HOLD_BEARING_MIN_GAP
                           ? std::atan2(hold.point.GetPositionY() - hodir->GetPositionY(),
                                        hold.point.GetPositionX() - hodir->GetPositionX())
                           : ULDUAR_HODIR_HOLD_FALLBACK_BEARING;
    }
    else if (!hodir->isMoving() && hodir->GetExactDist2d(&hold.point) > ULDUAR_HODIR_HOLD_REAIM_GAP)
    {
        // Parked off the point with the tank spot already in his reach, so nothing brings him back.
        // Aim again from where he stands; the tank walks round to the far side and he trails onto it.
        uint32 const now = getMSTime();
        if (!hold.offSince)
            hold.offSince = now;
        else if (getMSTimeDiff(hold.offSince, now) >= ULDUAR_HODIR_HOLD_REAIM_MS)
        {
            hold.bearing = std::atan2(hold.point.GetPositionY() - hodir->GetPositionY(),
                                      hold.point.GetPositionX() - hodir->GetPositionX());
            hold.offSince = 0;
        }
    }
    else
        hold.offSince = 0;

    return &hold;
}

// Off-tank a quarter turn off the main tank's line, so a Frozen Blows taunt leaves him on the fire.
// Static floor check: the campfire is in the dynamic tree, and the normal one clips this spot to its near edge.
static Position HodirTankSpot(Player* bot, HodirHoldLatch const& hold, bool offTank)
{
    float const angle = offTank ? hold.bearing - static_cast<float>(M_PI) / 2.0f : hold.bearing;
    return ValidateStaticFloorPoint(
        bot, Position(hold.point.GetPositionX() + std::cos(angle) * ULDUAR_HODIR_HOLD_TANK_OFFSET,
                      hold.point.GetPositionY() + std::sin(angle) * ULDUAR_HODIR_HOLD_TANK_OFFSET,
                      hold.point.GetPositionZ()));
}

Player* GetHodirStormCloudCarrier(PlayerbotAI* botAI, Player* bot)
{
    if (!bot || !GetHodir(botAI))
        return nullptr;

    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    uint32 const cloud = sSpellMgr->GetSpellIdForDifficulty(SPELL_HODIR_STORM_CLOUD, bot);

    // Lowest guid wins when the boss has two carries running, so every bot picks the same one to
    // gather on rather than each heading for whichever it happened to iterate first.
    Player* best = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->GetMapId() != bot->GetMapId())
            continue;

        if (!member->HasAura(cloud))
            continue;

        if (!best || member->GetGUID() < best->GetGUID())
            best = member;
    }

    return best;
}

bool GetHodirStormCloudRally(PlayerbotAI* botAI, Player* bot, Player* carrier, Position& out)
{
    if (!bot || !carrier)
        return false;

    Aura* cloud = carrier->GetAura(sSpellMgr->GetSpellIdForDifficulty(SPELL_HODIR_STORM_CLOUD, carrier));
    if (!cloud)
        return false;

    std::unordered_map<ObjectGuid, HodirStormRally>& latched = HodirLatchesFor(bot).stormRally;
    HodirStormRally& entry = latched[carrier->GetGUID()];

    time_t const applied = cloud->GetApplyTime();
    if (entry.applied != applied)
    {
        entry.applied = applied;

        // Seeded from where the carrier stood when the cloud landed, so a melee carrier gathers the
        // melee and a ranged one the casters.
        entry.rally = ValidateStaticFloorPoint(bot, carrier->GetPosition());
    }

    out = entry.rally;
    return true;
}

// A bot already holding a buff has something to lose by walking, so it keeps the short leash.
static float HodirBuffWalk(Player* bot)
{
    return bot->HasAura(SPELL_HODIR_STARLIGHT) || bot->HasAura(SPELL_HODIR_TOASTY_FIRE_AURA)
               ? ULDUAR_HODIR_BUFF_WALK
               : ULDUAR_HODIR_BUFF_WALK_UNBUFFED;
}

// Any number may share a zone, each on its own bearing from where it stands, so they spread round it.
// Latched per bot till the zone expires, since the boss moves under it.
static bool FindHodirStarlightStand(PlayerbotAI* botAI, Player* bot, Position& out)
{
    std::unordered_map<ObjectGuid, HodirStarlightLatch>& latched = HodirLatchesFor(bot).starlight;

    // Bounded rather than the room radius: this runs per bot per tick, and a zone further out than
    // this is well past what the bot would walk for anyway.
    std::vector<Position> const zones =
        GetDynamicObjectPositions(bot, ULDUAR_HODIR_STARLIGHT_SEARCH_RADIUS, SPELL_HODIR_STARLIGHT);
    if (zones.empty())
    {
        latched.erase(bot->GetGUID());

        return false;
    }

    // The druid casts where it is standing, which is usually next to Hodir, so a good share of the
    // zones on the floor are ones no caster can use.
    Unit* hodir = GetHodir(botAI);

    // Shared so a latched stand is re-checked against what a fresh pick faces. No walk gate here: a bot
    // already on its way is past that question.
    auto standRejects = [&](Position const& stand) -> char const*
    {
        // Starlight's own inner edge and the caster band's outer one. A zone the bot cannot shoot the
        // boss from is not a throughput lever, whatever haste it carries.
        if (hodir)
        {
            float const gap = stand.GetExactDist2d(hodir);
            if (gap < ULDUAR_HODIR_STARLIGHT_MIN_BOSS_GAP || gap > ULDUAR_HODIR_CASTER_MAX_BOSS_GAP)
                return "noreach";
        }

        return nullptr;
    };

    auto held = latched.find(bot->GetGUID());
    if (held != latched.end())
    {
        bool stillUp = false;
        for (Position const& zone : zones)
        {
            if (zone.GetExactDist2d(&held->second.zone) <= ULDUAR_HODIR_STARLIGHT_ZONE_MATCH)
            {
                stillUp = true;
                break;
            }
        }

        if (stillUp)
        {
            // Held through a short reject so a band-edge wobble can't swap zones, dropped after a long one.
            if (standRejects(held->second.stand))
            {
                uint32 const now = getMSTime();
                if (!held->second.rejectedSince)
                    held->second.rejectedSince = now;

                if (getMSTimeDiff(held->second.rejectedSince, now) < ULDUAR_HODIR_STARLIGHT_REJECT_DROP_MS)
                    return false;
            }
            else
            {
                held->second.rejectedSince = 0;
                out = held->second.stand;

                return true;
            }
        }

        // The zone expired, or its stand stayed out of the band too long, so the next sweep counts.
        latched.erase(held);
    }

    bool found = false;
    float bestWalk = 0.0f;
    Position bestZone;

    for (Position const& zone : zones)
    {
        float const walk = bot->GetExactDist2d(&zone);
        if (walk > HodirBuffWalk(bot))
            continue;

        if (found && walk >= bestWalk)
            continue;

        float const bearing = std::atan2(bot->GetPositionY() - zone.GetPositionY(),
                                         bot->GetPositionX() - zone.GetPositionX());

        Position const stand = ValidateStaticFloorPoint(
            bot, Position(zone.GetPositionX() + std::cos(bearing) * ULDUAR_HODIR_STARLIGHT_STAND_RADIUS,
                          zone.GetPositionY() + std::sin(bearing) * ULDUAR_HODIR_STARLIGHT_STAND_RADIUS,
                          zone.GetPositionZ()));

        if (standRejects(stand))
            continue;

        out = stand;
        bestZone = zone;
        bestWalk = walk;
        found = true;
    }

    if (found)
        latched[bot->GetGUID()] = HodirStarlightLatch{bestZone, out};

    return found;
}

// Second choice behind Starlight, though the two stack. No case for the fire he's held on: nothing
// within 8 yd of it is 15 from him, so the caster band throws it out.
static bool FindHodirFireStand(PlayerbotAI* botAI, Player* bot, Position& out)
{
    std::unordered_map<ObjectGuid, HodirFireLatch>& latched = HodirLatchesFor(bot).fireStand;

    std::list<Creature*> found;
    bot->GetCreatureListWithEntryInGrid(found, NPC_TOASTY_FIRE, ULDUAR_HODIR_ROOM_SEARCH_RADIUS);

    Unit* hodir = GetHodir(botAI);

    // Both ends of the caster band: a spot the bot cannot shoot him from is worth nothing, and one
    // closer in puts it among the melee.
    auto standRejects = [&](Position const& stand)
    {
        if (!hodir)
            return false;

        float const gap = stand.GetExactDist2d(hodir);
        return gap < ULDUAR_HODIR_RANGED_MIN_BOSS_GAP || gap > ULDUAR_HODIR_CASTER_MAX_BOSS_GAP;
    };

    // Nearest point of this fire to the bot, capped at the stand radius so it never sits on the aura's
    // edge. A bot already inside keeps the spot it is on, which is the whole walk saved.
    auto standFor = [&](Creature* fire)
    {
        float const gap = bot->GetExactDist2d(fire);
        if (gap <= ULDUAR_HODIR_FIRE_STAND_RADIUS)
            return bot->GetPosition();

        float const bearing = std::atan2(bot->GetPositionY() - fire->GetPositionY(),
                                         bot->GetPositionX() - fire->GetPositionX());
        return ValidateStaticFloorPoint(
            bot, Position(fire->GetPositionX() + std::cos(bearing) * ULDUAR_HODIR_FIRE_STAND_RADIUS,
                          fire->GetPositionY() + std::sin(bearing) * ULDUAR_HODIR_FIRE_STAND_RADIUS,
                          fire->GetPositionZ()));
    };

    auto held = latched.find(bot->GetGUID());
    if (held != latched.end())
    {
        for (Creature* fire : found)
            if (fire && fire->IsAlive() && fire->GetGUID() == held->second.fire)
            {
                if (standRejects(held->second.stand))
                    return false;

                out = held->second.stand;
                return true;
            }

        // The fire burned out or was put out by a Flash Freeze, so the sweep below is what counts.
        latched.erase(held);
    }

    bool picked = false;
    float bestWalk = 0.0f;
    ObjectGuid bestFire;

    for (Creature* fire : found)
    {
        if (!fire || !fire->IsAlive())
            continue;

        float const walk = bot->GetExactDist2d(fire);
        if (walk > HodirBuffWalk(bot) + ULDUAR_HODIR_FIRE_STAND_RADIUS)
            continue;

        if (picked && walk >= bestWalk)
            continue;

        Position const stand = standFor(fire);
        if (bot->GetExactDist2d(&stand) > HodirBuffWalk(bot) || standRejects(stand))
            continue;

        out = stand;
        bestFire = fire->GetGUID();
        bestWalk = walk;
        picked = true;
    }

    if (picked)
        latched[bot->GetGUID()] = HodirFireLatch{bestFire, out};

    return picked;
}

bool GetHodirStarlightZoneAt(PlayerbotAI* /*botAI*/, Player* bot, Position& out)
{
    if (!bot)
        return false;

    std::vector<Position> const zones =
        GetDynamicObjectPositions(bot, ULDUAR_HODIR_STARLIGHT_SEARCH_RADIUS, SPELL_HODIR_STARLIGHT);

    bool found = false;
    float bestDist = 0.0f;
    for (Position const& zone : zones)
    {
        float const dist = bot->GetExactDist2d(&zone);
        if (dist > ULDUAR_HODIR_STARLIGHT_RADIUS)
            continue;

        if (!found || dist < bestDist)
        {
            out = zone;
            bestDist = dist;
            found = true;
        }
    }

    return found;
}

bool IsHodirIcicleLethal(Creature* icicle)
{
    if (!icicle || !icicle->IsAlive())
        return false;

    TempSummon* summon = icicle->ToTempSummon();
    uint32 const remaining = summon ? summon->GetTimer() : 0;

    // Both icicles are TEMPSUMMON_TIMED_DESPAWN, so GetTimer counts the 7000ms down. Any other type
    // parks it at its start and every icicle reads live, the safe way to be wrong.
    return !remaining || remaining > ULDUAR_HODIR_ICICLE_SPENT_MS;
}

std::vector<HazardCircle> CollectHodirIcicleHazards(Player* bot, float radius, float smallClear, float bigClear)
{
    std::vector<HazardCircle> hazards;

    for (auto const& entry : {std::make_pair(static_cast<uint32>(NPC_HODIR_ICICLE_SMALL), smallClear),
                              std::make_pair(static_cast<uint32>(NPC_HODIR_ICICLE_DRIFT), bigClear)})
    {
        std::list<Creature*> found;
        bot->GetCreatureListWithEntryInGrid(found, entry.first, radius);
        for (Creature* icicle : found)
            if (IsHodirIcicleLethal(icicle))
                hazards.emplace_back(icicle->GetPosition(), entry.second);
    }

    return hazards;
}

// 2D distance from a point to the walk between two positions, endpoints included, so one test
// covers where the bot is, where it is headed, and everything it crosses on the way.
static float DistToSegment2d(Position const& point, Position const& from, Position const& to)
{
    float const dx = to.GetPositionX() - from.GetPositionX();
    float const dy = to.GetPositionY() - from.GetPositionY();
    float const lengthSq = dx * dx + dy * dy;

    float t = 0.0f;
    if (lengthSq > 0.0f)
    {
        t = ((point.GetPositionX() - from.GetPositionX()) * dx +
             (point.GetPositionY() - from.GetPositionY()) * dy) /
            lengthSq;
        t = std::clamp(t, 0.0f, 1.0f);
    }

    float const nearestX = from.GetPositionX() + dx * t;
    float const nearestY = from.GetPositionY() + dy * t;
    return std::hypot(point.GetPositionX() - nearestX, point.GetPositionY() - nearestY);
}

bool IsHodirWalkThroughLiveIcicle(Player* bot, Position const& dest, float smallRadius, float bigRadius)
{
    if (!bot)
        return false;

    Position const self = bot->GetPosition();
    for (auto const& entry : {std::make_pair(static_cast<uint32>(NPC_HODIR_ICICLE_SMALL), smallRadius),
                              std::make_pair(static_cast<uint32>(NPC_HODIR_ICICLE_DRIFT), bigRadius)})
    {
        std::list<Creature*> icicles;
        bot->GetCreatureListWithEntryInGrid(icicles, entry.first, ULDUAR_HODIR_ROOM_SEARCH_RADIUS);
        for (Creature* icicle : icicles)
            if (IsHodirIcicleLethal(icicle) && DistToSegment2d(icicle->GetPosition(), self, dest) < entry.second)
                return true;
    }

    return false;
}

bool GetHodirDodgePreference(PlayerbotAI* botAI, Player* bot, Position& out)
{
    if (!PlayerbotAI::IsMelee(bot))
    {
        float tolerance = 0.0f;
        return GetHodirAnchor(botAI, bot, out, tolerance);
    }

    Unit* boss = GetHodir(botAI);
    if (!boss)
        return false;

    out = boss->GetPosition();
    return true;
}

static bool DeriveHodirShuttleLeg(PlayerbotAI* botAI, Player* bot, Position& out)
{
    if (!bot)
        return false;

    bool const mainTank = PlayerbotAI::IsMainTank(bot);
    if (mainTank || PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
    {
        // Two fixed ends square to the hold line, since he follows: neither drags him off the fire, and
        // 6 yd apart spans two aura ticks.
        HodirHoldLatch const* hold = DeriveHodirHold(botAI, bot);
        if (!hold)
            return false;

        Position const spot = HodirTankSpot(bot, *hold, !mainTank);

        float const axis = hold->bearing + static_cast<float>(M_PI) / 2.0f;
        float const dx = std::cos(axis) * ULDUAR_HODIR_SHUTTLE_HALF_LEG;
        float const dy = std::sin(axis) * ULDUAR_HODIR_SHUTTLE_HALF_LEG;

        Position const legA = ValidateStaticFloorPoint(
            bot, Position(spot.GetPositionX() + dx, spot.GetPositionY() + dy, spot.GetPositionZ()));
        Position const legB = ValidateStaticFloorPoint(
            bot, Position(spot.GetPositionX() - dx, spot.GetPositionY() - dy, spot.GetPositionZ()));

        // Whichever end is further away, so the leg is always the full 6 yd and IsDuplicateMove
        // cannot refuse it for repeating the last destination.
        out = bot->GetExactDist2d(&legA) > bot->GetExactDist2d(&legB) ? legA : legB;
        return true;
    }

    // In a landed shelter during the cast, shed across it: standing out the 9s cast adds two stacks.
    if (IsHodirFlashFreezeIncoming(botAI) && IsHodirInLandedShelter(botAI, bot))
    {
        Creature* shelter = GetHodirShelter(botAI, bot);
        float const bearing = std::atan2(bot->GetPositionY() - shelter->GetPositionY(),
                                         bot->GetPositionX() - shelter->GetPositionX());
        float const dx = std::cos(bearing) * ULDUAR_HODIR_SHELTER_SHED_RADIUS;
        float const dy = std::sin(bearing) * ULDUAR_HODIR_SHELTER_SHED_RADIUS;

        Position const legA = ValidateStaticFloorPoint(
            bot, Position(shelter->GetPositionX() + dx, shelter->GetPositionY() + dy, shelter->GetPositionZ()));
        Position const legB = ValidateStaticFloorPoint(
            bot, Position(shelter->GetPositionX() - dx, shelter->GetPositionY() - dy, shelter->GetPositionZ()));

        out = bot->GetExactDist2d(&legA) > bot->GetExactDist2d(&legB) ? legA : legB;
        return true;
    }

    // In Starlight, shed across the zone with both ends inside it, full leg like the tank's: the shed
    // outranks the anchor and would otherwise walk the bot out of the buff.
    Position zone;
    if (GetHodirStarlightZoneAt(botAI, bot, zone))
    {
        float const bearing = std::atan2(bot->GetPositionY() - zone.GetPositionY(),
                                         bot->GetPositionX() - zone.GetPositionX());
        float const dx = std::cos(bearing) * ULDUAR_HODIR_STARLIGHT_SHED_RADIUS;
        float const dy = std::sin(bearing) * ULDUAR_HODIR_STARLIGHT_SHED_RADIUS;

        Position const legA = ValidateStaticFloorPoint(
            bot, Position(zone.GetPositionX() + dx, zone.GetPositionY() + dy, zone.GetPositionZ()));
        Position const legB = ValidateStaticFloorPoint(
            bot, Position(zone.GetPositionX() - dx, zone.GetPositionY() - dy, zone.GetPositionZ()));

        out = bot->GetExactDist2d(&legA) > bot->GetExactDist2d(&legB) ? legA : legB;
        return true;
    }

    // Everyone else: nearest point clear of allies, live icicles and, for ranged, Hodir. distanceStep
    // is the leg length, not a probe step: 6 yd is the shortest it returns and spans two aura ticks.
    auto sweepFor = [&](float declump, float smallClear, float bigClear)
    {
        std::vector<HazardCircle> hazards;

        for (ObjectGuid const& guid :
             botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest friendly players")->Get())
        {
            Unit* ally = botAI->GetUnit(guid);
            if (ally && ally->IsAlive() && ally != bot && bot->GetExactDist2d(ally) <= ULDUAR_HODIR_DODGE_LEASH)
                hazards.emplace_back(ally->GetPosition(), declump);
        }

        for (HazardCircle const& icicle :
             CollectHodirIcicleHazards(bot, ULDUAR_HODIR_ROOM_SEARCH_RADIUS, smallClear, bigClear))
            hazards.push_back(icicle);

        // Melee belong inside this, so only the bots the gap is protecting get it.
        if (!PlayerbotAI::IsMelee(bot))
            if (Unit* hodir = GetHodir(botAI))
                hazards.emplace_back(hodir->GetPosition(), ULDUAR_HODIR_RANGED_MIN_BOSS_GAP);

        return hazards;
    };

    Position preference;
    Position const* preferNear = GetHodirDodgePreference(botAI, bot, preference) ? &preference : nullptr;

    // At the room's edge the collision check pulls probes back onto the bot's own spot, which then wins
    // and MoveTo refuses as already there.
    auto const goesSomewhere = [bot](float x, float y)
    { return bot->GetExactDist2d(x, y) >= ULDUAR_HODIR_SHUTTLE_HALF_LEG; };

    Position leg = FindNearestPositionClearOfHazards(
        bot, sweepFor(ULDUAR_HODIR_DECLUMP_RADIUS, ULDUAR_HODIR_ICE_SHARDS_CLEAR, ULDUAR_HODIR_BIG_SHARDS_CLEAR),
        ULDUAR_HODIR_DODGE_LEASH, 2.0f * ULDUAR_HODIR_SHUTTLE_HALF_LEG, ULDUAR_HODIR_SWEEP_ANGLE_STEP, preferNear,
        goesSomewhere);

    // Nothing clear with margin: give up the clears' 2 yd and the declump before standing still, since
    // a bot that can't move sheds nothing.
    if (!leg.GetPositionX() && !leg.GetPositionY())
        leg = FindNearestPositionClearOfHazards(
            bot,
            sweepFor(ULDUAR_HODIR_SHUTTLE_HALF_LEG, ULDUAR_HODIR_ICE_SHARDS_RADIUS + ULDUAR_HODIR_DODGE_TRIGGER_MARGIN,
                     ULDUAR_HODIR_BIG_SHARDS_RADIUS + ULDUAR_HODIR_DODGE_TRIGGER_MARGIN),
            2.0f * ULDUAR_HODIR_DODGE_LEASH, 2.0f * ULDUAR_HODIR_SHUTTLE_HALF_LEG, ULDUAR_HODIR_SWEEP_ANGLE_STEP,
            preferNear, goesSomewhere);

    if (!leg.GetPositionX() && !leg.GetPositionY())
        return false;

    out = leg;
    return true;
}

bool GetHodirShuttleLeg(PlayerbotAI* botAI, Player* bot, Position& out)
{
    return DeriveHodirShuttleLeg(botAI, bot, out);
}

bool IsHodirBitingColdShedArmed(Player* bot)
{
    if (!bot)
        return false;

    // A fire counts as movement on every tick, so a bot in one sheds without the shuttle ever running.
    if (bot->HasAura(SPELL_HODIR_TOASTY_FIRE_AURA))
        return false;

    Aura* cold = bot->GetAura(SPELL_BITING_COLD_PLAYER_AURA);
    if (!cold)
        return false;

    // The extra stack in Starlight buys uninterrupted haste rather than an uninterrupted stand, and it
    // has to be the same number the action arms at or the two disagree about whether a shed is coming.
    uint32 const arm = bot->HasAura(SPELL_HODIR_STARLIGHT) ? ULDUAR_HODIR_BITING_COLD_SHED_STACKS_IN_STARLIGHT
                                                           : ULDUAR_HODIR_BITING_COLD_SHED_STACKS;

    return cold->GetStackAmount() >= arm;
}

static bool DeriveHodirAnchor(PlayerbotAI* botAI, Player* bot, Position& out, float& tolerance)
{
    if (!GetHodir(botAI))
        return false;

    bool const mainTank = PlayerbotAI::IsMainTank(bot);
    if (mainTank || PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
    {
        HodirHoldLatch const* hold = DeriveHodirHold(botAI, bot);
        if (!hold)
            return false;

        out = HodirTankSpot(bot, *hold, !mainTank);
        tolerance = ULDUAR_HODIR_TANK_HOLD_TOLERANCE;
        return true;
    }

    // Melee ride the boss. Pinning them would cost uptime, and he is on a fire whenever one is near
    // enough to drag him to.
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    // Ranged and healers get a buff stand or nothing: a home spot only walks them back out of a zone.
    // Neither is gated on holding the aura, which would re-target the bot the moment the buff landed.
    bool found = false;
    Position stand;
    if (FindHodirStarlightStand(botAI, bot, stand))
    {
        out = stand;
        tolerance = ULDUAR_HODIR_STARLIGHT_STAND_TOLERANCE;
        found = true;
    }
    else if (FindHodirFireStand(botAI, bot, stand))
    {
        out = stand;
        tolerance = ULDUAR_HODIR_FIRE_STAND_TOLERANCE;
        found = true;
    }

    return found;
}

bool GetHodirAnchor(PlayerbotAI* botAI, Player* bot, Position& out, float& tolerance)
{
    return DeriveHodirAnchor(botAI, bot, out, tolerance);
}

Player* GetHodirResistancePaladin(PlayerbotAI* /*botAI*/, Player* bot)
{
    Group* group = bot ? bot->GetGroup() : nullptr;
    if (!group)
        return nullptr;

    static uint32 const ranks[] = {SPELL_FROST_RESISTANCE_AURA_RANK_5, SPELL_FROST_RESISTANCE_AURA_RANK_4,
                                   SPELL_FROST_RESISTANCE_AURA_RANK_3, SPELL_FROST_RESISTANCE_AURA_RANK_2,
                                   SPELL_FROST_RESISTANCE_AURA_RANK_1};

    // Paladin tank first: a tank never leaves the hold point, so the 40 yd aura covers both Frozen Blows targets.
    Player* tank = nullptr;
    Player* dps = nullptr;
    Player* healer = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->getClass() != CLASS_PALADIN ||
            member->GetMapId() != bot->GetMapId())
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

bool IsHodirTrappedAllyBreaker(PlayerbotAI* botAI, Player* bot, Unit* block)
{
    if (!block)
        return false;

    bool breaker = false;
    Group* group = bot->GetGroup();
    if (!group)
        breaker = true;
    else
    {
        // Healers keep healing: the raid is taking 14000 every two seconds from icicles while this block
        // is up, and it dies to a handful of hits anyway.
        std::vector<Player*> candidates;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member->GetMapId() != bot->GetMapId())
                continue;

            PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
            if (!memberAI || PlayerbotAI::IsHeal(member) || PlayerbotAI::IsTank(member))
                continue;

            if (member->GetExactDist2d(block) > ULDUAR_HODIR_TRAPPED_ALLY_RANGE)
                continue;

            candidates.push_back(member);
        }

        // Guid, not distance. Distances change every tick, so a distance rank re-shuffles the set
        // constantly and bots drop off the block and back onto the boss between one tick and the next.
        std::sort(candidates.begin(), candidates.end(),
                  [](Player* left, Player* right) { return left->GetGUID() < right->GetGUID(); });

        // Window offset by block guid, so blocks up together draw disjoint breakers, not the same five.
        size_t const total = candidates.size();
        size_t const offset = total ? block->GetGUID().GetCounter() % total : 0;

        for (size_t i = 0; i < total && i < ULDUAR_HODIR_TRAPPED_ALLY_BREAKERS; ++i)
            if (candidates[(offset + i) % total] == bot)
            {
                breaker = true;
                break;
            }
    }

    return breaker;
}

// Freeing order. Mages first, since their fire brings back both the Biting Cold shed and Singed; then
// Starlight, then Storm Cloud.
enum class HodirHelperKind : uint8
{
    Mage,
    Druid,
    Shaman,
    Priest,
    None
};

static HodirHelperKind GetHodirHelperKind(Unit* block)
{
    if (!block || block->GetEntry() != NPC_HODIR_FLASH_FREEZE_BLOCK)
        return HodirHelperKind::None;

    TempSummon* summon = block->ToTempSummon();
    Unit* helper = summon ? summon->GetSummonerUnit() : nullptr;
    switch (helper ? helper->GetEntry() : 0)
    {
        case NPC_HODIR_MAGE_ALLIANCE_10:
        case NPC_HODIR_MAGE_ALLIANCE_25:
        case NPC_HODIR_MAGE_HORDE_10:
        case NPC_HODIR_MAGE_HORDE_25:
            return HodirHelperKind::Mage;
        case NPC_HODIR_DRUID_ALLIANCE_10:
        case NPC_HODIR_DRUID_ALLIANCE_25:
        case NPC_HODIR_DRUID_HORDE_10:
        case NPC_HODIR_DRUID_HORDE_25:
            return HodirHelperKind::Druid;
        case NPC_HODIR_SHAMAN_ALLIANCE_10:
        case NPC_HODIR_SHAMAN_ALLIANCE_25:
        case NPC_HODIR_SHAMAN_HORDE_10:
        case NPC_HODIR_SHAMAN_HORDE_25:
            return HodirHelperKind::Shaman;
        case NPC_HODIR_PRIEST_ALLIANCE_10:
        case NPC_HODIR_PRIEST_ALLIANCE_25:
        case NPC_HODIR_PRIEST_HORDE_10:
        case NPC_HODIR_PRIEST_HORDE_25:
            return HodirHelperKind::Priest;
        default:
            return HodirHelperKind::None;
    }
}

Unit* GetHodirAssignedHelperBlock(PlayerbotAI* botAI, Player* bot)
{
    Unit* boss = GetHodir(botAI);
    if (!bot || !boss)
        return nullptr;

    // Too late for a freed helper to pay back the damage its block soaks. Trapped raiders are found
    // separately and still get freed.
    if (boss->GetHealthPct() < ULDUAR_HODIR_LAST_WAVE_HEALTH_PCT)
        return nullptr;

    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    // Ranged carry this: a melee leaving the boss for a block gives up all its uptime. Falls back to
    // the rest only when the raid has no ranged.
    std::vector<Player*> ranged;
    std::vector<Player*> rest;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->GetMapId() != bot->GetMapId())
            continue;

        PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
        if (!memberAI || PlayerbotAI::IsHeal(member) || PlayerbotAI::IsTank(member))
            continue;

        if (PlayerbotAI::IsRanged(member))
            ranged.push_back(member);
        else
            rest.push_back(member);
    }

    std::vector<Player*>& candidates = ranged.empty() ? rest : ranged;

    // Before the sweep, not after: the group walk above is free next to a grid pass, and on a raid with
    // ranged this drops every melee out before it costs one.
    if (std::find(candidates.begin(), candidates.end(), bot) == candidates.end())
        return nullptr;

    // Swept from the boss, not the bot: a bot-centred sweep gives each bot its own block set, and the
    // raid then disagrees on who owns what.
    std::list<Creature*> found;
    boss->GetCreatureListWithEntryInGrid(found, NPC_HODIR_FLASH_FREEZE_BLOCK, ULDUAR_HODIR_TRAPPED_ALLY_RANGE);

    std::vector<Creature*> blocks;
    for (Creature* candidate : found)
        if (candidate && candidate->IsAlive())
            blocks.push_back(candidate);

    std::unordered_map<ObjectGuid, Position>& ranks = HodirLatchesFor(bot).blockRank;
    if (blocks.empty())
    {
        ranks.clear();
        return nullptr;
    }

    // Helper kind, then guid. Sorted, both lists read the same to every bot, so the greedy pass below
    // agrees across the raid with no shared state.
    std::sort(candidates.begin(), candidates.end(),
              [](Player* left, Player* right) { return left->GetGUID() < right->GetGUID(); });
    std::sort(blocks.begin(), blocks.end(),
              [](Creature* left, Creature* right)
              {
                  HodirHelperKind const leftKind = GetHodirHelperKind(left);
                  HodirHelperKind const rightKind = GetHodirHelperKind(right);
                  if (leftKind != rightKind)
                      return leftKind < rightKind;
                  return left->GetGUID() < right->GetGUID();
              });

    // One seat per breaker. A mage block takes several, everything else one.
    std::vector<Creature*> seats;
    for (Creature* block : blocks)
    {
        uint32 const want =
            GetHodirHelperKind(block) == HodirHelperKind::Mage ? ULDUAR_HODIR_MAGE_BLOCK_BREAKERS : 1;
        for (uint32 i = 0; i < want; ++i)
            seats.push_back(block);
    }

    // While a mage is still iced the wave is holding up the next fire, so it gets the ranged group.
    // After that, what is left on ice is worth a few bots and the rest belong back on the boss.
    bool const mageIced = std::any_of(blocks.begin(), blocks.end(), [](Creature* block)
                                      { return GetHodirHelperKind(block) == HodirHelperKind::Mage; });

    size_t const total = candidates.size();
    size_t budget = std::min<size_t>(
        mageIced ? ULDUAR_HODIR_HELPER_BLOCK_BREAKERS : ULDUAR_HODIR_LATE_BLOCK_BREAKERS, seats.size());
    if (total > ULDUAR_HODIR_HELPER_BLOCK_MIN_FREE)
        budget = std::min(budget, total - ULDUAR_HODIR_HELPER_BLOCK_MIN_FREE);
    else
        budget = std::min<size_t>(budget, 1);

    // Ranked on where each bot stood when the wave went up, latched till no block is left: live
    // distances re-shuffle every tick, and a guid rotation hands blocks across the room.
    std::vector<Position> spots(total);
    for (size_t i = 0; i < total; ++i)
    {
        auto const known = ranks.find(candidates[i]->GetGUID());
        if (known != ranks.end())
            spots[i] = known->second;
        else
            spots[i] = ranks[candidates[i]->GetGUID()] = candidates[i]->GetPosition();
    }

    // Seat order, every seat taking the nearest slot still free.
    std::vector<bool> taken(total, false);
    for (size_t i = 0; i < budget; ++i)
    {
        size_t best = total;
        float bestDist = 0.0f;
        for (size_t j = 0; j < total; ++j)
        {
            if (taken[j])
                continue;

            float const dist = seats[i]->GetExactDist2d(&spots[j]);
            if (best == total || dist < bestDist)
            {
                best = j;
                bestDist = dist;
            }
        }

        if (best == total)
            break;

        taken[best] = true;
        if (candidates[best] == bot)
            return seats[i];
    }

    return nullptr;
}
