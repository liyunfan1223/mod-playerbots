/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldEncounter_Thorim.h"

#include "Creature.h"
#include "DBCEnums.h"
#include "EncounterHelpers.h"
#include "Group.h"
#include "Map.h"
#include "MotionMaster.h"
#include "PetDefines.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RaidDifficulty.h"
#include "RaidHumanTankCall.h"
#include "RaidInstanceState.h"
#include "RaidTankDefensive.h"
#include "RtiTargetValue.h"
#include "Timer.h"
#include "UldScripts.h"
#include "UldTargets.h"
#include "Unit.h"
#include "WorldSession.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <list>
#include <string>
#include <vector>

using namespace EncounterHelpers;

const Position ULDUAR_THORIM_NEAR_ARENA_CENTER = Position(2134.9854f, -263.11853f, 419.8465f);
const Position ULDUAR_THORIM_NEAR_ENTRANCE_POSITION = Position(2172.4355f, -258.27957f, 418.47162f);
const Position ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_6_YARDS_1 = Position(2237.6187f, -265.08844f, 412.17548f);
const Position ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_6_YARDS_2 = Position(2237.2498f, -275.81122f, 412.17548f);
const Position ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_5_YARDS_1 = Position(2236.895f, -294.62448f, 412.1348f);
const Position ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_10_YARDS_1 = Position(2242.1162f, -310.15308f, 412.1348f);
const Position ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_10_YARDS_2 = Position(2242.018f, -318.66003f, 412.1348f);
const Position ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_10_YARDS_3 = Position(2242.1904f, -329.0533f, 412.1348f);
const Position ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_6_YARDS_1 = Position(2219.5417f, -264.77167f, 412.17548f);
const Position ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_6_YARDS_2 = Position(2217.446f, -275.85248f, 412.17548f);
const Position ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_5_YARDS_1 = Position(2217.8877f, -295.01193f, 412.13434f);
const Position ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_10_YARDS_1 = Position(2212.193f, -307.44992f, 412.1348f);
const Position ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_10_YARDS_2 = Position(2212.1353f, -318.20795f, 412.1348f);
const Position ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_10_YARDS_3 = Position(2212.1956f, -328.0144f, 412.1348f);
// 1 and 5 sit in the 18 yd necks (x 2126-2144), 2 to 4 hug the east wall, clear of the traps on the centreline.
// Z is the navmesh's settled height: the walk moves on exact waypoints and never corrects it.
const Position ULDUAR_THORIM_BALCONY_1 = Position(2141.0f, -408.0f, 438.247f);
const Position ULDUAR_THORIM_BALCONY_2 = Position(2151.0f, -396.0f, 438.247f);
const Position ULDUAR_THORIM_BALCONY_3 = Position(2152.0f, -365.0f, 438.744f);
const Position ULDUAR_THORIM_BALCONY_4 = Position(2151.0f, -332.0f, 438.247f);
const Position ULDUAR_THORIM_BALCONY_5 = Position(2137.5f, -318.0f, 438.222f);
const Position ULDUAR_THORIM_JUMP_START_POINT = Position(2137.137f, -291.19025f, 438.24753f, 1.7059844f);
const Position ULDUAR_THORIM_JUMP_END_POINT = Position(2137.8818f, -278.18942f, 419.66653f);
// Z as the navmesh settles each point, for the same reason as the waypoints above.
const Position ULDUAR_THORIM_BALCONY_HOLD1_SPOT = Position(2126.0f, -294.0f, 438.247f);
const Position ULDUAR_THORIM_BALCONY_HOLD2_SPOT = Position(2138.0f, -296.0f, 438.247f);
const Position ULDUAR_THORIM_BALCONY_HOLD3_SPOT = Position(2150.0f, -297.0f, 438.247f);
const Position ULDUAR_THORIM_BALCONY_HOLD4_SPOT = Position(2132.0f, -307.0f, 438.243f);
const Position ULDUAR_THORIM_BALCONY_HOLD5_SPOT = Position(2144.0f, -307.0f, 438.243f);
const Position ULDUAR_THORIM_BALCONY_HOLD6_SPOT = Position(2138.0f, -318.0f, 438.222f);
const Position ULDUAR_THORIM_PHASE2_TANK_SPOT = Position(2110.7483f, -252.65265f, 419.440f);
const Position ULDUAR_THORIM_PHASE2_RANGE1_SPOT = Position(2123.00f, -282.00f, 419.528f);
const Position ULDUAR_THORIM_PHASE2_RANGE2_SPOT = Position(2124.50f, -270.50f, 419.729f);
const Position ULDUAR_THORIM_PHASE2_RANGE3_SPOT = Position(2137.50f, -269.00f, 419.845f);
const Position ULDUAR_THORIM_PHASE2_RANGE4_SPOT = Position(2132.50f, -257.50f, 419.845f);
const Position ULDUAR_THORIM_PHASE2_RANGE5_SPOT = Position(2142.00f, -250.50f, 419.691f);
const Position ULDUAR_THORIM_PHASE2_RANGE6_SPOT = Position(2131.50f, -245.00f, 419.612f);
const Position ULDUAR_THORIM_PHASE2_OPENING1_SPOT = Position(2114.0f, -232.0f, 420.146f);
const Position ULDUAR_THORIM_PHASE2_OPENING2_SPOT = Position(2128.0f, -226.0f, 420.146f);
const Position ULDUAR_THORIM_PHASE2_OPENING3_SPOT = Position(2140.0f, -232.0f, 419.337f);
const Position ULDUAR_THORIM_PHASE2_OPENING4_SPOT = Position(2146.0f, -244.0f, 419.531f);
const Position ULDUAR_THORIM_PHASE2_MELEE1_SPOT = Position(2118.75f, -252.65f, 419.596f);
const Position ULDUAR_THORIM_PHASE2_MELEE2_SPOT = Position(2110.75f, -244.65f, 419.359f);
const Position ULDUAR_THORIM_PHASE2_MELEE3_SPOT = Position(2110.75f, -260.65f, 419.485f);
// 3.5 yd off the tank spot, so a taunt trade steps him under the ring's reposition tolerance and the ring
// keeps its latch. Z as the navmesh settles it.
const Position ULDUAR_THORIM_PHASE2_OFFTANK_SPOT = Position(2114.25f, -252.65f, 419.508f);

namespace
{

// Keyed by instance: trigger, action and multiplier each hold their own helper instance, so the state
// they must agree on is defined here and nowhere else.
RaidInstanceState<ThorimEncounterState> thorimStates;

ThorimEncounterState& ThorimStateFor(Player* bot) { return thorimStates.For(bot->GetInstanceId()); }

struct GauntletWaypoint
{
    Position const* position;
    // How close counts as standing here. Hand-tuned, wider where the lane has room around the spot.
    float radius;
};

std::array<GauntletWaypoint, ULDUAR_THORIM_GAUNTLET_WAYPOINTS> const leftLaneWaypoints = {{
    {&ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_6_YARDS_1, 6.0f},
    {&ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_6_YARDS_2, 6.0f},
    {&ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_5_YARDS_1, 5.0f},
    {&ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_10_YARDS_1, 10.0f},
    {&ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_10_YARDS_2, 10.0f},
    {&ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_10_YARDS_3, 10.0f},
}};

std::array<GauntletWaypoint, ULDUAR_THORIM_GAUNTLET_WAYPOINTS> const rightLaneWaypoints = {{
    {&ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_6_YARDS_1, 6.0f},
    {&ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_6_YARDS_2, 6.0f},
    {&ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_5_YARDS_1, 5.0f},
    {&ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_10_YARDS_1, 10.0f},
    {&ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_10_YARDS_2, 10.0f},
    {&ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_10_YARDS_3, 10.0f},
}};

std::array<GauntletWaypoint, ULDUAR_THORIM_GAUNTLET_WAYPOINTS> const& LaneWaypoints(bool leftLane)
{
    return leftLane ? leftLaneWaypoints : rightLaneWaypoints;
}

// Walked north, so y rises across the chain and the last entry is the jump start.
std::array<Position const*, ULDUAR_THORIM_BALCONY_WAYPOINTS> const balconyWaypoints = {{
    &ULDUAR_THORIM_BALCONY_1,
    &ULDUAR_THORIM_BALCONY_2,
    &ULDUAR_THORIM_BALCONY_3,
    &ULDUAR_THORIM_BALCONY_4,
    &ULDUAR_THORIM_BALCONY_5,
    &ULDUAR_THORIM_JUMP_START_POINT,
}};

ThorimEncounterState* FindState(Player const* bot)
{
    return bot ? thorimStates.Find(bot->GetInstanceId()) : nullptr;
}

// Upper half of the NearThorimEncounter gate. By distance alone Hodir's room passes, and everything
// behind the gate sweeps the grid or writes raid-wide state.
bool OnThorimBalcony(WorldObject const* who)
{
    if (!who)
        return false;

    float const x = who->GetPositionX();
    float const y = who->GetPositionY();
    float const z = who->GetPositionZ();

    return z >= ULDUAR_THORIM_WING_MAX_Z && z <= ULDUAR_THORIM_BALCONY_BOX_MAX_Z &&
           x >= ULDUAR_THORIM_BALCONY_BOX_MIN_X && x <= ULDUAR_THORIM_BALCONY_BOX_MAX_X &&
           y >= ULDUAR_THORIM_BALCONY_BOX_MIN_Y && y <= ULDUAR_THORIM_BALCONY_BOX_MAX_Y;
}

bool MemberCounts(Player const* member, uint32 instanceId)
{
    return member && member->IsAlive() && member->GetMapId() == ULDUAR_MAP_ID &&
           member->GetInstanceId() == instanceId;
}

// Alive or not, on purpose: dropping the dead from the count reshuffles the formation over one death.
bool HoldsFormationSlot(Player const* member, uint32 instanceId)
{
    return member && member->GetMapId() == ULDUAR_MAP_ID && member->GetInstanceId() == instanceId;
}

// One predicate for trigger and action, or a healer can get a different slot on alternating ticks.
bool TakesRangedSpot(Player* member)
{
    return member && !PlayerbotAI::IsTank(member) &&
           (PlayerbotAI::IsRanged(member) || PlayerbotAI::IsHeal(member));
}

bool TakesMeleeSlot(Player* member)
{
    // No tank, whatever its index says. A ring point is measured off the boss, so a tank standing on one
    // orbits him instead of walking him to the anchor. A third tank waits on the off-tank bearing.
    return member && !TakesRangedSpot(member) && !PlayerbotAI::IsTank(member);
}

// Least-loaded slot rather than first-free, so six melee land three-and-three instead of piling onto
// one bearing once the ring is full.
void EnsureMeleeSlot(Player* bot)
{
    ThorimEncounterState& state = ThorimStateFor(bot);

    Group* group = bot->GetGroup();
    if (!group)
    {
        state.meleeSlots[bot->GetGUID()] = 0;
        return;
    }

    uint32 const instanceId = bot->GetInstanceId();

    std::unordered_set<ObjectGuid> present;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();

        // Not MemberCounts: a slot dropped while a bot is dead comes back as whichever is least
        // loaded, which swings its bearing a quarter turn the moment it is rezzed.
        if (HoldsFormationSlot(member, instanceId) && TakesMeleeSlot(member))
            present.insert(member->GetGUID());
    }

    for (auto itr = state.meleeSlots.begin(); itr != state.meleeSlots.end();)
        itr = present.count(itr->first) ? std::next(itr) : state.meleeSlots.erase(itr);

    if (state.meleeSlots.count(bot->GetGUID()))
        return;

    std::array<uint8, ULDUAR_THORIM_MELEE_SLOTS> load = {};
    for (auto const& assignment : state.meleeSlots)
        if (assignment.second < ULDUAR_THORIM_MELEE_SLOTS)
            ++load[assignment.second];

    uint8 chosen = 0;
    for (uint8 slot = 1; slot < ULDUAR_THORIM_MELEE_SLOTS; ++slot)
        if (load[slot] < load[chosen])
            chosen = slot;

    state.meleeSlots[bot->GetGUID()] = chosen;
}

// Same shape as EnsureMeleeSlot. Latched, since a per-call round robin renumbers the camp behind every death.
void EnsureRangedSlot(Player* bot)
{
    ThorimEncounterState& state = ThorimStateFor(bot);

    Group* group = bot->GetGroup();
    if (!group)
    {
        state.rangedSlots[bot->GetGUID()] = 0;
        return;
    }

    uint32 const instanceId = bot->GetInstanceId();

    std::unordered_set<ObjectGuid> present;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (HoldsFormationSlot(member, instanceId) && TakesRangedSpot(member))
            present.insert(member->GetGUID());
    }

    for (auto itr = state.rangedSlots.begin(); itr != state.rangedSlots.end();)
        itr = present.count(itr->first) ? std::next(itr) : state.rangedSlots.erase(itr);

    if (state.rangedSlots.count(bot->GetGUID()))
        return;

    std::array<uint8, ULDUAR_THORIM_RANGED_SLOTS> load = {};
    for (auto const& assignment : state.rangedSlots)
        if (assignment.second < ULDUAR_THORIM_RANGED_SLOTS)
            ++load[assignment.second];

    uint8 chosen = 0;
    for (uint8 slot = 1; slot < ULDUAR_THORIM_RANGED_SLOTS; ++slot)
        if (load[slot] < load[chosen])
            chosen = slot;

    state.rangedSlots[bot->GetGUID()] = chosen;
}

bool RangedSlotOf(Player* bot, uint8& slot)
{
    ThorimEncounterState const* state = FindState(bot);
    if (!state)
        return false;

    auto const itr = state->rangedSlots.find(bot->GetGUID());
    if (itr == state->rangedSlots.end())
        return false;

    slot = itr->second;
    return true;
}

std::array<Position const*, ULDUAR_THORIM_OPENING_SLOTS> const openingSpots = {{
    &ULDUAR_THORIM_PHASE2_OPENING1_SPOT,
    &ULDUAR_THORIM_PHASE2_OPENING2_SPOT,
    &ULDUAR_THORIM_PHASE2_OPENING3_SPOT,
    &ULDUAR_THORIM_PHASE2_OPENING4_SPOT,
}};

std::array<Position const*, ULDUAR_THORIM_BALCONY_HOLD_SLOTS> const balconyHoldSpots = {{
    &ULDUAR_THORIM_BALCONY_HOLD1_SPOT,
    &ULDUAR_THORIM_BALCONY_HOLD2_SPOT,
    &ULDUAR_THORIM_BALCONY_HOLD3_SPOT,
    &ULDUAR_THORIM_BALCONY_HOLD4_SPOT,
    &ULDUAR_THORIM_BALCONY_HOLD5_SPOT,
    &ULDUAR_THORIM_BALCONY_HOLD6_SPOT,
}};

// Another member's squad, read straight off the split. GetThorimSquad only answers for the asking bot.
// No entry is the arena, same as there.
ThorimSquad SquadOf(ThorimEncounterState const& state, Player const* member)
{
    auto const itr = state.squads.find(member->GetGUID());
    return itr == state.squads.end() ? ThorimSquad::Arena : static_cast<ThorimSquad>(itr->second);
}

// Least-loaded, ties to the spot nearest the bot: the arena squad starts ringed round the landing
// point, and an index pick walks some of them straight through it.
template <size_t N, typename TakesSlot>
void EnsureNearestSlot(Player* bot, std::unordered_map<ObjectGuid, uint8>& slots,
                       std::array<Position const*, N> const& spots, TakesSlot const& takesSlot)
{
    Group* group = bot->GetGroup();
    if (!group)
    {
        slots[bot->GetGUID()] = 0;
        return;
    }

    uint32 const instanceId = bot->GetInstanceId();

    std::unordered_set<ObjectGuid> present;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (HoldsFormationSlot(member, instanceId) && takesSlot(member))
            present.insert(member->GetGUID());
    }

    for (auto itr = slots.begin(); itr != slots.end();)
        itr = present.count(itr->first) ? std::next(itr) : slots.erase(itr);

    if (slots.count(bot->GetGUID()))
        return;

    std::array<uint8, N> load = {};
    for (auto const& assignment : slots)
        if (assignment.second < N)
            ++load[assignment.second];

    uint8 chosen = 0;
    for (uint8 slot = 1; slot < N; ++slot)
    {
        bool const nearer = bot->GetExactDist2d(spots[slot]) < bot->GetExactDist2d(spots[chosen]);
        if (load[slot] < load[chosen] || (load[slot] == load[chosen] && nearer))
            chosen = slot;
    }

    slots[bot->GetGUID()] = chosen;
}

void EnsureOpeningSlot(Player* bot)
{
    ThorimEncounterState& state = ThorimStateFor(bot);
    EnsureNearestSlot(bot, state.openingSlots, openingSpots,
                      [&state](Player* member)
                      { return TakesRangedSpot(member) && SquadOf(state, member) != ThorimSquad::Gauntlet; });
}

void EnsureBalconyHoldSlot(Player* bot)
{
    ThorimEncounterState& state = ThorimStateFor(bot);
    EnsureNearestSlot(bot, state.balconyHoldSlots, balconyHoldSpots,
                      [&state](Player* member)
                      { return TakesRangedSpot(member) && SquadOf(state, member) == ThorimSquad::Gauntlet; });
}

bool SlotOf(std::unordered_map<ObjectGuid, uint8> const& slots, Player* bot, uint8& slot)
{
    auto const itr = slots.find(bot->GetGUID());
    if (itr == slots.end())
        return false;

    slot = itr->second;
    return true;
}

bool IsBotPlayer(Player const* member)
{
    WorldSession const* session = member ? member->GetSession() : nullptr;
    return session && session->IsHeadless();
}

// Latched once per pull. Bot-only picks: nothing here can walk a human, so a human in a gauntlet slot
// leaves the corridor a body short.
void AssignThorimSquads(Player* bot)
{
    ThorimEncounterState& state = ThorimStateFor(bot);
    if (state.squadsAssigned)
        return;

    Group* group = bot->GetGroup();
    if (!group)
    {
        state.squads[bot->GetGUID()] = static_cast<uint8>(ThorimSquad::Arena);
        state.squadsAssigned = true;
        return;
    }

    uint32 const instanceId = bot->GetInstanceId();

    std::vector<Player*> roster;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (MemberCounts(member, instanceId))
            roster.push_back(member);
    }

    // Nobody here yet means the raid is still zoning in; try again rather than latching an empty split.
    if (roster.empty())
        return;

    // GetMainTankGuid's fallback only sees tanks already alive in the instance. Latching while it reads
    // empty sends the only tank down the corridor and leaves the arena untanked.
    if (PlayerbotAI::GetMainTankGuid(group).IsEmpty())
        return;

    bool const twentyFive = IsRaid25Man(bot);
    uint32 healerQuota = twentyFive ? 2 : 1;

    // Sized to beat the ~170 s hard mode clock. The arena floor cap below still wins over it.
    uint32 dpsQuota = twentyFive ? 9 : 4;

    size_t const floorSize = static_cast<size_t>(ULDUAR_THORIM_ARENA_MIN_MEMBERS);
    size_t const cap = roster.size() > floorSize ? roster.size() - floorSize : 0;

    std::unordered_set<ObjectGuid> gauntlet;
    auto take = [&gauntlet, cap](Player* member)
    {
        if (gauntlet.size() >= cap)
            return false;

        gauntlet.insert(member->GetGUID());
        return true;
    };

    Player* gauntletTank = nullptr;
    for (Player* member : roster)
        if (IsBotPlayer(member) && PlayerbotAI::IsAssistTankOfIndex(member, 0))
        {
            gauntletTank = member;
            break;
        }

    if (!gauntletTank)
        for (Player* member : roster)
            if (IsBotPlayer(member) && PlayerbotAI::IsTank(member) && !PlayerbotAI::IsMainTank(member))
            {
                gauntletTank = member;
                break;
            }

    // A one-tank raid has none to spare. Backs up the IsMainTank guard for when GetMainTankGuid reads wrong.
    size_t tankCount = 0;
    for (Player* member : roster)
        if (PlayerbotAI::IsTank(member))
            ++tankCount;

    // The main tank holds the arena whatever the quota says: he is the one thing the adds and the
    // phase 2 pickup both need to still be standing there.
    if (gauntletTank && tankCount > 1 && !PlayerbotAI::IsMainTank(gauntletTank))
        take(gauntletTank);
    else
        ++dpsQuota;

    for (Player* member : roster)
    {
        if (!healerQuota)
            break;

        if (!IsBotPlayer(member) || gauntlet.count(member->GetGUID()) || PlayerbotAI::IsMainTank(member))
            continue;

        if (!PlayerbotAI::IsHeal(member) || !take(member))
            continue;

        --healerQuota;
    }

    for (Player* member : roster)
    {
        if (!dpsQuota)
            break;

        if (!IsBotPlayer(member) || gauntlet.count(member->GetGUID()) || PlayerbotAI::IsMainTank(member))
            continue;

        if (!PlayerbotAI::IsDps(member) || !take(member))
            continue;

        --dpsQuota;
    }

    // Bots only: humans are never picked, so filing them here labels one walking the corridor as arena.
    // TickHumanSquads reads theirs off position.
    for (Player* member : roster)
        if (IsBotPlayer(member))
            state.squads[member->GetGUID()] =
                static_cast<uint8>(gauntlet.count(member->GetGUID()) ? ThorimSquad::Gauntlet : ThorimSquad::Arena);

    state.squadsAssigned = true;
}

// Labels humans by position, which SquadOf reads for them. The latched split is never touched.
void TickHumanSquads(Player* bot)
{
    ThorimEncounterState& state = ThorimStateFor(bot);
    if (state.humanSquadScanMs &&
        GetMSTimeDiffToNow(state.humanSquadScanMs) < ULDUAR_THORIM_ENCOUNTER_SCAN_INTERVAL_MS)
        return;

    state.humanSquadScanMs = getMSTime();

    Group* group = bot->GetGroup();
    if (!group)
        return;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || IsBotPlayer(member) || member->GetMapId() != ULDUAR_MAP_ID)
            continue;

        // The same boundary the arena leash and the pet leash use, so "in the arena" means one thing
        // across the encounter. Anywhere else near Thorim is the corridor or the hallway above it.
        state.squads[member->GetGUID()] =
            static_cast<uint8>(ThorimInArenaBox(member) ? ThorimSquad::Arena : ThorimSquad::Gauntlet);
    }
}

bool MeleeSlotOf(Player* bot, uint8& slot)
{
    ThorimEncounterState const* state = FindState(bot);
    if (!state)
        return false;

    auto const itr = state->meleeSlots.find(bot->GetGUID());
    if (itr == state->meleeSlots.end())
        return false;

    slot = itr->second;
    return true;
}

// Off the tank spot, not the live tank, so neither the boss shuffling nor a tank's sidestep or death
// swings the ring.
float RingAnchorBearing(Unit* boss)
{
    return std::atan2(ULDUAR_THORIM_PHASE2_TANK_SPOT.GetPositionY() - boss->GetPositionY(),
                      ULDUAR_THORIM_PHASE2_TANK_SPOT.GetPositionX() - boss->GetPositionX());
}

float SlotBearing(float anchor, uint8 slot)
{
    // Three slots at 90 degree steps starting a quarter turn off the tank, which leaves the whole
    // tank side of the boss empty for the two of them.
    return Position::NormalizeOrientation(anchor + (static_cast<float>(M_PI) / 2.0f) * static_cast<float>(slot + 1));
}

float AbsAngleDelta(float first, float second)
{
    float delta = std::fabs(Position::NormalizeOrientation(first) - Position::NormalizeOrientation(second));
    if (delta > static_cast<float>(M_PI))
        delta = 2.0f * static_cast<float>(M_PI) - delta;

    return delta;
}

float BearingFromBoss(Unit* boss, float x, float y)
{
    return std::atan2(y - boss->GetPositionY(), x - boss->GetPositionX());
}

// The margin on top of the spell's own 75 degree arc covers Thorim re-orienting onto the orb between
// the tick that picks a way out and the tick a bot finishes walking it.
bool InLightningChargeCone(float bearing, float coneBearing)
{
    float const halfWidth = ULDUAR_THORIM_LIGHTNING_CHARGE_CONE_ANGLE / 2.0f + ULDUAR_THORIM_LIGHTNING_CHARGE_MARGIN;
    return AbsAngleDelta(bearing, coneBearing) <= halfWidth;
}

// Live Blizzard zones plus the bunny, which marks where the next zone drops within 2 s. Positions, since
// a stale guid would need re-resolving on every bearing.
std::vector<Position> const& ThorimBlizzardSpots(Player* bot)
{
    ThorimEncounterState& state = ThorimStateFor(bot);
    if (state.blizzardScanMs && GetMSTimeDiffToNow(state.blizzardScanMs) < ULDUAR_THORIM_ENCOUNTER_SCAN_INTERVAL_MS)
        return state.blizzardSpots;

    state.blizzardScanMs = getMSTime();
    state.blizzardSpots.clear();

    std::list<Creature*> bunnies;
    bot->GetCreatureListWithEntryInGrid(bunnies, NPC_SIF_BLIZZARD, ULDUAR_THORIM_BLIZZARD_SCAN_RANGE);
    for (Creature* bunny : bunnies)
        if (bunny && bunny->IsAlive())
            state.blizzardSpots.emplace_back(bunny->GetPositionX(), bunny->GetPositionY(), bunny->GetPositionZ());

    for (uint32 const zoneSpell : {SPELL_SIF_BLIZZARD_ZONE_10, SPELL_SIF_BLIZZARD_ZONE_25})
        for (Position const& zone : GetDynamicObjectPositions(bot, ULDUAR_THORIM_BLIZZARD_SCAN_RANGE, zoneSpell))
            state.blizzardSpots.push_back(zone);

    return state.blizzardSpots;
}

bool RingBearingClearOfBlizzard(Unit* boss, float bearing, std::vector<Position> const& zones)
{
    float const x = boss->GetPositionX() + std::cos(bearing) * ULDUAR_THORIM_MELEE_RING_RADIUS;
    float const y = boss->GetPositionY() + std::sin(bearing) * ULDUAR_THORIM_MELEE_RING_RADIUS;

    for (Position const& zone : zones)
        if (zone.GetExactDist2d(x, y) < ULDUAR_THORIM_RING_BLIZZARD_CLEARANCE)
            return false;

    return true;
}

// Breaks the deadband for a dodge, which at r 8 is a 5 yd chord or less and would never clear it.
// Only when the bot stands in the hazard and the spot does not.
bool RingSpotBeatsTheDeadband(PlayerbotAI* botAI, Player* bot, Position const& spot)
{
    // Cone first: 20k in one instant against a Blizzard tick's 4.3-4.9k.
    if (Unit* boss = GetThorim(botAI))
    {
        if (Unit* orb = ThorimChargedThunderOrb(botAI, SPELL_THORIM_LIGHTNING_ORB_VISUAL))
        {
            float const coneBearing = BearingFromBoss(boss, orb->GetPositionX(), orb->GetPositionY());
            if (InLightningChargeCone(BearingFromBoss(boss, bot->GetPositionX(), bot->GetPositionY()), coneBearing) &&
                !InLightningChargeCone(BearingFromBoss(boss, spot.GetPositionX(), spot.GetPositionY()), coneBearing))
                return true;
        }
    }

    std::vector<Position> const& zones = ThorimBlizzardSpots(bot);
    if (zones.empty())
        return false;

    bool standingInOne = false;
    for (Position const& zone : zones)
    {
        if (zone.GetExactDist2d(spot.GetPositionX(), spot.GetPositionY()) < ULDUAR_THORIM_RING_BLIZZARD_CLEARANCE)
            return false;

        if (zone.GetExactDist2d(bot->GetPositionX(), bot->GetPositionY()) < ULDUAR_THORIM_RING_BLIZZARD_CLEARANCE)
            standingInOne = true;
    }

    return standingInOne;
}

// Settled onto the mesh, since MoveTo fails silently off it. The range check catches the floor hole south of
// y -288 and is no melee-range guard: GetDistance subtracts his reach, so it only rejects past 15.3 yd.
bool RingPoint(Player* bot, Unit* boss, float bearing, Position& out)
{
    float x = boss->GetPositionX() + std::cos(bearing) * ULDUAR_THORIM_MELEE_RING_RADIUS;
    float y = boss->GetPositionY() + std::sin(bearing) * ULDUAR_THORIM_MELEE_RING_RADIUS;
    float z = bot->GetMapWaterOrGroundLevel(x, y, boss->GetPositionZ());
    if (z <= INVALID_HEIGHT)
        z = boss->GetPositionZ();

    bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
                                                   x, y, z, false);

    Position const candidate(x, y, z);
    if (boss->GetDistance(candidate) > boss->GetMeleeRange(bot))
        return false;

    out = candidate;
    return true;
}

Position const& RangedSpot(uint8 slot)
{
    static Position const* const spots[ULDUAR_THORIM_RANGED_SLOTS] = {
        &ULDUAR_THORIM_PHASE2_RANGE1_SPOT, &ULDUAR_THORIM_PHASE2_RANGE2_SPOT, &ULDUAR_THORIM_PHASE2_RANGE3_SPOT,
        &ULDUAR_THORIM_PHASE2_RANGE4_SPOT, &ULDUAR_THORIM_PHASE2_RANGE5_SPOT, &ULDUAR_THORIM_PHASE2_RANGE6_SPOT};

    return *spots[std::min<uint8>(slot, ULDUAR_THORIM_RANGED_SLOTS - 1)];
}

// The seven Thunder Orbs, in the order the shelter table below indexes them. Fixed props, one per
// pillar bunny at the same x,y, so this is only ever used to turn the lit orb into a table index.
std::array<Position, 7> const ULDUAR_THORIM_THUNDER_ORB_SPOTS = {
    Position(2145.50f, -222.62f, 433.30f), Position(2164.20f, -233.47f, 433.30f),
    Position(2164.55f, -293.00f, 433.30f), Position(2105.04f, -292.56f, 433.30f),
    Position(2092.95f, -263.00f, 433.30f), Position(2104.94f, -233.44f, 433.30f),
    Position(2124.30f, -222.60f, 433.30f)};

// A camp slot's spot while that orb is lit, solved offline and checked on the navmesh: 42.5 deg off the cone, 22+ yd
// off the tank spot, within 32 of the boss, 9+ from every other body (Chain Lightning jumps 8).
struct ThorimShelterSpot
{
    uint8 orbIndex;
    uint8 slot;
    Position spot;
};

std::array<ThorimShelterSpot, 12> const ULDUAR_THORIM_SHELTER_SPOTS = {{
    {0, 4, Position(2142.00f, -255.35f, 419.771f)},
    {0, 5, Position(2113.00f, -227.35f, 420.293f)},
    {1, 3, Position(2132.00f, -276.35f, 419.755f)},
    {1, 4, Position(2130.50f, -262.85f, 419.905f)},
    {1, 5, Position(2119.50f, -226.85f, 420.146f)},
    {2, 0, Position(2116.50f, -283.35f, 419.509f)},
    {2, 1, Position(2107.50f, -283.35f, 420.104f)},
    {2, 2, Position(2114.50f, -274.35f, 419.562f)},
    {2, 3, Position(2140.00f, -241.35f, 419.502f)},
    {3, 0, Position(2129.50f, -278.35f, 419.702f)},
    // Both under 3 yd from a home just inside the arc: they walk only because the camp moves at 1 yd.
    {3, 1, Position(2126.00f, -269.85f, 419.761f)},
    {6, 5, Position(2133.50f, -246.85f, 419.652f)},
}};

// Refuses a loose match rather than picking the nearest: a wrong index walks the bot into the cone,
// staying home doesn't.
bool ThorimThunderOrbIndex(Unit* orb, uint8& index)
{
    for (size_t i = 0; i < ULDUAR_THORIM_THUNDER_ORB_SPOTS.size(); ++i)
    {
        if (ULDUAR_THORIM_THUNDER_ORB_SPOTS[i].GetExactDist2d(orb->GetPositionX(), orb->GetPositionY()) <=
            ULDUAR_THORIM_THUNDER_ORB_MATCH_RADIUS)
        {
            index = static_cast<uint8>(i);
            return true;
        }
    }

    return false;
}

bool ThorimShelterFor(uint8 orbIndex, uint8 slot, Position& out)
{
    for (ThorimShelterSpot const& row : ULDUAR_THORIM_SHELTER_SPOTS)
    {
        if (row.orbIndex == orbIndex && row.slot == slot)
        {
            out = row.spot;
            return true;
        }
    }

    return false;
}

// Camp cone test, with its own 2.5 deg margin: boss drift barely swings bearings at camp range.
bool InLightningChargeConeRanged(float bearing, float coneBearing)
{
    float const halfWidth =
        ULDUAR_THORIM_LIGHTNING_CHARGE_CONE_ANGLE / 2.0f + ULDUAR_THORIM_LIGHTNING_CHARGE_RANGED_MARGIN;
    return AbsAngleDelta(bearing, coneBearing) <= halfWidth;
}

// Home, or the shelter while a covering orb is lit. Home the moment it goes dark, so nobody is still
// walking when the next Chain Lightning lands. Returns whether it sheltered, and off which orb.
bool ThorimRangedSpot(PlayerbotAI* botAI, Player* bot, Unit* boss, uint8 slot, Position& out, uint8& orbIndex)
{
    out = RangedSpot(slot);

    // Resolved before the state reference is taken, because it goes through ThorimStateFor itself.
    Unit* orb = ThorimChargedThunderOrb(botAI, SPELL_THORIM_LIGHTNING_ORB_VISUAL);

    ThorimEncounterState::RangedShelter& held = ThorimStateFor(bot).rangedShelters[bot->GetGUID()];

    if (!orb)
    {
        held = {};
        return false;
    }

    if (held.orb != orb->GetGUID())
    {
        held.orb = orb->GetGUID();
        held.sheltered = false;

        // An orb we cannot place is left unlatched, so the next tick tries again instead of holding a
        // bad index for the whole 15s.
        if (!ThorimThunderOrbIndex(orb, held.orbIndex))
            held.orb.Clear();
    }

    Position shelter;

    // Re-tested every tick while the orb is lit and we have not committed, so a boss that drifts into
    // covering this slot part way through the warning still gets an answer. Only ever false to true.
    if (!held.sheltered && held.orb == orb->GetGUID())
    {
        float const bearing = BearingFromBoss(boss, out.GetPositionX(), out.GetPositionY());
        float const coneBearing = BearingFromBoss(boss, orb->GetPositionX(), orb->GetPositionY());
        if (InLightningChargeConeRanged(bearing, coneBearing) && ThorimShelterFor(held.orbIndex, slot, shelter))
            held.sheltered = true;
    }

    if (!held.sheltered)
        return false;

    // The slot can still be reassigned under us by a death, and the new one may have no row for this
    // orb. Home is the only safe answer then.
    if (!ThorimShelterFor(held.orbIndex, slot, shelter))
    {
        held.sheltered = false;
        return false;
    }

    out = shelter;
    orbIndex = held.orbIndex;
    return true;
}

// Whether a candidate bearing keeps clear of the tank anchor and every other slot. Bearing alone: the tank sits at
// r 5.5, so 60 degrees off the anchor is only 7.1 yd of ground and a chain can still bridge the pair.
bool RingBearingSeparated(float candidate, uint8 slot,
                          std::array<float, ULDUAR_THORIM_MELEE_SLOTS> const& taken, float anchor)
{
    if (AbsAngleDelta(candidate, anchor) < ULDUAR_THORIM_RING_SLOT_SEPARATION)
        return false;

    for (uint8 other = 0; other < ULDUAR_THORIM_MELEE_SLOTS; ++other)
        if (other != slot && AbsAngleDelta(candidate, taken[other]) < ULDUAR_THORIM_RING_SLOT_SEPARATION)
            return false;

    return true;
}

// One answer for all three slots, so no slot's dodge walks it onto another. Relaxed cheapest first: separation
// (one 40-90k hop), then Blizzard zones (a bot holds its bearing while one sits on it), never the cone (~20k).
void SolveRingBearings(PlayerbotAI* botAI, Player* bot, Unit* boss)
{
    ThorimEncounterState& state = ThorimStateFor(bot);

    // Latched once a pull, off the tank spot rather than the live tank.
    if (!state.ringLatched)
    {
        state.ringAnchor = RingAnchorBearing(boss);
        for (uint8 slot = 0; slot < ULDUAR_THORIM_MELEE_SLOTS; ++slot)
        {
            state.ringSlots[slot].latched = SlotBearing(state.ringAnchor, slot);
            state.ringSlots[slot].composed = state.ringSlots[slot].latched;
        }

        state.ringLatched = true;
    }

    Unit* orb = ThorimChargedThunderOrb(botAI, SPELL_THORIM_LIGHTNING_ORB_VISUAL);
    ObjectGuid const orbGuid = orb ? orb->GetGUID() : ObjectGuid::Empty;

    // On the shared scan interval like the sweeps, except that a new orb re-solves at once: the cone
    // is the one input the ring gets no warning of.
    if (orbGuid == state.ringOrb && state.ringSolveMs &&
        GetMSTimeDiffToNow(state.ringSolveMs) < ULDUAR_THORIM_ENCOUNTER_SCAN_INTERVAL_MS)
        return;

    state.ringOrb = orbGuid;
    state.ringSolveMs = getMSTime();

    float const coneBearing = orb ? BearingFromBoss(boss, orb->GetPositionX(), orb->GetPositionY()) : 0.0f;
    std::vector<Position> const& zones = ThorimBlizzardSpots(bot);

    // Slots not yet solved this pass count at their latched bearing, so slot 0 does not step onto the
    // point slot 2 is about to be handed back.
    std::array<float, ULDUAR_THORIM_MELEE_SLOTS> taken{};
    for (uint8 slot = 0; slot < ULDUAR_THORIM_MELEE_SLOTS; ++slot)
        taken[slot] = state.ringSlots[slot].latched;

    float const step = 0.0349f;  // 2 degrees
    for (uint8 slot = 0; slot < ULDUAR_THORIM_MELEE_SLOTS; ++slot)
    {
        float const base = state.ringSlots[slot].latched;

        // Whichever side this slot answered on last time is tried first, so one new zone cannot flip it
        // clean across the ring.
        int8 const firstWay =
            Position::NormalizeOrientation(state.ringSlots[slot].composed - base) > float(M_PI) ? -1 : 1;

        float answer = base;
        bool found = false;
        for (uint8 relax = 0; relax < 3 && !found; ++relax)
        {
            for (uint8 tick = 0; tick <= 90 && !found; ++tick)
            {
                for (int8 turn = 0; turn < 2; ++turn)
                {
                    float const candidate = Position::NormalizeOrientation(
                        base + static_cast<float>((turn ? -firstWay : firstWay) * tick) * step);

                    bool ok = !orb || !InLightningChargeCone(candidate, coneBearing);
                    if (ok && relax < 2)
                        ok = RingBearingClearOfBlizzard(boss, candidate, zones);
                    if (ok && relax < 1)
                        ok = RingBearingSeparated(candidate, slot, taken, state.ringAnchor);

                    if (ok)
                    {
                        answer = candidate;
                        found = true;
                        break;
                    }

                    // Both ways are the same point at tick 0.
                    if (!tick)
                        break;
                }
            }
        }

        taken[slot] = answer;
        state.ringSlots[slot].composed = answer;
    }
}

// The bearing this bot's slot ended on. Solved for the whole ring, so everyone in a slot walks to the
// same point and no two slots answer onto each other.
float RingSlotBearing(PlayerbotAI* botAI, Player* bot, Unit* boss, uint8 slot)
{
    SolveRingBearings(botAI, bot, boss);

    ThorimEncounterState const* state = FindState(bot);
    if (!state || slot >= ULDUAR_THORIM_MELEE_SLOTS)
        return SlotBearing(RingAnchorBearing(boss), slot);

    return state->ringSlots[slot].composed;
}

// A melee bot parked 25 yd from the boss at zero DPS is worse off than an unspread one, so the static
// trio is only taken when it is actually in range.
bool StaticMeleeSpot(Unit* boss, uint8 slot, Position& out)
{
    static Position const* const spots[ULDUAR_THORIM_MELEE_SLOTS] = {
        &ULDUAR_THORIM_PHASE2_MELEE1_SPOT, &ULDUAR_THORIM_PHASE2_MELEE2_SPOT, &ULDUAR_THORIM_PHASE2_MELEE3_SPOT};

    if (slot >= ULDUAR_THORIM_MELEE_SLOTS)
        return false;

    Position const& spot = *spots[slot];
    if (boss->GetDistance(spot) > ULDUAR_THORIM_MELEE_RING_RADIUS + ULDUAR_THORIM_RING_ARRIVE_TOLERANCE)
        return false;

    out = spot;
    return true;
}

void TickRunicSmash(PlayerbotAI* botAI, Player* bot)
{
    if (!NearThorimEncounter(bot))
        return;

    ThorimEncounterState& state = ThorimStateFor(bot);
    if (state.smashScanMs && GetMSTimeDiffToNow(state.smashScanMs) < ULDUAR_THORIM_ENCOUNTER_SCAN_INTERVAL_MS)
        return;

    state.smashScanMs = getMSTime();

    Unit* colossus = GetThorimRunicColossus(botAI);

    // EVENT_RC_RUNIC_SMASH is cancelled in JustEngagedWith, so once he is tanked the corridor is safe
    // and the squad has no reason to keep favouring a lane.
    if (!colossus || !colossus->IsAlive() || colossus->IsInCombat())
    {
        state.runicSmashSide = 0;
        state.runicSmashSeenMs = 0;
        return;
    }

    if (colossus->FindCurrentSpellBySpellId(SPELL_THORIM_RUNIC_SMASH_LEFT))
    {
        state.runicSmashSide = SPELL_THORIM_RUNIC_SMASH_LEFT;
        state.runicSmashSeenMs = state.smashScanMs;
    }
    else if (colossus->FindCurrentSpellBySpellId(SPELL_THORIM_RUNIC_SMASH_RIGHT))
    {
        state.runicSmashSide = SPELL_THORIM_RUNIC_SMASH_RIGHT;
        state.runicSmashSeenMs = state.smashScanMs;
    }
}

}  // namespace

bool NearThorimEncounter(Player const* bot)
{
    if (!bot || bot->GetDistance(ULDUAR_THORIM_NEAR_ARENA_CENTER) > ULDUAR_THORIM_ENCOUNTER_PROXIMITY)
        return false;

    // Below the wing ceiling is arena and corridor, above it only the balcony box. Without the box every
    // node keyed off this goes quiet once the squad climbs the ramp.
    return bot->GetPositionZ() < ULDUAR_THORIM_WING_MAX_Z || OnThorimBalcony(bot);
}

Unit* GetThorim(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot)
        return nullptr;

    // Cached by guid: the sight list stops at Playerbots.SightDistance, well short of the balcony.
    // ObjectAccessor has no range, so NearThorimEncounter is what bounds the callers.
    ThorimEncounterState& state = ThorimStateFor(bot);
    if (state.bossGuid)
    {
        Unit* cached = botAI->GetUnit(state.bossGuid);
        if (cached && cached->IsAlive())
            return cached;

        state.bossGuid.Clear();
    }

    // Only until it is struck: the raid pulls from inside the arena, so this runs for the first tick
    // and then never again.
    Unit* boss = GetUldUnitByEntry(botAI, NPC_THORIM);
    if (boss)
        state.bossGuid = boss->GetGUID();

    return boss;
}

Unit* GetThorimRunicColossus(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot)
        return nullptr;

    // Cached because the multiplier asks on every melee action of every bot, and a 150 yd grid sweep
    // is far too heavy for that. One creature, so any bot's answer serves the whole instance.
    ThorimEncounterState& state = ThorimStateFor(bot);
    if (state.colossusScanMs && GetMSTimeDiffToNow(state.colossusScanMs) < ULDUAR_THORIM_ENCOUNTER_SCAN_INTERVAL_MS)
    {
        Unit* cached = botAI->GetUnit(state.colossusGuid);
        return cached && cached->IsAlive() ? cached : nullptr;
    }

    state.colossusScanMs = getMSTime();
    state.colossusGuid.Clear();

    Unit* colossus = bot->FindNearestCreature(NPC_RUNIC_COLOSSUS, ULDUAR_THORIM_COLOSSUS_SEARCH_RANGE, true);
    if (colossus)
        state.colossusGuid = colossus->GetGUID();

    return colossus;
}

void GatherThorimEncounterTargets(PlayerbotAI* botAI, ThorimEncounterTargets& out)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot)
        return;

    // No-LOS, because the arena pile routinely puts an add behind another one and a bot that cannot
    // see the Evoker this tick still has to count it as the thing to kill.
    GuidVector const& units = botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();
    for (ObjectGuid const& guid : units)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || !unit->IsHostileTo(bot))
            continue;

        if (bot->GetDistance(unit) > ULDUAR_THORIM_DPS_TARGET_RANGE)
            continue;

        switch (unit->GetEntry())
        {
            case NPC_DARK_RUNE_ACOLYTE_I:
            case NPC_DARK_RUNE_ACOLYTE_G:
                out.acolytes.push_back(unit);
                break;
            case NPC_DARK_RUNE_EVOKER:
                out.evokers.push_back(unit);
                break;
            case NPC_DARK_RUNE_CHAMPION:
                out.champions.push_back(unit);
                break;
            case NPC_DARK_RUNE_WARBRINGER:
                out.warbringers.push_back(unit);
                break;
            case NPC_DARK_RUNE_COMMONER:
                out.commoners.push_back(unit);
                break;
            case NPC_IRON_RING_GUARD:
            case NPC_IRON_HONOR_GUARD:
                out.guards.push_back(unit);
                break;
            case NPC_CAPTURED_MERCENARY_SOLDIER_ALLY:
            case NPC_CAPTURED_MERCENARY_SOLDIER_HORDE:
            case NPC_CAPTURED_MERCENARY_CAPTAIN_ALLY:
            case NPC_CAPTURED_MERCENARY_CAPTAIN_HORDE:
            case NPC_JORMUNGAR_BEHEMOT:
                out.trash.push_back(unit);
                break;
            case NPC_RUNIC_COLOSSUS:
                out.colossus = unit;
                break;
            case NPC_ANCIENT_RUNE_GIANT:
                out.runeGiant = unit;
                break;
            default:
                break;
        }
    }
}

bool ThorimDpsTargetAllowed(PlayerbotAI* botAI, Unit* target)
{
    if (!target || !target->IsAlive())
        return false;

    Unit* boss = GetThorim(botAI);
    if (!boss || boss->GetPositionZ() < ULDUAR_THORIM_AXIS_Z_FLOOR_THRESHOLD)
        return true;

    // Phase 1: neither he nor Sif can be hit from the floor, and both sit above the arena box, which the
    // arena target guard shuts a bot down for.
    return target != boss && target->GetEntry() != NPC_SIF;
}

namespace
{

// Holds the current target while it's in the tier, trading only for one clearly closer to `pivot`.
Unit* SelectThorimTierTarget(Unit* currentTarget, std::vector<Unit*> const& candidates, Position const& pivot)
{
    Unit* selected = nullptr;
    for (Unit* candidate : candidates)
        if (candidate && candidate == currentTarget)
        {
            selected = candidate;
            break;
        }

    for (Unit* candidate : candidates)
    {
        if (!candidate || candidate == selected)
            continue;

        if (!selected)
        {
            selected = candidate;
            continue;
        }

        float const held = selected->GetExactDist2d(pivot.GetPositionX(), pivot.GetPositionY());
        float const offered = candidate->GetExactDist2d(pivot.GetPositionX(), pivot.GetPositionY());
        if (offered + ULDUAR_THORIM_TARGET_SWITCH_MARGIN < held)
            selected = candidate;
    }

    return selected;
}

Unit* NoteThorimDpsTarget(Player* bot, Unit* target)
{
    if (bot)
        if (ThorimEncounterState* state = FindState(bot))
            state->dpsTargets[bot->GetGUID()] = target ? target->GetGUID() : ObjectGuid::Empty;

    return target;
}

}  // namespace

Unit* GetThorimDpsTarget(PlayerbotAI* botAI, Player* bot, Unit* currentTarget)
{
    if (!botAI || !bot || !NearThorimEncounter(bot))
        return nullptr;

    Unit* boss = GetThorim(botAI);
    if (!boss || !boss->IsAlive() || !boss->IsHostileTo(bot))
        return nullptr;

    // Phase 2 is the boss, except camp ranged clear surviving Evokers without leaving their spot.
    // Champions and Warbringers cost more boss time than the damage they deal is worth.
    if (boss->GetPositionZ() < ULDUAR_THORIM_AXIS_Z_FLOOR_THRESHOLD)
    {
        if (GetThorimPhase2Role(botAI, bot) == ThorimPhase2Role::Ranged)
        {
            ThorimEncounterTargets leftovers;
            GatherThorimEncounterTargets(botAI, leftovers);

            std::vector<Unit*> inside;
            for (Unit* candidate : leftovers.evokers)
                if (ThorimInArenaBox(candidate))
                    inside.push_back(candidate);

            if (Unit* pick = SelectThorimTierTarget(currentTarget, inside, ULDUAR_THORIM_NEAR_ARENA_CENTER))
                return NoteThorimDpsTarget(bot, pick);
        }

        return NoteThorimDpsTarget(bot, boss);
    }

    ThorimEncounterTargets targets;
    GatherThorimEncounterTargets(botAI, targets);

    if (GetThorimSquad(botAI, bot) == ThorimSquad::Gauntlet)
    {
        // Acolytes heal the pack, then the guards, then the two the corridor is gated on. Walk-in trash
        // is only alive before the corridor opens, and the range filter drops anything past 50 yd.
        std::vector<std::vector<Unit*> const*> const tiers = {&targets.acolytes, &targets.guards,
                                                              &targets.trash};
        for (auto const* tier : tiers)
            if (Unit* pick = SelectThorimTierTarget(currentTarget, *tier, *bot))
                return NoteThorimDpsTarget(bot, pick);

        if (targets.colossus)
            return NoteThorimDpsTarget(bot, targets.colossus);

        if (targets.runeGiant)
            return NoteThorimDpsTarget(bot, targets.runeGiant);

        // Phase 2 only starts on damage from a player above z 430. Gated on the bot's own height, or a
        // corridor bot at z 412 gets pulled 150 yd out of its lane.
        if (bot->GetPositionZ() > ULDUAR_THORIM_AXIS_Z_FLOOR_THRESHOLD)
            return NoteThorimDpsTarget(bot, boss);

        return NoteThorimDpsTarget(bot, nullptr);
    }

    // Arena: Acolytes first for everyone, they heal the wave. Then casters take Evokers from where they
    // stand and melee the Champions already on them. Commoners last, they barely hit.
    bool const melee = PlayerbotAI::IsMelee(bot);
    using TierOrder = std::array<std::vector<Unit*> const*, 6>;
    TierOrder const meleeTiers = {&targets.acolytes, &targets.champions, &targets.warbringers,
                                  &targets.evokers, &targets.commoners, &targets.trash};
    TierOrder const rangedTiers = {&targets.acolytes, &targets.evokers, &targets.champions,
                                   &targets.warbringers, &targets.commoners, &targets.trash};
    TierOrder const& tiers = melee ? meleeTiers : rangedTiers;

    // Melee pivot on themselves to get their nearest. Ranged share the centre, which makes them all focus
    // the same unit.
    Position const& pivot = melee ? static_cast<Position const&>(*bot) : ULDUAR_THORIM_NEAR_ARENA_CENTER;

    // Melee: first pass only within reach, so a Champion on the bot beats an Evoker across the room, then
    // one without, so a bot with nothing near still commits instead of idling.
    for (int pass = 0; pass < (melee ? 2 : 1); ++pass)
    {
        bool const limitReach = melee && pass == 0;

        for (auto const* tier : tiers)
        {
            std::vector<Unit*> inside;
            for (Unit* candidate : *tier)
            {
                if (!ThorimInArenaBox(candidate))
                    continue;

                if (limitReach && bot->GetExactDist2d(candidate) > ULDUAR_THORIM_MELEE_TARGET_REACH)
                    continue;

                inside.push_back(candidate);
            }

            if (Unit* pick = SelectThorimTierTarget(currentTarget, inside, pivot))
                return NoteThorimDpsTarget(bot, pick);
        }
    }

    return NoteThorimDpsTarget(bot, nullptr);
}

bool ThorimHasDpsTarget(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot || !NearThorimEncounter(bot))
        return false;

    // The cached entry outlives its pull, so without both gates a stale guid would shut the generic
    // picker down on a bot the encounter no longer steers.
    Unit* boss = GetThorim(botAI);
    if (!boss || !boss->IsAlive())
        return false;

    ThorimEncounterState const* state = FindState(bot);
    if (!state)
        return false;

    auto const itr = state->dpsTargets.find(bot->GetGUID());
    return itr != state->dpsTargets.end() && !itr->second.IsEmpty();
}

void ThorimClearStaleMarks(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot)
        return;

    // Per bot and every call: IsHighPriority short-circuits every find-target strategy for anything in
    // here, so one stale entry outranks the whole priority list.
    botAI->GetAiObjectContext()->GetValue<GuidVector>("prioritized targets")->Set({});

    Group* group = bot->GetGroup();
    if (!group)
        return;

    ThorimEncounterState& state = ThorimStateFor(bot);
    if (state.marksCleared)
        return;

    state.marksCleared = true;

    // RtiTargetValue hands an icon back before the smart picker runs, so a leftover skull, cross or moon
    // silently outranks everything chosen here.
    for (int8 const icon : {RtiTargetValue::skullIndex, RtiTargetValue::crossIndex, RtiTargetValue::moonIndex})
        if (group->GetTargetIcon(icon))
            group->SetTargetIcon(icon, bot->GetGUID(), ObjectGuid::Empty);
}

Position const& GetThorimGauntletWaypoint(bool leftLane, uint8 index)
{
    return *LaneWaypoints(leftLane)[std::min<uint8>(index, ULDUAR_THORIM_GAUNTLET_WAYPOINTS - 1)].position;
}

bool ThorimGauntletLaneIndexInLane(WorldObject const* who, bool leftLane, uint8& index)
{
    if (!who)
        return false;

    auto const& lane = LaneWaypoints(leftLane);

    bool standing = false;
    uint8 nearest = 0;
    float nearestDistance = std::numeric_limits<float>::max();

    for (uint8 i = 0; i < ULDUAR_THORIM_GAUNTLET_WAYPOINTS; ++i)
    {
        float const distance = who->GetDistance(*lane[i].position);
        if (distance < lane[i].radius)
            standing = true;

        if (distance < nearestDistance)
        {
            nearestDistance = distance;
            nearest = i;
        }
    }

    if (!standing)
        return false;

    index = nearest;
    return true;
}

bool ThorimGauntletLaneIndex(WorldObject const* who, uint8& index, bool& leftLane)
{
    if (ThorimGauntletLaneIndexInLane(who, true, index))
    {
        leftLane = true;
        return true;
    }

    if (ThorimGauntletLaneIndexInLane(who, false, index))
    {
        leftLane = false;
        return true;
    }

    return false;
}

bool ThorimResolveGauntletIndex(PlayerbotAI* botAI, Player* bot, uint8& index)
{
    if (!botAI || !bot)
        return false;

    bool lane = false;
    if (Unit* master = botAI->GetMaster())
        if (ThorimGauntletLaneIndex(master, index, lane))
            return true;

    return ThorimGauntletLaneIndex(bot, index, lane);
}

bool ThorimPreferredGauntletLane(PlayerbotAI* botAI, bool& useLeftLane)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot || bot->GetMapId() != ULDUAR_MAP_ID)
        return false;

    TickRunicSmash(botAI, bot);

    ThorimEncounterState const* state = FindState(bot);
    if (!state || !state->runicSmashSide)
        return false;

    // The left hand lights the bunnies that sit in the left lane, so the safe lane is always the one
    // opposite the hand that went up.
    useLeftLane = state->runicSmashSide == SPELL_THORIM_RUNIC_SMASH_RIGHT;
    return true;
}

bool ThorimRunicSmashImminent(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot || bot->GetMapId() != ULDUAR_MAP_ID)
        return false;

    TickRunicSmash(botAI, bot);

    ThorimEncounterState const* state = FindState(bot);
    if (!state || !state->runicSmashSeenMs)
        return false;

    return GetMSTimeDiffToNow(state->runicSmashSeenMs) < ULDUAR_THORIM_RUNIC_SMASH_LATCH_MS;
}

Position const& GetThorimBalconyWaypoint(uint8 index)
{
    return *balconyWaypoints[std::min<uint8>(index, ULDUAR_THORIM_BALCONY_WAYPOINTS - 1)];
}

Unit* GetThorimAncientRuneGiant(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot)
        return nullptr;

    // Cached the same way the Colossus is, and for the same reason: the balcony trigger asks once per
    // bot per tick and a 150 yd grid sweep at that rate is not worth one boolean.
    ThorimEncounterState& state = ThorimStateFor(bot);
    if (state.runeGiantScanMs && GetMSTimeDiffToNow(state.runeGiantScanMs) < ULDUAR_THORIM_ENCOUNTER_SCAN_INTERVAL_MS)
    {
        Unit* cached = botAI->GetUnit(state.runeGiantGuid);
        return cached && cached->IsAlive() ? cached : nullptr;
    }

    state.runeGiantScanMs = getMSTime();
    state.runeGiantGuid.Clear();

    Unit* giant = bot->FindNearestCreature(NPC_ANCIENT_RUNE_GIANT, ULDUAR_THORIM_COLOSSUS_SEARCH_RANGE, true);
    if (giant)
        state.runeGiantGuid = giant->GetGUID();

    return giant;
}

bool ThorimBalconyOpen(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot || !NearThorimEncounter(bot))
        return false;

    // Above the floor line, which up here means the hallway and Thorim's platform rather than the
    // corridor. The corridor squad has its own node and must not be dragged into this one.
    if (bot->GetPositionZ() <= ULDUAR_THORIM_AXIS_Z_FLOOR_THRESHOLD)
        return false;

    Unit* boss = GetThorim(botAI);
    if (!boss || !boss->IsAlive())
        return false;

    // The Giant's death opens the second doors and sets _isHitAllowed, so it is the one event that
    // turns the hallway into the squad's next job. While it lives, killing it is.
    return GetThorimAncientRuneGiant(botAI) == nullptr;
}

uint8 ThorimAdvanceBalconyStep(Player* bot)
{
    ThorimEncounterState* state = FindState(bot);
    if (!state)
        return 0;

    auto const& steps = state->balconyStep;
    auto const itr = steps.find(bot->GetGUID());
    uint8 step = itr == steps.end() ? 0 : itr->second;

    // Passing a waypoint's y counts as reaching it: the hallway runs one way, so a bot shoved north of a
    // point is done with it.
    auto const reached = [bot](uint8 index)
    {
        Position const& here = GetThorimBalconyWaypoint(index);
        return bot->GetExactDist2d(&here) <= ULDUAR_THORIM_BALCONY_ARRIVE_TOLERANCE ||
               bot->GetPositionY() > here.GetPositionY();
    };

    // Down first: a step held over from an earlier pull hands a bot on the ramp a mid-chain waypoint,
    // and the straight line there crosses a Paralytic Field bunny.
    while (step > 0 && !reached(step - 1))
        --step;

    // ULDUAR_THORIM_BALCONY_WAYPOINTS means arrived, and that has to be a latch: on the platform the squad
    // drifts while fighting, and re-anchoring it every tick would fight "reach melee".
    while (step < ULDUAR_THORIM_BALCONY_WAYPOINTS && reached(step))
        ++step;

    state->balconyStep[bot->GetGUID()] = step;
    return step;
}

namespace
{
// Only a non-tank melee bot near the fight, with the Colossus shielded, has a bail to track.
Unit* ThorimBarrierBailColossus(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot || bot->GetMapId() != ULDUAR_MAP_ID)
        return nullptr;

    if (!PlayerbotAI::IsMelee(bot) || PlayerbotAI::IsTank(bot) || !NearThorimEncounter(bot))
        return nullptr;

    return GetThorimRunicColossus(botAI);
}

bool ThorimColossusShielded(Unit* colossus)
{
    return colossus && colossus->IsAlive() && colossus->HasAura(SPELL_THORIM_RUNIC_BARRIER);
}
}  // namespace

void ThorimTickBarrierBail(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot || bot->GetMapId() != ULDUAR_MAP_ID)
        return;

    if (!PlayerbotAI::IsMelee(bot) || PlayerbotAI::IsTank(bot) || !NearThorimEncounter(bot))
        return;

    ThorimEncounterState& state = ThorimStateFor(bot);
    if (!ThorimColossusShielded(GetThorimRunicColossus(botAI)))
    {
        state.barrierBailing.erase(bot->GetGUID());
        return;
    }

    if (state.barrierBailing.count(bot->GetGUID()))
    {
        if (bot->GetHealthPct() >= ULDUAR_THORIM_BARRIER_RESUME_HEALTH_PCT)
            state.barrierBailing.erase(bot->GetGUID());

        return;
    }

    if (bot->GetHealthPct() <= ULDUAR_THORIM_BARRIER_BAIL_HEALTH_PCT)
        state.barrierBailing.insert(bot->GetGUID());
}

namespace
{

// Bots only: a human paladin would take the pick and never act on it, which is exactly how the shared node
// fails. Lowest guid wins so the pick holds still, and the aura has to be one this bot knows.
Player* ElectThorimFrostAuraPaladin(Group* group)
{
    Player* best = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
        if (!memberAI || !member->IsAlive() || member->GetMapId() != ULDUAR_MAP_ID ||
            member->getClass() != CLASS_PALADIN ||
            !memberAI->GetAiObjectContext()->GetValue<uint32>("spell id", "frost resistance aura")->Get())
            continue;

        if (!best || member->GetGUID() < best->GetGUID())
            best = member;
    }

    return best;
}

}  // namespace

void ThorimTickFrostAuraPaladin(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot || !NearThorimEncounter(bot))
        return;

    Group* group = bot->GetGroup();
    if (!group)
        return;

    ThorimEncounterState& state = ThorimStateFor(bot);
    uint32 const now = getMSTime();
    if (state.frostAuraScanMs &&
        getMSTimeDiff(state.frostAuraScanMs, now) < ULDUAR_THORIM_ENCOUNTER_SCAN_INTERVAL_MS)
        return;

    state.frostAuraScanMs = now;

    // Nobody until Sif fights: in normal mode she stays on her balcony casting nothing, so the aura would cost
    // the raid Devotion for no return.
    if (!state.sifJoined && !ThorimBlizzardSpots(bot).empty())
        state.sifJoined = true;

    Player* paladin = state.sifJoined ? ElectThorimFrostAuraPaladin(group) : nullptr;
    state.frostAuraPaladin = paladin ? paladin->GetGUID() : ObjectGuid::Empty;
}

bool IsThorimFrostAuraPaladin(Player const* bot)
{
    ThorimEncounterState const* state = FindState(bot);
    return bot && state && state->frostAuraPaladin == bot->GetGUID();
}

void ThorimTickHumanTankCalls(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot || bot->GetMapId() != ULDUAR_MAP_ID || !ThorimPhase2Active(botAI))
        return;

    Unit* boss = GetThorim(botAI);
    if (!boss)
        return;

    constexpr uint32 scanIntervalMs = 250;
    ThorimEncounterState& state = ThorimStateFor(bot);
    uint32 const now = getMSTime();
    if (getMSTimeDiff(state.humanCallScanMs, now) < scanIntervalMs)
        return;

    state.humanCallScanMs = now;

    // Tracked for bots too, so a human taking him back after a bot's turn starts a new turn.
    Unit* victim = boss->GetVictim();
    Player* holder = victim ? victim->ToPlayer() : nullptr;
    ObjectGuid const holderGuid = holder ? holder->GetGUID() : ObjectGuid::Empty;
    if (holderGuid != state.humanCallHolder)
    {
        state.humanCallHolder = holderGuid;
        state.humanCallTurnDone = false;
    }

    // Unbalancing Strike means a bot takes him in a moment, so a button now is wasted.
    if (!holder || IsBotPlayer(holder) || holder->HasAura(SPELL_UNBALANCING_STRIKE))
        return;

    uint32 const spellId = ReadyTankDefensiveSpell(holder, true, 0, 0);
    if (!spellId)
        return;

    if (holder->GetHealthPct() < ULDUAR_THORIM_HUMAN_CALL_LOW_PCT)
    {
        CallHumanTankDefensive(bot, holder, spellId, 0, "you are low");
        return;
    }

    uint32 const stacks = boss->GetAuraCount(SPELL_THORIM_LIGHTNING_CHARGE_BUFF);
    if (!state.humanCallTurnDone && stacks >= ULDUAR_THORIM_HUMAN_CALL_STACKS)
        state.humanCallTurnDone = CallHumanTankDefensive(
            bot, holder, spellId, 0, "Thorim hits hard (" + std::to_string(stacks) + " stacks)");
}

bool ThorimBarrierBailLatched(PlayerbotAI* botAI, Player* bot)
{
    if (!ThorimColossusShielded(ThorimBarrierBailColossus(botAI, bot)))
        return false;

    ThorimEncounterState const* state = FindState(bot);
    return state && state->barrierBailing.count(bot->GetGUID());
}

bool ThorimSplitActive(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!NearThorimEncounter(bot))
        return false;

    Unit* boss = GetThorim(botAI);
    if (!boss || !boss->IsHostileTo(bot))
        return false;

    // Combat, so the leash stays off the raid while it walks in and clears the six trash; the balcony
    // height, so it lets go the instant he drops for phase 2.
    return boss->IsInCombat() && boss->GetPositionZ() >= ULDUAR_THORIM_AXIS_Z_FLOOR_THRESHOLD;
}

ThorimSquad GetThorimSquad(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot || bot->GetMapId() != ULDUAR_MAP_ID)
        return ThorimSquad::None;

    if (!NearThorimEncounter(bot))
        return ThorimSquad::None;

    // Deliberately not gated on combat: the corridor squad forms up at the gate before the pull, and
    // the side it is on has to be decided by then.
    if (!GetThorim(botAI))
        return ThorimSquad::None;

    AssignThorimSquads(bot);
    TickHumanSquads(bot);

    ThorimEncounterState const* state = FindState(bot);
    if (!state)
        return ThorimSquad::None;

    auto const itr = state->squads.find(bot->GetGUID());
    return itr == state->squads.end() ? ThorimSquad::Arena : static_cast<ThorimSquad>(itr->second);
}

bool ThorimInArenaBox(WorldObject const* who)
{
    if (!who)
        return false;

    float const x = who->GetPositionX();
    float const y = who->GetPositionY();

    return x > ULDUAR_THORIM_ARENA_BOX_MIN_X && x < ULDUAR_THORIM_ARENA_BOX_MAX_X &&
           y > ULDUAR_THORIM_ARENA_BOX_MIN_Y && y < ULDUAR_THORIM_ARENA_BOX_MAX_Y &&
           who->GetPositionZ() < ULDUAR_THORIM_ARENA_BOX_MAX_Z;
}

namespace
{

// How long before a pet that is still outside may be told again. MoveFollow restarts the walk, so a
// command every tick leaves the pet running on the spot and never arriving.
constexpr uint32 ULDUAR_THORIM_PET_RECALL_INTERVAL_MS = 2000;

}  // namespace

bool ThorimStrayPets(PlayerbotAI* botAI, Player* bot, std::vector<Unit*>& out)
{
    out.clear();

    if (!botAI || !bot || !ThorimSplitActive(botAI))
        return false;

    ThorimSquad const squad = GetThorimSquad(botAI, bot);
    if (squad == ThorimSquad::None)
        return false;

    ThorimEncounterState const* state = FindState(bot);
    if (!state)
        return false;

    Position const& centre = ULDUAR_THORIM_NEAR_ARENA_CENTER;

    for (Unit* pet : bot->m_Controlled)
    {
        // m_Controlled, not GetPet(): Raise Dead, Feral Spirits and Army are guardians. Totems never move
        // or pull anything.
        if (!pet || !pet->IsAlive() || pet->IsTotem())
            continue;

        if (!pet->IsPet() && !pet->IsGuardian())
            continue;

        if (!pet->IsInWorld() || pet->GetMapId() != bot->GetMapId())
            continue;

        // Arena pets get their owner's leash, which still covers an add 24 yd out. Gauntlet pets get the
        // owner instead, or a pet sent at something unreachable walks the whole corridor.
        bool home;
        if (squad == ThorimSquad::Arena)
            home = ThorimInArenaBox(pet) && pet->GetExactDist2d(centre.GetPositionX(), centre.GetPositionY()) <=
                                                ULDUAR_THORIM_ARENA_LEASH_RADIUS;
        else
            home = pet->GetExactDist2d(bot->GetPositionX(), bot->GetPositionY()) <=
                   ULDUAR_THORIM_PET_OWNER_LEASH_RADIUS;

        if (home)
            continue;

        // Time-based rather than a "already told it" latch, which would never fire twice for a pet
        // that strays, comes home and strays again.
        auto const sent = state->petRecallMs.find(pet->GetGUID());
        if (sent != state->petRecallMs.end() &&
            GetMSTimeDiffToNow(sent->second) < ULDUAR_THORIM_PET_RECALL_INTERVAL_MS)
            continue;

        out.push_back(pet);
    }

    return !out.empty();
}

void ThorimRecallPet(Player* bot, Unit* pet)
{
    if (!bot || !pet)
        return;

    ThorimEncounterState* state = FindState(bot);
    if (!state)
        return;

    state->petRecallMs[pet->GetGUID()] = getMSTime();
    state->petRecalls[bot->GetGUID()] = pet->GetGUID();

    pet->AttackStop();
    pet->CastStop();
    pet->GetMotionMaster()->MoveFollow(bot, PET_FOLLOW_DIST, pet->GetFollowAngle());

    // Without CharmInfo the stop and walk above are the whole recall. PetAI::CanAttack lets a
    // COMMAND_FOLLOW pet fight again once IsReturning clears, so this holds it only for the walk home.
    CharmInfo* charmInfo = pet->GetCharmInfo();
    if (!charmInfo)
        return;

    charmInfo->SetCommandState(COMMAND_FOLLOW);
    charmInfo->SetIsCommandAttack(false);
    charmInfo->SetIsAtStay(false);
    charmInfo->SetIsReturning(true);
    charmInfo->SetIsCommandFollow(true);
    charmInfo->SetIsFollowing(false);
    charmInfo->RemoveStayPosition();
}

bool ThorimArenaLeashBreached(PlayerbotAI* botAI, Player* bot)
{
    if (!bot || !ThorimSplitActive(botAI))
        return false;

    if (GetThorimSquad(botAI, bot) != ThorimSquad::Arena)
        return false;

    // The radius is the working leash; the box is the backstop, because it is the box the boss script
    // actually scans.
    float const distance = bot->GetExactDist2d(ULDUAR_THORIM_NEAR_ARENA_CENTER.GetPositionX(),
                                               ULDUAR_THORIM_NEAR_ARENA_CENTER.GetPositionY());

    // Melee are the only ones with a reason to leave the pile, so they get the tighter radius and
    // everyone else keeps the box backstop, since their anchor already holds them at 10-14 yd.
    bool const melee = !PlayerbotAI::IsTank(bot) && !PlayerbotAI::IsRanged(bot);
    float const leash = melee ? ULDUAR_THORIM_ARENA_MELEE_LEASH : ULDUAR_THORIM_ARENA_LEASH_RADIUS;

    return distance > leash || !ThorimInArenaBox(bot);
}

namespace
{

// From the latched squad, dead included, so a death never renumbers the formation. Shared by the ring
// and the Charge Orb dodge so the two agree on slots.
bool ThorimArenaRingOrder(PlayerbotAI* botAI, Player* bot, size_t& slot, size_t& total)
{
    Group* group = bot->GetGroup();
    ThorimEncounterState const* state = FindState(bot);
    if (!group || !state)
        return false;

    std::vector<Player*> ringMembers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != bot->GetMapId())
            continue;

        auto const squad = state->squads.find(member->GetGUID());
        if (squad == state->squads.end() || static_cast<ThorimSquad>(squad->second) != ThorimSquad::Arena)
            continue;

        PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
        if (!memberAI || !PlayerbotAI::IsRanged(member) || PlayerbotAI::IsTank(member))
            continue;

        ringMembers.push_back(member);
    }

    if (ringMembers.empty())
        return false;

    // Ranged dps ahead of healers, ties on guid. Both keys read the same on every bot, so nobody has
    // to be told which slot is theirs.
    std::sort(ringMembers.begin(), ringMembers.end(), [](Player* left, Player* right)
    {
        bool const leftDps = PlayerbotAI::IsRangedDps(left);
        bool const rightDps = PlayerbotAI::IsRangedDps(right);
        if (leftDps != rightDps)
            return leftDps;
        return left->GetGUID() < right->GetGUID();
    });

    for (size_t i = 0; i < ringMembers.size(); ++i)
        if (ringMembers[i] == bot)
        {
            slot = i;
            total = ringMembers.size();
            return true;
        }

    return false;
}

// Settles a spot onto reachable ground. False means use the centre: the collision walk drags it back
// toward a bot outside the pit, and a spot outside the leash would freeze that bot for good.
bool ThorimSettleArenaPoint(Player* bot, Position& spot)
{
    Position const& centre = ULDUAR_THORIM_NEAR_ARENA_CENTER;

    float x = spot.GetPositionX();
    float y = spot.GetPositionY();
    float z = bot->GetMapWaterOrGroundLevel(x, y, centre.GetPositionZ());
    if (z <= INVALID_HEIGHT)
        z = centre.GetPositionZ();

    bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                   bot->GetPositionZ(), x, y, z, false);

    spot = Position(x, y, z);
    return spot.GetExactDist2d(centre.GetPositionX(), centre.GetPositionY()) <=
           ULDUAR_THORIM_ARENA_LEASH_RADIUS;
}

bool GetThorimArenaRingSlot(PlayerbotAI* botAI, Player* bot, Position& out)
{
    size_t slot = 0;
    size_t total = 0;
    if (!ThorimArenaRingOrder(botAI, bot, slot, total))
        return false;

    // Bearing from the gate to the centre, so slot 0 lands on the far side of the room from the
    // corridor rather than in its mouth. Two fixed points, so the layout never rotates.
    Position const& centre = ULDUAR_THORIM_NEAR_ARENA_CENTER;
    Position const& gate = ULDUAR_THORIM_NEAR_ENTRANCE_POSITION;
    float const baseAngle = std::atan2(centre.GetPositionY() - gate.GetPositionY(),
                                       centre.GetPositionX() - gate.GetPositionX());

    size_t const inner = std::min<size_t>(ULDUAR_THORIM_ARENA_RING_INNER_SLOTS, total);

    // Nobody stands on the centre itself: that is the tank's spot and the pile of adds on him.
    float radius = ULDUAR_THORIM_ARENA_RING_INNER;
    float angle = baseAngle + 2.0f * static_cast<float>(M_PI) * static_cast<float>(slot) / static_cast<float>(inner);

    if (slot >= inner)
    {
        size_t const outerCount = total - inner;
        radius = ULDUAR_THORIM_ARENA_RING_OUTER;
        angle = baseAngle +
                2.0f * static_cast<float>(M_PI) * static_cast<float>(slot - inner) / static_cast<float>(outerCount);
    }

    angle = Position::NormalizeOrientation(angle);

    Position spot(centre.GetPositionX() + std::cos(angle) * radius,
                  centre.GetPositionY() + std::sin(angle) * radius, centre.GetPositionZ());

    out = ThorimSettleArenaPoint(bot, spot) ? spot : centre;
    return true;
}

// Nearest spot clear of the charged orb, or false if `from` is already clear. Straight out from the
// orb: the shortest way out, and it keeps each bot's bearing so the ring stays spread.
bool ThorimPushOutOfOrbField(PlayerbotAI* botAI, Player* bot, Position const& from, Position& out)
{
    Unit* orb = ThorimChargedThunderOrb(botAI, SPELL_THORIM_CHARGE_ORB);
    if (!orb)
        return false;

    // Measured in 2D on purpose. Lightning Shock's 35 yd is a 3D radius from an orb hanging 13.5 yd
    // overhead, so what has to be cleared on the floor is the circle underneath it, not the sphere.
    float const danger = ULDUAR_THORIM_CHARGED_ORB_RADIUS + ULDUAR_THORIM_CHARGED_ORB_MARGIN;
    float const gap = from.GetExactDist2d(orb->GetPositionX(), orb->GetPositionY());
    if (gap > danger)
        return false;

    Position const& centre = ULDUAR_THORIM_NEAR_ARENA_CENTER;
    float bearing = std::atan2(centre.GetPositionY() - orb->GetPositionY(),
                               centre.GetPositionX() - orb->GetPositionX());
    if (gap > 1.0f)
        bearing = std::atan2(from.GetPositionY() - orb->GetPositionY(),
                             from.GetPositionX() - orb->GetPositionX());

    Position spot(orb->GetPositionX() + std::cos(bearing) * danger,
                  orb->GetPositionY() + std::sin(bearing) * danger, centre.GetPositionZ());

    // Every orb is 42 yd out, so the centre clears the field by 5.7 yd: a safe fallback for a spot the
    // leash rejects or one the collision walk dragged back into the field.
    bool safe = ThorimSettleArenaPoint(bot, spot);
    if (safe && spot.GetExactDist2d(orb->GetPositionX(), orb->GetPositionY()) < ULDUAR_THORIM_CHARGED_ORB_RADIUS)
        safe = false;

    out = safe ? spot : centre;
    return true;
}

}  // namespace

bool GetThorimArenaAnchor(PlayerbotAI* botAI, Player* bot, Position& out)
{
    if (!botAI || !bot)
        return false;

    if (GetThorimSquad(botAI, bot) != ThorimSquad::Arena)
        return false;

    Unit* boss = GetThorim(botAI);
    if (!boss || !boss->IsAlive() || !boss->IsHostileTo(bot))
        return false;

    // Phase 1 only. Once he drops to the floor the phase 2 ring owns the room.
    if (boss->GetPositionZ() < ULDUAR_THORIM_AXIS_Z_FLOOR_THRESHOLD)
        return false;

    if (PlayerbotAI::IsTank(bot))
    {
        out = ULDUAR_THORIM_NEAR_ARENA_CENTER;
    }
    else if (PlayerbotAI::IsRanged(bot))
    {
        if (!GetThorimArenaRingSlot(botAI, bot, out))
            return false;
    }
    // Melee form up on the tank spot before the pull, then run free in combat on the 24 yd leash.
    else if (botAI->GetState() == BOT_STATE_COMBAT)
    {
        return false;
    }
    else
    {
        out = ULDUAR_THORIM_NEAR_ARENA_CENTER;
    }

    // The anchor is cleared of the orb field, not just the bot: the field burns for 15s, and an anchor
    // left inside it walks the bot straight back in the moment the dodge lets go.
    Position moved;
    if (ThorimPushOutOfOrbField(botAI, bot, out, moved))
        out = moved;

    return true;
}

bool ThorimArenaAnchorNeedsMove(PlayerbotAI* /*botAI*/, Player* bot, Position const& spot)
{
    if (!bot)
        return false;

    ThorimEncounterState& state = ThorimStateFor(bot);
    float const distance = bot->GetExactDist2d(spot.GetPositionX(), spot.GetPositionY());

    // Arrival lets the anchor guard switch the movers off, so a spot outside the leash never counts:
    // the bot would be frozen and out of bounds at once.
    bool const spotIsLeashed =
        spot.GetExactDist2d(ULDUAR_THORIM_NEAR_ARENA_CENTER.GetPositionX(),
                            ULDUAR_THORIM_NEAR_ARENA_CENTER.GetPositionY()) <=
        ULDUAR_THORIM_ARENA_LEASH_RADIUS;

    if (state.arenaAnchorArrived.count(bot->GetGUID()))
    {
        if (spotIsLeashed && distance <= ULDUAR_THORIM_RING_REPOSITION_TOLERANCE)
            return false;

        state.arenaAnchorArrived.erase(bot->GetGUID());
        return true;
    }

    if (!spotIsLeashed || distance > ULDUAR_THORIM_RING_ARRIVE_TOLERANCE)
        return true;

    state.arenaAnchorArrived.insert(bot->GetGUID());
    return false;
}

bool ThorimArenaAnchorSettled(PlayerbotAI* botAI, Player* bot)
{
    if (!bot)
        return false;

    Position anchor;
    if (!GetThorimArenaAnchor(botAI, bot, anchor))
        return false;

    ThorimEncounterState const* state = FindState(bot);
    return state && state->arenaAnchorArrived.count(bot->GetGUID()) > 0;
}

bool ThorimChargedOrbEscape(PlayerbotAI* botAI, Player* bot, Position& out)
{
    if (!botAI || !bot || GetThorimSquad(botAI, bot) != ThorimSquad::Arena)
        return false;

    // Phase 1 only. Charge Orb stops firing once Thorim drops to the floor, and the orb that lights up
    // after that is Lightning Charge, which has its own node and its own answer.
    Unit* boss = GetThorim(botAI);
    if (!boss || !boss->IsAlive() || boss->GetPositionZ() < ULDUAR_THORIM_AXIS_Z_FLOOR_THRESHOLD)
        return false;

    Position const here(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
    return ThorimPushOutOfOrbField(botAI, bot, here, out);
}

void ThorimNoteOrbEscape(Player* bot, Position const& spot)
{
    if (!bot)
        return;

    // Deadbanded, because the escape point is derived from the bot's own drifting coordinates.
    ThorimEncounterState& state = ThorimStateFor(bot);
    auto const noted = state.orbEscapes.find(bot->GetGUID());
    if (noted != state.orbEscapes.end() &&
        noted->second.GetExactDist2d(spot.GetPositionX(), spot.GetPositionY()) <= 1.0f)
        return;

    state.orbEscapes[bot->GetGUID()] = spot;
}

void ThorimNoteFollowMasterStripped(Player* bot)
{
    if (!bot)
        return;

    ThorimStateFor(bot).followMasterStripped.insert(bot->GetGUID());
}

bool ThorimFollowMasterStripped(Player const* bot)
{
    ThorimEncounterState const* state = FindState(bot);
    return state && state->followMasterStripped.count(bot->GetGUID()) > 0;
}

bool ThorimPhase2Active(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!NearThorimEncounter(bot))
        return false;

    Unit* boss = GetThorim(botAI);
    if (!boss || !boss->IsInWorld() || boss->IsDuringRemoveFromWorld() || !boss->IsHostileTo(bot))
        return false;

    // He fights the whole gauntlet from his balcony and only drops to the arena floor for phase 2.
    return boss->GetPositionZ() < ULDUAR_THORIM_AXIS_Z_FLOOR_THRESHOLD;
}

bool ThorimPhase2ElapsedMs(PlayerbotAI* botAI, uint32& elapsedMs)
{
    if (!ThorimPhase2Active(botAI))
        return false;

    Unit* boss = GetThorim(botAI);
    if (!boss || !boss->IsInCombat())
        return false;

    // Write once, so a second ask this tick reads the same clock. Never 0, since 0 means not started.
    ThorimEncounterState& state = ThorimStateFor(botAI->GetBot());
    if (!state.phase2StartMs)
        state.phase2StartMs = std::max<uint32>(getMSTime(), 1);

    elapsedMs = GetMSTimeDiffToNow(state.phase2StartMs);
    return true;
}

bool ThorimPhase2OpeningHold(PlayerbotAI* botAI, Player* bot, Position const& holdSpot)
{
    uint32 elapsed = 0;
    if (!bot || !ThorimPhase2ElapsedMs(botAI, elapsed))
        return false;

    ThorimEncounterState& state = ThorimStateFor(bot);
    if (state.openingReleased.count(bot->GetGUID()))
        return false;

    if (elapsed < ULDUAR_THORIM_OPENING_HOLD_MIN_MS)
        return true;

    Unit* boss = GetThorim(botAI);
    bool const atAnchor =
        boss && boss->GetExactDist2d(&ULDUAR_THORIM_PHASE2_TANK_SPOT) <= ULDUAR_THORIM_OPENING_SETTLED_RADIUS;

    if (elapsed < ULDUAR_THORIM_OPENING_HOLD_MAX_MS && !atAnchor && !ThorimCampSpotInLitCone(botAI, holdSpot))
        return true;

    // Latched here, where the "go" answer shows up. Only ever adds, so a second ask this tick agrees.
    state.openingReleased.insert(bot->GetGUID());
    return false;
}

bool ThorimBalconyHoldSpot(PlayerbotAI* botAI, Player* bot, Position& out)
{
    if (!botAI || !bot || !TakesRangedSpot(bot) || GetThorimSquad(botAI, bot) != ThorimSquad::Gauntlet)
        return false;

    // Nothing to hand out once the wait is over for everyone. Also keeps a bot that only gets up here
    // late from being given a slot it never uses.
    uint32 elapsed = 0;
    if (!ThorimPhase2ElapsedMs(botAI, elapsed) || elapsed >= ULDUAR_THORIM_OPENING_HOLD_MAX_MS)
        return false;

    EnsureBalconyHoldSlot(bot);

    uint8 slot = 0;
    if (!SlotOf(ThorimStateFor(bot).balconyHoldSlots, bot, slot))
        return false;

    Position const& spot = *balconyHoldSpots[std::min<uint8>(slot, ULDUAR_THORIM_BALCONY_HOLD_SLOTS - 1)];
    if (!ThorimPhase2OpeningHold(botAI, bot, spot))
        return false;

    out = spot;
    return true;
}

// The arena squad's side of the same wait, on the floor.
static bool ThorimOpeningSpot(PlayerbotAI* botAI, Player* bot, Position& out)
{
    if (ThorimEncounterState const* state = FindState(bot); !state || state->openingReleased.count(bot->GetGUID()))
        return false;

    uint32 elapsed = 0;
    if (!ThorimPhase2ElapsedMs(botAI, elapsed) || elapsed >= ULDUAR_THORIM_OPENING_HOLD_MAX_MS)
        return false;

    if (GetThorimSquad(botAI, bot) == ThorimSquad::Gauntlet)
        return false;

    EnsureOpeningSlot(bot);

    uint8 slot = 0;
    if (!SlotOf(ThorimStateFor(bot).openingSlots, bot, slot))
        return false;

    Position const& spot = *openingSpots[std::min<uint8>(slot, ULDUAR_THORIM_OPENING_SLOTS - 1)];
    if (!ThorimPhase2OpeningHold(botAI, bot, spot))
        return false;

    out = spot;
    return true;
}

static ThorimPhase2Role ResolveThorimPhase2Role(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot)
        return ThorimPhase2Role::None;

    // The latch, never the boss's victim: the role picks the destination, so following the victim relocates
    // both tanks on every trade. Never the ring either, since a ring point is measured off the boss.
    if (PlayerbotAI::IsTank(bot))
        return IsThorimAnchorTank(bot) ? ThorimPhase2Role::MainTank : ThorimPhase2Role::OffTank;

    if (TakesRangedSpot(bot))
        return ThorimPhase2Role::Ranged;

    return ThorimPhase2Role::MeleeRing;
}

ThorimPhase2Role GetThorimPhase2Role(PlayerbotAI* botAI, Player* bot)
{
    return ResolveThorimPhase2Role(botAI, bot);
}

namespace
{

// A bot tank down on the arena floor. The floor test keeps a tank still on the balcony off an arena spot.
bool ThorimTankCanAnchor(Player* member)
{
    return member && GET_PLAYERBOT_AI(member) && member->IsAlive() && member->GetMapId() == ULDUAR_MAP_ID &&
           PlayerbotAI::IsTank(member) && member->GetPositionZ() <= ULDUAR_THORIM_AXIS_Z_FLOOR_THRESHOLD;
}

// Bots only: a human cannot be steered onto a spot, which is why the role used to read the boss's victim
// instead. The held pick wins while it can still stand there, so a taunt trade relocates nobody.
Player* ElectThorimAnchorTank(PlayerbotAI* botAI, Group* group, ObjectGuid held)
{
    Unit* boss = GetThorim(botAI);
    Player* holdsBoss = nullptr;
    Player* lowest = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!ThorimTankCanAnchor(member))
            continue;

        if (member->GetGUID() == held)
            return member;

        if (boss && boss->GetVictim() == member)
            holdsBoss = member;

        if (!lowest || member->GetGUID() < lowest->GetGUID())
            lowest = member;
    }

    // Whoever already has him first, so the opening pick needs no walk, then lowest guid to settle it.
    return holdsBoss ? holdsBoss : lowest;
}

}  // namespace

void ThorimTickAnchorTank(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot || !NearThorimEncounter(bot))
        return;

    ThorimEncounterState& state = ThorimStateFor(bot);

    // Cleared outside the phase rather than on reset, so a latch cannot ride into the next pull.
    if (!ThorimPhase2Active(botAI))
    {
        state.anchorTank = ObjectGuid::Empty;
        state.anchorTankScanMs = 0;
        return;
    }

    Group* group = bot->GetGroup();
    if (!group)
        return;

    uint32 const now = getMSTime();
    if (state.anchorTankScanMs &&
        getMSTimeDiff(state.anchorTankScanMs, now) < ULDUAR_THORIM_ENCOUNTER_SCAN_INTERVAL_MS)
        return;

    state.anchorTankScanMs = now;

    Player* anchor = ElectThorimAnchorTank(botAI, group, state.anchorTank);
    state.anchorTank = anchor ? anchor->GetGUID() : ObjectGuid::Empty;
}

bool IsThorimAnchorTank(Player const* bot)
{
    ThorimEncounterState const* state = FindState(bot);
    return bot && state && state->anchorTank == bot->GetGUID();
}

bool TryGetThorimPhase2Spot(PlayerbotAI* botAI, Player* bot, ThorimPhase2Role role, Position& position)
{
    if (!botAI || !bot)
        return false;

    // No spot for a bot still on the balcony: the only walkable way down is ~300 yd back through the
    // corridor. The balcony node drops it over the edge.
    if (bot->GetPositionZ() > ULDUAR_THORIM_AXIS_Z_FLOOR_THRESHOLD)
        return false;

    if (role == ThorimPhase2Role::MainTank)
    {
        position = ULDUAR_THORIM_PHASE2_TANK_SPOT;
        return true;
    }

    Unit* boss = GetThorim(botAI);

    if (role == ThorimPhase2Role::Ranged)
    {
        EnsureRangedSlot(bot);

        uint8 slot = 0;
        if (!RangedSlotOf(bot, slot))
            return false;

        position = RangedSpot(slot);

        // No boss, no bearing, so no shelter. The spot on its own is still the right answer.
        if (!boss)
            return true;

        // Before the shelter latch, which then does not run during the wait. It picks the lit orb up
        // on its first call after, since that orb is new to it.
        Position opening;
        if (ThorimOpeningSpot(botAI, bot, opening))
        {
            position = opening;
            return true;
        }

        uint8 orbIndex = 0;
        ThorimRangedSpot(botAI, bot, boss, slot, position, orbIndex);
        return true;
    }

    if (role == ThorimPhase2Role::OffTank)
    {
        // A fixed point, not a bearing off the boss: a ring point moves with him, so a tank towing him from
        // one drifts the pair across the room. Still inside taunt range for the Unbalancing Strike swap.
        position = ULDUAR_THORIM_PHASE2_OFFTANK_SPOT;
        return true;
    }

    if (!boss)
        return false;

    if (role != ThorimPhase2Role::MeleeRing)
        return false;

    EnsureMeleeSlot(bot);

    uint8 slot = 0;
    if (!MeleeSlotOf(bot, slot))
        return false;

    // One answer for the whole ring rather than a slide of this bot's own, so a cone or a zone cannot
    // walk two slots together. It only moves when the orb changes or a zone lands on the arc.
    float const bearing = RingSlotBearing(botAI, bot, boss, slot);

    if (RingPoint(bot, boss, bearing, position))
        return true;

    return StaticMeleeSpot(boss, slot, position);
}

namespace
{

// The slot this bot's pets belong on: a ring melee's own, anyone else's off its guid. On a slot on purpose: a
// chain held inside one cluster spends its late hops on pets, not the next slot's players.
uint8 ThorimPetRingSlot(PlayerbotAI* botAI, Player* bot)
{
    uint8 slot = 0;
    if (GetThorimPhase2Role(botAI, bot) == ThorimPhase2Role::MeleeRing && MeleeSlotOf(bot, slot))
        return slot;

    return static_cast<uint8>(bot->GetGUID().GetCounter() % ULDUAR_THORIM_MELEE_SLOTS);
}

}  // namespace

bool ThorimPetsOffRingSlot(PlayerbotAI* botAI, Player* bot, std::vector<Unit*>& out)
{
    out.clear();

    if (!botAI || !bot || !ThorimPhase2Active(botAI))
        return false;

    Unit* boss = GetThorim(botAI);
    ThorimEncounterState const* state = FindState(bot);
    if (!boss || !state || !state->ringLatched)
        return false;

    float const bearing = state->ringSlots[ThorimPetRingSlot(botAI, bot)].composed;

    for (Unit* pet : bot->m_Controlled)
    {
        // Same net the leash casts: Army, Feral Spirits and the rest are guardians rather than pets and
        // they carry a chain hop exactly like one. Totems never move and the chain skips them anyway.
        if (!pet || !pet->IsAlive() || pet->IsTotem())
            continue;

        if (!pet->IsPet() && !pet->IsGuardian())
            continue;

        // Only the ones already on the boss. A pet sent at an add is doing its job, and one that has
        // left the room belongs to the leash, which runs first and shares the throttle below.
        if (pet->GetVictim() != boss)
            continue;

        if (AbsAngleDelta(BearingFromBoss(boss, pet->GetPositionX(), pet->GetPositionY()), bearing) <=
            ULDUAR_THORIM_PET_SLOT_TOLERANCE)
            continue;

        auto const sent = state->petRecallMs.find(pet->GetGUID());
        if (sent != state->petRecallMs.end() &&
            GetMSTimeDiffToNow(sent->second) < ULDUAR_THORIM_PET_RECALL_INTERVAL_MS)
            continue;

        out.push_back(pet);
    }

    return !out.empty();
}

void ThorimSendPetToRingSlot(PlayerbotAI* botAI, Player* bot, Unit* pet)
{
    if (!botAI || !bot || !pet)
        return;

    Unit* boss = GetThorim(botAI);
    ThorimEncounterState* state = FindState(bot);
    if (!boss || !state || !state->ringLatched)
        return;

    state->petRecallMs[pet->GetGUID()] = getMSTime();

    // A chase angle is measured off the target's facing, and Thorim turns with his tank, so the world
    // bearing is converted here every time rather than held.
    float const relative = Position::NormalizeOrientation(
        state->ringSlots[ThorimPetRingSlot(botAI, bot)].composed - boss->GetOrientation());

    // Range 0 like PetAI's own chase, and no command or stop: the pet keeps hitting him, just from a
    // bearing that doesn't bridge two slots.
    pet->GetMotionMaster()->MoveChase(boss, ChaseRange(0.0f, 0.0f),
                                      ChaseAngle(relative, ULDUAR_THORIM_PET_SLOT_TOLERANCE));
}

bool ThorimRingWantsMove(PlayerbotAI* botAI, Player* bot, Position const& spot)
{
    if (!bot)
        return false;

    ThorimEncounterState const* state = FindState(bot);

    // Wider once arrived, so a ring recomputed off a moving boss does not have the bot sliding in
    // place. A moving bot casts nothing.
    float const tolerance = state && state->ringArrived.count(bot->GetGUID())
                                ? ULDUAR_THORIM_RING_REPOSITION_TOLERANCE
                                : ULDUAR_THORIM_RING_ARRIVE_TOLERANCE;

    // Escape ORed outside the tolerance, not nested under arrived: nested, a refused order is never
    // retried, since the bot re-latches inside the 3 yd arrive test.
    return bot->GetDistance(spot) > tolerance || RingSpotBeatsTheDeadband(botAI, bot, spot);
}

void ThorimRingMarkArrived(Player* bot)
{
    if (bot)
        ThorimStateFor(bot).ringArrived.insert(bot->GetGUID());
}

void ThorimRingClearArrived(Player* bot)
{
    if (bot)
        ThorimStateFor(bot).ringArrived.erase(bot->GetGUID());
}

bool ThorimSpotUnderBlizzard(Player* bot, Position const& spot)
{
    if (!bot)
        return false;

    for (Position const& zone : ThorimBlizzardSpots(bot))
        if (zone.GetExactDist2d(spot.GetPositionX(), spot.GetPositionY()) < ULDUAR_THORIM_RING_BLIZZARD_CLEARANCE)
            return true;

    return false;
}

// 2D distance from a point to the walk between two positions, ends included. Own name, so it cannot
// clash with Hodir's file static copy in a unity build.
static float ThorimSegmentDistance2d(Position const& point, Position const& from, Position const& to)
{
    float const dx = to.GetPositionX() - from.GetPositionX();
    float const dy = to.GetPositionY() - from.GetPositionY();
    float const lengthSq = dx * dx + dy * dy;

    float t = 0.0f;
    if (lengthSq > 0.0f)
    {
        t = ((point.GetPositionX() - from.GetPositionX()) * dx + (point.GetPositionY() - from.GetPositionY()) * dy) /
            lengthSq;
        t = std::clamp(t, 0.0f, 1.0f);
    }

    float const nearestX = from.GetPositionX() + dx * t;
    float const nearestY = from.GetPositionY() + dy * t;
    return std::sqrt((point.GetPositionX() - nearestX) * (point.GetPositionX() - nearestX) +
                     (point.GetPositionY() - nearestY) * (point.GetPositionY() - nearestY));
}

bool ThorimWalkUnderBlizzard(Player* bot, Position const& from, Position const& to)
{
    if (!bot)
        return false;

    for (Position const& zone : ThorimBlizzardSpots(bot))
        if (ThorimSegmentDistance2d(zone, from, to) < ULDUAR_THORIM_RING_BLIZZARD_CLEARANCE)
            return true;

    return false;
}

bool ThorimCampSpotInLitCone(PlayerbotAI* botAI, Position const& spot)
{
    Unit* boss = GetThorim(botAI);
    if (!boss)
        return false;

    Unit* orb = ThorimChargedThunderOrb(botAI, SPELL_THORIM_LIGHTNING_ORB_VISUAL);
    if (!orb)
        return false;

    return InLightningChargeConeRanged(BearingFromBoss(boss, spot.GetPositionX(), spot.GetPositionY()),
                                       BearingFromBoss(boss, orb->GetPositionX(), orb->GetPositionY()));
}

bool ThorimCampBlizzardEscape(PlayerbotAI* botAI, Player* bot, Position& out)
{
    if (!botAI || !bot)
        return false;

    Unit* boss = GetThorim(botAI);
    if (!boss)
        return false;

    // Copied, since the lookups below go back through the shared state.
    std::vector<Position> const zones = ThorimBlizzardSpots(bot);
    if (zones.empty())
        return false;

    std::vector<HazardCircle> circles;
    circles.reserve(zones.size());
    for (Position const& zone : zones)
        circles.emplace_back(zone, ULDUAR_THORIM_RING_BLIZZARD_CLEARANCE);

    // Toward home or the shelter, so the walk back is short once the zone expires.
    Position spot;
    if (!TryGetThorimPhase2Spot(botAI, bot, ThorimPhase2Role::Ranged, spot))
        spot = bot->GetPosition();

    Unit* orb = ThorimChargedThunderOrb(botAI, SPELL_THORIM_LIGHTNING_ORB_VISUAL);
    float const coneBearing = orb ? BearingFromBoss(boss, orb->GetPositionX(), orb->GetPositionY()) : 0.0f;

    auto const accept = [boss, orb, coneBearing](float x, float y)
    {
        if (boss->GetExactDist2d(x, y) > ULDUAR_THORIM_CAMP_MAX_BOSS_RANGE)
            return false;

        return !orb || !InLightningChargeConeRanged(BearingFromBoss(boss, x, y), coneBearing);
    };

    Position const clear = FindNearestPositionClearOfHazards(bot, circles, ULDUAR_THORIM_CAMP_BLIZZARD_ESCAPE_RADIUS,
                                                             ULDUAR_THORIM_SWEEP_STEP, ULDUAR_THORIM_SWEEP_ANGLE_STEP,
                                                             &spot, accept);
    if (clear == Position())
        return false;

    out = clear;
    return true;
}

bool ThorimMeleeRingSettled(PlayerbotAI* botAI, Player* bot)
{
    if (!bot || GetThorimPhase2Role(botAI, bot) != ThorimPhase2Role::MeleeRing)
        return false;

    if (!ThorimPhase2Active(botAI))
        return false;

    // Same floor test TryGetThorimPhase2Spot uses. The guard this feeds zeroes nearly every mover, so a
    // stale latch on the balcony would freeze the bot up there.
    if (bot->GetPositionZ() > ULDUAR_THORIM_AXIS_Z_FLOOR_THRESHOLD)
        return false;

    ThorimEncounterState const* state = FindState(bot);
    return state && state->ringArrived.count(bot->GetGUID()) > 0;
}

bool ThorimTankOnAnchor(PlayerbotAI* botAI, Player* bot)
{
    if (!bot || !ThorimPhase2Active(botAI))
        return false;

    ThorimPhase2Role const role = GetThorimPhase2Role(botAI, bot);
    if (role != ThorimPhase2Role::MainTank && role != ThorimPhase2Role::OffTank)
        return false;

    // Same floor test the spot itself uses, or a tank still on the balcony is frozen up there.
    if (bot->GetPositionZ() > ULDUAR_THORIM_AXIS_Z_FLOOR_THRESHOLD)
        return false;

    Position spot;
    return TryGetThorimPhase2Spot(botAI, bot, role, spot) &&
           bot->GetDistance(spot) <= ULDUAR_THORIM_RING_ARRIVE_TOLERANCE;
}

Unit* ThorimChargedThunderOrb(PlayerbotAI* botAI, uint32 markerSpell)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot)
        return nullptr;

    // One shared sweep per interval, so no two bots disagree on the lit orb. One cache slot per marker,
    // or asking for both rescans on every call.
    ThorimEncounterState& state = ThorimStateFor(bot);
    bool const lightning = markerSpell == SPELL_THORIM_LIGHTNING_ORB_VISUAL;
    ObjectGuid& cachedGuid = lightning ? state.lightningOrbGuid : state.chargedOrbGuid;
    uint32& scanMs = lightning ? state.lightningOrbScanMs : state.orbScanMs;

    if (scanMs && GetMSTimeDiffToNow(scanMs) < ULDUAR_THORIM_ENCOUNTER_SCAN_INTERVAL_MS)
    {
        Unit* cached = botAI->GetUnit(cachedGuid);
        return cached && cached->HasAura(markerSpell) ? cached : nullptr;
    }

    scanMs = getMSTime();

    // The orbs are non-attackable pillar props, so they never appear in the target values.
    Unit* found = nullptr;
    std::list<Creature*> orbs;
    bot->GetCreatureListWithEntryInGrid(orbs, NPC_THORIM_THUNDER_ORB, ULDUAR_THORIM_LIGHTNING_CHARGE_RANGE);
    for (Creature* orb : orbs)
    {
        if (orb && orb->HasAura(markerSpell))
        {
            found = orb;
            break;
        }
    }

    cachedGuid = found ? found->GetGUID() : ObjectGuid::Empty;
    return found;
}

bool ThorimEncounterStateIsStale(PlayerbotAI* botAI)
{
    // GetThorim has no range, so without this gate a raid parked on Hodir reads an idle full-health
    // Thorim as a reset every tick.
    if (!NearThorimEncounter(botAI ? botAI->GetBot() : nullptr))
        return false;

    Unit* boss = GetThorim(botAI);
    if (!boss)
        return false;

    ThorimEncounterState* state = FindState(botAI->GetBot());
    if (!state)
        return false;

    // Full health alone is a trap: he watches the whole gauntlet from his balcony untouched.
    // JustEngagedWith fires at the pull, so combat state is what separates the two.
    if (boss->GetHealth() < boss->GetMaxHealth() || boss->IsInCombat())
    {
        state->engagedSeen = true;

        // A live pull is what arms the next round of resets. Nobody clears anything while he is up,
        // so this is empty on all but the first tick after a wipe.
        state->resetDone.clear();

        // Still up on the balcony means phase 2 has not started this pull, whatever the last one left.
        if (boss->GetPositionZ() >= ULDUAR_THORIM_AXIS_Z_FLOOR_THRESHOLD)
            state->phase2StartMs = 0;

        return false;
    }

    // Idle before the first pull is not a reset: reading it as one clears the squad split the corridor
    // just formed, and the next tick forms it again.
    if (!state->engagedSeen)
        return false;

    // Per bot, not raid-wide. Every member has its own latches to drop and each has to get a turn.
    return state->resetDone.count(botAI->GetBot()->GetGUID()) == 0;
}

bool ThorimBotHasEncounterState(Player* bot)
{
    ThorimEncounterState const* state = FindState(bot);
    if (!state)
        return false;

    // Everything the reset below drops. Leave one out and a bot holding only that never trips the
    // trigger, so the latch rides into the next pull.
    return state->meleeSlots.count(bot->GetGUID()) || state->ringArrived.count(bot->GetGUID()) ||
           state->barrierBailing.count(bot->GetGUID()) || state->squads.count(bot->GetGUID()) ||
           state->followMasterStripped.count(bot->GetGUID()) ||
           state->arenaAnchorArrived.count(bot->GetGUID()) || state->balconyStep.count(bot->GetGUID()) ||
           state->orbEscapes.count(bot->GetGUID()) || state->dpsTargets.count(bot->GetGUID()) ||
           state->petRecalls.count(bot->GetGUID()) || state->openingSlots.count(bot->GetGUID()) ||
           state->balconyHoldSlots.count(bot->GetGUID()) || state->openingReleased.count(bot->GetGUID()) ||
           state->runicSmashSide;
}

void ResetThorimEncounterState(Player* bot, bool clearInstance)
{
    if (!bot)
        return;

    if (clearInstance)
    {
        thorimStates.Reset(bot->GetInstanceId());
        return;
    }

    ThorimEncounterState* state = FindState(bot);
    if (!state)
        return;

    // Two halves, and they run on different schedules. This one is this bot's own latches and it has
    // to run for every member: they are what decide where a bot thinks it already walked to.
    state->meleeSlots.erase(bot->GetGUID());
    state->ringArrived.erase(bot->GetGUID());
    state->barrierBailing.erase(bot->GetGUID());
    state->followMasterStripped.erase(bot->GetGUID());
    state->arenaAnchorArrived.erase(bot->GetGUID());
    state->orbEscapes.erase(bot->GetGUID());
    state->petRecalls.erase(bot->GetGUID());
    state->balconyStep.erase(bot->GetGUID());
    state->dpsTargets.erase(bot->GetGUID());
    state->rangedSlots.erase(bot->GetGUID());
    state->rangedShelters.erase(bot->GetGUID());
    state->openingSlots.erase(bot->GetGUID());
    state->balconyHoldSlots.erase(bot->GetGUID());
    state->openingReleased.erase(bot->GetGUID());

    // Per pet rather than clearing the map: the rest of it belongs to the other bots in the instance,
    // who are not resetting.
    for (Unit* pet : bot->m_Controlled)
        if (pet)
            state->petRecallMs.erase(pet->GetGUID());

    // Marks this bot done for the cycle. ThorimEncounterStateIsStale reads it, and wipes it the next
    // time he is in combat.
    bool const firstThisCycle = state->resetDone.empty();
    state->resetDone.insert(bot->GetGUID());
    if (!firstThisCycle)
        return;

    // Instance-wide half (lane, split, scan caches), run by whichever bot gets here first. Per bot, it
    // would clear the split 25 times and AssignThorimSquads would re-form it behind each.
    state->runicSmashSide = 0;
    state->runicSmashSeenMs = 0;
    state->smashScanMs = 0;
    state->chargedOrbGuid = ObjectGuid::Empty;
    state->orbScanMs = 0;
    state->lightningOrbGuid = ObjectGuid::Empty;
    state->lightningOrbScanMs = 0;
    state->ringSlots = {};
    state->ringAnchor = 0.0f;
    state->ringLatched = false;
    state->ringOrb.Clear();
    state->ringSolveMs = 0;
    state->rangedShelters.clear();
    state->blizzardSpots.clear();
    state->blizzardScanMs = 0;
    state->bossGuid.Clear();
    state->colossusGuid.Clear();
    state->colossusScanMs = 0;
    state->runeGiantGuid.Clear();
    state->runeGiantScanMs = 0;
    state->squads.clear();
    state->squadsAssigned = false;
    state->sifJoined = false;
    state->humanSquadScanMs = 0;
    state->marksCleared = false;
    state->phase2StartMs = 0;
    state->openingReleased.clear();
}
