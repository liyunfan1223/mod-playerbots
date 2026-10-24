/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldEncounter_Algalon.h"

#include "Action.h"
#include "AiFactory.h"
#include "AiObjectContext.h"
#include "CellImpl.h"
#include "Chat.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Group.h"
#include "InstanceScript.h"
#include "Item.h"
#include "LastMovementValue.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "PaladinHelper.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RaidHumanTankCall.h"
#include "RaidInstanceState.h"
#include "RaidTankDefensive.h"
#include "ServerFacade.h"
#include "SharedDefines.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "Timer.h"
#include "UldEncounterGate.h"
#include "Unit.h"
#include "WorldPacket.h"
#include <RtiTargetValue.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iterator>
#include <limits>
#include <list>
#include <optional>
#include <set>
#include <string>
#include <tuple>

using namespace EncounterHelpers;

// Checked on the map 603 navmesh: every point below is on mesh at 0.04, settled Z 417.321, out to 44 yd on the hold's
// bearing. Angles are from the hold's line to his landing spot.
const Position ULDUAR_ALGALON_ROOM_CENTER = Position(1632.668f, -302.7656f, 417.3211f);
const Position ULDUAR_ALGALON_PICKUP_SLOT = Position(1633.781f, -310.688f, 417.321f);
const Position ULDUAR_ALGALON_PULL_OFFTANK_SLOT = Position(1628.668f, -309.694f, 417.321f);
// 38 and 30 yd out at bearing -82, the middle of the gap between the two star spawns on that side.
const Position ULDUAR_ALGALON_TANK_SLOT = Position(1637.957f, -340.396f, 417.321f);
const Position ULDUAR_ALGALON_HOLD_SPOT = Position(1636.843f, -332.474f, 417.321f);
// 4.1-7.2 yd from him wherever he stops between 27 and 32 yd out.
const Position ULDUAR_ALGALON_OFFTANK_SLOT = Position(1633.300f, -336.001f, 417.321f);

const std::array<Position, ULDUAR_ALGALON_TOTAL_SLOTS> ULDUAR_ALGALON_RING_SLOTS = {{
    Position(1617.869f, -333.468f, 417.321f),  // healer, 19 yd from the hold at +85 deg
    Position(1655.356f, -328.200f, 417.321f),  // healer, 19 yd from the hold at -85 deg
    Position(1619.354f, -339.898f, 417.321f),  // healer, 19 yd from the hold at +105 deg
    Position(1655.702f, -334.789f, 417.321f),  // healer, 19 yd from the hold at -105 deg
    Position(1612.390f, -327.276f, 417.321f),  // ranged, 25 yd from the hold at +70 deg
    Position(1658.917f, -320.737f, 417.321f),  // ranged, 25 yd from the hold at -70 deg
    Position(1611.877f, -333.782f, 417.321f),  // ranged, 25 yd from the hold at +85 deg
    Position(1661.202f, -326.850f, 417.321f),  // ranged, 25 yd from the hold at -85 deg
    Position(1613.067f, -340.199f, 417.321f),  // ranged, 25 yd from the hold at +100 deg
    Position(1661.828f, -333.346f, 417.321f),  // ranged, 25 yd from the hold at -100 deg
    Position(1613.806f, -311.731f, 417.321f),  // ranged, 31 yd from the hold at +40 deg
    Position(1653.271f, -306.184f, 417.321f),  // ranged, 31 yd from the hold at -40 deg
    Position(1609.222f, -318.400f, 417.321f),  // ranged, 31 yd from the hold at +55 deg
    Position(1659.515f, -311.332f, 417.321f),  // ranged, 31 yd from the hold at -55 deg
    Position(1606.521f, -326.028f, 417.321f),  // ranged, 31 yd from the hold at +70 deg
    Position(1664.215f, -317.920f, 417.321f),  // ranged, 31 yd from the hold at -70 deg
    Position(1605.886f, -334.096f, 417.321f),  // ranged, 31 yd from the hold at +85 deg
    Position(1667.049f, -325.500f, 417.321f),  // ranged, 31 yd from the hold at -85 deg
}};

const std::array<Position, ULDUAR_ALGALON_MELEE_STAGE_SPOTS> ULDUAR_ALGALON_MELEE_STAGE_SLOTS = {{
    Position(1621.852f, -331.950f, 417.321f),  // 15 yd from the hold at +80 deg
    Position(1651.109f, -327.838f, 417.321f),  // 15 yd from the hold at -80 deg
    Position(1621.989f, -334.561f, 417.321f),  // 15 yd from the hold at +90 deg
    Position(1651.697f, -330.386f, 417.321f),  // 15 yd from the hold at -90 deg
    Position(1622.577f, -337.109f, 417.321f),  // 15 yd from the hold at +100 deg
    Position(1651.834f, -332.997f, 417.321f),  // 15 yd from the hold at -100 deg
    Position(1623.599f, -339.516f, 417.321f),  // 15 yd from the hold at +110 deg
    Position(1651.515f, -335.592f, 417.321f),  // 15 yd from the hold at -110 deg
    Position(1623.447f, -342.940f, 417.321f),  // 17 yd from the hold at +120 deg
    Position(1652.605f, -338.842f, 417.321f),  // 17 yd from the hold at -120 deg
}};

static RaidInstanceState<AlgalonEncounterState> algalonEncounterStates;

namespace
{

Creature* FindAlgalon(Player* bot)
{
    if (!bot || bot->GetMapId() != ULDUAR_MAP_ID)
        return nullptr;

    InstanceScript* instance = bot->GetInstanceScript();
    if (!instance)
        return nullptr;

    // The guid lookup has no liveness filter, and a dead boss would hold every gate open through loot.
    Creature* algalon = instance->GetCreature(ULD_BOSS_ALGALON);
    return algalon && algalon->IsAlive() ? algalon : nullptr;
}

AlgalonEncounterState* FindState(Player* bot)
{
    if (!bot || bot->GetMapId() != ULDUAR_MAP_ID)
        return nullptr;

    return algalonEncounterStates.Find(bot->GetInstanceId());
}

Spell* CurrentBigBang(Unit* boss)
{
    if (!boss || !boss->HasUnitState(UNIT_STATE_CASTING))
        return nullptr;

    if (Spell* spell = boss->FindCurrentSpellBySpellId(SPELL_ALGALON_BIG_BANG))
        return spell;

    return boss->FindCurrentSpellBySpellId(SPELL_ALGALON_BIG_BANG_25);
}

Player* FindGroupMember(Group* group, ObjectGuid const& guid)
{
    if (!group || guid.IsEmpty())
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->GetGUID() == guid)
            return member;
    }

    return nullptr;
}

bool InFight(Player* member)
{
    return member && member->IsInWorld() && member->IsAlive() && member->GetMapId() == ULDUAR_MAP_ID;
}

// Alive and in the room's phase, which is all Big Bang and his evade check count.
bool CanStayOut(Player* member) { return InFight(member) && !IsAlgalonPhased(member); }

bool IsDamageDealer(Player* member) { return !PlayerbotAI::IsHeal(member) && !PlayerbotAI::IsTank(member); }

bool IsRangedDps(Player* member) { return PlayerbotAI::IsRanged(member) && IsDamageDealer(member); }

bool IsBot(Player* member) { return member && GET_PLAYERBOT_AI(member); }

// Nearest living, unphased raid member: who a new hole or an explosion lands on.
float NearestRaidMember(Group* group, Position const& spot)
{
    float nearest = std::numeric_limits<float>::max();
    if (!group)
        return nearest;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (CanStayOut(member))
            nearest = std::min(nearest, member->GetExactDist2d(spot));
    }

    return nearest;
}

float SegmentDistance(Position const& from, float toX, float toY, Position const& point)
{
    float const dx = toX - from.GetPositionX();
    float const dy = toY - from.GetPositionY();
    float const lengthSq = dx * dx + dy * dy;
    float t = 0.0f;
    if (lengthSq > 0.0f)
        t = ((point.GetPositionX() - from.GetPositionX()) * dx + (point.GetPositionY() - from.GetPositionY()) * dy) /
            lengthSq;

    t = std::clamp(t, 0.0f, 1.0f);
    return std::hypot(from.GetPositionX() + t * dx - point.GetPositionX(),
                      from.GetPositionY() + t * dy - point.GetPositionY());
}

bool BigBangDueWithin(AlgalonEncounterState const& state, uint32 seconds, uint32 now)
{
    if (!state.nextBigBangMs || state.bigBangCasting)
        return false;

    // Signed, so a cast running late reads as due rather than a full cycle away.
    int32 const left = static_cast<int32>(state.nextBigBangMs - now);
    return left <= static_cast<int32>(seconds * IN_MILLISECONDS);
}

// Kept while a Big Bang is close: the raid needs one standing when it lands.
uint8 ReservedHoles(AlgalonEncounterState const& state, uint32 now)
{
    return state.bigBangCasting || BigBangDueWithin(state, ULDUAR_ALGALON_SHELTER_WINDOW_SECONDS, now) ? 1 : 0;
}

AlgalonScanUnit const* FindScanned(std::vector<AlgalonScanUnit> const& units, ObjectGuid const& guid)
{
    for (AlgalonScanUnit const& unit : units)
        if (unit.guid == guid)
            return &unit;

    return nullptr;
}

Creature* Resolve(Player* bot, ObjectGuid const& guid)
{
    Creature* creature = guid.IsEmpty() ? nullptr : ObjectAccessor::GetCreature(*bot, guid);
    return creature && creature->IsAlive() ? creature : nullptr;
}

bool IsKiteHole(AlgalonEncounterState const& state, AlgalonScanUnit const& hole)
{
    for (auto const& kite : state.kiteHoleAssignments)
        if (kite.second == hole.guid)
            return true;

    return false;
}

// Nearest, but not a hole a constellation is being walked into unless it is the only one.
AlgalonScanUnit const* NearestShelter(AlgalonEncounterState const& state, Position const& from)
{
    constexpr float kiteHolePenalty = 1000.0f;
    AlgalonScanUnit const* nearest = nullptr;
    float bestDistance = std::numeric_limits<float>::max();
    for (AlgalonScanUnit const& hole : state.holes)
    {
        float const distance = from.GetExactDist2d(hole.position) + (IsKiteHole(state, hole) ? kiteHolePenalty : 0.0f);
        if (distance < bestDistance)
        {
            nearest = &hole;
            bestDistance = distance;
        }
    }

    return nearest;
}

// The hole a hider runs to this cast: the one latched for it, else the nearest shelter. A handler holding
// constellations takes the one it was walking them to, where nobody else hides, since they close it behind it.
AlgalonScanUnit const* ShelterHole(Player* bot, AlgalonEncounterState const& state)
{
    if (state.handler == bot->GetGUID() && AlgalonHoldsConstellation(bot))
    {
        auto const kite = state.kiteHoleAssignments.find(bot->GetGUID());
        if (kite != state.kiteHoleAssignments.end())
            if (AlgalonScanUnit const* hole = FindScanned(state.holes, kite->second))
                return hole;
    }

    auto const latched = state.shelterAssignments.find(bot->GetGUID());
    if (latched != state.shelterAssignments.end())
        if (AlgalonScanUnit const* hole = FindScanned(state.holes, latched->second))
            return hole;

    return NearestShelter(state, bot->GetPosition());
}

// The hole nearest the hold spot, where the ringed raid stands, while a Big Bang is due and another is left
// for the kite.
AlgalonScanUnit const* RaidShelterHole(AlgalonEncounterState const& state, uint32 now)
{
    if (!ReservedHoles(state, now) || state.holes.size() < 2)
        return nullptr;

    AlgalonScanUnit const* nearest = nullptr;
    float bestDistance = std::numeric_limits<float>::max();
    for (AlgalonScanUnit const& hole : state.holes)
    {
        float const distance = hole.position.GetExactDist2d(ULDUAR_ALGALON_HOLD_SPOT);
        if (distance < bestDistance)
        {
            nearest = &hole;
            bestDistance = distance;
        }
    }

    return nearest;
}

struct AlgalonRoomCreatureCheck
{
    bool operator()(Creature* creature) const
    {
        switch (creature->GetEntry())
        {
            case PB_NPC_COLLAPSING_STAR:
            case PB_NPC_BLACK_HOLE:
            case PB_NPC_WORM_HOLE:
            case PB_NPC_LIVING_CONSTELLATION:
            case NPC_ALGALON_ASTEROID_TARGET_1:
            case NPC_ALGALON_ASTEROID_TARGET_2:
            case PB_NPC_UNLEASHED_DARK_MATTER:
                break;
            default:
                return false;
        }

        return creature->IsAlive() &&
               creature->GetExactDist2d(ULDUAR_ALGALON_ROOM_CENTER.GetPositionX(),
                                        ULDUAR_ALGALON_ROOM_CENTER.GetPositionY()) <= ULDUAR_ALGALON_SCAN_RADIUS;
    }
};

// Appends to the star's health log and sets its loss rate from the slope. A short log reads as the drain.
void LogStarHealth(AlgalonEncounterState const& state, AlgalonScanUnit& star, uint32 now,
                   std::unordered_map<ObjectGuid, std::vector<std::pair<uint32, float>>>& logs)
{
    std::vector<std::pair<uint32, float>> log;
    auto const previous = state.starHealthLog.find(star.guid);
    if (previous != state.starHealthLog.end())
        log = previous->second;

    if (log.empty() || getMSTimeDiff(log.back().first, now) >= ULDUAR_ALGALON_STAR_LOG_STEP_MS)
        log.emplace_back(now, star.healthPct);

    while (log.size() > 1 && getMSTimeDiff(log.front().first, now) > ULDUAR_ALGALON_STAR_LOG_MS)
        log.erase(log.begin());

    uint32 const span = getMSTimeDiff(log.front().first, now);
    star.lossMeasured = span >= ULDUAR_ALGALON_STAR_LOG_MIN_MS;
    if (star.lossMeasured)
        star.lossPctPerSec =
            std::max(ULDUAR_ALGALON_STAR_DRAIN_PCT_PER_SEC,
                     (log.front().second - star.healthPct) * IN_MILLISECONDS / static_cast<float>(span));

    logs[star.guid] = std::move(log);
}

uint32 PendingPeriodicHp(Creature* star)
{
    uint64 pending = 0;
    for (AuraType const type :
         {SPELL_AURA_PERIODIC_DAMAGE, SPELL_AURA_PERIODIC_LEECH, SPELL_AURA_PERIODIC_DAMAGE_PERCENT})
    {
        for (AuraEffect const* effect : star->GetAuraEffectsByType(type))
        {
            int32 const left = effect->GetTotalTicks() - static_cast<int32>(effect->GetTickNumber());
            if (effect->GetId() == SPELL_ALGALON_COLLAPSE || left <= 0 || effect->GetAmount() <= 0)
                continue;

            uint32 const tick = type == SPELL_AURA_PERIODIC_DAMAGE_PERCENT
                                    ? star->CountPctFromMaxHealth(effect->GetAmount())
                                    : static_cast<uint32>(effect->GetAmount());
            pending += static_cast<uint64>(tick) * left;
        }
    }

    return static_cast<uint32>(std::min<uint64>(pending, star->GetHealth()));
}

void ScanRoom(Creature* boss, AlgalonEncounterState& state, uint32 now)
{
    std::vector<AlgalonScanUnit> const previousStars = std::move(state.stars);
    bool const hadWormHoles = state.wormHolesSeen;

    state.stars.clear();
    state.holes.clear();
    state.constellations.clear();
    state.markers.clear();
    state.darkMatter.clear();
    std::unordered_map<ObjectGuid, uint32> markerSeenMs;
    std::unordered_map<ObjectGuid, std::vector<std::pair<uint32, float>>> starHealthLog;

    // The searcher takes its phase mask from its first argument. His is the room's, while a bot's
    // turns to 16 inside a hole and would see nothing here.
    std::list<Creature*> found;
    AlgalonRoomCreatureCheck check;
    Acore::CreatureListSearcher<AlgalonRoomCreatureCheck> searcher(boss, found, check);
    Cell::VisitObjects(ULDUAR_ALGALON_ROOM_CENTER.GetPositionX(), ULDUAR_ALGALON_ROOM_CENTER.GetPositionY(),
                       boss->GetMap(), searcher, ULDUAR_ALGALON_SCAN_RADIUS);

    for (Creature* creature : found)
    {
        AlgalonScanUnit unit;
        unit.guid = creature->GetGUID();
        unit.position = creature->GetPosition();
        Unit* victim = creature->GetVictim();
        unit.victim = victim ? victim->GetGUID() : ObjectGuid::Empty;
        unit.healthPct = creature->GetHealthPct();
        unit.active = !creature->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE);

        switch (creature->GetEntry())
        {
            case PB_NPC_COLLAPSING_STAR:
            {
                unit.health = creature->GetHealth();
                unit.pendingHp = PendingPeriodicHp(creature);
                unit.tickHp = std::max<uint32>(1, creature->CountPctFromMaxHealth(1));
                if (AuraEffect const* drain = creature->GetAuraEffect(SPELL_ALGALON_COLLAPSE, EFFECT_0))
                    unit.nextTickMs = std::max(0, drain->GetPeriodicTimer());
                unit.under = state.underStars.count(unit.guid) > 0;
                LogStarHealth(state, unit, now, starHealthLog);
                state.stars.push_back(unit);
                break;
            }
            case PB_NPC_WORM_HOLE:
                state.wormHolesSeen = true;
                [[fallthrough]];
            case PB_NPC_BLACK_HOLE:
                state.holes.push_back(unit);
                break;
            case PB_NPC_LIVING_CONSTELLATION:
                // Evading leaves it selectable; its script's flag drops on the reset.
                unit.active = unit.active && creature->AI() && creature->AI()->GetData(0);
                state.constellations.push_back(unit);
                break;
            case PB_NPC_UNLEASHED_DARK_MATTER:
                state.darkMatter.push_back(unit);
                break;
            default:
            {
                auto const seen = state.markerSeenMs.find(unit.guid);
                if (seen == state.markerSeenMs.end())
                    state.nextSmashImpactMs =
                        now + ULDUAR_ALGALON_COSMIC_SMASH_IMPACT_MS + ULDUAR_ALGALON_COSMIC_SMASH_INTERVAL_MS;

                uint32 const seenMs = seen != state.markerSeenMs.end() ? seen->second : now;
                markerSeenMs[unit.guid] = seenMs;
                if (getMSTimeDiff(seenMs, now) <= ULDUAR_ALGALON_MARKER_LIFETIME_MS)
                    state.markers.push_back(unit);
                break;
            }
        }
    }

    state.markerSeenMs = std::move(markerSeenMs);
    state.starHealthLog = std::move(starHealthLog);

    bool const phaseTwoStarted = state.wormHolesSeen && !hadWormHoles;
    // By guid, not by count: a death in the same scan as the minute's refill leaves the count unchanged.
    auto const gone = [&state](AlgalonScanUnit const& star) { return !FindScanned(state.stars, star.guid); };
    bool const starDied = std::any_of(previousStars.begin(), previousStars.end(), gone);
    if (starDied && !phaseTwoStarted)
        state.lastStarDeathMs = now;
}

// Bots only: a human can't be handed a job.
Player* FindTracker(Group* group)
{
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (InFight(member) && GET_PLAYERBOT_AI(member))
            return member;
    }

    return nullptr;
}

std::vector<Player*> OrderedTanks(Player* bot)
{
    std::vector<Player*> tanks;
    auto add = [&tanks](Player* tank)
    {
        if (tank && std::find(tanks.begin(), tanks.end(), tank) == tanks.end())
            tanks.push_back(tank);
    };

    add(GetGroupMainTank(bot));
    for (uint8 index = 0; index < 3; ++index)
        add(GetGroupAssistTank(bot, index));

    return tanks;
}

// Two bots trade him on their own; with one, the human tank has to be told when to taunt.
std::vector<Player*> WantedSwapPair(std::vector<Player*> const& tanks)
{
    std::vector<Player*> bots;
    std::copy_if(tanks.begin(), tanks.end(), std::back_inserter(bots), IsBot);
    std::vector<Player*> pair = bots.size() >= 2 ? bots : tanks;
    if (pair.size() > 2)
        pair.resize(2);

    return pair;
}

// Re-derived until the pull, so a role change or a tank joining late still counts. During the pull a
// member who died or left is replaced, or the survivor eats the 5th Phase Punch with nobody to take him.
void EnsureSwapPair(Player* bot, Creature* boss, AlgalonEncounterState& state, std::vector<Player*> const& tanks)
{
    Group* group = bot->GetGroup();
    if (!group)
        return;

    bool const fighting = boss->IsInCombat();
    std::vector<ObjectGuid> leaving;
    for (ObjectGuid const& guid : state.swapPair)
    {
        Player* member = FindGroupMember(group, guid);
        if (!member || (fighting && !InFight(member)))
            leaving.push_back(guid);
    }

    std::vector<Player*> const wanted = WantedSwapPair(tanks);
    if (!fighting)
    {
        for (ObjectGuid const& guid : state.swapPair)
        {
            auto const same = [&guid](Player* tank) { return tank->GetGUID() == guid; };
            if (std::none_of(wanted.begin(), wanted.end(), same))
                leaving.push_back(guid);
        }
    }

    for (ObjectGuid const& guid : leaving)
        state.swapPair.erase(guid);

    // OrderedTanks returns living tanks only, so a replacement is someone who can take him. Bots first:
    // a human has to be called.
    std::vector<Player*> candidates = wanted;
    if (fighting)
    {
        candidates.clear();
        std::copy_if(tanks.begin(), tanks.end(), std::back_inserter(candidates), IsBot);
        std::copy_if(tanks.begin(), tanks.end(), std::back_inserter(candidates),
                     [](Player* tank) { return !IsBot(tank); });
    }

    for (Player* tank : candidates)
    {
        if (state.swapPair.size() >= 2)
            break;

        if (InFight(tank))
            state.swapPair.insert(tank->GetGUID());
    }
}

// Beacon and Sacred Shield cover both tanks from one healer, then shields, then hots.
uint8 TankHealerRank(Player* member)
{
    switch (member->getClass())
    {
        case CLASS_PALADIN:
            return 0;
        case CLASS_PRIEST:
            return AiFactory::GetPlayerSpecTab(member) == PRIEST_TAB_DISCIPLINE ? 1 : 4;
        case CLASS_DRUID:
            return 2;
        case CLASS_SHAMAN:
            return 3;
        default:
            return 5;
    }
}

bool BetterTankHealer(Player* a, Player* b)
{
    uint8 const aRank = TankHealerRank(a);
    uint8 const bRank = TankHealerRank(b);
    return aRank != bRank ? aRank < bRank : a->GetGUID() < b->GetGUID();
}

// Humans count towards the raid's healers but are never handed the tanks.
uint8 TankHealerSlots(Player* bot, Group* group)
{
    uint8 healers = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (InFight(member) && PlayerbotAI::IsHeal(member))
            ++healers;
    }

    uint8 const wanted =
        bot->GetMap()->Is25ManRaid() ? ULDUAR_ALGALON_TANK_HEALERS_25 : ULDUAR_ALGALON_TANK_HEALERS_10;
    return std::min<uint8>(wanted, healers / 2);
}

// Re-derived until the pull like the pair. During it a dead or departed healer is replaced and the list
// trimmed to what the living healers allow. A phased one stays: Big Bang phases them all at once.
void EnsureTankHealers(Player* bot, Creature* boss, AlgalonEncounterState& state)
{
    Group* group = bot->GetGroup();
    if (!group)
        return;

    uint8 const slots = TankHealerSlots(bot, group);
    state.tankHealerSlots = slots;

    std::vector<Player*> candidates;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (InFight(member) && IsBot(member) && PlayerbotAI::IsHeal(member))
            candidates.push_back(member);
    }

    std::sort(candidates.begin(), candidates.end(), BetterTankHealer);
    if (!boss->IsInCombat() && candidates.size() > slots)
        candidates.resize(slots);

    std::vector<ObjectGuid> leaving;
    for (ObjectGuid const& guid : state.tankHealers)
    {
        auto const same = [&guid](Player* healer) { return healer->GetGUID() == guid; };
        if (std::none_of(candidates.begin(), candidates.end(), same))
            leaving.push_back(guid);
    }

    for (ObjectGuid const& guid : leaving)
        state.tankHealers.erase(guid);

    for (Player* candidate : candidates)
    {
        if (state.tankHealers.size() >= slots)
            break;

        state.tankHealers.insert(candidate->GetGUID());
    }

    while (state.tankHealers.size() > slots)
    {
        Player* worst = nullptr;
        for (ObjectGuid const& guid : state.tankHealers)
        {
            Player* healer = FindGroupMember(group, guid);
            if (healer && (!worst || BetterTankHealer(worst, healer)))
                worst = healer;
        }

        if (!worst)
            break;

        state.tankHealers.erase(worst->GetGUID());
    }
}

// CanCastSpell lets an out of range cast through.
bool TankHealReachable(Player* bot, Player* target)
{
    return CanStayOut(target) && bot->GetDistance2d(target) <= sPlayerbotAIConfig.HealDistance &&
           bot->IsWithinLOSInMap(target);
}

uint32 CastTimeMs(PlayerbotAI* botAI, char const* spell)
{
    uint32 const spellId = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", spell)->Get();
    SpellInfo const* info = spellId ? sSpellMgr->GetSpellInfo(spellId) : nullptr;
    return info ? info->CalcCastTime(botAI->GetBot()) : 0;
}

Player* PairPartner(Group* group, AlgalonEncounterState const& state, Player* tank)
{
    for (ObjectGuid const& guid : state.swapPair)
        if (guid != tank->GetGUID())
            return FindGroupMember(group, guid);

    return nullptr;
}

// The holder has nothing physical left for the coming hit and the partner has; Divine Protection cools 2-3 min
// against a 90.5 s cast. Bots only, and a tank soak only: with a disperser latched the holder hides.
bool DefensiveHandoverDue(AlgalonEncounterState const& state, Player* holder, Player* taker, uint32 now)
{
    if (!state.disperser.IsEmpty() || !IsBot(holder) || !IsBot(taker) ||
        GetAlgalonPhasePunchStacks(taker) > ULDUAR_ALGALON_HANDOVER_PARTNER_STACKS ||
        !BigBangDueWithin(state, ULDUAR_ALGALON_EARLY_SWAP_SECONDS, now))
        return false;

    // The soak defensive goes up with ULDUAR_ALGALON_SOAK_DEFENSIVE_MS of the cast left.
    int32 const untilNeeded = static_cast<int32>(state.nextBigBangMs - now) +
                              static_cast<int32>(ULDUAR_ALGALON_BIG_BANG_CAST_MS) - ULDUAR_ALGALON_SOAK_DEFENSIVE_MS;
    uint32 const within = static_cast<uint32>(std::max(0, untilNeeded));
    return !ReadyTankDefensive(holder, true, within) && ReadyTankDefensive(taker, true, within);
}

// The 4th stack, or when a tank soaks, a holder deep in stacks or out of defensives with Big Bang close: the
// Phase Punch behind the cast hits the soaker while the raid is phased; a hiding holder takes it with the raid back.
bool SwapDue(AlgalonEncounterState const& state, Player* holder, Player* taker, uint32 now)
{
    uint8 const own = GetAlgalonPhasePunchStacks(taker);
    uint8 const theirs = GetAlgalonPhasePunchStacks(holder);
    if (own >= ULDUAR_ALGALON_PHASE_PUNCH_SWAP_STACKS)
        return false;

    if (theirs >= ULDUAR_ALGALON_PHASE_PUNCH_SWAP_STACKS || DefensiveHandoverDue(state, holder, taker, now))
        return true;

    return state.disperser.IsEmpty() && theirs >= ULDUAR_ALGALON_EARLY_SWAP_STACKS &&
           own <= ULDUAR_ALGALON_EARLY_SWAP_PARTNER_STACKS &&
           BigBangDueWithin(state, ULDUAR_ALGALON_EARLY_SWAP_SECONDS, now);
}

// Raid warning to the humans only: bots read the encounter state, not chat.
void SendAlgalonCall(Player* sender, std::string const& text)
{
    PlayerbotAI* senderAI = GET_PLAYERBOT_AI(sender);
    if (!senderAI)
        return;

    std::vector<Player*> const humans = senderAI->GetRealPlayersInGroup();
    if (humans.empty())
        return;

    WorldPacket data;
    ChatHandler::BuildChatPacket(data, CHAT_MSG_RAID_WARNING, LANG_UNIVERSAL, sender->GetGUID(), {}, text,
                                 CHAT_TAG_NONE, sender->GetName());
    for (Player* human : humans)
        ServerFacade::instance().SendPacket(human, &data);
}

Player* BossHolder(Creature* boss)
{
    Unit* victim = boss->GetVictim();
    return victim ? victim->ToPlayer() : nullptr;
}

// The swap and the pickup fire on the tank that takes him, and a human has no trigger to fire them.
void CallHumanTank(Group* group, PlayerbotAI* botAI, Creature* boss, AlgalonEncounterState& state, uint32 now)
{
    if (state.bigBangCasting ||
        (state.lastTankCallMs && getMSTimeDiff(state.lastTankCallMs, now) < ULDUAR_ALGALON_TANK_CALL_MS))
        return;

    Player* holder = BossHolder(boss);
    if (!holder || !state.swapPair.count(holder->GetGUID()))
    {
        Player* pickup = GetAlgalonPickupTank(botAI);
        Player* sender = FindTracker(group);
        if (!pickup || IsBot(pickup) || !sender)
            return;

        std::string const on = holder ? " he is on " + holder->GetName() : " he is loose";
        SendAlgalonCall(sender, pickup->GetName() + ", taunt Algalon now:" + on);
        state.lastTankCallMs = now;
        return;
    }

    if (!IsBot(holder) || !CanStayOut(holder))
        return;

    Player* partner = PairPartner(group, state, holder);
    if (!partner || IsBot(partner) || !CanStayOut(partner) || !SwapDue(state, holder, partner, now))
        return;

    SendAlgalonCall(holder, partner->GetName() + ", taunt Algalon now: " + holder->GetName() + " has " +
                                std::to_string(GetAlgalonPhasePunchStacks(holder)) + " Phase Punch stacks");
    state.lastTankCallMs = now;
}

// Two calls, because a human sitting in the void for the whole 8 s cast is fed to the phase-16 Dark
// Matter, and one stepping out early lets the 10 s phase lapse before the hit.
void CallBigBang(Group* group, Creature* boss, AlgalonEncounterState& state, bool castStarted)
{
    Player* tracker = FindTracker(group);
    if (!tracker)
        return;

    Player* soaker = FindGroupMember(group, state.bigBangSoaker);
    if (castStarted)
    {
        state.bigBangHideCalled = false;
        char const* const kit =
            soaker && soaker->HasSpell(SPELL_ALGALON_DISPERSION) ? " with Dispersion" : " with a defensive";
        std::string stays =
            soaker ? soaker->GetName() + " stays out" + kit + ". Everyone else" : std::string("Everyone");
        // A human holder handed the soak is told outright, or it stays out and takes him back.
        Player* holder = BossHolder(boss);
        if (soaker && holder && holder != soaker && !IsBot(holder) && state.swapPair.count(holder->GetGUID()))
            stays = holder->GetName() + ", hide too: " + soaker->GetName() + " takes it. Everyone else";

        SendAlgalonCall(tracker, "Big Bang in 8 s: " + stays + ", get next to a black hole");
        return;
    }

    Spell* cast = CurrentBigBang(boss);
    if (state.bigBangHideCalled || !cast || cast->GetCastTimeRemaining() > ULDUAR_ALGALON_HIDE_CALL_MS)
        return;

    state.bigBangHideCalled = true;
    std::string const who = soaker ? "everyone but " + soaker->GetName() + " into" : std::string("into");
    SendAlgalonCall(tracker, "Big Bang: " + who + " a black hole NOW and stay in it until it lands");
}

// Not onto a loose boss, whose next swing lands on whoever is nearest, unless nobody ever picks him up.
void ReleaseMelee(Creature* boss, AlgalonEncounterState& state, uint32 now)
{
    if (state.meleeReleased || !state.introEndMs)
        return;

    uint32 const sinceIntro = getMSTimeDiff(state.introEndMs, now);
    Player* holder = BossHolder(boss);
    bool const held = holder && state.swapPair.count(holder->GetGUID());
    bool const onSpot =
        held && holder->GetExactDist2d(ULDUAR_ALGALON_TANK_SLOT) <= ULDUAR_ALGALON_HOLDER_ON_SPOT_RADIUS;
    if (onSpot || (held && sinceIntro >= ULDUAR_ALGALON_MELEE_RELEASE_MS) ||
        sinceIntro >= ULDUAR_ALGALON_MELEE_RELEASE_ANYWAY_MS)
        state.meleeReleased = true;
}

// A bot whose Dispersion is back by the time the soak casts it, `castByMs` from now. Its 90% leaves a priest
// about 6k of the hit, so the tanks hide and keep their buttons for his swings.
Player* ElectDisperser(Group* group, int32 castByMs, Player* except)
{
    Player* best = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member == except || !CanStayOut(member) || !IsBot(member) || !member->HasSpell(SPELL_ALGALON_DISPERSION) ||
            static_cast<int64>(member->GetSpellCooldownDelay(SPELL_ALGALON_DISPERSION)) > castByMs)
            continue;

        if (!best || member->GetGUID() < best->GetGUID())
            best = member;
    }

    return best;
}

Player* ElectBackup(Group* group, Player* soaker)
{
    Player* best = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member == soaker || !CanStayOut(member) || !GET_PLAYERBOT_AI(member) || !AlgalonCanDisperse(member))
            continue;

        if (!best || member->GetGUID() < best->GetGUID())
            best = member;
    }

    return best;
}

// A physical button for a hit this far off: back off cooldown in time, or already up and lasting past it.
bool SoakCovered(Player* tank, uint32 hitMs)
{
    uint32 const castBy = hitMs > static_cast<uint32>(ULDUAR_ALGALON_SOAK_DEFENSIVE_MS)
                              ? hitMs - static_cast<uint32>(ULDUAR_ALGALON_SOAK_DEFENSIVE_MS)
                              : 0;
    return ReadyTankDefensive(tank, true, castBy) || RunningTankDefensive(tank, true, hitMs);
}

// A tank handed the soak mid run keeps going on its forced booking while its walk back waits behind it,
// and he may turn to it by threat, so no taunt action ever runs to stop it.
void StopRunIntoHole(Player* tank, AlgalonEncounterState const& state)
{
    PlayerbotAI* tankAI = GET_PLAYERBOT_AI(tank);
    if (!tankAI)
        return;

    LastMovement& last = tankAI->GetAiObjectContext()->GetValue<LastMovement&>("last movement")->Get();
    if (!last.msTime)
        return;

    for (AlgalonScanUnit const& hole : state.holes)
    {
        if (hole.position.GetExactDist2d(last.lastMoveToX, last.lastMoveToY) <= ULDUAR_ALGALON_SHELTER_RADIUS)
        {
            tank->StopMoving();
            last.clear();
            return;
        }
    }
}

// The bot swap tank other than `tank`, able to stay out and take the Phase Punch behind the hit.
Player* SoakingPartner(Group* group, AlgalonEncounterState const& state, Player* tank, uint8 maxStacks)
{
    for (ObjectGuid const& guid : state.swapPair)
    {
        Player* partner = FindGroupMember(group, guid);
        if (partner && partner != tank && IsBot(partner) && CanStayOut(partner) &&
            GetAlgalonPhasePunchStacks(partner) <= maxStacks)
            return partner;
    }

    return nullptr;
}

void ElectBigBangSoakers(Player* bot, Creature* boss, AlgalonEncounterState& state)
{
    Group* group = bot->GetGroup();
    if (!group)
        return;

    Spell* cast = CurrentBigBang(boss);
    int32 const remaining =
        cast ? static_cast<int32>(cast->GetCastTimeRemaining()) : static_cast<int32>(ULDUAR_ALGALON_BIG_BANG_CAST_MS);
    int32 const castBy = remaining - ULDUAR_ALGALON_SOAK_DISPERSION_MS;
    if (Player* disperser = ElectDisperser(group, castBy, nullptr))
    {
        Player* backup = ElectDisperser(group, castBy, disperser);
        state.bigBangSoaker = disperser->GetGUID();
        state.bigBangBackup = backup ? backup->GetGUID() : ObjectGuid::Empty;
        return;
    }

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    Unit* victim = boss->GetVictim();
    Player* holder = victim ? victim->ToPlayer() : nullptr;

    Player* soaker = nullptr;
    if (CanStayOut(holder) && IsAlgalonSwapTank(holder))
    {
        soaker = holder;
        // A human with nothing for the hit hides or dies; a bot partner with a button takes it instead.
        if (!IsBot(holder) && !SoakCovered(holder, ULDUAR_ALGALON_BIG_BANG_CAST_MS))
        {
            Player* partner = SoakingPartner(group, state, holder, ULDUAR_ALGALON_HANDOVER_PARTNER_STACKS);
            if (partner && SoakCovered(partner, ULDUAR_ALGALON_BIG_BANG_CAST_MS))
                soaker = partner;
        }
    }

    if (!soaker && botAI)
        soaker = GetAlgalonPickupTank(botAI);

    // Nobody who should have him does, but whoever he is on still keeps him from evading.
    if (!soaker && CanStayOut(holder))
        soaker = holder;

    Player* backup = ElectBackup(group, soaker);
    state.bigBangSoaker = soaker ? soaker->GetGUID() : ObjectGuid::Empty;
    state.bigBangBackup = backup ? backup->GetGUID() : ObjectGuid::Empty;
}

// A soaker lost mid cast is replaced at once, or he resets. After a tank soaker the bot swap tank goes first, since
// he stays on a tank once the raid is back; after a priest the backup, already out, so no tank stops mid run.
void KeepBigBangSoaker(Player* bot, AlgalonEncounterState& state)
{
    Group* group = bot->GetGroup();
    if (!group)
        return;

    Player* soaker = FindGroupMember(group, state.bigBangSoaker);
    Player* backup = FindGroupMember(group, state.bigBangBackup);
    if (!CanStayOut(soaker))
    {
        bool const priestLost =
            soaker ? soaker->HasSpell(SPELL_ALGALON_DISPERSION) : !state.disperser.IsEmpty();
        bool const backupFirst = priestLost && CanStayOut(backup);
        // One stack under the swap: the Phase Punch behind the hit takes it to the swap, not past it.
        if (Player* partner = backupFirst ? nullptr
                                          : SoakingPartner(group, state, soaker,
                                                           ULDUAR_ALGALON_PHASE_PUNCH_SWAP_STACKS - 1))
        {
            soaker = partner;
            state.bigBangSoaker = soaker->GetGUID();
            StopRunIntoHole(partner, state);
        }
        else if (CanStayOut(backup))
        {
            soaker = backup;
            backup = nullptr;
            state.bigBangSoaker = soaker->GetGUID();
        }
    }

    // Also retried every tick, so a priest whose Dispersion comes off cooldown mid cast still joins.
    if (!CanStayOut(backup))
    {
        backup = ElectBackup(group, soaker);
        state.bigBangBackup = backup ? backup->GetGUID() : ObjectGuid::Empty;
    }
}

// A bot tank outside the swap pair, else the pair bot not keeping him.
Player* ElectHandler(Player* bot, AlgalonEncounterState const& state, std::vector<Player*> const& tanks)
{
    for (Player* tank : tanks)
        if (InFight(tank) && IsBot(tank) && !state.swapPair.count(tank->GetGUID()))
            return tank;

    Group* group = bot->GetGroup();
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!group || !botAI)
        return nullptr;

    Player* holder = GetAlgalonBossTank(botAI);
    Player* keeper = holder && state.swapPair.count(holder->GetGUID()) ? holder : GetAlgalonPickupTank(botAI);
    for (ObjectGuid const& guid : state.swapPair)
    {
        Player* tank = FindGroupMember(group, guid);
        if (tank != keeper && InFight(tank) && IsBot(tank))
            return tank;
    }

    return nullptr;
}

void ElectStarTeam(Player* bot, AlgalonEncounterState& state)
{
    Group* group = bot->GetGroup();
    if (!group)
        return;

    std::vector<ObjectGuid> leaving;
    for (ObjectGuid const& guid : state.starTeam)
    {
        Player* member = FindGroupMember(group, guid);
        if (!InFight(member) || !IsDamageDealer(member))
            leaving.push_back(guid);
    }

    for (ObjectGuid const& guid : leaving)
        state.starTeam.erase(guid);

    uint8 const size =
        bot->GetMap()->Is25ManRaid() ? ULDUAR_ALGALON_STAR_TEAM_25 : ULDUAR_ALGALON_STAR_TEAM_10;
    if (state.starTeam.size() >= size)
        return;

    std::vector<Player*> candidates;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (InFight(member) && IsDamageDealer(member) && GET_PLAYERBOT_AI(member) &&
            !state.starTeam.count(member->GetGUID()))
            candidates.push_back(member);
    }

    // Ranged first, since a star wanders; melee only fill a raid short on ranged, or no star ever dies.
    std::sort(candidates.begin(), candidates.end(),
              [](Player* a, Player* b)
              {
                  bool const aRanged = IsRangedDps(a);
                  bool const bRanged = IsRangedDps(b);
                  return aRanged != bRanged ? aRanged : a->GetGUID() < b->GetGUID();
              });

    for (Player* candidate : candidates)
    {
        if (state.starTeam.size() >= size)
            break;

        state.starTeam.insert(candidate->GetGUID());
    }
}

// The hole lands where the star dies. Under a swap tank or the handler it phases them and sets him or
// their adds loose; inside his melee ring (`bossRing`) it phases the melee.
bool StarClearOfTanks(AlgalonEncounterState const& state, Group* group, Unit* boss, AlgalonScanUnit const& star,
                      bool bossRing)
{
    if (bossRing && boss && boss->GetExactDist2d(star.position) < ULDUAR_ALGALON_STAR_BOSS_CLEARANCE)
        return false;

    for (ObjectGuid const& guid : state.swapPair)
    {
        Player* tank = FindGroupMember(group, guid);
        if (CanStayOut(tank) && tank->GetExactDist2d(star.position) < ULDUAR_ALGALON_STAR_TANK_CLEARANCE)
            return false;
    }

    Player* handler = FindGroupMember(group, state.handler);
    return !CanStayOut(handler) || handler->GetExactDist2d(star.position) >= ULDUAR_ALGALON_STAR_TANK_CLEARANCE;
}

enum class StarFilter
{
    Any,
    Tanks,
    TanksAndRing,
    // Also clear of every raid member.
    Clear,
};

bool StarPassesFilter(AlgalonEncounterState const& state, Group* group, Unit* boss, AlgalonScanUnit const& star,
                      StarFilter filter)
{
    if (filter == StarFilter::Clear && NearestRaidMember(group, star.position) < ULDUAR_ALGALON_STAR_CLEAR_RADIUS)
        return false;

    return filter == StarFilter::Any || StarClearOfTanks(state, group, boss, star, filter != StarFilter::Tanks);
}

// For `urgent`, which wants a hole as fast as possible, wherever it lands.
AlgalonScanUnit const* LowestStar(AlgalonEncounterState const& state)
{
    AlgalonScanUnit const* lowest = nullptr;
    // One planned under the cast only when nothing else is left: it dies there anyway.
    for (AlgalonScanUnit const& star : state.stars)
        if (!lowest || lowest->under > star.under ||
            (lowest->under == star.under && star.healthPct < lowest->healthPct))
            lowest = &star;

    return lowest;
}

// How long the team needs to walk into range of a star from where it stands.
float TeamWalkSeconds(AlgalonEncounterState const& state, Group* group, AlgalonScanUnit const& star)
{
    float total = 0.0f;
    uint32 count = 0;
    for (ObjectGuid const& guid : state.starTeam)
    {
        Player* member = FindGroupMember(group, guid);
        if (!CanStayOut(member))
            continue;

        total += member->GetExactDist2d(star.position);
        ++count;
    }

    if (!count)
        return 0.0f;

    float const mean = total / static_cast<float>(count);
    return std::max(0.0f, mean - ULDUAR_ALGALON_STAR_TEAM_REACH) / ULDUAR_ALGALON_RUN_SPEED;
}

// The planner works in ms from now.
struct StarSpan
{
    int32 from;
    int32 to;
};

int32 MsUntil(uint32 when, uint32 now) { return static_cast<int32>(when - now); }

// The drain's death of a star, in ms from its scan: the next tick plus a second per tick of health left.
int32 DrainDeathMs(AlgalonScanUnit const& star)
{
    uint32 const ticks = (star.health + star.tickHp - 1) / star.tickHp;
    return star.nextTickMs + (static_cast<int32>(ticks) - 1) * static_cast<int32>(IN_MILLISECONDS);
}

int32 StarDeathAt(AlgalonScanUnit const& star, float lossPctPerSec)
{
    return static_cast<int32>(star.healthPct / lossPctPerSec * IN_MILLISECONDS);
}

struct StarBlackout
{
    StarSpan span;
    char const* label;
};

struct MergedBlackout
{
    StarSpan span;
    std::string label;
};

struct StarPlan
{
    // An explosion in one of these lands on a raid that can't take it.
    std::vector<StarBlackout> blackouts;
    // The same time with overlaps joined, for sums: three meteors are six markers.
    std::vector<MergedBlackout> merged;
    bool hadDeath = false;
    int32 lastDeath = 0;
    // Each star's death at its measured loss, the team's damage included for the one it is on.
    std::vector<std::pair<AlgalonScanUnit const*, int32>> deaths;
};

StarPlan PlanStars(AlgalonEncounterState const& state, uint32 now)
{
    StarPlan plan;

    // nextBigBangMs moves on to the next cycle the moment a cast starts, so the last one's tail comes from
    // its end.
    if (state.lastBigBangEndMs)
    {
        int32 const tail =
            MsUntil(state.lastBigBangEndMs, now) + static_cast<int32>(ULDUAR_ALGALON_STAR_BIG_BANG_TAIL_MS);
        if (tail > 0)
            plan.blackouts.push_back({{0, tail}, "prebang"});
    }

    if (state.nextBigBangMs && !state.bigBangCasting)
    {
        int32 const cast = MsUntil(state.nextBigBangMs, now);
        plan.blackouts.push_back({{cast - static_cast<int32>(ULDUAR_ALGALON_STAR_BIG_BANG_LEAD_MS),
                                   cast + static_cast<int32>(ULDUAR_ALGALON_BIG_BANG_CAST_MS +
                                                             ULDUAR_ALGALON_STAR_BIG_BANG_TAIL_MS)},
                                  "prebang"});
    }

    // Not `markers`: they drop out 5.5 s after the sighting, before the impact window ends.
    for (auto const& [marker, seenMs] : state.markerSeenMs)
    {
        int32 const impact = MsUntil(seenMs + ULDUAR_ALGALON_COSMIC_SMASH_IMPACT_MS, now);
        if (impact + static_cast<int32>(ULDUAR_ALGALON_STAR_IMPACT_TAIL_MS) < 0)
            continue;

        plan.blackouts.push_back({{impact - static_cast<int32>(ULDUAR_ALGALON_STAR_IMPACT_LEAD_MS),
                                   impact + static_cast<int32>(ULDUAR_ALGALON_STAR_IMPACT_TAIL_MS)},
                                  "smash"});
    }

    // Off the clock too: a marker shows 4.8 s ahead, too late to plan a kill round it.
    if (state.nextSmashImpactMs)
    {
        for (uint8 i = 0; i < ULDUAR_ALGALON_SMASH_FORECAST; ++i)
        {
            int32 const impact = MsUntil(state.nextSmashImpactMs, now) +
                                 i * static_cast<int32>(ULDUAR_ALGALON_COSMIC_SMASH_INTERVAL_MS);
            plan.blackouts.push_back({{impact - static_cast<int32>(ULDUAR_ALGALON_STAR_IMPACT_LEAD_MS),
                                       impact + static_cast<int32>(ULDUAR_ALGALON_STAR_IMPACT_TAIL_MS)},
                                      "smash"});
        }
    }

    std::vector<StarBlackout> sorted = plan.blackouts;
    std::sort(sorted.begin(), sorted.end(),
              [](StarBlackout const& a, StarBlackout const& b) { return a.span.from < b.span.from; });
    for (StarBlackout const& blackout : sorted)
    {
        if (blackout.span.to < 0)
            continue;

        if (plan.merged.empty() || blackout.span.from > plan.merged.back().span.to)
        {
            plan.merged.push_back({blackout.span, blackout.label});
            continue;
        }

        MergedBlackout& last = plan.merged.back();
        last.span.to = std::max(last.span.to, blackout.span.to);
        if (last.label.find(blackout.label) == std::string::npos)
            last.label += std::string("+") + blackout.label;
    }

    plan.hadDeath = state.lastStarDeathMs != 0;
    plan.lastDeath = plan.hadDeath ? MsUntil(state.lastStarDeathMs, now) : 0;
    // Under-stars die inside the cast's blackout, on the plan of their own.
    for (AlgalonScanUnit const& star : state.stars)
        if (!star.under)
            plan.deaths.emplace_back(&star, StarDeathAt(star, star.lossPctPerSec));

    return plan;
}

int32 ForecastDeath(StarPlan const& plan, AlgalonScanUnit const* star)
{
    for (auto const& [candidate, death] : plan.deaths)
        if (candidate == star)
            return death;

    return 0;
}

// Why an explosion somewhere in `when` would land badly, or nullptr. A span, because a kill's finish is only
// known to within the team's speed.
char const* StarCollision(StarPlan const& plan, StarSpan when, AlgalonScanUnit const* self, int32 separation)
{
    for (StarBlackout const& blackout : plan.blackouts)
        if (when.from <= blackout.span.to && when.to >= blackout.span.from)
            return blackout.label;

    if (plan.hadDeath && when.from < plan.lastDeath + separation)
        return "gap";

    for (auto const& [star, death] : plan.deaths)
        if (star != self && when.from < death + separation && when.to > death - separation)
            return "spacing";

    return nullptr;
}

// How far `when` stays from every other explosion, or -1 inside a blackout.
int32 StarSeparation(StarPlan const& plan, StarSpan when, AlgalonScanUnit const* self)
{
    for (StarBlackout const& blackout : plan.blackouts)
        if (when.from <= blackout.span.to && when.to >= blackout.span.from)
            return -1;

    auto const away = [&when](int32 death)
    { return death < when.from ? when.from - death : (death > when.to ? death - when.to : 0); };

    int32 nearest = std::numeric_limits<int32>::max();
    if (plan.hadDeath)
        nearest = away(plan.lastDeath);

    for (auto const& [star, death] : plan.deaths)
        if (star != self)
            nearest = std::min(nearest, away(death));

    return nearest;
}

// When the team's kill would land: walk, start, then speed; the focus's own once hit. Others go up to the team's
// last speed, or one it just left reads clear and is reopened; a star already losing kill speed pays no start.
StarSpan FinishBand(AlgalonEncounterState const& state, Group* group, AlgalonScanUnit const& star)
{
    int32 const walk = static_cast<int32>(TeamWalkSeconds(state, group, star) * IN_MILLISECONDS);
    bool const hit = star.lossMeasured && star.lossPctPerSec >= ULDUAR_ALGALON_STAR_KILL_MEASURED_PCT_PER_SEC;
    if (hit && star.guid == state.focusStar)
    {
        int32 const finish = walk + StarDeathAt(star, star.lossPctPerSec);
        return {finish, finish};
    }

    int32 const start = walk + (hit ? 0 : static_cast<int32>(ULDUAR_ALGALON_STAR_KILL_START_MS));
    float const fast = std::max(ULDUAR_ALGALON_STAR_KILL_FAST_PCT_PER_SEC, state.teamKillPctPerSec);
    return {start + StarDeathAt(star, fast), start + StarDeathAt(star, ULDUAR_ALGALON_STAR_KILL_SLOW_PCT_PER_SEC)};
}

// Spacing i explosions takes i gaps plus the kills, plus the walk and every blackout on the way, so the i-th
// soonest death sets a deadline. Stars of one wave spawn and drain together.
bool SerialDue(StarPlan const& plan, AlgalonEncounterState const& state, Group* group, AlgalonScanUnit const& star)
{
    int32 const death = ForecastDeath(plan, &star);
    uint32 index = 0;
    for (auto const& [other, otherDeath] : plan.deaths)
        if (other != &star && (otherDeath < death || (otherDeath == death && other < &star)))
            ++index;

    int32 budget = static_cast<int32>((index + 1) * ULDUAR_ALGALON_STAR_SERIAL_SECONDS * IN_MILLISECONDS) +
                   static_cast<int32>(TeamWalkSeconds(state, group, star) * IN_MILLISECONDS);
    for (MergedBlackout const& blackout : plan.merged)
        budget += std::max(0, std::min(death, blackout.span.to) - std::max(0, blackout.span.from));

    return death <= budget;
}

// Raid wide and unavoidable: everyone it hits must survive its top roll, or be at full when their max is below it.
template <typename Hit>
uint32 RaidUnderExplosionLine(Player* bot, Group* group, Hit hit)
{
    if (!group)
        return 0;

    uint32 const blast =
        bot->GetMap()->Is25ManRaid() ? ULDUAR_ALGALON_STAR_EXPLOSION_MAX_25 : ULDUAR_ALGALON_STAR_EXPLOSION_MAX_10;
    uint32 under = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (hit(member) && member->GetHealth() < std::min(blast, member->GetMaxHealth()))
            ++under;
    }

    return under;
}

// Phased members too: they are back long before a kill lands, and left out, the gate opened 1.6 s after a Big Bang
// hit with the raid still drained by the void.
bool RaidAffordsExplosion(Player* bot, Group* group) { return !RaidUnderExplosionLine(bot, group, InFight); }

// The star the team may kill now, or nullptr with the reason it may not. Every death is planned against the
// others' forecasts, Big Bang and the meteors.
char const* StarWindow(Player* bot, Unit* boss, AlgalonEncounterState const& state, uint32 now,
                       AlgalonScanUnit const*& star)
{
    star = nullptr;
    Group* group = bot->GetGroup();
    if (state.stars.empty())
        return "none";

    // No hole and a Big Bang coming: the one this star leaves behind is worth more than its damage.
    if (state.urgent)
    {
        star = LowestStar(state);
        return "urgent";
    }

    if (state.bigBangCasting)
        return "bigbang";

    StarPlan const plan = PlanStars(state, now);
    int32 const gap = static_cast<int32>(ULDUAR_ALGALON_STAR_PACING_GAP_MS);

    // Ahead of the gap, or any other death drops a kill half done. Dropped, a star keeps dying at its own slope
    // up to the cap, so a drop only goes when that dies clear or sooner than the kill lands.
    AlgalonScanUnit const* focus = FindScanned(state.stars, state.focusStar);
    if (focus && !focus->under)
    {
        bool const placed =
            state.focusByDeadline
                ? StarClearOfTanks(state, group, boss, *focus, false)
                : NearestRaidMember(group, focus->position) >= ULDUAR_ALGALON_STAR_CLEAR_RADIUS - 2.0f &&
                      StarClearOfTanks(state, group, boss, *focus, true);
        if (placed)
        {
            int32 const keep = static_cast<int32>(ULDUAR_ALGALON_STAR_KEEP_GAP_MS);
            StarSpan const finish = FinishBand(state, group, *focus);
            int32 const dropped =
                StarDeathAt(*focus, std::min(focus->lossPctPerSec, ULDUAR_ALGALON_STAR_DROPPED_PCT_PER_SEC));
            if (!StarCollision(plan, finish, focus, keep) ||
                (StarCollision(plan, {dropped, dropped}, focus, keep) && finish.to <= dropped))
            {
                star = focus;
                return state.focusByDeadline ? "deadline" : "open";
            }
        }
    }

    if (state.lastStarDeathMs && getMSTimeDiff(state.lastStarDeathMs, now) < ULDUAR_ALGALON_STAR_PACING_GAP_MS)
        return "gap";

    // Due: its own death lands badly, or it dies no later than a star the serial deadline makes due, since in a
    // tied wave only the last rank would fire. Waiting on raid health or clear ground lets two go off at once.
    int32 serialLine = std::numeric_limits<int32>::min();
    for (auto const& [candidate, death] : plan.deaths)
        if (SerialDue(plan, state, group, *candidate))
            serialLine = std::max(serialLine, death);

    std::vector<AlgalonScanUnit const*> due;
    for (auto const& [candidate, death] : plan.deaths)
    {
        bool const collides = death <= static_cast<int32>(ULDUAR_ALGALON_STAR_DUE_HORIZON_MS) &&
                              StarCollision(plan, {death, death}, candidate, gap);
        if (collides || death <= serialLine)
            due.push_back(candidate);
    }

    std::sort(due.begin(), due.end(), [&plan](AlgalonScanUnit const* a, AlgalonScanUnit const* b)
              { return ForecastDeath(plan, a) < ForecastDeath(plan, b); });

    if (!due.empty())
    {
        char const* blocked = "near";
        AlgalonScanUnit const* fallback = nullptr;
        int32 fallbackSeparation = -1;
        for (StarFilter filter : {StarFilter::TanksAndRing, StarFilter::Tanks})
        {
            for (AlgalonScanUnit const* candidate : due)
            {
                if (!StarPassesFilter(state, group, boss, *candidate, filter))
                    continue;

                StarSpan const finish = FinishBand(state, group, *candidate);
                char const* collision = StarCollision(plan, finish, candidate, gap);
                if (!collision)
                {
                    star = candidate;
                    return "deadline";
                }

                if (!std::strcmp(blocked, "near"))
                    blocked = collision;

                int32 const separation = StarSeparation(plan, finish, candidate);
                if (separation > fallbackSeparation)
                {
                    fallback = candidate;
                    fallbackSeparation = separation;
                }
            }
        }

        // Nothing lands clear: still kill the one that lands further from the rest than their own deaths do,
        // or two due stars 2 s apart both drain out together.
        int32 natural = std::numeric_limits<int32>::max();
        for (AlgalonScanUnit const* candidate : due)
        {
            int32 const death = ForecastDeath(plan, candidate);
            natural = std::min(natural, std::max(0, StarSeparation(plan, {death, death}, candidate)));
        }

        if (fallback && fallbackSeparation > natural)
        {
            star = fallback;
            return "deadline";
        }

        return blocked;
    }

    if (!RaidAffordsExplosion(bot, group))
        return "raidhp";

    std::vector<AlgalonScanUnit const*> open;
    for (AlgalonScanUnit const& candidate : state.stars)
        if (!candidate.under && StarPassesFilter(state, group, boss, candidate, StarFilter::Clear))
            open.push_back(&candidate);

    std::sort(open.begin(), open.end(), [&plan](AlgalonScanUnit const* a, AlgalonScanUnit const* b)
              { return ForecastDeath(plan, a) < ForecastDeath(plan, b); });

    char const* blocked = "near";
    for (AlgalonScanUnit const* candidate : open)
    {
        char const* collision = StarCollision(plan, FinishBand(state, group, *candidate), candidate, gap);
        if (!collision)
        {
            star = candidate;
            return "open";
        }

        if (!std::strcmp(blocked, "near"))
            blocked = collision;
    }

    return blocked;
}

void EnsureAlgalonSlotAssignments(Player* bot, AlgalonEncounterState& state)
{
    Group* group = bot->GetGroup();
    if (!group)
        return;

    for (auto it = state.slotAssignments.begin(); it != state.slotAssignments.end();)
    {
        Player* holder = FindGroupMember(group, it->first);
        if (!holder || holder->GetMapId() != ULDUAR_MAP_ID || !AlgalonTakesRingSlot(holder))
            it = state.slotAssignments.erase(it);
        else
            ++it;
    }

    std::array<bool, ULDUAR_ALGALON_TOTAL_SLOTS> used = {};
    for (auto const& assignment : state.slotAssignments)
        if (assignment.second < ULDUAR_ALGALON_TOTAL_SLOTS)
            used[assignment.second] = true;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != ULDUAR_MAP_ID || !AlgalonTakesRingSlot(member) ||
            state.slotAssignments.count(member->GetGUID()))
            continue;

        // Healers prefer the inner ring (heal range is 30 yd to the tank), but a raid with more healers
        // than inner slots still places everyone.
        bool const wantsHealerSlot = PlayerbotAI::IsHeal(member);
        bool placed = false;
        for (uint8 pass = 0; pass < 2 && !placed; ++pass)
        {
            for (uint8 slotIndex = 0; slotIndex < ULDUAR_ALGALON_TOTAL_SLOTS; ++slotIndex)
            {
                bool const healerSlot = slotIndex < ULDUAR_ALGALON_HEALER_SLOTS;
                if (used[slotIndex] || (pass == 0 && healerSlot != wantsHealerSlot))
                    continue;

                state.slotAssignments[member->GetGUID()] = slotIndex;
                used[slotIndex] = true;
                placed = true;
                break;
            }
        }
    }
}

AlgalonPhase ReadPhase(Creature* boss, AlgalonEncounterState const& state)
{
    if (state.engagedSeen && boss->GetFaction() == FACTION_FRIENDLY)
        return AlgalonPhase::Won;

    if (!boss->IsInCombat())
        return AlgalonPhase::Idle;

    if (boss->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        return AlgalonPhase::Intro;

    return state.wormHolesSeen ? AlgalonPhase::Two : AlgalonPhase::One;
}

// Same order on every bot, so the arc fills in step. Past ten melee, two share a spot.
bool MeleeStageSpot(Player* bot, AlgalonEncounterState const& state, Position& spot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<ObjectGuid> staged;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (IsBot(member) && member->GetMapId() == ULDUAR_MAP_ID && !PlayerbotAI::IsRanged(member) &&
            !state.swapPair.count(member->GetGUID()))
            staged.push_back(member->GetGUID());
    }

    std::sort(staged.begin(), staged.end());
    auto const own = std::find(staged.begin(), staged.end(), bot->GetGUID());
    if (own == staged.end())
        return false;

    spot = ULDUAR_ALGALON_MELEE_STAGE_SLOTS[static_cast<size_t>(own - staged.begin()) %
                                            ULDUAR_ALGALON_MELEE_STAGE_SPOTS];
    return true;
}

void ClampIntoRoom(float& x, float& y)
{
    float const fromCenter = ULDUAR_ALGALON_ROOM_CENTER.GetExactDist2d(x, y);
    if (fromCenter <= ULDUAR_ALGALON_ROOM_CLAMP)
        return;

    float const scale = ULDUAR_ALGALON_ROOM_CLAMP / fromCenter;
    x = ULDUAR_ALGALON_ROOM_CENTER.GetPositionX() + (x - ULDUAR_ALGALON_ROOM_CENTER.GetPositionX()) * scale;
    y = ULDUAR_ALGALON_ROOM_CENTER.GetPositionY() + (y - ULDUAR_ALGALON_ROOM_CENTER.GetPositionY()) * scale;
}

// `distance` from `origin` towards `toward`, inside the room, or false if that is in his reach.
bool SpotOffFrom(Unit* boss, Position const& origin, Position const& toward, float distance, Position& spot)
{
    float const angle = origin.GetAngle(&toward);
    float x = origin.GetPositionX() + std::cos(angle) * distance;
    float y = origin.GetPositionY() + std::sin(angle) * distance;
    ClampIntoRoom(x, y);
    if (boss && boss->GetExactDist2d(x, y) < ULDUAR_ALGALON_FORMATION_CLEARANCE)
        return false;

    spot = Position(x, y, ULDUAR_ALGALON_ROOM_CENTER.GetPositionZ());
    return true;
}

// A hole further from the bot's own spot than it can run in the cast: it waits on the way there instead.
// Measured from the spot, not the bot, or the bot would walk back the moment it got there.
bool FarHoleSpot(Player* bot, Unit* boss, AlgalonEncounterState const& state, Position const& anchor,
                 Position& spot)
{
    // The disperser soaks the coming hit where it stands.
    if (state.holes.empty() || state.swapPair.count(bot->GetGUID()) || state.handler == bot->GetGUID() ||
        state.disperser == bot->GetGUID())
        return false;

    if (!state.bigBangCasting && !BigBangDueWithin(state, ULDUAR_ALGALON_FAR_HOLE_SECONDS, getMSTime()))
        return false;

    if (AlgalonStaysOut(bot))
        return false;

    // At 17.5 yd/s the boots cover a far hole within the cast.
    if (state.bigBangCasting ? state.nitroRunners.count(bot->GetGUID()) > 0 : NitroBoostsReady(bot))
        return false;

    // With a star under, the window opens once the last hider is in.
    bool const under = !state.underStars.empty();
    float const far = under ? ULDUAR_ALGALON_UNDER_HOLE_REACH : ULDUAR_ALGALON_FAR_HOLE_YARDS;
    float const standoff = under ? ULDUAR_ALGALON_UNDER_HOLE_REACH : ULDUAR_ALGALON_NEAR_HOLE_STANDOFF;
    AlgalonScanUnit const* nearest = NearestShelter(state, anchor);
    if (!nearest || nearest->position.GetExactDist2d(anchor) <= far)
        return false;

    if (SpotOffFrom(boss, nearest->position, anchor, standoff, spot))
        return true;

    // Straight towards the spot is in his reach, as for the healers ringing him: the nearest point round the
    // hole that isn't.
    float closest = std::numeric_limits<float>::max();
    for (uint8 step = 0; step < ULDUAR_ALGALON_SWEEP_BEARINGS; ++step)
    {
        float const angle = static_cast<float>(step) * ULDUAR_ALGALON_SWEEP_ANGLE_STEP;
        float const x = nearest->position.GetPositionX() + std::cos(angle) * standoff;
        float const y = nearest->position.GetPositionY() + std::sin(angle) * standoff;
        float const distance = anchor.GetExactDist2d(x, y);
        if (distance >= closest || !AlgalonSpotInRoom(x, y) ||
            (boss && boss->GetExactDist2d(x, y) < ULDUAR_ALGALON_FORMATION_CLEARANCE))
            continue;

        closest = distance;
        spot = Position(x, y, ULDUAR_ALGALON_ROOM_CENTER.GetPositionZ());
    }

    return closest != std::numeric_limits<float>::max();
}

// A ranged star-team member whose focus star is out of range of its ring slot works it from a spot of its
// own: with the slot as its spot, "reach spell" and the slot node pull it opposite ways.
bool StarTeamSpot(Player* bot, Unit* boss, AlgalonEncounterState const& state, Position const& anchor,
                  Position& spot)
{
    if (!IsAlgalonStarTeam(bot))
        return false;

    AlgalonScanUnit const* star = FindScanned(state.stars, state.focusStar);
    return star && star->position.GetExactDist2d(anchor) > ULDUAR_ALGALON_STAR_TEAM_REACH &&
           SpotOffFrom(boss, star->position, anchor, ULDUAR_ALGALON_STAR_TEAM_STANDOFF, spot);
}

// Swap tanks wait in his reach of the landing spot, then the holder walks him to the tank slot and the other
// waits in his reach there. The pickup tank keeps its spot until then, or a missed taunt walks it away from him.
char const* SwapTankSlot(Player* bot, PlayerbotAI* botAI, Position& slot)
{
    Unit* boss = GetAlgalon(botAI);
    bool const selectable = boss && !boss->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) && AlgalonEngaged(botAI);
    Player* holder = selectable ? GetAlgalonBossTank(botAI) : nullptr;
    bool const pairHolds = holder && IsAlgalonSwapTank(holder);
    if (pairHolds && holder == bot)
    {
        // Facing him, the pull's first holder builds threat and blocks; on the walk out he hits its back.
        AlgalonEncounterState const* state = FindState(bot);
        if (state && state->firstHolder == bot->GetGUID() &&
            getMSTimeDiff(state->firstHoldMs, getMSTime()) < ULDUAR_ALGALON_PULL_HOLD_MS)
        {
            slot = ULDUAR_ALGALON_PICKUP_SLOT;
            return "pickup";
        }

        slot = ULDUAR_ALGALON_TANK_SLOT;
        return "tank";
    }

    if (!pairHolds && GetAlgalonPickupTank(botAI) == bot)
    {
        slot = ULDUAR_ALGALON_PICKUP_SLOT;
        return "pickup";
    }

    if (!selectable)
        slot = ULDUAR_ALGALON_PULL_OFFTANK_SLOT;
    else if (!pairHolds || holder->GetExactDist2d(ULDUAR_ALGALON_TANK_SLOT) <= ULDUAR_ALGALON_HOLDER_ON_SPOT_RADIUS)
        slot = ULDUAR_ALGALON_OFFTANK_SLOT;
    else
    {
        // Held anywhere else, by a human or on the walk out: in his reach, on the tank slot's side.
        float const angle = boss->GetAngle(&ULDUAR_ALGALON_TANK_SLOT);
        slot = Position(boss->GetPositionX() + std::cos(angle) * ULDUAR_ALGALON_OFFTANK_STEP,
                        boss->GetPositionY() + std::sin(angle) * ULDUAR_ALGALON_OFFTANK_STEP,
                        ULDUAR_ALGALON_ROOM_CENTER.GetPositionZ());
    }

    return "offtank";
}

// The undisplaced spot and what it is, or nullptr. A far hole before Big Bang beats a far star.
char const* RawSlot(Player* bot, Position& slot)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    AlgalonEncounterState const* state = FindState(bot);
    if (!botAI || !state)
        return nullptr;

    if (IsAlgalonSwapTank(bot))
        return SwapTankSlot(bot, botAI, slot);

    Unit* boss = GetAlgalon(botAI);
    char const* kind = nullptr;
    if (AlgalonMeleeStaged(bot))
        kind = MeleeStageSpot(bot, *state, slot) ? "stage" : nullptr;
    else if (AlgalonTakesRingSlot(bot))
    {
        auto const assignment = state->slotAssignments.find(bot->GetGUID());
        if (assignment != state->slotAssignments.end() && TryGetAlgalonSlotPosition(assignment->second, slot))
            kind = "slot";
    }

    if (!state->engaged || !boss)
        return kind;

    // A released melee bot has no spot of its own; it belongs at him.
    Position const anchor = kind ? slot : boss->GetPosition();
    Position spot;
    if (FarHoleSpot(bot, boss, *state, anchor, spot))
    {
        slot = spot;
        return "nearhole";
    }

    if (kind && PlayerbotAI::IsRanged(bot) && StarTeamSpot(bot, boss, *state, anchor, spot))
    {
        slot = spot;
        return "star";
    }

    return kind;
}

bool StarDying(AlgalonScanUnit const& star)
{
    return star.healthPct <= ULDUAR_ALGALON_STAR_DYING_HP_PCT ||
           star.healthPct / star.lossPctPerSec * IN_MILLISECONDS <= ULDUAR_ALGALON_STAR_DYING_MS;
}

bool DyingStarNear(AlgalonEncounterState const& state, Position const& spot, float radius)
{
    for (AlgalonScanUnit const& star : state.stars)
        if (StarDying(star) && spot.GetExactDist2d(star.position) < radius)
            return true;

    return false;
}

bool ClearOfHoles(AlgalonEncounterState const& state, float x, float y)
{
    for (AlgalonScanUnit const& hole : state.holes)
        if (hole.position.GetExactDist2d(x, y) < ULDUAR_ALGALON_HOLE_EXIT_RADIUS)
            return false;

    return true;
}

bool ClearOfMarkers(AlgalonEncounterState const& state, float x, float y)
{
    for (AlgalonScanUnit const& marker : state.markers)
        if (marker.position.GetExactDist2d(x, y) < ULDUAR_ALGALON_COSMIC_SMASH_CLEARANCE)
            return false;

    return true;
}

// The first hole a straight walk passes within `clearance` of. A walker in or phased by a field may cross only
// that one, since a neighbour 8 yd off phases it again, and may not come closer to a field it stands beside.
AlgalonScanUnit const* HoleAcrossPath(AlgalonEncounterState const& state, Position const& from, float x, float y,
                                      bool phased, float clearance = ULDUAR_ALGALON_HOLE_PATH_CLEARANCE)
{
    float const inside = phased ? ULDUAR_ALGALON_HOLE_EXIT_RADIUS : ULDUAR_ALGALON_SHELTER_RADIUS;
    AlgalonScanUnit const* leaving = nullptr;
    float nearest = std::numeric_limits<float>::max();
    for (AlgalonScanUnit const& hole : state.holes)
    {
        float const start = hole.position.GetExactDist2d(from);
        if (start <= inside && start < nearest)
        {
            leaving = &hole;
            nearest = start;
        }
    }

    for (AlgalonScanUnit const& hole : state.holes)
    {
        if (&hole == leaving)
            continue;

        float const start = hole.position.GetExactDist2d(from);
        float const limit = std::min(clearance, std::max(start - 0.5f, ULDUAR_ALGALON_SHELTER_RADIUS));
        if (SegmentDistance(from, x, y, hole.position) < limit)
            return &hole;
    }

    return nullptr;
}

// Round the hole, preferring a point with a clear line on to the slot. A point that brings the slot no closer is
// skipped: the bot's own spot would win every later search and the walk would stall beside the hole.
bool FindHoleDetour(Player* bot, AlgalonEncounterState const& state, AlgalonScanUnit const& hole, Position const& to,
                    Position& waypoint)
{
    bool const phased = IsAlgalonPhased(bot);
    float const remaining = bot->GetExactDist2d(to);
    float best = std::numeric_limits<float>::max();
    bool bestThrough = false;
    for (uint8 step = 0; step < ULDUAR_ALGALON_SWEEP_BEARINGS; ++step)
    {
        float const angle = static_cast<float>(step) * ULDUAR_ALGALON_SWEEP_ANGLE_STEP;
        float const x = hole.position.GetPositionX() + std::cos(angle) * ULDUAR_ALGALON_HOLE_DETOUR_RADIUS;
        float const y = hole.position.GetPositionY() + std::sin(angle) * ULDUAR_ALGALON_HOLE_DETOUR_RADIUS;
        float const onward = to.GetExactDist2d(x, y);
        if (onward >= remaining - 1.0f || !AlgalonSpotInRoom(x, y) || !ClearOfHoles(state, x, y) ||
            !ClearOfMarkers(state, x, y) || HoleAcrossPath(state, bot->GetPosition(), x, y, phased))
            continue;

        Position const point(x, y, to.GetPositionZ());
        bool const through = !HoleAcrossPath(state, point, to.GetPositionX(), to.GetPositionY(), false);
        float const legs = bot->GetExactDist2d(x, y) + onward;
        if ((through && !bestThrough) || (through == bestThrough && legs < best))
        {
            best = legs;
            bestThrough = through;
            waypoint = point;
        }
    }

    if (best == std::numeric_limits<float>::max())
        return false;

    waypoint = ValidateFloorPoint(bot, waypoint);
    return true;
}

// Holes land wherever a star died, so a spot can end up buried. Step aside from the spot itself, nearest
// ring first, rather than abandon the formation.
bool DisplaceFromHoles(Player* bot, AlgalonEncounterState const& state, Position const& slot, Position& position)
{
    for (float radius = ULDUAR_ALGALON_SWEEP_STEP; radius <= ULDUAR_ALGALON_SLOT_DISPLACE_RADIUS;
         radius += ULDUAR_ALGALON_SWEEP_STEP)
    {
        for (uint8 step = 0; step < ULDUAR_ALGALON_SWEEP_BEARINGS; ++step)
        {
            float const angle = static_cast<float>(step) * ULDUAR_ALGALON_SWEEP_ANGLE_STEP;
            float const x = slot.GetPositionX() + std::cos(angle) * radius;
            float const y = slot.GetPositionY() + std::sin(angle) * radius;
            if (!ClearOfHoles(state, x, y) || !ClearOfMarkers(state, x, y) || !AlgalonSpotInRoom(x, y))
                continue;

            position = ValidateFloorPoint(bot, Position(x, y, slot.GetPositionZ()));
            return true;
        }
    }

    return false;
}

// Returns the rule that placed it, "blocked" for a waypoint round a hole; `found` says whether `position`
// is somewhere to walk to.
char const* DeriveSlot(Player* bot, Position& position, bool& found)
{
    found = false;
    Position slot;
    char const* const kind = RawSlot(bot, slot);
    if (!kind)
        return "none";

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    // Test the slot, not the bot: a bot that already dodged passes trivially and walks back under it.
    if (AlgalonCosmicSmashMarkerNear(botAI, slot, ULDUAR_ALGALON_COSMIC_SMASH_CLEARANCE) ||
        DyingStarNear(*FindState(bot), slot, ULDUAR_ALGALON_STAR_DYING_RADIUS))
        return "smash";

    AlgalonEncounterState const* state = FindState(bot);
    char const* branch = kind;
    position = slot;
    if (!ClearOfHoles(*state, slot.GetPositionX(), slot.GetPositionY()))
    {
        if (!DisplaceFromHoles(bot, *state, slot, position))
            return "none";

        branch = "buried";
    }

    // Phased bots too: Big Bang's strip can land mid-walk, and a field the walk clips then phases them again
    // for 10s.
    if (bot->GetExactDist2d(position) > ULDUAR_ALGALON_SLOT_TOLERANCE)
    {
        if (AlgalonScanUnit const* hole = HoleAcrossPath(*state, bot->GetPosition(), position.GetPositionX(),
                                                         position.GetPositionY(), IsAlgalonPhased(bot)))
        {
            Position waypoint;
            found = FindHoleDetour(bot, *state, *hole, position, waypoint);
            position = waypoint;
            return "blocked";
        }
    }

    found = true;
    return branch;
}

// When a star dies, from its own slope, or now once at `lowPct` or less. One the team just left dies at what it
// lost since, never under the drain, capped like a dropped star until that's measured: the slope still has the
// team's burst, and a cover went up on one at 23% that lived 11 more seconds.
int32 ExplosionDueMs(AlgalonEncounterState const& state, AlgalonScanUnit const& star, float lowPct)
{
    if (star.healthPct <= lowPct)
        return 0;

    float loss = star.lossPctPerSec;
    auto const left = state.starLeft.find(star.guid);
    if (left != state.starLeft.end())
    {
        uint32 const since = getMSTimeDiff(left->second.first, state.lastTickMs);
        if (since >= ULDUAR_ALGALON_STAR_LOG_STEP_MS)
            loss = std::max(ULDUAR_ALGALON_STAR_DRAIN_PCT_PER_SEC,
                            (left->second.second - star.healthPct) * IN_MILLISECONDS / static_cast<float>(since));
        else
            loss = std::min(loss, ULDUAR_ALGALON_STAR_DROPPED_PCT_PER_SEC);
    }

    return StarDeathAt(star, loss);
}

// Planned under the coming cast and its drain still ends inside the window: it hits the soaker alone, in
// Dispersion, and a cover spent on it is gone for the stars after the strip.
bool DiesUnder(AlgalonEncounterState const& state, AlgalonScanUnit const& star)
{
    return star.under && star.tickHp && state.lastTickMs &&
           DrainDeathMs(star) <= MsUntil(state.underToMs, state.lastTickMs);
}

// The star forecast to die soonest within `leadMs`. With `bot`, only one still alive: the scan lags a death by up
// to a tick, and the explosion leaves the bot under its line, pressing for a star already gone. `underToo` keeps
// the stars planned under the cast, for whoever they catch outside a field.
AlgalonScanUnit const* ExplodingStar(AlgalonEncounterState const& state, uint32 leadMs,
                                     float lowPct = ULDUAR_ALGALON_EXPLOSION_LOW_PCT, Player* bot = nullptr,
                                     bool underToo = false)
{
    AlgalonScanUnit const* soonest = nullptr;
    int32 best = std::numeric_limits<int32>::max();
    for (AlgalonScanUnit const& star : state.stars)
    {
        int32 const death = ExplosionDueMs(state, star, lowPct);
        if (death <= static_cast<int32>(leadMs) && death < best && (underToo || !DiesUnder(state, star)) &&
            (!bot || Resolve(bot, star.guid)))
        {
            soonest = &star;
            best = death;
        }
    }

    return soonest;
}

bool CaughtOutside(AlgalonEncounterState const& state, Player* member, int32 deathMs);

// The hit to survive from `first` and the live stars dying within the stack window of it, the soonest few,
// each covered only if it dies before the cover running on the bot ends. In the cast (`underToo`) a later one
// counts only if the bot is still outside a field then: 8 bots pressed at full health for a second star that
// went off once most were phased.
float StackedHit(AlgalonEncounterState const& state, Player* bot, AlgalonScanUnit const& first, float topRoll,
                 int32 coverMs, bool underToo = false)
{
    int32 const from = ExplosionDueMs(state, first, ULDUAR_ALGALON_EXPLOSION_LOW_PCT);
    std::vector<int32> deaths;
    for (AlgalonScanUnit const& star : state.stars)
    {
        int32 const death = ExplosionDueMs(state, star, ULDUAR_ALGALON_EXPLOSION_LOW_PCT);
        if (death <= from + static_cast<int32>(ULDUAR_ALGALON_EXPLOSION_STACK_MS) &&
            (underToo || !DiesUnder(state, star)) && Resolve(bot, star.guid) &&
            (!underToo || star.guid == first.guid || CaughtOutside(state, bot, death)))
            deaths.push_back(death);
    }

    if (deaths.empty())
        deaths.push_back(from);

    std::sort(deaths.begin(), deaths.end());
    deaths.resize(std::min<size_t>(deaths.size(), ULDUAR_ALGALON_EXPLOSION_STACK_MAX));
    float hit = 0.0f;
    for (int32 const death : deaths)
        hit += topRoll * (death <= coverMs ? ULDUAR_ALGALON_COVERED_EXPLOSION_FACTOR : 1.0f);

    return hit;
}

// From just before a star explosion or a meteor until the healers, who spend those seconds on the raid, are
// back on the tanks.
bool RaidHitWindow(AlgalonEncounterState const& state, uint32 now)
{
    for (uint32 const hit : {state.lastStarDeathMs, state.lastSmashImpactMs})
        if (hit && getMSTimeDiff(hit, now) <= ULDUAR_ALGALON_TANK_WINDOW_TAIL_MS)
            return true;

    if (ExplodingStar(state, ULDUAR_ALGALON_TANK_WINDOW_LEAD_MS))
        return true;

    for (auto const& [marker, seenMs] : state.markerSeenMs)
    {
        int32 const impact = MsUntil(seenMs + ULDUAR_ALGALON_COSMIC_SMASH_IMPACT_MS, now);
        if (impact >= 0 && impact <= static_cast<int32>(ULDUAR_ALGALON_TANK_WINDOW_LEAD_MS))
            return true;
    }

    return false;
}

// RunningTankDefensive's buttons, or an external on it.
bool TankCutRunning(Player* holder)
{
    return RunningTankDefensive(holder, true, 0) || holder->HasAura(SPELL_ALGALON_PAIN_SUPPRESSION) ||
           holder->HasAura(SPELL_ALGALON_HAND_OF_SACRIFICE) || holder->HasAura(SPELL_ALGALON_GUARDIAN_SPIRIT);
}

// Why the holder needs a cut now, or nullptr. The partner takes him within a second when a swap is due, and a
// cutter on the outgoing tank is spent for nothing. By stacks, so a human partner counts too.
char const* TankCutReason(AlgalonEncounterState const& state, Player* holder, Player* partner, uint32 now)
{
    bool const swapping = partner && CanStayOut(partner) && SwapDue(state, holder, partner, now);
    if (RaidHitWindow(state, now) && !swapping && holder->GetHealthPct() < ULDUAR_ALGALON_TANK_WINDOW_PCT)
        return "window";

    return holder->GetHealthPct() < ULDUAR_ALGALON_TANK_DEFENSIVE_PCT ? "low" : nullptr;
}

// A human holder has no trigger for his own button, so he's told in the spots a bot holder presses one, under
// the same reserve: never the button his Big Bang soak needs.
void CallHumanTankCut(Group* group, PlayerbotAI* botAI, Creature* boss, AlgalonEncounterState const& state,
                      uint32 now)
{
    Player* holder = BossHolder(boss);
    if (state.bigBangCasting || !holder || IsBot(holder) || !state.swapPair.count(holder->GetGUID()) ||
        !CanStayOut(holder) || TankCutRunning(holder))
        return;

    if (state.lastBigBangEndMs && getMSTimeDiff(state.lastBigBangEndMs, now) < ULDUAR_ALGALON_BIG_BANG_STILL_MS &&
        state.lastSoaker != holder->GetGUID())
        return;

    char const* why = TankCutReason(state, holder, PairPartner(group, state, holder), now);
    uint32 const spellId = why ? ReadyTankDefensiveSpell(holder, true, 0, 0) : 0;
    if (!spellId)
        return;

    bool const reserved = state.disperser.IsEmpty() && AlgalonLoneHolder(botAI) != holder;
    if (reserved && AlgalonCooldownMs(holder, spellId) > AlgalonMsToBigBangHit(botAI))
        return;

    if (Player* sender = FindTracker(group))
        CallHumanTankDefensive(sender, holder, spellId, 0,
                               std::strcmp(why, "window") ? "you are low" : "stars or meteors are hitting the raid");
}

void CallHumanSoak(Group* group, Creature* boss, AlgalonEncounterState& state)
{
    Player* soaker = FindGroupMember(group, state.bigBangSoaker);
    Spell* cast = CurrentBigBang(boss);
    if (state.humanSoakCalled || !soaker || IsBot(soaker) || !CanStayOut(soaker) || !cast)
        return;

    int32 const remaining = cast->GetCastTimeRemaining();
    Player* sender = FindTracker(group);
    if (remaining < 0 || remaining > ULDUAR_ALGALON_HUMAN_SOAK_CALL_MS || !sender)
        return;

    state.humanSoakCalled = true;
    uint32 const hitInMs = static_cast<uint32>(remaining);
    // Once per cast already, so an earlier call can't hold this one back.
    CallHumanTankDefensive(sender, soaker, ReadyTankDefensiveSpell(soaker, true, 0, hitInMs), hitInMs,
                           "Big Bang hits you", 0);
}

// CanCastSpell skips the cost, and a provider out of mana would keep its turn while every cast fails. Runes
// aren't checked.
bool CanPay(Player* caster, SpellInfo const* info)
{
    if (info->PowerType >= MAX_POWERS || info->PowerType == POWER_RUNE)
        return true;

    return static_cast<int32>(caster->GetPower(Powers(info->PowerType))) >=
           info->CalcPowerCost(caster, info->GetSchoolMask());
}

// The provider's spell id for `spell` when it knows it, can pay for it and has it off cooldown, else 0. The
// cooldown is also CanCastSpell's to refuse, but only after ExternalReaches has paid for range and sight.
uint32 ExternalSpell(Player* provider, char const* spell)
{
    PlayerbotAI* providerAI = GET_PLAYERBOT_AI(provider);
    if (!providerAI || !CanStayOut(provider))
        return 0;

    uint32 const spellId = providerAI->GetAiObjectContext()->GetValue<uint32>("spell id", spell)->Get();
    SpellInfo const* info = spellId ? sSpellMgr->GetSpellInfo(spellId) : nullptr;
    return info && CanPay(provider, info) && !provider->HasSpellCooldown(spellId) ? spellId : 0;
}

// Whether the provider can put an ExternalSpell id on `target` now. Range and sight are checked here:
// CanCastSpell lets an out of range cast through.
bool ExternalReaches(Player* provider, uint32 spellId, Player* target)
{
    PlayerbotAI* providerAI = GET_PLAYERBOT_AI(provider);
    SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
    if (!providerAI || !info)
        return false;

    if (provider != target && (!provider->IsWithinDistInMap(target, info->GetMaxRange(true, provider)) ||
                               !provider->IsWithinLOSInMap(target)))
        return false;

    return providerAI->CanCastSpell(spellId, target);
}

uint32 ExternalReady(Player* provider, char const* spell, Player* target)
{
    uint32 const spellId = ExternalSpell(provider, spell);
    return spellId && ExternalReaches(provider, spellId, target) ? spellId : 0;
}

// The class rule: it moves 30% of his hits onto the paladin, and one Hand per paladin on a target.
uint32 SacrificeSpell(Player* paladin, Player* holder)
{
    if (paladin == holder || paladin->getClass() != CLASS_PALADIN ||
        paladin->GetHealthPct() < ULDUAR_ALGALON_SACRIFICE_MIN_PCT ||
        ai::paladin::HasAnyPaladinHandFromCaster(holder, paladin))
        return 0;

    return ExternalSpell(paladin, "hand of sacrifice");
}

// A passive talent is only in the talent map, never the spell map HasSpell reads.
bool HasDivineGuardian(Player* paladin)
{
    uint8 const spec = paladin->GetActiveSpec();
    return paladin->HasTalent(SPELL_ALGALON_DIVINE_GUARDIAN_RANK_1, spec) ||
           paladin->HasTalent(SPELL_ALGALON_DIVINE_GUARDIAN_RANK_2, spec);
}

// One cover per explosion: Divine Guardian on anyone, or Aura Mastery on the shadow aura paladin.
bool ExplosionCovered(Group* group, AlgalonEncounterState const& state)
{
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (InFight(member) && member->HasAura(SPELL_ALGALON_DIVINE_GUARDIAN))
            return true;
    }

    Player* auraPaladin = FindGroupMember(group, state.shadowAuraPaladin);
    return auraPaladin && auraPaladin->HasAura(SPELL_ALGALON_AURA_MASTERY);
}

// Prayer of Shadow Protection already gives the aura's 130 (both aura 143, highest only), so it only pays doubled
// by Aura Mastery. With that one spent it moves to one whose Aura Mastery is back, else a second never goes up.
Player* ElectShadowAuraPaladin(Group* group, ObjectGuid const& current)
{
    Player* kept = nullptr;
    Player* ready = nullptr;
    Player* lowest = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
        if (!memberAI || !InFight(member) || member->getClass() != CLASS_PALADIN ||
            !member->HasActiveSpell(SPELL_ALGALON_AURA_MASTERY) ||
            !memberAI->GetAiObjectContext()->GetValue<uint32>("spell id", "shadow resistance aura")->Get())
            continue;

        if (member->GetGUID() == current)
            kept = member;

        if (!lowest || member->GetGUID() < lowest->GetGUID())
            lowest = member;

        if (!member->HasSpellCooldown(SPELL_ALGALON_AURA_MASTERY) && (!ready || member->GetGUID() < ready->GetGUID()))
            ready = member;
    }

    // Its own Aura Mastery doubles its own aura, so it keeps the slot while that runs.
    if (kept && (kept->HasAura(SPELL_ALGALON_AURA_MASTERY) || !kept->HasSpellCooldown(SPELL_ALGALON_AURA_MASTERY)))
        return kept;

    if (ready)
        return ready;

    return kept ? kept : lowest;
}

// Per hole, the hiders still out: bots by their shelter, humans by the nearest hole, kite hole included, since
// the call only says "a black hole".
void CountHidersOut(Group* group, AlgalonEncounterState& state)
{
    auto const nearest = [&state](Player* human)
    {
        AlgalonScanUnit const* best = nullptr;
        for (AlgalonScanUnit const& hole : state.holes)
            if (!best || human->GetExactDist2d(hole.position) < human->GetExactDist2d(best->position))
                best = &hole;

        return best;
    };

    state.hidersOut.clear();
    if (!state.bigBangCasting)
        return;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!CanStayOut(member))
            continue;

        // A chased one goes in last, so it can't be waiting on itself.
        ObjectGuid const guid = member->GetGUID();
        if (guid == state.bigBangSoaker || guid == state.bigBangBackup || guid == state.handler ||
            AlgalonHoldsConstellation(member))
            continue;

        AlgalonScanUnit const* hole = IsBot(member) ? ShelterHole(member, state) : nearest(member);
        if (hole)
            ++state.hidersOut[hole->guid];
    }
}

}  // namespace

Unit* GetAlgalon(PlayerbotAI* botAI) { return botAI ? FindAlgalon(botAI->GetBot()) : nullptr; }

bool AlgalonInRoom(Player* bot)
{
    return bot && bot->GetMapId() == ULDUAR_MAP_ID &&
           bot->GetExactDist2d(ULDUAR_ALGALON_ROOM_CENTER.GetPositionX(), ULDUAR_ALGALON_ROOM_CENTER.GetPositionY()) <=
               ULDUAR_ALGALON_ROOM_RADIUS &&
           bot->GetPositionZ() >= ULDUAR_ALGALON_ROOM_Z_MIN && bot->GetPositionZ() <= ULDUAR_ALGALON_ROOM_Z_MAX;
}

bool AlgalonPresent(PlayerbotAI* botAI)
{
    Unit* boss = GetAlgalon(botAI);
    return boss && boss->GetFaction() != FACTION_FRIENDLY && AlgalonInRoom(botAI->GetBot());
}

bool AlgalonEngaged(PlayerbotAI* botAI)
{
    Unit* boss = GetAlgalon(botAI);
    return boss && boss->IsInCombat() && boss->GetFaction() != FACTION_FRIENDLY;
}

bool AlgalonInPhaseOne(PlayerbotAI* botAI)
{
    AlgalonEncounterState const* state = FindState(botAI ? botAI->GetBot() : nullptr);
    return state && state->phase == AlgalonPhase::One;
}

bool IsAlgalonPhased(Unit const* unit) { return unit && !(unit->GetPhaseMask() & PHASEMASK_NORMAL); }

bool AlgalonInOpener(PlayerbotAI* botAI)
{
    AlgalonEncounterState const* state = FindState(botAI ? botAI->GetBot() : nullptr);
    if (!state)
        return false;

    return state->phase == AlgalonPhase::One && state->introEndMs &&
           getMSTimeDiff(state->introEndMs, getMSTime()) <= ULDUAR_ALGALON_OPENER_MS;
}

Player* GetAlgalonOpenerTank(PlayerbotAI* botAI)
{
    Player* holder = GetAlgalonBossTank(botAI);
    return holder && IsAlgalonSwapTank(holder) ? holder : nullptr;
}

char const* AlgalonThreatRedirectSpell(Player* bot)
{
    switch (bot->getClass())
    {
        case CLASS_HUNTER:
            return "misdirection";
        case CLASS_ROGUE:
            return "tricks of the trade";
        default:
            return nullptr;
    }
}

namespace
{
// The health (lo, hi] that has the drain alone kill a star on a tick inside [fromMs, toMs], both ms from the
// time `nextTickMs` was read. Tick k lands at nextTickMs + (k - 1) s.
bool UnderBand(uint32 tickHp, int32 nextTickMs, int32 fromMs, int32 toMs, uint32& lo, uint32& hi)
{
    int32 const first = fromMs - nextTickMs;
    int32 const last = toMs - nextTickMs;
    if (last < 0)
        return false;

    int32 const second = static_cast<int32>(IN_MILLISECONDS);
    int32 const kLo = first <= 0 ? 1 : (first + second - 1) / second + 1;
    int32 const kHi = last / second + 1;
    if (kHi < kLo)
        return false;

    lo = static_cast<uint32>(kLo - 1) * tickHp;
    hi = static_cast<uint32>(kHi) * tickHp;
    return true;
}

// Half a tick under the band's top. A spell landing after the stop is 1-2%, inside a band 4-5 ticks wide;
// one that overshoots below it is released `early` and paced.
uint32 StopLineHp(AlgalonScanUnit const& star)
{
    uint32 const half = star.tickHp / 2;
    return std::max(star.underLo + 1, star.underHi > half ? star.underHi - half : 0);
}

// DoTs keep ticking once the team stops, so their ticks to come count as done.
uint32 BulkLeftHp(AlgalonScanUnit const& star)
{
    uint32 const stop = StopLineHp(star);
    uint32 const due = star.health > star.pendingHp ? star.health - star.pendingHp : 0;
    return due > stop ? due - stop : 0;
}

// The band falls with the drain, so the gap closes at the team's speed less the drain. The bulk's own once
// measured, never above the default.
int32 BulkWorkMs(AlgalonEncounterState const& state, AlgalonScanUnit const& star)
{
    uint32 const left = BulkLeftHp(star);
    if (!left)
        return 0;

    float const measured = state.bulkPctPerSec > 0.0f ? state.bulkPctPerSec : state.teamKillPctPerSec;
    float const pctPerSec = measured > 0.0f ? std::min(measured, ULDUAR_ALGALON_UNDER_TEAM_PCT_PER_SEC)
                                            : ULDUAR_ALGALON_UNDER_TEAM_PCT_PER_SEC;
    float const closing = std::max(1.0f, pctPerSec - ULDUAR_ALGALON_STAR_DRAIN_PCT_PER_SEC);
    return static_cast<int32>(left / (star.tickHp * closing) * IN_MILLISECONDS) + ULDUAR_ALGALON_UNDER_SWITCH_MS;
}

// Through the cast and until the strip is past: the window is the running cast's.
bool UnderFrozen(AlgalonEncounterState const& state, uint32 now)
{
    return state.bigBangCasting ||
           (state.lastBigBangEndMs && getMSTimeDiff(state.lastBigBangEndMs, now) < ULDUAR_ALGALON_UNDER_KEEP_MS);
}

// Each scan rebuilds the stars, so the bands come from the latched window every tick.
void MarkUnder(AlgalonEncounterState& state, uint32 now)
{
    int32 const from = MsUntil(state.underFromMs, now);
    int32 const to = MsUntil(state.underToMs, now);
    bool const frozen = UnderFrozen(state, now);
    for (AlgalonScanUnit& star : state.stars)
    {
        star.under = state.underStars.count(star.guid) > 0;
        star.underLo = 0;
        star.underHi = 0;
        if (star.under && star.tickHp)
            UnderBand(star.tickHp, star.nextTickMs, from, to, star.underLo, star.underHi);

        star.underDying =
            star.under && frozen && star.tickHp && DrainDeathMs(star) <= ULDUAR_ALGALON_UNDER_DYING_MS;
    }
}

// With no hole at all, the star nearest the hold spot dies before the cast for one; latched until it does.
void PlanShelterStar(AlgalonEncounterState& state)
{
    if (!state.holes.empty())
    {
        state.shelterStar = ObjectGuid::Empty;
        return;
    }

    if (FindScanned(state.stars, state.shelterStar))
        return;

    AlgalonScanUnit const* nearest = nullptr;
    for (AlgalonScanUnit const& star : state.stars)
        if (!nearest || star.position.GetExactDist2d(ULDUAR_ALGALON_HOLD_SPOT) <
                            nearest->position.GetExactDist2d(ULDUAR_ALGALON_HOLD_SPOT))
            nearest = &star;

    state.shelterStar = nearest ? nearest->guid : ObjectGuid::Empty;
}

// Off the plan for good: it dies too soon for the hide, its work no longer fits before the cast, or no soaker
// is left.
void ReleaseUnderStar(AlgalonEncounterState& state, AlgalonScanUnit const& star)
{
    state.underReleased.insert(star.guid);
}

// Plans under the next cast every star the drain can kill in its window, if the team has time for the work.
// Disperser only: a tank would take each explosion whole. A planned star stays planned; frozen through the cast.
void PlanUnderStars(AlgalonEncounterState& state, uint32 now)
{
    if (UnderFrozen(state, now))
    {
        MarkUnder(state, now);
        return;
    }

    if (state.underPlanCycle != state.bigBangCount)
    {
        // A low speed plans no star, so no bulk measures it again.
        if (state.bulkMeasuredCycle != state.underPlanCycle)
            state.bulkPctPerSec = 0.0f;

        state.underPlanCycle = state.bigBangCount;
        state.underStars.clear();
        state.underReleased.clear();
        state.underLateMs.clear();
        state.bulkStar = ObjectGuid::Empty;
    }

    PlanShelterStar(state);
    if (!state.nextBigBangMs || state.phase != AlgalonPhase::One)
    {
        state.underStars.clear();
        MarkUnder(state, now);
        return;
    }

    int32 const toCast = MsUntil(state.nextBigBangMs, now);

    // A soaker missing for a moment, phased or re-elected, keeps the plan. Gone for good, each star goes back to
    // pacing: a tank soaking would take every explosion whole.
    if (state.disperser.IsEmpty())
    {
        if (!state.underNoSoakMs)
            state.underNoSoakMs = now;

        if (getMSTimeDiff(state.underNoSoakMs, now) >= ULDUAR_ALGALON_UNDER_SOAKER_GRACE_MS)
        {
            for (AlgalonScanUnit const& star : state.stars)
                if (state.underStars.count(star.guid))
                    ReleaseUnderStar(state, star);

            state.underStars.clear();
        }

        MarkUnder(state, now);
        return;
    }

    state.underNoSoakMs = 0;
    int32 const toHit = toCast + static_cast<int32>(ULDUAR_ALGALON_BIG_BANG_CAST_MS);
    int32 const from = toHit - ULDUAR_ALGALON_UNDER_LEAD_MS;
    int32 const to = toHit + ULDUAR_ALGALON_UNDER_TAIL_MS;
    // The earliest death the hide stretch still gets everyone in a pulse and a half before.
    int32 const stretchFrom = toHit - (ULDUAR_ALGALON_HIDE_MARGIN_MS + ULDUAR_ALGALON_UNDER_HIDE_STRETCH_MS -
                                       ULDUAR_ALGALON_UNDER_DEATH_LEAD_MS);
    state.underFromMs = now + static_cast<uint32>(std::max(0, from));
    state.underToMs = now + static_cast<uint32>(std::max(0, to));

    std::unordered_set<ObjectGuid> const planned = std::move(state.underStars);
    state.underStars.clear();
    bool const releasing =
        toCast <= ULDUAR_ALGALON_UNDER_RELEASE_FROM_MS && toCast > ULDUAR_ALGALON_UNDER_RELEASE_TO_MS;
    std::vector<AlgalonScanUnit*> kept;
    std::vector<AlgalonScanUnit*> candidates;
    for (AlgalonScanUnit& star : state.stars)
    {
        star.underLo = 0;
        star.underHi = 0;
        if (state.underReleased.count(star.guid) || star.guid == state.shelterStar || !star.tickHp ||
            !UnderBand(star.tickHp, star.nextTickMs, from, to, star.underLo, star.underHi))
            continue;

        if (!planned.count(star.guid))
        {
            if (DrainDeathMs(star) >= from)
                candidates.push_back(&star);

            continue;
        }

        // Short of the window, it stays only while the hide stretch still gets everyone in before it.
        if (DrainDeathMs(star) < stretchFrom)
        {
            ReleaseUnderStar(state, star);
            continue;
        }

        kept.push_back(&star);
    }

    // One at a time, as the bulk takes them: its latched one first, then the one with most left.
    std::sort(kept.begin(), kept.end(), [&state](AlgalonScanUnit const* a, AlgalonScanUnit const* b)
              {
                  if ((a->guid == state.bulkStar) != (b->guid == state.bulkStar))
                      return a->guid == state.bulkStar;

                  return BulkLeftHp(*a) > BulkLeftHp(*b);
              });

    int32 work = 0;
    for (AlgalonScanUnit* star : kept)
    {
        // A late release is for good, so only once the work, every star's ahead of it included, has overrun for
        // a while.
        int32 const own = BulkWorkMs(state, *star);
        if (releasing && work + own > toCast)
        {
            auto const since = state.underLateMs.try_emplace(star->guid, now).first;
            if (getMSTimeDiff(since->second, now) >= ULDUAR_ALGALON_UNDER_LATE_HOLD_MS)
            {
                ReleaseUnderStar(state, *star);
                continue;
            }
        }
        else
            state.underLateMs.erase(star->guid);

        work += own;
        state.underStars.insert(star->guid);
    }

    // A new star only while the team's work still ends well before the cast: the biggest jobs are paced instead.
    int32 const available = toCast - ULDUAR_ALGALON_UNDER_FINISH_MS;
    std::sort(candidates.begin(), candidates.end(), [&state](AlgalonScanUnit const* a, AlgalonScanUnit const* b)
              { return BulkWorkMs(state, *a) < BulkWorkMs(state, *b); });

    for (AlgalonScanUnit const* star : candidates)
    {
        int32 const own = BulkWorkMs(state, *star);
        if (work + own > available)
            break;

        work += own;
        state.underStars.insert(star->guid);
    }

    MarkUnder(state, now);
}

// The under-star the team works down, kept until done, then the one with most left. A bulk starts once the work
// left plus the slack reaches the cast, then stays latched: gated each tick, a fast team would flip on and off it.
AlgalonScanUnit const* UnderBulkStar(AlgalonEncounterState& state, uint32 now)
{
    AlgalonScanUnit const* latched = FindScanned(state.stars, state.bulkStar);
    if (!latched || !latched->under || !BulkLeftHp(*latched))
    {
        state.bulkStar = ObjectGuid::Empty;
        latched = nullptr;
    }

    if (state.bigBangCasting || !state.nextBigBangMs)
        return nullptr;

    if (latched)
        return latched;

    AlgalonScanUnit const* next = nullptr;
    int32 work = 0;
    for (AlgalonScanUnit const& star : state.stars)
    {
        if (!star.under || !BulkLeftHp(star))
            continue;

        work += BulkWorkMs(state, star);
        if (!next || BulkLeftHp(star) > BulkLeftHp(*next))
            next = &star;
    }

    if (!next || MsUntil(state.nextBigBangMs, now) > work + ULDUAR_ALGALON_UNDER_BULK_SLACK_MS)
        return nullptr;

    state.bulkStar = next->guid;
    return next;
}

// Casts on an under-star land after the team moves on, and channels keep hitting: each stops at the star's
// stop line. A paced kill of one is left alone.
void StopCastsAtLine(Player* bot, Group* group, AlgalonEncounterState const& state, char const* window)
{
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!IsBot(member))
            continue;

        for (CurrentSpellTypes const type : {CURRENT_GENERIC_SPELL, CURRENT_CHANNELED_SPELL, CURRENT_AUTOREPEAT_SPELL})
        {
            Spell* spell = member->GetCurrentSpell(type);
            ObjectGuid const target = spell ? spell->m_targets.GetUnitTargetGUID() : ObjectGuid::Empty;
            AlgalonScanUnit const* scanned = target.IsEmpty() ? nullptr : FindScanned(state.stars, target);
            if (!scanned || !scanned->under || (target == state.focusStar && std::strcmp(window, "under")))
                continue;

            Creature* star = Resolve(bot, target);
            if (star && star->GetHealth() <= StopLineHp(*scanned) + scanned->pendingHp)
                member->InterruptSpell(type);
        }
    }
}

// A bulk span over, its average speed from past its start becomes the bulk's.
void CommitBulkSpeed(AlgalonEncounterState& state, uint32 now)
{
    AlgalonScanUnit const* star = FindScanned(state.stars, state.bulkFrom);
    uint32 const span = getMSTimeDiff(state.bulkFromMs, now);
    if (star && span >= ULDUAR_ALGALON_STAR_LOG_MS && star->healthPct < state.bulkFromPct)
    {
        float const speed = (state.bulkFromPct - star->healthPct) * IN_MILLISECONDS / span;
        state.bulkPctPerSec = std::min(speed, ULDUAR_ALGALON_STAR_KILL_PEAK_PCT_PER_SEC);
        state.bulkMeasuredCycle = state.bigBangCount;
    }

    state.bulkFrom = ObjectGuid::Empty;
}

// With a star dying under the running cast, how much earlier every hider goes in.
int32 UnderHideExtraMs(AlgalonEncounterState const& state, int32 remaining, uint32 now)
{
    if (!state.bigBangCasting || state.underStars.empty())
        return 0;

    // Drain deaths are from the last scan.
    int32 const age = state.lastTickMs ? static_cast<int32>(getMSTimeDiff(state.lastTickMs, now)) : 0;
    int32 earliest = std::numeric_limits<int32>::max();
    for (AlgalonScanUnit const& star : state.stars)
        if (star.under && star.tickHp)
            earliest = std::min(earliest, DrainDeathMs(star) - age);

    int32 extra = ULDUAR_ALGALON_UNDER_HIDE_EXTRA_MS;
    if (earliest != std::numeric_limits<int32>::max())
        extra = std::max(extra, remaining - earliest + ULDUAR_ALGALON_UNDER_DEATH_LEAD_MS -
                                    ULDUAR_ALGALON_HIDE_MARGIN_MS);

    return std::min(extra, ULDUAR_ALGALON_UNDER_HIDE_STRETCH_MS);
}
}  // namespace

void AlgalonTickEncounterState(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot || bot->GetMapId() != ULDUAR_MAP_ID)
        return;

    uint32 const instanceId = bot->GetInstanceId();
    AlgalonEncounterState* state = &algalonEncounterStates.For(instanceId);
    uint32 const now = getMSTime();
    if (state->lastTickMs && getMSTimeDiff(state->lastTickMs, now) < ULDUAR_ALGALON_STATE_TICK_MS)
        return;

    state->lastTickMs = now;

    Creature* boss = FindAlgalon(bot);

    // A wipe despawns him and summons a fresh one 2s later, and every bot is dead or phased while that
    // happens. Instance state is the only thing that notices, never one bot's view of the room.
    bool const wiped = !state->boss.IsEmpty() &&
                       (!boss || boss->GetGUID() != state->boss || (state->engagedSeen && !boss->IsInCombat()));
    if (wiped)
    {
        algalonEncounterStates.Reset(instanceId);
        state = &algalonEncounterStates.For(instanceId);
        state->lastTickMs = now;
    }

    if (!boss)
        return;

    state->boss = boss->GetGUID();
    if (boss->IsInCombat())
        state->engagedSeen = true;

    Group* group = bot->GetGroup();
    if (!group)
        return;

    // Built once: the pair and the handler both read it, and each build walks the group four times.
    std::vector<Player*> const tanks = OrderedTanks(bot);
    EnsureSwapPair(bot, boss, *state, tanks);
    EnsureTankHealers(bot, boss, *state);

    if (!state->introEndMs && boss->IsInCombat() && !boss->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
    {
        state->introEndMs = now;
        state->nextBigBangMs = now + ULDUAR_ALGALON_FIRST_BIG_BANG_MS;
        state->nextSmashImpactMs = now + ULDUAR_ALGALON_FIRST_SMASH_IMPACT_MS;
    }

    EnsureAlgalonSlotAssignments(bot, *state);

    if (boss->IsInCombat())
        ScanRoom(boss, *state, now);

    AlgalonPhase const phase = ReadPhase(boss, *state);
    state->phase = phase;
    state->engaged = phase == AlgalonPhase::Intro || phase == AlgalonPhase::One || phase == AlgalonPhase::Two;
    if (!state->engaged)
        return;

    ReleaseMelee(boss, *state, now);

    // Only the pickup itself: a pair tank first taking him later, off a non-tank, would walk him back in.
    if (!state->firstHoldMs && state->introEndMs)
    {
        Player* holder = BossHolder(boss);
        if (holder && state->swapPair.count(holder->GetGUID()))
        {
            state->firstHoldMs = now;
            if (getMSTimeDiff(state->introEndMs, now) <= ULDUAR_ALGALON_PULL_HOLD_MS)
                state->firstHolder = holder->GetGUID();
        }
    }

    // A forecast meteor that never showed moves on a cycle.
    while (state->nextSmashImpactMs && MsUntil(state->nextSmashImpactMs, now) +
                                               static_cast<int32>(ULDUAR_ALGALON_STAR_IMPACT_TAIL_MS) < 0)
        state->nextSmashImpactMs += ULDUAR_ALGALON_COSMIC_SMASH_INTERVAL_MS;

    bool const casting = CurrentBigBang(boss) != nullptr;
    if (casting && !state->bigBangCasting)
    {
        ++state->bigBangCount;
        state->nextBigBangMs = now + ULDUAR_ALGALON_BIG_BANG_INTERVAL_MS;
        state->shelterAssignments.clear();
        // The window was planned off the predicted cast; the real one's hit decides.
        if (Spell* bang = CurrentBigBang(boss))
        {
            uint32 const hit = now + static_cast<uint32>(std::max(0, bang->GetCastTimeRemaining()));
            state->underFromMs = hit - ULDUAR_ALGALON_UNDER_LEAD_MS;
            state->underToMs = hit + ULDUAR_ALGALON_UNDER_TAIL_MS;
        }

        state->nitroRunners.clear();
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (IsBot(member) && NitroBoostsReady(member))
                state->nitroRunners.insert(member->GetGUID());
        }
        ElectBigBangSoakers(bot, boss, *state);
        CallBigBang(group, boss, *state, true);
        state->humanSoakCalled = false;
    }
    else if (casting)
    {
        KeepBigBangSoaker(bot, *state);
        CallBigBang(group, boss, *state, false);
        CallHumanSoak(group, boss, *state);
    }
    else if (state->bigBangCasting)
    {
        state->lastBigBangEndMs = now;
        state->lastSoaker = state->bigBangSoaker;
        state->bigBangSoaker = ObjectGuid::Empty;
        state->bigBangBackup = ObjectGuid::Empty;
    }

    state->bigBangCasting = casting;
    state->bigBang = casting ? state->bigBangCount : 0;
    state->holeCount = static_cast<uint32>(state->holes.size());
    CountHidersOut(group, *state);

    // Held through the cast: the soak itself puts the disperser's Dispersion on cooldown.
    if (!casting)
    {
        int32 const castBy = state->nextBigBangMs ? MsUntil(state->nextBigBangMs, now) +
                                                        static_cast<int32>(ULDUAR_ALGALON_BIG_BANG_CAST_MS) -
                                                        ULDUAR_ALGALON_SOAK_DISPERSION_MS
                                                  : std::numeric_limits<int32>::max();
        Player* disperser = ElectDisperser(group, castBy, nullptr);
        state->disperser = disperser ? disperser->GetGUID() : ObjectGuid::Empty;
    }

    for (auto const& [marker, seenMs] : state->markerSeenMs)
    {
        uint32 const impact = seenMs + ULDUAR_ALGALON_COSMIC_SMASH_IMPACT_MS;
        if (MsUntil(impact, now) <= 0 && (!state->lastSmashImpactMs || MsUntil(impact, state->lastSmashImpactMs) > 0))
            state->lastSmashImpactMs = impact;
    }

    // Cast for the Divine Guardian it procs, which stays; left up, it splits the party's explosion damage onto
    // the paladin.
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (IsBot(member) && member->HasAura(SPELL_ALGALON_DIVINE_SACRIFICE, member->GetGUID()))
            member->RemoveOwnedAura(SPELL_ALGALON_DIVINE_SACRIFICE);
    }

    state->urgent = phase == AlgalonPhase::One && state->holes.empty() &&
                    (casting || BigBangDueWithin(*state, ULDUAR_ALGALON_SHELTER_WINDOW_SECONDS, now));

    Player* handler = ElectHandler(bot, *state, tanks);
    state->handler = handler ? handler->GetGUID() : ObjectGuid::Empty;

    if (phase != AlgalonPhase::Intro)
    {
        CallHumanTank(group, botAI, boss, *state, now);
        CallHumanTankCut(group, botAI, boss, *state, now);
    }

    if (phase != AlgalonPhase::One)
    {
        state->focusStar = ObjectGuid::Empty;
        state->shadowAuraPaladin = ObjectGuid::Empty;
        return;
    }

    Player* auraPaladin = ElectShadowAuraPaladin(group, state->shadowAuraPaladin);
    state->shadowAuraPaladin = auraPaladin ? auraPaladin->GetGUID() : ObjectGuid::Empty;

    ElectStarTeam(bot, *state);
    PlanUnderStars(*state, now);

    if (AlgalonScanUnit const* focus = FindScanned(state->stars, state->focusStar))
        if (focus->lossMeasured && focus->lossPctPerSec >= ULDUAR_ALGALON_STAR_KILL_MEASURED_PCT_PER_SEC)
            state->teamKillPctPerSec = std::min(focus->lossPctPerSec, ULDUAR_ALGALON_STAR_KILL_PEAK_PCT_PER_SEC);

    AlgalonScanUnit const* star = nullptr;
    char const* window = StarWindow(bot, boss, *state, now, star);
    // No paced kill now: the team takes the next under-star down towards its band.
    if (!star && !state->bigBangCasting)
    {
        if (AlgalonScanUnit const* bulk = UnderBulkStar(*state, now))
        {
            star = bulk;
            window = "under";
        }
    }
    ObjectGuid const previous = state->focusStar;
    ObjectGuid const next = star ? star->guid : ObjectGuid::Empty;
    for (auto left = state->starLeft.begin(); left != state->starLeft.end();)
        left = getMSTimeDiff(left->second.first, now) > ULDUAR_ALGALON_STAR_LOG_MS ? state->starLeft.erase(left)
                                                                                    : std::next(left);

    if (next != previous)
    {
        state->focusByDeadline = false;
        state->focusSinceMs = now;
        CommitBulkSpeed(*state, now);
        if (AlgalonScanUnit const* left = FindScanned(state->stars, previous))
            state->starLeft[previous] = {now, left->healthPct};

        state->starLeft.erase(next);
    }

    // One way: a kill taken again at a deadline keeps the deadline's looser position test from then on.
    if (star && !std::strcmp(window, "deadline"))
        state->focusByDeadline = true;

    state->focusStar = next;
    StopCastsAtLine(bot, group, *state, window);

    // From past the walk in and the first casts, which BulkWorkMs charges apart.
    if (star && !std::strcmp(window, "under") && state->bulkFrom.IsEmpty() &&
        getMSTimeDiff(state->focusSinceMs, now) >= static_cast<uint32>(ULDUAR_ALGALON_UNDER_SWITCH_MS))
    {
        state->bulkFrom = star->guid;
        state->bulkFromMs = now;
        state->bulkFromPct = star->healthPct;
    }
}

bool AlgalonBigBangCasting(PlayerbotAI* botAI) { return CurrentBigBang(GetAlgalon(botAI)) != nullptr; }

int32 AlgalonBigBangRemainingMs(PlayerbotAI* botAI)
{
    Spell* spell = CurrentBigBang(GetAlgalon(botAI));
    return spell ? spell->GetCastTimeRemaining() : -1;
}

bool AlgalonBigBangWithin(PlayerbotAI* botAI, uint32 seconds)
{
    AlgalonEncounterState const* state = FindState(botAI ? botAI->GetBot() : nullptr);
    return state && BigBangDueWithin(*state, seconds, getMSTime());
}

bool AlgalonBigBangLatched(Player* bot)
{
    AlgalonEncounterState const* state = FindState(bot);
    return state && state->engaged && state->bigBangCasting;
}

int32 AlgalonMsToBigBangHit(PlayerbotAI* botAI)
{
    AlgalonEncounterState const* state = FindState(botAI ? botAI->GetBot() : nullptr);
    if (!state || !state->engaged || !state->nextBigBangMs)
        return std::numeric_limits<int32>::max();

    if (state->bigBangCasting)
        return 0;

    // Signed: a cast running late is imminent, not a cycle away.
    int32 const left = static_cast<int32>(state->nextBigBangMs - getMSTime()) +
                       static_cast<int32>(ULDUAR_ALGALON_BIG_BANG_CAST_MS);
    return std::max(0, left);
}

Player* GetAlgalonRescueTarget(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    AlgalonEncounterState const* state = FindState(bot);
    if (!state || !state->engaged || !state->lastBigBangEndMs ||
        getMSTimeDiff(state->lastBigBangEndMs, getMSTime()) > ULDUAR_ALGALON_RESCUE_WINDOW_MS)
        return nullptr;

    Player* soaker = FindGroupMember(bot->GetGroup(), state->lastSoaker);
    return CanStayOut(soaker) && soaker->GetHealthPct() < ULDUAR_ALGALON_RESCUE_HP_PCT ? soaker : nullptr;
}

char const* GetAlgalonSoakerExternal(PlayerbotAI* botAI, Player* soaker)
{
    if (!botAI || !soaker)
        return nullptr;

    if (!soaker->HasAura(SPELL_ALGALON_PAIN_SUPPRESSION) && botAI->CanCastSpell("pain suppression", soaker))
        return "pain suppression";

    if (!soaker->HasAura(SPELL_ALGALON_GUARDIAN_SPIRIT) && botAI->CanCastSpell("guardian spirit", soaker))
        return "guardian spirit";

    if (!soaker->HasAura(SPELL_ALGALON_HAND_OF_SACRIFICE) && botAI->CanCastSpell("hand of sacrifice", soaker))
        return "hand of sacrifice";

    return nullptr;
}

char const* GetAlgalonSoakerPrep(PlayerbotAI* botAI, Player* soaker)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot || !soaker)
        return nullptr;

    std::vector<char const*> layers;
    switch (bot->getClass())
    {
        case CLASS_PRIEST:
            if (!soaker->HasAura(SPELL_ALGALON_WEAKENED_SOUL))
                layers.push_back("power word: shield");
            layers.push_back("renew");
            break;
        case CLASS_DRUID:
            layers = {"rejuvenation", "lifebloom"};
            break;
        case CLASS_SHAMAN:
            layers.push_back("riptide");
            // One Earth Shield per target, whoever cast it.
            if (!botAI->HasAura("earth shield", soaker))
                layers.push_back("earth shield");
            break;
        case CLASS_PALADIN:
            layers.push_back("sacred shield");
            break;
        default:
            break;
    }

    for (char const* layer : layers)
        if (!botAI->HasAura(layer, soaker, false, true) && botAI->CanCastSpell(layer, soaker))
            return layer;

    return nullptr;
}

char const* GetAlgalonRescueSpell(PlayerbotAI* botAI, Player* soaker)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot || !soaker)
        return nullptr;

    // Instants first: the next swing lands about a second after the raid is back.
    std::vector<char const*> heals;
    switch (bot->getClass())
    {
        case CLASS_PALADIN:
            if (soaker->GetHealthPct() < ULDUAR_ALGALON_RESCUE_LAY_ON_HANDS_PCT)
                heals.push_back("lay on hands");
            heals.push_back("holy shock");
            heals.push_back("flash of light");
            break;
        case CLASS_SHAMAN:
            heals = {"riptide", "lesser healing wave"};
            break;
        case CLASS_DRUID:
            heals = {"swiftmend", "nourish"};
            break;
        case CLASS_PRIEST:
            heals.push_back("penance");
            if (!soaker->HasAura(SPELL_ALGALON_WEAKENED_SOUL))
                heals.push_back("power word: shield");
            heals.push_back("flash heal");
            break;
        default:
            break;
    }

    for (char const* heal : heals)
        if (botAI->CanCastSpell(heal, soaker))
            return heal;

    return nullptr;
}

bool IsAlgalonTankHealer(Player* bot)
{
    AlgalonEncounterState const* state = FindState(bot);
    return state && state->engaged && state->tankHealers.count(bot->GetGUID());
}

AlgalonTankHealPick GetAlgalonTankHealPick(PlayerbotAI* botAI)
{
    AlgalonTankHealPick pick;
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    AlgalonEncounterState const* state = FindState(bot);
    Group* group = bot ? bot->GetGroup() : nullptr;
    if (!state || !group || !state->engaged || !state->tankHealers.count(bot->GetGUID()) || IsAlgalonPhased(bot))
        return pick;

    std::vector<Player*> pair;
    for (ObjectGuid const& guid : state->swapPair)
        if (Player* tank = FindGroupMember(group, guid))
            pair.push_back(tank);

    // Loose, or on someone outside the pair: the lower pair tank is the one he lands on next.
    Player* holder = GetAlgalonBossTank(botAI);
    if (std::find(pair.begin(), pair.end(), holder) == pair.end())
    {
        holder = nullptr;
        for (Player* tank : pair)
            if (CanStayOut(tank) && (!holder || tank->GetHealthPct() < holder->GetHealthPct()))
                holder = tank;
    }

    Player* partner = nullptr;
    for (Player* tank : pair)
        if (tank != holder)
            partner = tank;

    // Sight is a raycast: one per tank, and only for a tank whose health already asks for a heal.
    std::optional<bool> holderReach;
    std::optional<bool> partnerReach;
    auto const reachable = [&](Player* tank)
    {
        if (!tank)
            return false;

        std::optional<bool>& reach = tank == holder ? holderReach : partnerReach;
        if (!reach)
            reach = TankHealReachable(bot, tank);

        return *reach;
    };
    auto const below = [](Player* unit, float pct) { return unit && unit->GetHealthPct() < pct; };

    if (below(holder, ULDUAR_ALGALON_TANK_CRITICAL_PCT) && reachable(holder))
        pick = {holder, true};
    else if (below(bot, ULDUAR_ALGALON_TANK_HEALER_SELF_PCT))
        pick = {bot, true};
    else if (below(partner, ULDUAR_ALGALON_TANK_CRITICAL_PCT) && reachable(partner))
        pick = {partner, true};
    else if (below(holder, ULDUAR_ALGALON_HOLDER_URGENT_PCT) && reachable(holder))
        pick = {holder, true};
    else
    {
        bool const holderNeeds = below(holder, ULDUAR_ALGALON_HOLDER_TOPOFF_PCT) && reachable(holder);
        bool const partnerNeeds = below(partner, ULDUAR_ALGALON_OFFTANK_TOPOFF_PCT) && reachable(partner);
        bool const partnerFirst = partnerNeeds && holderNeeds &&
                                  partner->GetHealthPct() + ULDUAR_ALGALON_OFFTANK_MARGIN_PCT <= holder->GetHealthPct();
        if (holderNeeds && !partnerFirst)
            pick.target = holder;
        else if (partnerNeeds)
            pick.target = partner;
    }

    return pick;
}

// Never Lay on Hands, Pain Suppression, Guardian Spirit or Hand of Sacrifice: the tank defensive node spends
// those one at a time.
char const* GetAlgalonTankHealSpell(PlayerbotAI* botAI, AlgalonTankHealPick const& pick)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    Player* target = pick.target;
    if (!bot || !target)
        return nullptr;

    bool const onHolder = target != bot && GetAlgalonBossTank(botAI) == target;
    bool const shieldable = !target->HasAura(SPELL_ALGALON_WEAKENED_SOUL);
    std::vector<char const*> spells;
    switch (bot->getClass())
    {
        case CLASS_PALADIN:
            if (pick.critical)
            {
                // Flash goes ahead of Holy Light only when Infusion of Light makes it instant. Hard cast
                // it heals about a third as much for nearly the same cast time.
                bool const instantFlash = !CastTimeMs(botAI, "flash of light");
                spells = {"holy shock", instantFlash ? "flash of light" : "holy light",
                          instantFlash ? "holy light" : "flash of light"};
            }
            else
            {
                // One Sacred Shield per paladin, so it follows the holder.
                if (onHolder)
                    spells.push_back("sacred shield");
                spells.insert(spells.end(), {"holy light", "flash of light"});
            }
            break;
        case CLASS_PRIEST:
            if (shieldable)
                spells.push_back("power word: shield");
            if (pick.critical)
                spells.insert(spells.end(), {"penance", "flash heal"});
            else
                spells.insert(spells.end(), {"prayer of mending", "renew", "penance", "greater heal", "flash heal"});
            break;
        case CLASS_DRUID:
            if (pick.critical)
                spells = {"swiftmend", "nourish", "regrowth"};
            else
                spells = {"lifebloom", "rejuvenation", "nourish", "regrowth"};
            break;
        case CLASS_SHAMAN:
            if (pick.critical)
                spells = {"riptide", "lesser healing wave", "healing wave"};
            else
            {
                // One Earth Shield per target, whoever cast it.
                if (onHolder && !botAI->HasAura("earth shield", target))
                    spells.push_back("earth shield");
                spells.insert(spells.end(), {"riptide", "healing wave", "lesser healing wave"});
            }
            break;
        default:
            break;
    }

    static std::set<std::string> const layers = {"sacred shield", "prayer of mending", "renew", "rejuvenation",
                                                 "riptide"};
    int32 const slack = AlgalonHideSlackMs(bot);
    for (char const* spell : spells)
    {
        if (layers.count(spell) && botAI->HasAura(spell, target, false, true))
            continue;

        if (!std::strcmp(spell, "lifebloom") && botAI->HasAura(spell, target, true, true))
            continue;

        // Its walk to the hole would cut the cast off.
        if (slack != std::numeric_limits<int32>::max() &&
            static_cast<int32>(CastTimeMs(botAI, spell)) + ULDUAR_ALGALON_PREP_GCD_MS > slack)
            continue;

        if (botAI->CanCastSpell(spell, target))
            return spell;
    }

    return nullptr;
}

Player* GetAlgalonBeaconTarget(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    AlgalonEncounterState const* state = FindState(bot);
    Group* group = bot ? bot->GetGroup() : nullptr;
    if (!state || !group || bot->getClass() != CLASS_PALADIN || !IsAlgalonTankHealer(bot))
        return nullptr;

    // Before the pickup the pickup tank is the one he lands on.
    Player* holder = GetAlgalonBossTank(botAI);
    if (!holder || !state->swapPair.count(holder->GetGUID()))
        holder = GetAlgalonPickupTank(botAI);

    Player* target = nullptr;
    for (ObjectGuid const& guid : state->swapPair)
    {
        Player* tank = FindGroupMember(group, guid);
        if (tank && tank != holder && TankHealReachable(bot, tank))
            target = tank;
    }

    // A lone holder still gets the paladin's self heals through it.
    if (!target && holder && TankHealReachable(bot, holder))
        target = holder;

    return target && !botAI->HasAura("beacon of light", target, false, true) ? target : nullptr;
}

bool AlgalonDispersionSoak(Player* bot)
{
    AlgalonEncounterState const* state = FindState(bot);
    return state && state->engaged && !state->disperser.IsEmpty();
}

namespace
{
Player* ExplosionCoverProvider(PlayerbotAI* botAI, char const*& cover);
}  // namespace

AlgalonTankCut GetAlgalonTankCut(PlayerbotAI* botAI)
{
    AlgalonTankCut const none;
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    AlgalonEncounterState const* state = FindState(bot);
    Group* group = bot ? bot->GetGroup() : nullptr;
    if (!state || !group || !state->engaged || IsAlgalonPhased(bot))
        return none;

    // Every provider is the holder, a paladin or a priest; the rest of the raid asks this every tick.
    Player* holder = GetAlgalonBossTank(botAI);
    if (!holder || !IsAlgalonSwapTank(holder) || !CanStayOut(holder) ||
        (bot != holder && bot->getClass() != CLASS_PALADIN && bot->getClass() != CLASS_PRIEST))
        return none;

    // Nothing reaches a hiding holder from the cast's start until he moves again; a tank that soaked is still in
    // his reach.
    uint32 const now = getMSTime();
    if (state->bigBangCasting ||
        (state->lastBigBangEndMs && getMSTimeDiff(state->lastBigBangEndMs, now) < ULDUAR_ALGALON_BIG_BANG_STILL_MS &&
         state->lastSoaker != holder->GetGUID()))
        return none;

    auto const claimed = [now](uint32 claimMs)
    { return claimMs && getMSTimeDiff(claimMs, now) < ULDUAR_ALGALON_TANK_DEFENSIVE_CLAIM_MS; };

    // With no disperser a tank soaks the next hit, and what it and its externals need then stays unspent unless
    // back in time, or the holder is alone and dying first soaks nothing.
    int32 const untilHit = AlgalonMsToBigBangHit(botAI);
    bool const reserved = state->disperser.IsEmpty() && AlgalonLoneHolder(botAI) != holder;
    auto const usable = [&](Player* provider, uint32 spellId)
    { return spellId && (!reserved || AlgalonCooldownMs(provider, spellId) <= untilHit); };

    // The reserve goes ahead of range, sight and the cast check: every provider asks this of every other one
    // each tick, and a reserved spell is refused again on the next.
    auto const ready = [&](Player* provider, uint32 spellId, Player* target)
    { return usable(provider, spellId) && ExternalReaches(provider, spellId, target) ? spellId : 0; };

    // A full heal, so over a cutter too. Another paladin's first: on himself it puts up Forbearance.
    if (holder->GetHealthPct() < ULDUAR_ALGALON_TANK_LAY_ON_HANDS_PCT && !claimed(state->layOnHandsClaimMs))
    {
        // Forbearance from either locks the other out, and an unspent Divine Protection is lost to a self Lay on
        // Hands for two minutes.
        if (IsBot(holder) && !TankCutRunning(holder) && !claimed(state->tankCutClaimMs))
        {
            if (uint32 const spellId = ready(holder, ExternalSpell(holder, "divine protection"), holder))
                return holder == bot ? AlgalonTankCut{holder, spellId, "low"} : none;
        }

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* paladin = ref->GetSource();
            if (paladin == holder || !IsBot(paladin) || paladin->getClass() != CLASS_PALADIN)
                continue;

            if (uint32 const spellId = ready(paladin, ExternalSpell(paladin, "lay on hands"), holder))
                return paladin == bot ? AlgalonTankCut{holder, spellId, "loh"} : none;
        }

        if (IsBot(holder))
        {
            if (uint32 const spellId = ready(holder, ExternalSpell(holder, "lay on hands"), holder))
                return holder == bot ? AlgalonTankCut{holder, spellId, "loh"} : none;
        }
    }

    if (TankCutRunning(holder) || claimed(state->tankCutClaimMs))
        return none;

    Player* partner = PairPartner(group, *state, holder);
    char const* why = TankCutReason(*state, holder, partner, now);
    if (!why)
        return none;

    auto const turn = [&](Player* provider, uint32 spellId)
    { return provider == bot ? AlgalonTankCut{holder, spellId, why} : none; };

    // The paladin due to cover the same explosion keeps its global for that: both nodes can fire in one tick.
    char const* cover = nullptr;
    Player* coverer = !std::strcmp(why, "window") ? ExplosionCoverProvider(botAI, cover) : nullptr;

    // The partner is idle in plate, so the 30% it takes over costs it least.
    if (IsBot(partner) && partner != coverer)
    {
        if (uint32 const spellId = ready(partner, SacrificeSpell(partner, holder), holder))
            return turn(partner, spellId);
    }

    // Off cooldown within a global cooldown counts: the externals wait for it rather than spend a longer one.
    if (IsBot(holder) && holder != coverer)
    {
        if (char const* own = ReadyTankDefensive(holder, true, ULDUAR_ALGALON_PREP_GCD_MS))
        {
            PlayerbotAI* holderAI = GET_PLAYERBOT_AI(holder);
            uint32 const ownId = holderAI->GetAiObjectContext()->GetValue<uint32>("spell id", own)->Get();
            SpellInfo const* info = ownId ? sSpellMgr->GetSpellInfo(ownId) : nullptr;
            if (usable(holder, ownId) && info && CanPay(holder, info))
            {
                uint32 const spellId = holder == bot ? NextTankDefensive(botAI, bot, true) : 0;
                return spellId ? AlgalonTankCut{holder, spellId, why} : none;
            }
        }
    }

    for (bool const healers : {false, true})
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* paladin = ref->GetSource();
            if (!paladin || paladin->getClass() != CLASS_PALADIN || paladin == partner || paladin == coverer ||
                !IsBot(paladin) || PlayerbotAI::IsHeal(paladin) != healers)
                continue;

            if (uint32 const spellId = ready(paladin, SacrificeSpell(paladin, holder), holder))
                return turn(paladin, spellId);
        }
    }

    for (char const* spell : {"pain suppression", "guardian spirit"})
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* priest = ref->GetSource();
            if (!priest || priest->getClass() != CLASS_PRIEST || !IsBot(priest))
                continue;

            if (uint32 const spellId = ready(priest, ExternalSpell(priest, spell), holder))
                return turn(priest, spellId);
        }
    }

    return none;
}

int32 AlgalonCooldownMs(Player* bot, uint32 spellId)
{
    SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
    if (!info)
        return 0;

    int32 recovery = static_cast<int32>(info->RecoveryTime);
    int32 category = static_cast<int32>(info->CategoryRecoveryTime);
    if (recovery > 0)
        bot->ApplySpellMod(spellId, SPELLMOD_COOLDOWN, recovery);

    if (category > 0 && !info->HasAttribute(SPELL_ATTR6_NO_CATEGORY_COOLDOWN_MODS))
        bot->ApplySpellMod(spellId, SPELLMOD_COOLDOWN, category);

    return std::max(recovery, category);
}

void ClaimAlgalonTankCut(Player* bot, AlgalonTankCut const& cut)
{
    AlgalonEncounterState* state = FindState(bot);
    if (!state || !cut.target)
        return;

    bool const layOnHands = !std::strcmp(cut.why, "loh");
    (layOnHands ? state->layOnHandsClaimMs : state->tankCutClaimMs) = getMSTime();
}

namespace
{
// An under-star dies on its drain, any other on its slope.
int32 StarDeathMs(AlgalonEncounterState const& state, AlgalonScanUnit const& star)
{
    return star.under && star.tickHp ? DrainDeathMs(star)
                                     : ExplosionDueMs(state, star, ULDUAR_ALGALON_EXPLOSION_LOW_PCT);
}

// Who covers the star about to explode, and with what: one per star while it lasts, none while one is up. In the
// cast only those still outside a field when it dies count.
Player* ExplosionCoverProvider(PlayerbotAI* botAI, char const*& cover)
{
    cover = nullptr;
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    AlgalonEncounterState const* state = FindState(bot);
    Group* group = bot ? bot->GetGroup() : nullptr;
    if (!state || !group || state->phase != AlgalonPhase::One)
        return nullptr;

    bool const casting = state->bigBangCasting;
    AlgalonScanUnit const* star = ExplodingStar(*state, ULDUAR_ALGALON_EXPLOSION_COVER_LEAD_MS,
                                                ULDUAR_ALGALON_EXPLOSION_COVER_HP_PCT, bot, casting);
    if (!star || ExplosionCovered(group, *state) ||
        (star->guid == state->coverStar &&
         getMSTimeDiff(state->coverClaimMs, getMSTime()) < ULDUAR_ALGALON_EXPLOSION_COVER_MS))
        return nullptr;

    int32 const death = StarDeathMs(*state, *star);
    auto const hit = [state, casting, death](Player* member)
    { return casting ? CaughtOutside(*state, member, death) : CanStayOut(member); };
    if (RaidUnderExplosionLine(bot, group, hit) < ULDUAR_ALGALON_EXPLOSION_COVER_MIN_UNDER)
        return nullptr;

    // Its own aura is the one it doubles, so only with that aura up.
    Player* auraPaladin = FindGroupMember(group, state->shadowAuraPaladin);
    PlayerbotAI* auraAI = auraPaladin ? GET_PLAYERBOT_AI(auraPaladin) : nullptr;
    if (auraAI && !IsAlgalonPhased(auraPaladin) &&
        auraAI->HasAura("shadow resistance aura", auraPaladin, false, true) &&
        ExternalReady(auraPaladin, "aura mastery", auraPaladin))
    {
        cover = "aura mastery";
        return auraPaladin;
    }

    // The holder last: its globals go on him. It pays one, since the tick strips 64205 at once.
    Player* holder = GetAlgalonBossTank(botAI);
    for (bool const holderTurn : {false, true})
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* paladin = ref->GetSource();
            if ((paladin == holder) != holderTurn || !IsBot(paladin) || paladin->getClass() != CLASS_PALADIN ||
                IsAlgalonPhased(paladin) || !HasDivineGuardian(paladin) ||
                paladin->GetHealthPct() < ULDUAR_ALGALON_SACRIFICE_MIN_PCT)
                continue;

            if (ExternalReady(paladin, "divine sacrifice", paladin))
            {
                cover = "divine sacrifice";
                return paladin;
            }
        }
    }

    return nullptr;
}
}  // namespace

char const* GetAlgalonExplosionCover(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot || bot->getClass() != CLASS_PALADIN)
        return nullptr;

    char const* cover = nullptr;
    return ExplosionCoverProvider(botAI, cover) == bot ? cover : nullptr;
}

void ClaimAlgalonExplosionCover(Player* bot, char const* /*cover*/)
{
    AlgalonEncounterState* state = FindState(bot);
    AlgalonScanUnit const* star = state ? ExplodingStar(*state, ULDUAR_ALGALON_EXPLOSION_COVER_LEAD_MS,
                                                        ULDUAR_ALGALON_EXPLOSION_COVER_HP_PCT, bot,
                                                        state->bigBangCasting)
                                        : nullptr;
    if (!star)
        return;

    state->coverStar = star->guid;
    state->coverClaimMs = getMSTime();
}

namespace
{
struct PersonalButton
{
    char const* spell;
    // By id where the name lookup misses: it compares the DBC name, and "anti magic shell" isn't "Anti-Magic Shell".
    uint32 spellId;
    // What is left of the hit, by factor and by flat absorb.
    float factor;
    uint32 absorb;
    // Keeps the field's phase off, so never in the cast: the hide run strips it.
    bool immunity = false;
};

// Cheapest cooldown first, so a bubble or a block only goes when nothing shorter will do. Cloak (90% to resist,
// not a cut) sits behind sure ones of the same cooldown.
PersonalButton const PERSONAL_BUTTONS[] = {
    {"shadow ward", 0, 1.0f, 3300},
    {"anti magic shell", SPELL_ALGALON_ANTI_MAGIC_SHELL, 0.25f, 0, true},
    {"barkskin", 0, 0.8f, 0},
    {"shamanistic rage", 0, 0.7f, 0},
    {"cloak of shadows", 0, 0.1f, 0, true},
    {"icebound fortitude", 0, 0.7f, 0},
    {"dispersion", 0, 0.1f, 0},
    {"divine protection", 0, 0.5f, 0},
    {"ice block", 0, 0.0f, 0, true},
    {"divine shield", 0, 0.0f, 0, true},
};

uint32 PersonalButtonId(Player* bot, PersonalButton const& button)
{
    if (button.spellId)
        return bot->HasSpell(button.spellId) ? button.spellId : 0;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    return botAI ? botAI->GetAiObjectContext()->GetValue<uint32>("spell id", button.spell)->Get() : 0;
}

// How long the bot stays covered: Divine Guardian on it, or within 40 yd of the shadow aura paladin under Aura
// Mastery. 0 when neither.
int32 ExplosionCoverMs(Player* bot, Group* group, AlgalonEncounterState const& state)
{
    int32 left = 0;
    if (Aura const* guardian = bot->GetAura(SPELL_ALGALON_DIVINE_GUARDIAN))
        left = guardian->GetDuration();

    Player* auraPaladin = FindGroupMember(group, state.shadowAuraPaladin);
    Aura const* mastery = auraPaladin ? auraPaladin->GetAura(SPELL_ALGALON_AURA_MASTERY) : nullptr;
    if (mastery && bot->IsWithinDistInMap(auraPaladin, ULDUAR_ALGALON_AURA_RANGE))
        left = std::max(left, mastery->GetDuration());

    return left;
}

bool ItemCoolingDown(Player* bot, Item* item)
{
    ItemTemplate const* proto = item->GetTemplate();
    for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
        if (proto->Spells[i].SpellTrigger == ITEM_SPELLTRIGGER_ON_USE && proto->Spells[i].SpellId > 0 &&
            bot->HasSpellCooldown(static_cast<uint32>(proto->Spells[i].SpellId)))
            return true;

    return false;
}

// A healthstone, else a healing potion, usable now: all a third of the raid has against the hit. Not the
// healthstone on the move: using it stops the bot, mid dodge too.
char const* ReadyConsumable(PlayerbotAI* botAI)
{
    AiObjectContext* context = botAI->GetAiObjectContext();
    bool const moving = botAI->GetBot()->isMoving();
    for (char const* name : {"healthstone", "healing potion"})
    {
        if (moving && !std::strcmp(name, "healthstone"))
            continue;

        Action* action = context->GetAction(name);
        if (!action || !action->isPossible() || !action->isUseful())
            continue;

        std::vector<Item*> const items = context->GetValue<std::vector<Item*>>("inventory items", name)->Get();
        if (!items.empty() && !ItemCoolingDown(botAI->GetBot(), items.front()))
            return name;
    }

    return nullptr;
}
}  // namespace

AlgalonPersonalPick GetAlgalonExplosionPersonal(PlayerbotAI* botAI)
{
    AlgalonPersonalPick pick;
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    AlgalonEncounterState const* state = FindState(bot);
    Group* group = bot ? bot->GetGroup() : nullptr;
    if (!state || !group || state->phase != AlgalonPhase::One || IsAlgalonPhased(bot) ||
        state->swapPair.count(bot->GetGUID()))
        return pick;

    // In the cast, only a star going off before the bot is in a field: the rest hit the void.
    bool const casting = state->bigBangCasting;
    AlgalonScanUnit const* star = ExplodingStar(*state, ULDUAR_ALGALON_EXPLOSION_PERSONAL_LEAD_MS,
                                                ULDUAR_ALGALON_EXPLOSION_LOW_PCT, bot, casting);
    if (!star || (casting && !CaughtOutside(*state, bot, StarDeathMs(*state, *star))))
        return pick;

    auto const pressed = state->personalStars.find(bot->GetGUID());
    if (pressed != state->personalStars.end() && pressed->second == star->guid)
        return pick;

    float const topRoll = static_cast<float>(
        bot->GetMap()->Is25ManRaid() ? ULDUAR_ALGALON_STAR_EXPLOSION_MAX_25 : ULDUAR_ALGALON_STAR_EXPLOSION_MAX_10);
    float const hit = StackedHit(*state, bot, *star, topRoll, ExplosionCoverMs(bot, group, *state), casting);
    float const health = static_cast<float>(bot->GetHealth());
    if (health > hit)
        return pick;

    // One already running: two stars close together would otherwise spend a second one.
    std::array<uint32, std::size(PERSONAL_BUTTONS)> spellIds;
    for (size_t i = 0; i < spellIds.size(); ++i)
    {
        spellIds[i] = PersonalButtonId(bot, PERSONAL_BUTTONS[i]);
        if (spellIds[i] && bot->HasAura(spellIds[i]))
            return pick;
    }

    int32 const untilHit = AlgalonMsToBigBangHit(botAI);
    // The top roll is the worst case, so with no button clearing it the strongest still goes up for a lower roll.
    uint32 strongest = 0;
    float strongestLeft = std::numeric_limits<float>::max();
    for (size_t i = 0; i < spellIds.size(); ++i)
    {
        PersonalButton const& button = PERSONAL_BUTTONS[i];
        float const left = hit * button.factor - button.absorb;
        if ((left >= health && left >= strongestLeft) || (casting && button.immunity))
            continue;

        uint32 const spellId = spellIds[i];
        SpellInfo const* info = spellId ? sSpellMgr->GetSpellInfo(spellId) : nullptr;
        if (!info || !CanPay(bot, info) || !botAI->CanCastSpell(spellId, bot))
            continue;

        // The soaker's, and a backup's that wouldn't be back for the hit: the reserve's own test.
        if (!std::strcmp(button.spell, "dispersion") &&
            (state->disperser == bot->GetGUID() || untilHit == 0 ||
             AlgalonCooldownMs(bot, spellId) > untilHit - ULDUAR_ALGALON_SOAK_DISPERSION_MS))
            continue;

        if (left < health)
        {
            pick.spellId = spellId;
            return pick;
        }

        strongest = spellId;
        strongestLeft = left;
    }

    // Only where the heal lands whole and lifts the bot over the line: a full bot under it would burn its one
    // potion for nothing.
    float const heal = static_cast<float>(ULDUAR_ALGALON_EXPLOSION_CONSUMABLE_HEAL);
    if (static_cast<float>(bot->GetMaxHealth()) - health >= heal && health + heal > hit)
        pick.consumable = ReadyConsumable(botAI);

    if (!pick.consumable)
        pick.spellId = strongest;

    return pick;
}

void ClaimAlgalonExplosionPersonal(Player* bot)
{
    AlgalonEncounterState* state = FindState(bot);
    AlgalonScanUnit const* star = state ? ExplodingStar(*state, ULDUAR_ALGALON_EXPLOSION_PERSONAL_LEAD_MS,
                                                        ULDUAR_ALGALON_EXPLOSION_LOW_PCT, bot, state->bigBangCasting)
                                        : nullptr;
    if (!star)
        return;

    state->personalStars[bot->GetGUID()] = star->guid;
}

bool IsAlgalonShadowAuraPaladin(Player* bot)
{
    AlgalonEncounterState const* state = FindState(bot);
    return state && state->engaged && state->shadowAuraPaladin == bot->GetGUID();
}

Player* GetAlgalonBigBangSoaker(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    AlgalonEncounterState const* state = FindState(bot);
    return state ? FindGroupMember(bot->GetGroup(), state->bigBangSoaker) : nullptr;
}

Player* GetAlgalonBigBangBackup(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    AlgalonEncounterState const* state = FindState(bot);
    return state ? FindGroupMember(bot->GetGroup(), state->bigBangBackup) : nullptr;
}

bool AlgalonHoldsConstellation(Player* bot)
{
    AlgalonEncounterState const* state = FindState(bot);
    if (!state)
        return false;

    for (AlgalonScanUnit const& constellation : state->constellations)
        if (constellation.active && constellation.victim == bot->GetGUID())
            return true;

    return false;
}

bool AlgalonStaysOut(Player* bot)
{
    AlgalonEncounterState const* state = FindState(bot);
    if (!state)
        return false;

    ObjectGuid const guid = bot->GetGUID();
    return state->bigBangSoaker == guid || state->bigBangBackup == guid;
}

bool AlgalonTankCaughtOut(Player* bot)
{
    AlgalonEncounterState const* state = FindState(bot);
    if (!state || (!state->swapPair.count(bot->GetGUID()) && state->handler != bot->GetGUID()))
        return false;

    return GetAlgalonHideRole(bot) == AlgalonHideRole::NoShelter;
}

bool AlgalonCanDisperse(Player* bot)
{
    return bot && bot->HasSpell(SPELL_ALGALON_DISPERSION) && !bot->HasSpellCooldown(SPELL_ALGALON_DISPERSION);
}

namespace
{
// Boots not yet used run the first stretch boosted. Once used, the bot's speed already carries them.
int32 HideWalkMs(AlgalonEncounterState const& state, Player* bot, float walk)
{
    float const speed = std::max(1.0f, bot->GetSpeed(MOVE_RUN));
    int32 walkMs = static_cast<int32>(walk / speed * IN_MILLISECONDS);
    if (state.nitroRunners.count(bot->GetGUID()) && !bot->HasAura(NITRO_BOOSTS_SPEED_SPELL) &&
        walk > ULDUAR_ALGALON_NITRO_MIN_WALK)
    {
        float const fast = speed * NITRO_BOOSTS_SPEED_FACTOR;
        float const boosted = fast * NITRO_BOOSTS_DURATION_MS / IN_MILLISECONDS;
        walkMs = walk <= boosted
                     ? static_cast<int32>(walk / fast * IN_MILLISECONDS)
                     : static_cast<int32>(NITRO_BOOSTS_DURATION_MS + (walk - boosted) / speed * IN_MILLISECONDS);
        // Never under the unboosted cutoff walk, or a step inside it adds most of a second at once.
        walkMs = std::max(walkMs, static_cast<int32>(ULDUAR_ALGALON_NITRO_MIN_WALK / speed * IN_MILLISECONDS));
    }

    return walkMs;
}

// One never timed reads as landed a marker's lifetime ago.
int32 SmashImpactMs(AlgalonEncounterState const& state, AlgalonScanUnit const& marker, uint32 now)
{
    auto const seen = state.markerSeenMs.find(marker.guid);
    return seen == state.markerSeenMs.end() ? -static_cast<int32>(ULDUAR_ALGALON_MARKER_LIFETIME_MS)
                                            : MsUntil(seen->second + ULDUAR_ALGALON_COSMIC_SMASH_IMPACT_MS, now);
}

// Where the walker is `ms` into a walk of `walk` yd from `from` towards `to` that takes `walkMs`.
Position WalkPoint(Position const& from, Position const& to, float walk, int32 ms, int32 walkMs)
{
    float const length = from.GetExactDist2d(to);
    float const share = walkMs > 0 ? std::clamp(static_cast<float>(ms) / walkMs, 0.0f, 1.0f) : 1.0f;
    float const t = length > 0.0f ? std::min(1.0f, walk * share / length) : 0.0f;
    return Position(from.GetPositionX() + (to.GetPositionX() - from.GetPositionX()) * t,
                    from.GetPositionY() + (to.GetPositionY() - from.GetPositionY()) * t, to.GetPositionZ());
}

// A meteor landing inside the doubled damage ring of the walker, on a straight walk of `walk` yd from `from` towards
// `to` set off in `startMs`. Before the walk the bot stands where it dodged. After it, it's on the next leg, or at
// its hole for `tailMs` until the field's pulse phases it.
AlgalonScanUnit const* SmashAcrossPath(AlgalonEncounterState const& state, Position const& from, Position const& to,
                                       float walk, int32 startMs, int32 walkMs, int32 tailMs, uint32 now)
{
    for (AlgalonScanUnit const& marker : state.markers)
    {
        // Past its nominal time it may still be up to the spread from landing.
        int32 const into = SmashImpactMs(state, marker, now) - startMs;
        if (into <= -ULDUAR_ALGALON_COSMIC_SMASH_IMPACT_SPREAD_MS || into >= walkMs + tailMs)
            continue;

        Position const at = WalkPoint(from, to, walk, into, walkMs);
        Position const late = WalkPoint(from, to, walk, into + ULDUAR_ALGALON_COSMIC_SMASH_IMPACT_SPREAD_MS, walkMs);
        if (SegmentDistance(at, late.GetPositionX(), late.GetPositionY(), marker.position) <
            ULDUAR_ALGALON_COSMIC_SMASH_MIN_CLEARANCE)
            return &marker;
    }

    return nullptr;
}

// A leg passing a field other than the one at `to`, which would phase the walker early, away from its own hole.
bool ClipsOtherHole(AlgalonEncounterState const& state, Position const& from, Position const& via, Position const& to)
{
    for (AlgalonScanUnit const& hole : state.holes)
        if (hole.position.GetExactDist2d(to) > 0.5f &&
            SegmentDistance(from, via.GetPositionX(), via.GetPositionY(), hole.position) <
                ULDUAR_ALGALON_HOLE_PATH_CLEARANCE)
            return true;

    return false;
}

// The shortest walk round the meteor through a point on a ring about its impact, set off in `startMs`: no meteor
// lands by the walker on either leg, and neither clips a field but the hole it heads for.
bool FindSmashDetour(Player* bot, AlgalonEncounterState const& state, AlgalonScanUnit const& smash, Position const& to,
                     int32 startMs, uint32 now, Position& waypoint, float& length)
{
    Position const from = bot->GetPosition();
    float best = std::numeric_limits<float>::max();
    for (uint8 step = 0; step < ULDUAR_ALGALON_SWEEP_BEARINGS; ++step)
    {
        float const angle = static_cast<float>(step) * ULDUAR_ALGALON_SWEEP_ANGLE_STEP;
        float const x = smash.position.GetPositionX() + std::cos(angle) * ULDUAR_ALGALON_SMASH_DETOUR_RADIUS;
        float const y = smash.position.GetPositionY() + std::sin(angle) * ULDUAR_ALGALON_SMASH_DETOUR_RADIUS;
        if (!AlgalonSpotInRoom(x, y) || !ClearOfHoles(state, x, y))
            continue;

        Position const point(x, y, to.GetPositionZ());
        if (ClipsOtherHole(state, from, point, to) || ClipsOtherHole(state, point, to, to))
            continue;

        float const first = from.GetExactDist2d(point);
        float const second = std::max(0.0f, point.GetExactDist2d(to) - ULDUAR_ALGALON_SHELTER_IN_RADIUS);
        int32 const walkMs = HideWalkMs(state, bot, first + second);
        int32 const firstMs = first + second > 0.0f ? static_cast<int32>(walkMs * first / (first + second)) : 0;
        if (SmashAcrossPath(state, from, point, first, startMs, firstMs, 0, now) ||
            SmashAcrossPath(state, point, to, second, startMs + firstMs, walkMs - firstMs,
                            ULDUAR_ALGALON_FIELD_PULSE_MS, now))
            continue;

        float const legs = from.GetExactDist2d(x, y) + to.GetExactDist2d(x, y);
        if (legs < best)
        {
            best = legs;
            waypoint = point;
        }
    }

    if (best == std::numeric_limits<float>::max())
        return false;

    length = best;
    return true;
}

// Straight to the hole, or round the first meteor that would land by the bot on the walk set off in `startMs`.
// Returns the walk's length. With no way round, the straight one, and `clear` false.
float HideRoute(AlgalonEncounterState const& state, Player* bot, Position const& shelter, int32 startMs, Position& next,
                bool& clear)
{
    next = shelter;
    clear = true;
    float const straight = bot->GetExactDist2d(shelter);
    if (state.markers.empty())
        return straight;

    uint32 const now = getMSTime();
    float const walk = std::max(0.0f, straight - ULDUAR_ALGALON_SHELTER_IN_RADIUS);
    AlgalonScanUnit const* smash = SmashAcrossPath(state, bot->GetPosition(), shelter, walk, startMs,
                                                   HideWalkMs(state, bot, walk), ULDUAR_ALGALON_FIELD_PULSE_MS, now);
    if (!smash)
        return straight;

    float length = straight;
    if (!FindSmashDetour(bot, state, *smash, shelter, startMs, now, next, length))
    {
        next = shelter;
        clear = false;
        return straight;
    }

    return length;
}

// A live meteor landing in a Big Bang cast: the one running, or the next if it starts first.
bool SmashLandsInCast(AlgalonEncounterState const& state, uint32 now)
{
    if (state.markers.empty())
        return false;

    if (state.bigBangCasting)
        return true;

    if (!state.nextBigBangMs)
        return false;

    int32 const castIn = MsUntil(state.nextBigBangMs, now);
    for (AlgalonScanUnit const& marker : state.markers)
        if (SmashImpactMs(state, marker, now) >= castIn)
            return true;

    return false;
}

// Unphased and not yet in a field when a star dying in `deathMs` goes off: no hole, staying out, or its walk (a
// bot's after the wait it still plans) and a pulse and a half run past it. Not the soaker, in Dispersion for it.
bool CaughtOutside(AlgalonEncounterState const& state, Player* member, int32 deathMs)
{
    if (!CanStayOut(member) || member->GetGUID() == state.bigBangSoaker)
        return false;

    if (!IsBot(member))
    {
        AlgalonScanUnit const* hole = NearestShelter(state, member->GetPosition());
        if (!hole)
            return true;

        float const walk = std::max(0.0f, member->GetExactDist2d(hole->position) - ULDUAR_ALGALON_SHELTER_IN_RADIUS);
        float const speed = std::max(1.0f, member->GetSpeed(MOVE_RUN));
        return static_cast<int32>(walk / speed * IN_MILLISECONDS) + ULDUAR_ALGALON_LAST_IN_MARGIN_MS > deathMs;
    }

    AlgalonScanUnit const* hole = ShelterHole(member, state);
    int32 const slack = AlgalonHideSlackMs(member);
    if (!hole || slack == std::numeric_limits<int32>::max())
        return true;

    Position next;
    bool clear = true;
    float const route = HideRoute(state, member, hole->position, std::max(0, slack), next, clear);
    int32 const walkMs = HideWalkMs(state, member, std::max(0.0f, route - ULDUAR_ALGALON_SHELTER_IN_RADIUS));
    return std::max(0, slack) + walkMs + ULDUAR_ALGALON_LAST_IN_MARGIN_MS > deathMs;
}
}  // namespace

int32 AlgalonHideSlackMs(Player* bot)
{
    if (!AlgalonBigBangLatched(bot) || IsAlgalonPhased(bot) || AlgalonStaysOut(bot))
        return std::numeric_limits<int32>::max();

    AlgalonEncounterState const* state = FindState(bot);
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!state || !botAI || state->holes.empty())
        return std::numeric_limits<int32>::max();

    // The hole it will run to, not the nearest: past a kite hole the run is longer.
    AlgalonScanUnit const* shelter = ShelterHole(bot, *state);
    if (!shelter)
        return std::numeric_limits<int32>::max();

    Position const& to = shelter->position;
    float const straight = std::max(0.0f, bot->GetExactDist2d(to) - ULDUAR_ALGALON_SHELTER_IN_RADIUS);
    int32 const straightMs = HideWalkMs(*state, bot, straight);
    int32 const remaining = AlgalonBigBangRemainingMs(botAI);
    uint32 const now = getMSTime();

    // Caught mid run, or in the field too late to be phased, it would have no button up; standing, it casts one.
    bool const tank = state->swapPair.count(bot->GetGUID()) || state->handler == bot->GetGUID();
    if (tank && remaining < straightMs + ULDUAR_ALGALON_LAST_IN_MARGIN_MS)
        return std::numeric_limits<int32>::max();

    int32 const underExtra = UnderHideExtraMs(*state, remaining, now);
    int32 margin = ULDUAR_ALGALON_HIDE_MARGIN_MS + underExtra;
    // Its constellations follow it in and close the hole, so it goes last while others still head there. Not with
    // a star under: the window opens once the others are due in, and waiting on a straggler would leave it out.
    bool last = false;
    if (AlgalonHoldsConstellation(bot) && !underExtra)
    {
        auto const out = state->hidersOut.find(shelter->guid);
        last = out != state->hidersOut.end() && out->second;
        if (last)
            margin = ULDUAR_ALGALON_LAST_IN_MARGIN_MS + ULDUAR_ALGALON_HOLD_LEAVE_LEAD_MS;
    }

    int32 const slack = remaining - straightMs - margin;
    int32 const start = std::max(0, slack);
    auto const crossed = [&](int32 from)
    {
        return SmashAcrossPath(*state, bot->GetPosition(), to, straight, from, straightMs,
                               ULDUAR_ALGALON_FIELD_PULSE_MS, now);
    };
    AlgalonScanUnit const* const smash = crossed(start);
    if (!smash)
        return slack;

    // Phased first, nothing lands on it. Not for one going last, it'd close the hole on the others.
    if (!last)
    {
        // Set off already and still in first: keeps going.
        if (start > 0 && state->shelterAssignments.count(bot->GetGUID()) && !crossed(0))
            return 0;

        int32 early = start;
        for (AlgalonScanUnit const* across = smash; across; across = early >= 0 ? crossed(early) : nullptr)
            early = SmashImpactMs(*state, *across, now) - straightMs - ULDUAR_ALGALON_FIELD_PULSE_MS -
                    ULDUAR_ALGALON_SMASH_IN_LEAD_MS;

        if (early >= 0)
            return early;
    }

    // Else round it, or straight through when nothing goes round.
    Position next;
    bool round = true;
    float const route = HideRoute(*state, bot, to, start, next, round);
    int32 const walkMs = HideWalkMs(*state, bot, std::max(0.0f, route - ULDUAR_ALGALON_SHELTER_IN_RADIUS));
    int32 const routeSlack = remaining - walkMs - margin;

    // Too far to be in first: it stays where it dodged until they've landed, while it still makes the hit. With a
    // star under too, since out there it still gets the cover or its button, and a meteor within 6 yd always killed.
    // Not once on its way round: the spot it would stop on was never checked against them.
    if (!round || routeSlack > 0 || !state->shelterAssignments.count(bot->GetGUID()))
    {
        int32 late = start;
        for (AlgalonScanUnit const* across = smash; across; across = crossed(late))
            late = SmashImpactMs(*state, *across, now) + ULDUAR_ALGALON_COSMIC_SMASH_IMPACT_SPREAD_MS;

        if (late + straightMs + ULDUAR_ALGALON_LAST_IN_MARGIN_MS <= remaining)
            return late;
    }

    if (tank && remaining < walkMs + ULDUAR_ALGALON_LAST_IN_MARGIN_MS)
        return std::numeric_limits<int32>::max();

    return routeSlack;
}

bool AlgalonShouldRunForShelter(Player* bot) { return AlgalonHideSlackMs(bot) <= 0; }

void AlgalonBoostHideRun(Player* bot, float walk)
{
    AlgalonEncounterState* state = FindState(bot);
    if (!state || walk <= ULDUAR_ALGALON_NITRO_MIN_WALK || bot->HasAura(NITRO_BOOSTS_SPEED_SPELL))
        return;

    if (state->nitroRunners.count(bot->GetGUID()))
    {
        // Gone for good, the slack would keep timing a boosted walk. A refused use is tried again next tick.
        if (NitroBoostsReady(bot))
        {
            UseNitroBoosts(bot);
            return;
        }

        state->nitroRunners.erase(bot->GetGUID());
    }

    // Only a run already late, like one to a far hole after the near one closed in the cast: walking, it lands
    // after the field's last pulse.
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI ||
        AlgalonBigBangRemainingMs(botAI) >= HideWalkMs(*state, bot, walk) + ULDUAR_ALGALON_LAST_IN_MARGIN_MS)
        return;

    char const* const speed = bot->getClass() == CLASS_HUNTER  ? "aspect of the cheetah"
                              : bot->getClass() == CLASS_ROGUE ? "sprint"
                                                               : nullptr;
    if (speed && !botAI->HasAura(speed, bot) && botAI->CanCastSpell(speed, bot))
        botAI->CastSpell(speed, bot);
}

AlgalonHideRole GetAlgalonHideRole(Player* bot)
{
    AlgalonEncounterState const* state = FindState(bot);
    ObjectGuid const guid = bot ? bot->GetGUID() : ObjectGuid::Empty;

    AlgalonHideRole role = AlgalonHideRole::None;
    Position shelter;
    if (!state || !state->engaged)
        role = AlgalonHideRole::None;
    else if (IsAlgalonPhased(bot))
        role = AlgalonHoleNear(bot, ULDUAR_ALGALON_HOLE_EXIT_RADIUS) ? AlgalonHideRole::In : AlgalonHideRole::Exit;
    else if (!state->bigBangCasting)
        role = AlgalonHideRole::None;
    else if (state->bigBangSoaker == guid)
        role = AlgalonHideRole::Soak;
    else if (state->bigBangBackup == guid)
        role = AlgalonHideRole::Backup;
    else
    {
        int32 const slack = AlgalonHideSlackMs(bot);
        if (slack == std::numeric_limits<int32>::max())
            role = AlgalonHideRole::NoShelter;
        else if (slack > 0)
            role = AlgalonHoldsConstellation(bot) ? AlgalonHideRole::Hold : AlgalonHideRole::Wait;
        else
            role = GetAlgalonShelter(bot, shelter) ? AlgalonHideRole::Run : AlgalonHideRole::NoShelter;
    }

    return role;
}

bool GetAlgalonShelter(Player* bot, Position& shelter)
{
    AlgalonEncounterState* state = FindState(bot);
    if (!state || state->holes.empty())
        return false;

    AlgalonScanUnit const* hole = ShelterHole(bot, *state);
    if (!hole)
        return false;

    auto const latched = state->shelterAssignments.find(bot->GetGUID());
    if (latched == state->shelterAssignments.end() || latched->second != hole->guid)
        state->shelterAssignments[bot->GetGUID()] = hole->guid;

    shelter = hole->position;
    return true;
}

bool AlgalonHideRoute(Player* bot, Position const& shelter, Position& next)
{
    next = shelter;
    AlgalonEncounterState const* state = FindState(bot);
    if (!state)
        return true;

    bool clear = true;
    HideRoute(*state, bot, shelter, 0, next, clear);
    if (next != shelter)
        next = ValidateFloorPoint(bot, next);

    return clear;
}

bool AlgalonHoleNear(Player* bot, float radius, Position* hole)
{
    AlgalonEncounterState const* state = FindState(bot);
    if (!state)
        return false;

    for (AlgalonScanUnit const& candidate : state->holes)
    {
        if (bot->GetExactDist2d(candidate.position) > radius)
            continue;

        if (hole)
            *hole = candidate.position;

        return true;
    }

    return false;
}

Player* GetAlgalonBossTank(PlayerbotAI* botAI)
{
    Unit* boss = GetAlgalon(botAI);
    Unit* victim = boss ? boss->GetVictim() : nullptr;
    return victim ? victim->ToPlayer() : nullptr;
}

bool IsAlgalonSwapTank(Player* player)
{
    AlgalonEncounterState const* state = FindState(player);
    return state && state->swapPair.count(player->GetGUID());
}

uint8 GetAlgalonPhasePunchStacks(Player* player)
{
    Aura* aura = player ? player->GetAura(SPELL_ALGALON_PHASE_PUNCH) : nullptr;
    return aura ? aura->GetStackAmount() : 0;
}

Player* GetAlgalonPickupTank(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    AlgalonEncounterState const* state = FindState(bot);
    Group* group = bot ? bot->GetGroup() : nullptr;
    if (!state || !group)
        return nullptr;

    // A human isn't on a trigger, so a bot takes him whenever one can, unless it is already at the swap
    // stack count: a fifth Phase Punch phases it out again within seconds.
    auto rank = [](Player* tank)
    {
        uint8 const stacks = GetAlgalonPhasePunchStacks(tank);
        return std::make_tuple(stacks >= ULDUAR_ALGALON_PHASE_PUNCH_SWAP_STACKS ? 1 : 0, IsBot(tank) ? 0 : 1, stacks,
                               tank->GetGUID());
    };

    Player* best = nullptr;
    for (ObjectGuid const& guid : state->swapPair)
    {
        Player* tank = FindGroupMember(group, guid);
        if (CanStayOut(tank) && (!best || rank(tank) < rank(best)))
            best = tank;
    }

    return best;
}

bool AlgalonShouldPickUp(Player* bot)
{
    PlayerbotAI* botAI = bot ? GET_PLAYERBOT_AI(bot) : nullptr;
    if (!botAI || IsAlgalonPhased(bot) || !IsAlgalonSwapTank(bot) || !AlgalonEngaged(botAI))
        return false;

    Unit* boss = GetAlgalon(botAI);
    if (!boss || boss->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        return false;

    Unit* victim = boss->GetVictim();
    Player* holder = victim ? victim->ToPlayer() : nullptr;

    // Mid cast the elected soaker is the one staying out, so it is the one to take him, off the other swap
    // tank too when that one was handed the soak.
    if (AlgalonBigBangLatched(bot))
        return GetAlgalonBigBangSoaker(botAI) == bot && holder != bot;

    if (holder && IsAlgalonSwapTank(holder) && !IsAlgalonPhased(holder))
        return false;

    return GetAlgalonPickupTank(botAI) == bot;
}

bool AlgalonShouldSwap(Player* bot)
{
    PlayerbotAI* botAI = bot ? GET_PLAYERBOT_AI(bot) : nullptr;
    AlgalonEncounterState const* state = FindState(bot);
    if (!botAI || !state || IsAlgalonPhased(bot) || !IsAlgalonSwapTank(bot) || !AlgalonEngaged(botAI))
        return false;

    Player* holder = GetAlgalonBossTank(botAI);
    if (!holder || holder == bot || !IsAlgalonSwapTank(holder))
        return false;

    // Mid cast the holder soaks or hides with the taker. Taking him now leaves the soaker hiding and the wrong
    // tank out, or a hider out of its hole.
    if (AlgalonBigBangCasting(botAI))
        return false;

    return IsAlgalonPhased(holder) || SwapDue(*state, holder, bot, getMSTime());
}

bool AlgalonTakesBossNow(Player* bot) { return AlgalonShouldPickUp(bot) || AlgalonShouldSwap(bot); }

bool AlgalonYieldsBoss(Player* bot)
{
    // The pair and the handler are the tanks there are; reading the bot's role instead walks its
    // strategies, and the damage guard asks this for every action every damage dealer pops.
    AlgalonEncounterState const* state = FindState(bot);
    if (!state || (!state->swapPair.count(bot->GetGUID()) && state->handler != bot->GetGUID()))
        return false;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
        return false;

    Player* holder = GetAlgalonBossTank(botAI);
    if (!holder || holder == bot || !IsAlgalonSwapTank(holder) || IsAlgalonPhased(holder))
        return false;

    return !AlgalonShouldPickUp(bot) && !AlgalonShouldSwap(bot);
}

Player* AlgalonLoneHolder(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    AlgalonEncounterState const* state = FindState(bot);
    Player* holder = GetAlgalonBossTank(botAI);
    if (!state || !holder || !IsAlgalonSwapTank(holder) || !CanStayOut(holder))
        return nullptr;

    for (ObjectGuid const& guid : state->swapPair)
    {
        Player* partner = FindGroupMember(bot->GetGroup(), guid);
        if (partner && partner != holder && CanStayOut(partner))
            return nullptr;
    }

    return holder;
}

bool AlgalonOffTankWorking(Player* bot)
{
    PlayerbotAI* botAI = bot ? GET_PLAYERBOT_AI(bot) : nullptr;
    if (!botAI || !IsAlgalonSwapTank(bot))
        return false;

    Player* holder = GetAlgalonBossTank(botAI);
    if (!holder || holder == bot || !IsAlgalonSwapTank(holder))
        return false;

    Unit* target = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    return target && target != GetAlgalon(botAI);
}

Player* GetAlgalonHandler(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    AlgalonEncounterState const* state = FindState(bot);
    return state ? FindGroupMember(bot->GetGroup(), state->handler) : nullptr;
}

bool IsAlgalonHandler(Player* bot)
{
    AlgalonEncounterState const* state = FindState(bot);
    return state && bot->GetGroup() && state->handler == bot->GetGUID();
}

namespace
{
bool ConstellationLow(Unit const* unit)
{
    return unit->GetPositionZ() - ULDUAR_ALGALON_ROOM_CENTER.GetPositionZ() <= ULDUAR_ALGALON_CONSTELLATION_LOW_Z;
}

bool HeldKitePossible(AlgalonEncounterState const& state, Player* bot, uint32 now);
}  // namespace

Unit* GetAlgalonHandlerConstellation(Player* bot)
{
    AlgalonEncounterState* state = FindState(bot);
    if (!state)
        return nullptr;

    if (!state->engaged || state->handler != bot->GetGUID())
    {
        state->kiteHoleAssignments.erase(bot->GetGUID());
        return nullptr;
    }

    // Loose ones first: a constellation follows its victim into a hole at Big Bang and closes it, so all end up
    // on the handler, who hides last. Barrage hits a random raid member anyway, so holding them costs nothing.
    uint32 const now = getMSTime();
    bool const gathering =
        state->bigBangCasting || BigBangDueWithin(*state, ULDUAR_ALGALON_GATHER_LEAD_SECONDS, now);
    Creature* loose = nullptr;
    Creature* held = nullptr;
    // Only a low one is worth a kite: a hole can't take one still up in the air.
    Creature* heldLow = nullptr;
    float looseDistance = std::numeric_limits<float>::max();
    for (AlgalonScanUnit const& constellation : state->constellations)
    {
        Creature* unit = constellation.active ? Resolve(bot, constellation.guid) : nullptr;
        if (!unit)
            continue;

        if (unit->GetVictim() == bot)
        {
            held = held ? held : unit;
            heldLow = heldLow || !ConstellationLow(unit) ? heldLow : unit;
            continue;
        }

        // Up in the air it waits until it lands, bar the gather: one is stuck up there for each wave.
        if (!gathering && !ConstellationLow(unit))
            continue;

        float const distance = bot->GetExactDist2d(unit->GetPositionX(), unit->GetPositionY());
        if (distance < looseDistance)
        {
            loose = unit;
            looseDistance = distance;
        }
    }

    bool const spare = state->holes.size() > ReservedHoles(*state, now);
    // Held ones go first while the taunt cools, or loose ones taunted 8 s apart after every swap hold the kite up.
    // A pass past its stage walk is finished first: taunting from behind latches a new kite and drops it.
    auto const pass = state->kites.find(bot->GetGUID());
    bool const passUnderWay = pass != state->kites.end() && pass->second.looseTarget.IsEmpty() &&
                              pass->second.stage != AlgalonKiteStage::Stage;
    bool const kiteFirst = loose && heldLow && !gathering && spare &&
                           (passUnderWay || !ClassTauntReady(GET_PLAYERBOT_AI(bot))) &&
                           HeldKitePossible(*state, bot, now);
    Unit* chosen = nullptr;
    if (loose && !kiteFirst)
        chosen = loose;
    else if (!(held && gathering) && heldLow && spare)
        chosen = heldLow;

    // Gathering keeps the kite hole: the handler still hides in it, last, where nobody else does. Not the
    // raid's: a kite hole is left out of everyone else's pick.
    AlgalonScanUnit const* raidHole = RaidShelterHole(*state, now);
    auto const kite = state->kiteHoleAssignments.find(bot->GetGUID());
    bool const keepKite = held && gathering &&
                          !(raidHole && kite != state->kiteHoleAssignments.end() && kite->second == raidHole->guid);
    if (!chosen && !keepKite)
        state->kiteHoleAssignments.erase(bot->GetGUID());

    return chosen;
}

namespace
{
// `offset` from the hole along `angle`, else either side by the swing: in the room, still that side of the
// hole and outside its field, and off every other hole's.
bool KiteSpotNear(AlgalonEncounterState const& state, AlgalonScanUnit const& hole, float angle, float offset,
                  Position& spot)
{
    for (float const swing : {0.0f, ULDUAR_ALGALON_KITE_SWING, -ULDUAR_ALGALON_KITE_SWING})
    {
        float const bearing = angle + swing;
        float x = hole.position.GetPositionX() + std::cos(bearing) * offset;
        float y = hole.position.GetPositionY() + std::sin(bearing) * offset;
        ClampIntoRoom(x, y);

        float const dx = x - hole.position.GetPositionX();
        float const dy = y - hole.position.GetPositionY();
        if (std::hypot(dx, dy) < ULDUAR_ALGALON_KITE_MIN_OFFSET || dx * std::cos(angle) + dy * std::sin(angle) <= 0.0f)
            continue;

        bool const clear =
            std::none_of(state.holes.begin(), state.holes.end(), [&](AlgalonScanUnit const& other)
                         { return other.guid != hole.guid &&
                                  other.position.GetExactDist2d(x, y) < ULDUAR_ALGALON_HOLE_EXIT_RADIUS; });
        if (!clear)
            continue;

        spot = Position(x, y, hole.position.GetPositionZ());
        return true;
    }

    return false;
}

// Not the raid's hole with a Big Bang due, nor the only one the swap tanks reach from the hold spot.
bool KiteableHole(AlgalonEncounterState const& state, AlgalonScanUnit const& hole, uint32 now)
{
    if (&hole == RaidShelterHole(state, now))
        return false;

    if (hole.position.GetExactDist2d(ULDUAR_ALGALON_HOLD_SPOT) > ULDUAR_ALGALON_TANK_SHELTER_REACH)
        return true;

    return std::any_of(state.holes.begin(), state.holes.end(), [&hole](AlgalonScanUnit const& other)
        { return other.guid != hole.guid &&
                 other.position.GetExactDist2d(ULDUAR_ALGALON_HOLD_SPOT) <= ULDUAR_ALGALON_TANK_SHELTER_REACH; });
}

// The far spot fits, and for held ones the stage spot on their side too.
bool KiteHoleFits(AlgalonEncounterState const& state, AlgalonScanUnit const& hole, Position const& from, bool loose)
{
    Position spot;
    float const toward = hole.position.GetAngle(&from);
    float const away = Position::NormalizeOrientation(toward + static_cast<float>(M_PI));
    return KiteSpotNear(state, hole, away, ULDUAR_ALGALON_BLACK_HOLE_KITE_OFFSET, spot) &&
           (loose || KiteSpotNear(state, hole, toward, ULDUAR_ALGALON_KITE_STAGE_OFFSET, spot));
}

void DropKite(AlgalonEncounterState& state, ObjectGuid const& guid)
{
    state.kiteHoleAssignments.erase(guid);
    state.kites.erase(guid);
}

// Its latched hole still kiteable, else one that fits from where the low held ones are.
bool HeldKitePossible(AlgalonEncounterState const& state, Player* bot, uint32 now)
{
    float qx = 0.0f;
    float qy = 0.0f;
    uint32 count = 0;
    for (AlgalonScanUnit const& scanned : state.constellations)
    {
        Creature* unit = scanned.active ? Resolve(bot, scanned.guid) : nullptr;
        if (!unit || unit->GetVictim() != bot || !ConstellationLow(unit))
            continue;

        qx += unit->GetPositionX();
        qy += unit->GetPositionY();
        ++count;
    }

    if (!count)
        return false;

    auto const latched = state.kiteHoleAssignments.find(bot->GetGUID());
    if (latched != state.kiteHoleAssignments.end())
        if (AlgalonScanUnit const* hole = FindScanned(state.holes, latched->second))
            if (KiteableHole(state, *hole, now))
                return true;

    Position const from(qx / count, qy / count, bot->GetPositionZ());
    return std::any_of(state.holes.begin(), state.holes.end(), [&](AlgalonScanUnit const& hole)
                       { return KiteableHole(state, hole, now) && KiteHoleFits(state, hole, from, false); });
}

// No hole to spare, or the raid gathering for Big Bang: a loose one is taunted from where the handler stands.
bool BehindAllowed(AlgalonEncounterState const& state, uint32 now)
{
    return !state.bigBangCasting && !BigBangDueWithin(state, ULDUAR_ALGALON_GATHER_LEAD_SECONDS, now) &&
           state.holes.size() > ReservedHoles(state, now);
}
}  // namespace

bool GetAlgalonKiteSpot(Player* bot, Unit* constellation, Position& spot)
{
    AlgalonEncounterState* state = FindState(bot);
    if (!state || !constellation)
        return false;

    uint32 const now = getMSTime();
    ObjectGuid const guid = bot->GetGUID();

    // Where they come from: every one on the handler, or the loose one it is about to taunt.
    std::vector<Creature*> held;
    bool allCasting = true;
    bool anyCasting = false;
    float qx = 0.0f;
    float qy = 0.0f;
    for (AlgalonScanUnit const& scanned : state->constellations)
    {
        Creature* unit = scanned.active ? Resolve(bot, scanned.guid) : nullptr;
        if (!unit || unit->GetVictim() != bot || !ConstellationLow(unit))
            continue;

        held.push_back(unit);
        qx += unit->GetPositionX();
        qy += unit->GetPositionY();
        bool const casting = unit->HasUnitState(UNIT_STATE_CASTING);
        allCasting = allCasting && casting;
        anyCasting = anyCasting || casting;
    }

    bool const loose = constellation->GetVictim() != bot;
    Position const from = loose || held.empty()
                              ? constellation->GetPosition()
                              : Position(qx / held.size(), qy / held.size(), constellation->GetPositionZ());

    AlgalonScanUnit const* hole = nullptr;
    auto const latched = state->kiteHoleAssignments.find(guid);
    if (latched != state->kiteHoleAssignments.end())
        hole = FindScanned(state->holes, latched->second);

    // A loose one gets the hole nearest itself, never one picked for another.
    auto const previous = state->kites.find(guid);
    bool const otherTarget =
        loose && (previous == state->kites.end() || previous->second.looseTarget != constellation->GetGUID());
    if (hole && (!KiteableHole(*state, *hole, now) || otherTarget))
    {
        hole = nullptr;
        DropKite(*state, guid);
    }

    if (!hole)
    {
        // Nearest to where they come from, not to the handler: they are what has to arrive.
        std::vector<AlgalonScanUnit const*> candidates;
        for (AlgalonScanUnit const& candidate : state->holes)
            if (KiteableHole(*state, candidate, now))
                candidates.push_back(&candidate);

        std::sort(candidates.begin(), candidates.end(), [&from](AlgalonScanUnit const* a, AlgalonScanUnit const* b)
                  { return from.GetExactDist2d(a->position) < from.GetExactDist2d(b->position); });

        for (AlgalonScanUnit const* candidate : candidates)
        {
            if (KiteHoleFits(*state, *candidate, from, loose))
            {
                hole = candidate;
                break;
            }
        }

        if (!hole)
        {
            DropKite(*state, guid);
            return false;
        }

        state->kiteHoleAssignments[guid] = hole->guid;
        AlgalonKite fresh;
        fresh.sinceMs = now;
        if (loose)
            fresh.looseTarget = constellation->GetGUID();
        state->kites[guid] = fresh;
    }

    AlgalonKite& kite = state->kites[guid];
    float const toward = hole->position.GetAngle(&from);
    float const away = Position::NormalizeOrientation(toward + static_cast<float>(M_PI));

    auto const enter = [&kite, now](AlgalonKiteStage next)
    {
        kite.stage = next;
        kite.sinceMs = now;
        kite.hasSpot = false;
    };

    if (loose)
    {
        // Latched, or it would circle the hole with the one chasing someone else; a new one once out of reach.
        if (!kite.hasSpot || constellation->GetExactDist2d(kite.spot) > ULDUAR_ALGALON_TAUNT_REACH)
            kite.hasSpot = KiteSpotNear(*state, *hole, away, ULDUAR_ALGALON_BLACK_HOLE_KITE_OFFSET, kite.spot);
    }
    else
    {
        // On the handler now, so the pass is its own, not the taunt's.
        kite.looseTarget = ObjectGuid::Empty;

        // Already past the hole from them, after a taunt from behind or a pass round: it waits where it is.
        if (kite.stage == AlgalonKiteStage::Stage)
        {
            float const dx = bot->GetPositionX() - hole->position.GetPositionX();
            float const dy = bot->GetPositionY() - hole->position.GetPositionY();
            float const out = std::hypot(dx, dy);
            if (out >= ULDUAR_ALGALON_KITE_MIN_OFFSET && out <= ULDUAR_ALGALON_KITE_STAGE_OFFSET &&
                dx * std::cos(away) + dy * std::sin(away) >= out * 0.5f)
                enter(AlgalonKiteStage::Cross);
        }

        // A pass that missed: one rests beside the handler outside the field.
        if (kite.stage == AlgalonKiteStage::Parked &&
            getMSTimeDiff(kite.sinceMs, now) >= ULDUAR_ALGALON_KITE_RETRY_MS &&
            std::any_of(held.begin(), held.end(), [bot, hole](Creature* unit)
                        { return bot->GetExactDist2d(unit) <= ULDUAR_ALGALON_KITE_BESIDE &&
                                 hole->position.GetExactDist2d(unit) > ULDUAR_ALGALON_CONSTELLATION_EAT_RADIUS; }))
            enter(AlgalonKiteStage::Stage);

        // A walk that never arrives was blocked or pushed off: the next call starts over.
        if ((kite.stage == AlgalonKiteStage::Stage || kite.stage == AlgalonKiteStage::Cross) &&
            getMSTimeDiff(kite.sinceMs, now) >= ULDUAR_ALGALON_KITE_STALL_MS)
        {
            DropKite(*state, guid);
            return false;
        }

        if (kite.stage == AlgalonKiteStage::Wait &&
            (allCasting || (anyCasting && getMSTimeDiff(kite.sinceMs, now) >= ULDUAR_ALGALON_KITE_WAIT_MS)))
            enter(AlgalonKiteStage::Cross);

        if (!kite.hasSpot)
        {
            bool const near = kite.stage == AlgalonKiteStage::Stage || kite.stage == AlgalonKiteStage::Wait;
            kite.hasSpot = KiteSpotNear(*state, *hole, near ? toward : away,
                                        near ? ULDUAR_ALGALON_KITE_STAGE_OFFSET : ULDUAR_ALGALON_BLACK_HOLE_KITE_OFFSET,
                                        kite.spot);
        }

        // Arrived, the spot stays for the next stage: waiting at it, or parked at it.
        if (kite.hasSpot && bot->GetExactDist2d(kite.spot) <= ULDUAR_ALGALON_SLOT_TOLERANCE &&
            (kite.stage == AlgalonKiteStage::Stage || kite.stage == AlgalonKiteStage::Cross))
        {
            Position const reached = kite.spot;
            enter(kite.stage == AlgalonKiteStage::Stage ? AlgalonKiteStage::Wait : AlgalonKiteStage::Parked);
            kite.spot = reached;
            kite.hasSpot = true;
        }
    }

    if (!kite.hasSpot)
    {
        DropKite(*state, guid);
        return false;
    }

    spot = ValidateFloorPoint(bot, kite.spot);
    return true;
}

bool AlgalonKiteBehind(Player* bot, Unit* constellation, Position& spot)
{
    AlgalonEncounterState* state = FindState(bot);
    if (!state || !constellation || constellation->GetVictim() == bot || !BehindAllowed(*state, getMSTime()))
        return false;

    if (!GetAlgalonKiteSpot(bot, constellation, spot))
        return false;

    // Out of taunt reach from there: no kite hole is held for it, or everyone else's pick would skip it.
    if (constellation->GetExactDist2d(spot) > ULDUAR_ALGALON_TAUNT_REACH)
    {
        DropKite(*state, bot->GetGUID());
        return false;
    }

    return true;
}

bool AlgalonKitingBehind(Player* bot, Unit* constellation)
{
    AlgalonEncounterState const* state = FindState(bot);
    if (!state || !constellation || constellation->GetVictim() == bot || !BehindAllowed(*state, getMSTime()))
        return false;

    auto const kite = state->kites.find(bot->GetGUID());
    return kite != state->kites.end() && kite->second.hasSpot &&
           kite->second.looseTarget == constellation->GetGUID();
}

Unit* GetAlgalonLooseDarkMatter(Player* bot)
{
    AlgalonEncounterState const* state = FindState(bot);
    if (!state || !state->engaged || state->handler != bot->GetGUID())
        return nullptr;

    Unit* loosest = nullptr;
    float lowestVictimHealth = std::numeric_limits<float>::max();
    for (AlgalonScanUnit const& scanned : state->darkMatter)
    {
        Creature* darkMatter = Resolve(bot, scanned.guid);
        Unit* victim = darkMatter ? darkMatter->GetVictim() : nullptr;
        if (!darkMatter || victim == bot)
            continue;

        float const victimHealth = victim ? victim->GetHealthPct() : 100.0f;
        if (victimHealth < lowestVictimHealth)
        {
            loosest = darkMatter;
            lowestVictimHealth = victimHealth;
        }
    }

    return loosest;
}

bool IsAlgalonStarTeam(Player* bot)
{
    AlgalonEncounterState const* state = FindState(bot);
    if (!state)
        return false;

    return state->starTeam.count(bot->GetGUID()) || (state->urgent && IsRangedDps(bot));
}

Unit* GetAlgalonFocusStar(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    AlgalonEncounterState const* state = FindState(bot);
    return state ? Resolve(bot, state->focusStar) : nullptr;
}

uint8 AlgalonAliveStarCount(PlayerbotAI* botAI)
{
    AlgalonEncounterState const* state = FindState(botAI ? botAI->GetBot() : nullptr);
    return state ? static_cast<uint8>(state->stars.size()) : 0;
}

bool AlgalonCosmicSmashThreatens(Player* bot)
{
    PlayerbotAI* botAI = bot ? GET_PLAYERBOT_AI(bot) : nullptr;
    return botAI && !IsAlgalonPhased(bot) &&
           AlgalonCosmicSmashMarkerNear(botAI, bot->GetPosition(), ULDUAR_ALGALON_COSMIC_SMASH_TRIGGER_RADIUS);
}

bool AlgalonDyingStarThreatens(Player* bot)
{
    AlgalonEncounterState const* state = FindState(bot);
    return state && state->engaged && !IsAlgalonPhased(bot) &&
           DyingStarNear(*state, bot->GetPosition(), ULDUAR_ALGALON_STAR_DYING_RADIUS);
}

bool AlgalonCosmicSmashMarkerNear(PlayerbotAI* botAI, Position const& spot, float radius)
{
    AlgalonEncounterState const* state = FindState(botAI ? botAI->GetBot() : nullptr);
    if (!state)
        return false;

    for (AlgalonScanUnit const& marker : state->markers)
        if (spot.GetExactDist2d(marker.position.GetPositionX(), marker.position.GetPositionY()) <= radius)
            return true;

    return false;
}

bool AlgalonSmashHoldsGround(Player* bot)
{
    PlayerbotAI* botAI = bot ? GET_PLAYERBOT_AI(bot) : nullptr;
    AlgalonEncounterState const* state = FindState(bot);
    return botAI && state && !IsAlgalonPhased(bot) &&
           (AlgalonCosmicSmashMarkerNear(botAI, bot->GetPosition(), ULDUAR_ALGALON_COSMIC_SMASH_HOLD_RADIUS) ||
            DyingStarNear(*state, bot->GetPosition(), ULDUAR_ALGALON_STAR_DYING_HOLD_RADIUS));
}

std::vector<HazardCircle> GetAlgalonDodgeHazards(Player* bot, float markerClearance)
{
    std::vector<HazardCircle> hazards;
    AlgalonEncounterState const* state = FindState(bot);
    if (!state)
        return hazards;

    for (AlgalonScanUnit const& marker : state->markers)
        hazards.emplace_back(marker.position, markerClearance);

    for (AlgalonScanUnit const& hole : state->holes)
        hazards.emplace_back(hole.position, ULDUAR_ALGALON_HOLE_EXIT_RADIUS);

    // In the void, a star dying under the cast is a hole where the bot would stand at the strip.
    bool const phased = IsAlgalonPhased(bot);
    for (AlgalonScanUnit const& star : state->stars)
        if (StarDying(star) || (phased && star.underDying))
            hazards.emplace_back(star.position, ULDUAR_ALGALON_STAR_DYING_RADIUS);

    return hazards;
}

bool AlgalonUnderStarNear(Player* bot, float radius)
{
    AlgalonEncounterState const* state = FindState(bot);
    if (!state)
        return false;

    return std::any_of(state->stars.begin(), state->stars.end(), [bot, radius](AlgalonScanUnit const& star)
                       { return star.underDying && bot->GetExactDist2d(star.position) < radius; });
}

bool AlgalonSpotInRoom(float x, float y)
{
    return ULDUAR_ALGALON_ROOM_CENTER.GetExactDist2d(x, y) <= ULDUAR_ALGALON_ROOM_CLAMP;
}

bool AlgalonPathCrossesHole(Player* bot, float x, float y, float clearance)
{
    AlgalonEncounterState const* state = FindState(bot);
    return state && HoleAcrossPath(*state, bot->GetPosition(), x, y, IsAlgalonPhased(bot), clearance);
}

bool AlgalonWalkEntersField(Player* bot, float x, float y, float standClearance)
{
    AlgalonEncounterState const* state = FindState(bot);
    if (!state || state->holes.empty())
        return false;

    for (AlgalonScanUnit const& hole : state->holes)
        if (hole.position.GetExactDist2d(x, y) < standClearance)
            return true;

    return HoleAcrossPath(*state, bot->GetPosition(), x, y, false, ULDUAR_ALGALON_FIELD_PATH_CLEARANCE) != nullptr;
}

bool AlgalonWalkAround(Player* bot, Position const& to, Position& next)
{
    next = to;
    AlgalonEncounterState const* state = FindState(bot);
    if (!state)
        return true;

    AlgalonScanUnit const* hole =
        HoleAcrossPath(*state, bot->GetPosition(), to.GetPositionX(), to.GetPositionY(), IsAlgalonPhased(bot));
    return !hole || FindHoleDetour(bot, *state, *hole, to, next);
}

bool AlgalonPhasedInField(Player* bot)
{
    return IsAlgalonPhased(bot) && AlgalonHoleNear(bot, ULDUAR_ALGALON_HOLE_EXIT_RADIUS);
}

bool AlgalonStarWithin(PlayerbotAI* botAI, Position const& spot, float radius, ObjectGuid const& except)
{
    AlgalonEncounterState const* state = FindState(botAI ? botAI->GetBot() : nullptr);
    if (!state)
        return false;

    for (AlgalonScanUnit const& star : state->stars)
        if (star.guid != except && spot.GetExactDist2d(star.position) <= radius)
            return true;

    return false;
}

bool AlgalonHeadsForFarHole(Player* bot)
{
    // Rule predicates ask this for every action: answer the common case before deriving a spot.
    AlgalonEncounterState const* state = FindState(bot);
    if (!state || !state->engaged || state->holes.empty() ||
        (!state->bigBangCasting && !BigBangDueWithin(*state, ULDUAR_ALGALON_FAR_HOLE_SECONDS, getMSTime())))
        return false;

    Position spot;
    char const* kind = RawSlot(bot, spot);
    return kind && std::strcmp(kind, "nearhole") == 0;
}

bool AlgalonTakesRingSlot(Player* bot)
{
    return bot && PlayerbotAI::IsRanged(bot) && !PlayerbotAI::IsMainTank(bot) &&
           !PlayerbotAI::IsAssistTankOfIndex(bot, 0, true);
}

bool AlgalonMeleeStaged(Player* bot)
{
    PlayerbotAI* botAI = bot ? GET_PLAYERBOT_AI(bot) : nullptr;
    AlgalonEncounterState const* state = FindState(bot);
    return botAI && state && !state->meleeReleased && !PlayerbotAI::IsRanged(bot) && !IsAlgalonSwapTank(bot) &&
           AlgalonPresent(botAI);
}

bool TryGetAlgalonSlotPosition(uint8 slotIndex, Position& position)
{
    if (slotIndex >= ULDUAR_ALGALON_TOTAL_SLOTS)
        return false;

    position = ULDUAR_ALGALON_RING_SLOTS[slotIndex];
    return true;
}

bool TryGetAlgalonSlot(Player* bot, Position& position, bool* waypoint)
{
    if (!bot)
        return false;

    bool found = false;
    char const* branch = DeriveSlot(bot, position, found);

    if (waypoint)
        *waypoint = std::strcmp(branch, "blocked") == 0;

    return found;
}

bool GetAlgalonSlotAnchor(Player* bot, Position& slot) { return bot && RawSlot(bot, slot); }

bool GetAlgalonDodgeAnchor(Player* bot, Position& anchor)
{
    AlgalonEncounterState const* state = FindState(bot);
    if (state && state->engaged && !IsAlgalonPhased(bot) && !AlgalonStaysOut(bot) &&
        SmashLandsInCast(*state, getMSTime()))
    {
        // Before the cast the latch is the last cast's.
        AlgalonScanUnit const* hole =
            state->bigBangCasting ? ShelterHole(bot, *state) : NearestShelter(*state, bot->GetPosition());
        if (hole)
        {
            anchor = hole->position;
            return true;
        }
    }

    return GetAlgalonSlotAnchor(bot, anchor);
}

bool AlgalonHiderInCast(Player* bot)
{
    return AlgalonBigBangLatched(bot) && !IsAlgalonPhased(bot) && !AlgalonStaysOut(bot);
}

void AppendAlgalonTargetExclusions(PlayerbotAI* botAI, GuidSet& exclusions)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    AlgalonEncounterState const* state = FindState(bot);
    if (!state || !state->engaged)
        return;

    if (AlgalonYieldsBoss(bot))
        exclusions.insert(state->boss);

    bool const starTeam = IsAlgalonStarTeam(bot);
    for (AlgalonScanUnit const& unit : state->stars)
        if (!starTeam || unit.guid != state->focusStar)
            exclusions.insert(unit.guid);

    bool const handler = state->handler == bot->GetGUID();
    if (!handler)
        for (AlgalonScanUnit const& unit : state->constellations)
            exclusions.insert(unit.guid);

    // With no bot to collect them, Dark Matter is left to whoever it chases rather than to nobody.
    if (!handler && !state->handler.IsEmpty())
        for (AlgalonScanUnit const& unit : state->darkMatter)
            exclusions.insert(unit.guid);
}

bool AlgalonMayDamage(Player* bot, Unit* target)
{
    AlgalonEncounterState const* state = FindState(bot);
    if (!state || !state->engaged || !target)
        return true;

    // Ahead of the skull, which raids put on him: a yielding tank hitting him takes him back by threat.
    if (target->GetEntry() == PB_NPC_ALGALON)
        return !AlgalonYieldsBoss(bot);

    // A skull a player put there is a call the raid made; "attack rti target" ignores the exclusions.
    ObjectGuid const guid = target->GetGUID();
    Group* group = bot->GetGroup();
    if (group && group->GetTargetIcon(RtiTargetValue::skullIndex) == guid)
        return true;

    bool const handler = state->handler == bot->GetGUID();
    switch (target->GetEntry())
    {
        case PB_NPC_COLLAPSING_STAR:
            return guid == state->focusStar && IsAlgalonStarTeam(bot);
        case PB_NPC_LIVING_CONSTELLATION:
            return handler;
        case PB_NPC_UNLEASHED_DARK_MATTER:
            return handler || state->handler.IsEmpty();
        default:
            return true;
    }
}
