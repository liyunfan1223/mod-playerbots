/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldEncounter_Vezax.h"

#include "Creature.h"
#include "EncounterHelpers.h"
#include "Group.h"
#include "InstanceScript.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RaidInstanceState.h"
#include "Spell.h"
#include "SpellAuras.h"
#include "Timer.h"
#include "UldHardMode.h"
#include "UldScripts.h"
#include "Unit.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <array>
#include <list>
#include <unordered_map>

using namespace EncounterHelpers;

// Vezax' own spawn point, and the point the Saronite Vapors charge to when they merge.
const Position ULDUAR_VEZAX_ANCHOR = Position(1852.78f, 81.3856f, 342.461f);

static RaidInstanceState<VezaxEncounterState> vezaxEncounterStates;

// Apart from the encounter state: a wipe resets that before every owner has had its pets back.
struct VezaxHushedPets
{
    std::unordered_map<ObjectGuid, ReactStates> stances;
};

static RaidInstanceState<VezaxHushedPets> vezaxHushedPets;

namespace
{

Position VezaxPositionAt(Position const& centre, float bearing, float radius)
{
    return Position(centre.GetPositionX() + std::cos(bearing) * radius,
                    centre.GetPositionY() + std::sin(bearing) * radius, centre.GetPositionZ());
}

// Radius out along the camp bearing, offset across it. Slot and strafe share it, so the strafe is a pure
// sideways step.
Position VezaxCampPosition(Position const& boss, float radius, float tangentialOffset)
{
    float const forward = ULDUAR_VEZAX_ARC_ORIENTATION;
    float const across = ULDUAR_VEZAX_ARC_ORIENTATION + static_cast<float>(M_PI) / 2.0f;

    return Position(
        boss.GetPositionX() + std::cos(forward) * radius + std::cos(across) * tangentialOffset,
        boss.GetPositionY() + std::sin(forward) * radius + std::sin(across) * tangentialOffset,
        boss.GetPositionZ());
}

// [0,10) group L, [10,20) group R, row-major inside each.
bool VezaxSlotIsRightGroup(uint8 slotIndex) { return slotIndex >= ULDUAR_VEZAX_GROUP_SLOTS; }

uint8 VezaxSlotRow(uint8 slotIndex)
{
    return static_cast<uint8>((slotIndex % ULDUAR_VEZAX_GROUP_SLOTS) / ULDUAR_VEZAX_CAMP_FILES);
}

uint8 VezaxSlotFile(uint8 slotIndex)
{
    return static_cast<uint8>(slotIndex % ULDUAR_VEZAX_CAMP_FILES);
}

// Tangential offset of a slot from the camp centre line, signed: negative is group L. File 0 is the
// inner one of its group, so the two inner files end up 2 * GROUP_OFFSET - FILE_SPACING apart.
float VezaxSlotTangentialOffset(uint8 slotIndex)
{
    float const fromGroupCentre =
        (static_cast<float>(VezaxSlotFile(slotIndex)) - 0.5f) * ULDUAR_VEZAX_CAMP_FILE_SPACING;
    float const magnitude = ULDUAR_VEZAX_CAMP_GROUP_OFFSET + fromGroupCentre;

    return VezaxSlotIsRightGroup(slotIndex) ? magnitude : -magnitude;
}

// Radius of a slot from the boss. Row 0 is nearest, and the rows straddle the camp radius so the
// middle of the camp sits where CAMP_RADIUS says it does.
float VezaxSlotRadius(uint8 slotIndex)
{
    float const centred =
        static_cast<float>(VezaxSlotRow(slotIndex)) - (ULDUAR_VEZAX_CAMP_ROWS - 1) * 0.5f;

    return ULDUAR_VEZAX_CAMP_RADIUS + centred * ULDUAR_VEZAX_CAMP_ROW_SPACING;
}

// Alternates groups so neither fills first, near rows first so a short camp is short at the back.
constexpr std::array<uint8, ULDUAR_VEZAX_TOTAL_SLOTS> VEZAX_SLOT_FILL_ORDER = {{
    0, 10, 1, 11,  // row 0, alternating groups
    2, 12, 3, 13,  // row 1
    4, 14, 5, 15,  // row 2
    6, 16, 7, 17,  // row 3
    8, 18, 9, 19,  // row 4
}};

bool VezaxTakesSlot(Player* member)
{
    return member && PlayerbotAI::IsRanged(member) && !PlayerbotAI::IsMainTank(member);
}

// The slot a bot dodges from, and the one that decides which side it carries a mark to. Both read the
// assignment rather than where the bot currently stands, so a group keeps its shape.
bool TryGetVezaxDodgeSlot(Player* bot, uint8& slotIndex)
{
    VezaxEncounterState const* state = bot ? vezaxEncounterStates.Find(bot->GetInstanceId()) : nullptr;
    if (!state)
        return false;

    auto const assignmentItr = state->slotAssignments.find(bot->GetGUID());
    if (assignmentItr == state->slotAssignments.end())
        return false;

    slotIndex = assignmentItr->second;
    return true;
}

}  // namespace

Unit* GetVezax(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    InstanceScript* instance = bot ? bot->GetInstanceScript() : nullptr;
    if (!instance)
        return nullptr;

    // The guid lookup has no liveness filter, so a dead boss is rejected here or every gate stays open
    // through the loot roll.
    Creature* vezax = instance->GetCreature(ULD_DATA_VEZAX);
    return vezax && vezax->IsAlive() ? vezax : nullptr;
}

// Combat, not just alive: the guid lookup answers from anywhere on the map, and the guards keyed on this
// zero the target pickers and hold caster damage.
bool VezaxEncounterActive(PlayerbotAI* botAI)
{
    Unit* vezax = GetVezax(botAI);
    return vezax && vezax->IsInCombat();
}

namespace
{
enum class VezaxFormation
{
    Outside,
    NoBoss,
    Idle,
    On
};

VezaxFormation ReadVezaxFormation(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot)
        return VezaxFormation::Outside;

    Unit* vezax = GetVezax(botAI);
    if (!vezax)
        return VezaxFormation::NoBoss;

    // Off him, not his spawn: every slot and reach spell's stop are measured from him, and this has to
    // hold all of them wherever he settles.
    bool const inRoom =
        bot->GetExactDist2d(vezax) <= ULDUAR_VEZAX_ARENA_RADIUS &&
        std::fabs(bot->GetPositionZ() - vezax->GetPositionZ()) <= ULDUAR_VEZAX_ARENA_HEIGHT;
    if (!inRoom)
        return VezaxFormation::Outside;

    return vezax->IsInCombat() ? VezaxFormation::On : VezaxFormation::Idle;
}
}  // namespace

bool VezaxFormationActive(PlayerbotAI* botAI) { return ReadVezaxFormation(botAI) == VezaxFormation::On; }

void GatherVezaxHazards(Player* bot, std::vector<VezaxHazard>& hazards, float searchRadius)
{
    hazards.clear();
    if (!bot)
        return;

    for (Position const& position :
         GetDynamicObjectPositions(bot, searchRadius, SPELL_VEZAX_SHADOW_CRASH_FIELD))
    {
        VezaxHazard hazard;
        hazard.position = position;
        hazards.push_back(hazard);
    }
}

bool TryGetVezaxNearestHazard(Player* bot, std::vector<VezaxHazard> const& hazards,
                              VezaxHazard& hazard)
{
    if (!bot)
        return false;

    bool found = false;
    float bestDistance = std::numeric_limits<float>::max();

    for (VezaxHazard const& candidate : hazards)
    {
        float const distance =
            bot->GetExactDist2d(candidate.position.GetPositionX(), candidate.position.GetPositionY());
        if (found && distance >= bestDistance)
            continue;

        hazard = candidate;
        bestDistance = distance;
        found = true;
    }

    return found;
}

bool VezaxCanSoakShadowCrashField(Player* bot)
{
    // Same test as a camp slot, healers included: nothing else on this boss restores a point of mana.
    if (!bot || !PlayerbotAI::IsRanged(bot) || PlayerbotAI::IsMainTank(bot))
        return false;

    return bot->GetMaxPower(POWER_MANA) > 0;
}

bool TryGetVezaxSlotPosition(Player* bot, uint8 slotIndex, Position& position)
{
    if (slotIndex >= ULDUAR_VEZAX_TOTAL_SLOTS)
        return false;

    // Boss-relative: every distance the camp is built on is his, and a ranged pull can settle him yards
    // off his spawn.
    PlayerbotAI* botAI = bot ? GET_PLAYERBOT_AI(bot) : nullptr;
    Unit* vezax = botAI ? GetVezax(botAI) : nullptr;
    if (!vezax)
        return false;

    position = VezaxCampPosition(vezax->GetPosition(), VezaxSlotRadius(slotIndex),
                                 VezaxSlotTangentialOffset(slotIndex));
    return true;
}

float VezaxSlotTolerance(Player* bot)
{
    if (!bot)
        return ULDUAR_VEZAX_SLOT_TOLERANCE;

    return PlayerbotAI::IsMainTank(bot) ? ULDUAR_VEZAX_TANK_SLOT_TOLERANCE
                                        : ULDUAR_VEZAX_SLOT_TOLERANCE;
}

void EnsureVezaxSlotAssignments(Player* bot)
{
    Group* group = bot ? bot->GetGroup() : nullptr;
    if (!group || bot->GetMapId() != ULDUAR_MAP_ID)
        return;

    VezaxEncounterState& state = vezaxEncounterStates.For(bot->GetInstanceId());

    // Drop assignments whose holder left the instance or changed role.
    std::vector<ObjectGuid> stale;
    for (auto const& assignment : state.slotAssignments)
    {
        Player* holder = nullptr;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->GetGUID() == assignment.first)
            {
                holder = member;
                break;
            }
        }

        if (!holder || holder->GetMapId() != ULDUAR_MAP_ID || !VezaxTakesSlot(holder))
            stale.push_back(assignment.first);
    }

    for (ObjectGuid const& guid : stale)
        state.slotAssignments.erase(guid);

    std::array<bool, ULDUAR_VEZAX_TOTAL_SLOTS> used = {};
    for (auto const& assignment : state.slotAssignments)
        if (assignment.second < ULDUAR_VEZAX_TOTAL_SLOTS)
            used[assignment.second] = true;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != ULDUAR_MAP_ID || !VezaxTakesSlot(member))
            continue;

        if (state.slotAssignments.find(member->GetGUID()) != state.slotAssignments.end())
            continue;

        for (uint8 slotIndex : VEZAX_SLOT_FILL_ORDER)
        {
            if (used[slotIndex])
                continue;

            state.slotAssignments[member->GetGUID()] = slotIndex;
            used[slotIndex] = true;
            break;
        }
    }
}

bool TryGetVezaxSlot(Player* bot, Position& position)
{
    if (!bot)
        return false;

    // The main tank holds the anchor, which is Vezax's own spawn: he is already in melee range of a
    // tank standing there, so he never walks, and every radius measured from that point stays true.
    if (PlayerbotAI::IsMainTank(bot))
    {
        position = ULDUAR_VEZAX_ANCHOR;
        return true;
    }

    if (!VezaxTakesSlot(bot))
        return false;

    EnsureVezaxSlotAssignments(bot);

    VezaxEncounterState const* state = vezaxEncounterStates.Find(bot->GetInstanceId());
    if (!state)
        return false;

    auto const assignmentItr = state->slotAssignments.find(bot->GetGUID());
    // Camp full, 21+ healers and ranged: the bot holds no position.
    if (assignmentItr == state->slotAssignments.end())
        return false;

    return TryGetVezaxSlotPosition(bot, assignmentItr->second, position);
}

bool TryGetVezaxMarkSpot(Player* bot, Position& position)
{
    if (!bot)
        return false;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    Unit* vezax = botAI ? GetVezax(botAI) : nullptr;
    if (!vezax)
        return false;

    // Off the boss, not the anchor: the camp is boss-relative, and a fixed spot latches IsDuplicateMove
    // for 5s of the 10s debuff after one aborted move.
    uint8 slotIndex = 0;
    if (TryGetVezaxDodgeSlot(bot, slotIndex))
    {
        // An arc keeps the distance from him, so the band holds and the 15 yd strafe can't close the gap.
        float const side = VezaxSlotIsRightGroup(slotIndex) ? ULDUAR_VEZAX_MARK_SIDE_OFFSET
                                                            : -ULDUAR_VEZAX_MARK_SIDE_OFFSET;
        float const bearing = ULDUAR_VEZAX_ARC_ORIENTATION + side / ULDUAR_VEZAX_CAMP_RADIUS;

        position = VezaxPositionAt(vezax->GetPosition(), Position::NormalizeOrientation(bearing),
                                   ULDUAR_VEZAX_CAMP_RADIUS);
        return true;
    }

    // A marked melee has no camp side to step to. South of the boss is the one direction that is
    // away from both the ball and the camp.
    std::array<float, ULDUAR_VEZAX_MARK_SPOT_COUNT> const bearings = {
        ULDUAR_VEZAX_ARC_ORIENTATION + ULDUAR_VEZAX_MARK_SPOT_ARC_OFFSET,
        ULDUAR_VEZAX_ARC_ORIENTATION - ULDUAR_VEZAX_MARK_SPOT_ARC_OFFSET,
        ULDUAR_VEZAX_ARC_ORIENTATION + static_cast<float>(M_PI)};

    bool found = false;
    float bestDistance = std::numeric_limits<float>::max();

    for (float bearing : bearings)
    {
        Position const candidate = VezaxPositionAt(
            vezax->GetPosition(), Position::NormalizeOrientation(bearing),
            ULDUAR_VEZAX_MARK_SPOT_RADIUS);

        float const distance = bot->GetExactDist2d(candidate.GetPositionX(), candidate.GetPositionY());
        if (found && distance >= bestDistance)
            continue;

        position = candidate;
        bestDistance = distance;
        found = true;
    }

    return found;
}

Unit* GetVezaxMarkedAlly(Player* bot)
{
    Group* group = bot ? bot->GetGroup() : nullptr;
    if (!group)
        return nullptr;

    Unit* nearest = nullptr;
    float bestDistance = std::numeric_limits<float>::max();

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() || member->GetMapId() != ULDUAR_MAP_ID)
            continue;

        if (!member->HasAura(SPELL_MARK_OF_THE_FACELESS))
            continue;

        float const distance = bot->GetExactDist2d(member);
        if (distance > ULDUAR_VEZAX_MARK_BREAK_DISTANCE || (nearest && distance >= bestDistance))
            continue;

        nearest = member;
        bestDistance = distance;
    }

    return nearest;
}

bool TryGetVezaxMarkBreakSpot(Player* bot, Unit* marked, Position& spot)
{
    if (!bot || !marked)
        return false;

    // The leech really reaches ~16.5 (DBC 15 plus the victim's combat reach), and the clearance adds 1.5.
    std::vector<Position> const avoid = {marked->GetPosition()};
    Position const candidate = FindNearestPositionClearOfHazards(
        bot, avoid, ULDUAR_VEZAX_MARK_BREAK_DISTANCE, ULDUAR_VEZAX_HAZARD_LOCAL_SEARCH_RADIUS);

    if (candidate == Position())
        return false;

    spot = candidate;
    return true;
}

bool TryGetVezaxShadowCrashImpact(PlayerbotAI* botAI, Position& impact)
{
    if (!GetVezax(botAI))
        return false;

    VezaxEncounterState const* state = vezaxEncounterStates.Find(botAI->GetBot()->GetInstanceId());
    if (!state || !state->crashWindowMs ||
        getMSTimeDiff(state->crashCastMs, getMSTime()) >= state->crashWindowMs)
        return false;

    impact = state->crashImpact;
    return true;
}

void VezaxNoteShadowCrash(Unit* vezax, Position const& impact, uint32 flightMs)
{
    if (!vezax)
        return;

    VezaxEncounterState& state = vezaxEncounterStates.For(vezax->GetInstanceId());
    state.crashImpact = impact;
    state.crashCastMs = getMSTime();
    state.crashWindowMs = flightMs + ULDUAR_VEZAX_SHADOW_CRASH_LAND_SLACK_MS;
}

bool VezaxHardModePending(PlayerbotAI* botAI)
{
    if (!IsVezaxHardModeActive(botAI) || !VezaxEncounterActive(botAI))
        return false;

    // No state yet just means nothing has happened in this pull.
    VezaxEncounterState const* state = vezaxEncounterStates.Find(botAI->GetBot()->GetInstanceId());
    return !state || (!state->vaporKilled && !state->animusSummoned);
}

bool VezaxDamageHeld(PlayerbotAI* botAI)
{
    if (!IsVezaxHardModeActive(botAI))
        return false;

    float const line =
        PlayerbotAI::IsTank(botAI->GetBot()) ? ULDUAR_VEZAX_TANK_HOLD_HEALTH_PCT : ULDUAR_VEZAX_HOLD_HEALTH_PCT;
    Unit* vezax = GetVezax(botAI);
    return vezax && VezaxHardModePending(botAI) && !vezax->HasAura(SPELL_VEZAX_SARONITE_BARRIER) &&
           vezax->GetHealthPct() <= line;
}

void VezaxHushPet(Creature* pet)
{
    // First stance only, or a second hush would save passive as the one to give back.
    vezaxHushedPets.For(pet->GetInstanceId()).stances.emplace(pet->GetGUID(), pet->GetReactState());
    pet->SetReactState(REACT_PASSIVE);
}

void VezaxUnhushPet(Creature* pet)
{
    VezaxHushedPets* hushed = vezaxHushedPets.Find(pet->GetInstanceId());
    if (!hushed)
        return;

    auto const it = hushed->stances.find(pet->GetGUID());
    if (it == hushed->stances.end())
        return;

    pet->SetReactState(it->second);
    hushed->stances.erase(it);
}

bool VezaxIsHushedPet(Creature* pet)
{
    VezaxHushedPets const* hushed = vezaxHushedPets.Find(pet->GetInstanceId());
    return hushed && hushed->stances.count(pet->GetGUID());
}

bool VezaxHarmfulCastOn(Spell const* spell, Unit* vezax)
{
    return spell && spell->m_targets.GetUnitTargetGUID() == vezax->GetGUID() && !spell->GetSpellInfo()->IsPositive();
}

void VezaxNoteVaporKilled(Unit* vapor)
{
    if (vapor)
        vezaxEncounterStates.For(vapor->GetInstanceId()).vaporKilled = true;
}

void VezaxNoteAnimusSummoned(Unit* vapor)
{
    if (!vapor)
        return;

    VezaxEncounterState& state = vezaxEncounterStates.For(vapor->GetInstanceId());
    state.animusSummoned = true;
    state.animusSummonMs = getMSTime();
}

void VezaxNoteVaporSummon(Unit* vezax)
{
    if (vezax)
        ++vezaxEncounterStates.For(vezax->GetInstanceId()).vaporSummons;
}

bool VezaxAnimusDue(PlayerbotAI* botAI)
{
    if (!VezaxHardModePending(botAI))
        return false;

    VezaxEncounterState const* state = vezaxEncounterStates.Find(botAI->GetBot()->GetInstanceId());
    return state && state->vaporSummons >= ULDUAR_VEZAX_REDIRECT_SAVE_SUMMON;
}

bool TryGetVezaxAnimusAge(PlayerbotAI* botAI, uint32& ageMs)
{
    VezaxEncounterState const* state = vezaxEncounterStates.Find(botAI->GetBot()->GetInstanceId());
    if (!state || !state->animusSummoned)
        return false;

    ageMs = getMSTimeDiff(state->animusSummonMs, getMSTime());
    return true;
}

char const* VezaxAnimusRedirectSpell(Player* bot)
{
    if (!bot)
        return nullptr;

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

bool IsVezaxAnimusOnBot(Player* bot)
{
    if (!bot)
        return false;

    for (Unit* attacker : bot->getAttackers())
        if (attacker->GetEntry() == NPC_VEZAX_SARONITE_ANIMUS && attacker->GetVictim() == bot)
            return true;

    return false;
}

bool VezaxAnimusPhaseActive(PlayerbotAI* botAI)
{
    if (!IsVezaxHardModeActive(botAI))
        return false;

    Unit* vezax = GetVezax(botAI);
    return vezax && vezax->HasAura(SPELL_VEZAX_SARONITE_BARRIER);
}

float VezaxMeleeHoldDistance(Player* bot, Unit* vezax)
{
    return ULDUAR_VEZAX_SHADOW_CRASH_MIN_RANGE + vezax->GetCombatReach() + bot->GetCombatReach() -
           ULDUAR_VEZAX_ANIMUS_MELEE_MARGIN;
}

namespace
{
// Guid rank among the melee bots. More bots than lanes double up, which costs nothing: inside the hold
// distance nobody gets picked.
uint8 VezaxAnimusMeleeLane(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return 0;

    uint8 rank = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() || member->GetMapId() != bot->GetMapId())
            continue;

        if (!GET_PLAYERBOT_AI(member) || PlayerbotAI::IsRanged(member) || PlayerbotAI::IsMainTank(member))
            continue;

        if (member->GetGUID() < bot->GetGUID())
            ++rank;
    }

    return rank % ULDUAR_VEZAX_ANIMUS_MELEE_LANES;
}
}  // namespace

bool TryGetVezaxAnimusMeleeSpot(Player* bot, Unit* animus, Position& spot)
{
    PlayerbotAI* botAI = bot ? GET_PLAYERBOT_AI(bot) : nullptr;
    Unit* vezax = botAI ? GetVezax(botAI) : nullptr;
    if (!vezax || !animus)
        return false;

    float const ax = animus->GetPositionX();
    float const ay = animus->GetPositionY();
    float const vx = vezax->GetPositionX();
    float const vy = vezax->GetPositionY();
    float const apart = std::hypot(vx - ax, vy - ay);

    // Sideways unit vector across the line between them. Stacked on one spot there is no line, and
    // every lane collapses onto the midpoint.
    float const acrossX = apart > 0.1f ? -(vy - ay) / apart : 0.0f;
    float const acrossY = apart > 0.1f ? (vx - ax) / apart : 0.0f;

    // Lanes 0, 1, 2, 3, 4 sit at 0, +1, -1, +2, -2 spreads off the midpoint.
    uint8 const lane = VezaxAnimusMeleeLane(bot);
    float const side = lane % 2 ? 1.0f : -1.0f;

    float const holdVezax = VezaxMeleeHoldDistance(bot, vezax);
    float const holdAnimus = bot->GetMeleeRange(animus) - 1.0f;

    // A wide lane comes in toward the middle until it fits both reaches.
    for (int steps = (lane + 1) / 2; steps >= 0; --steps)
    {
        float const offset = side * steps * ULDUAR_VEZAX_ANIMUS_MELEE_SPREAD;
        float const x = (ax + vx) / 2.0f + acrossX * offset;
        float const y = (ay + vy) / 2.0f + acrossY * offset;

        if (std::hypot(x - vx, y - vy) > holdVezax || std::hypot(x - ax, y - ay) > holdAnimus)
            continue;

        spot = Position(x, y, (animus->GetPositionZ() + vezax->GetPositionZ()) / 2.0f);
        return true;
    }

    return false;
}

bool VezaxDodgesShadowCrash(Player* bot) { return bot && !PlayerbotAI::IsMainTank(bot); }

bool TryGetVezaxDodgeSpot(Player* bot, Position const& impact, Position& spot)
{
    if (!bot)
        return false;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    Unit* vezax = botAI ? GetVezax(botAI) : nullptr;

    // The whole group strafes the same way and arrives intact. A per-bot nearest-clear search scatters
    // it and lands some back under the next crash.
    uint8 slotIndex = 0;
    if (vezax && TryGetVezaxDodgeSlot(bot, slotIndex))
    {
        float const strafe = VezaxSlotIsRightGroup(slotIndex) ? ULDUAR_VEZAX_CAMP_STRAFE
                                                              : -ULDUAR_VEZAX_CAMP_STRAFE;
        Position const strafed =
            VezaxCampPosition(vezax->GetPosition(), VezaxSlotRadius(slotIndex),
                              VezaxSlotTangentialOffset(slotIndex) + strafe);

        // Checked, not assumed: the strafe direction is fixed per group and ignores where the crash went.
        if (strafed.GetExactDist2d(impact.GetPositionX(), impact.GetPositionY()) >=
            ULDUAR_VEZAX_SHADOW_CRASH_IMPACT_RADIUS)
        {
            spot = strafed;
            return true;
        }
    }

    // No slot, no boss, or the lane is buried. Anything out of the blast beats standing in it.
    std::vector<Position> const avoid = {impact};
    Position const candidate = FindNearestPositionClearOfHazards(
        bot, avoid, ULDUAR_VEZAX_SHADOW_CRASH_DODGE_CLEARANCE,
        ULDUAR_VEZAX_SHADOW_CRASH_DODGE_SEARCH_RADIUS);

    if (candidate == Position())
        return false;

    spot = candidate;
    return true;
}

void ResetVezaxEncounterState(Player* bot, bool clearInstance)
{
    if (!bot)
        return;

    if (clearInstance)
    {
        vezaxEncounterStates.Reset(bot->GetInstanceId());
        return;
    }

    VezaxEncounterState* state = vezaxEncounterStates.Find(bot->GetInstanceId());
    if (!state)
        return;

    state->slotAssignments.erase(bot->GetGUID());
}

bool VezaxHasEncounterState(Player* bot)
{
    return bot && vezaxEncounterStates.Find(bot->GetInstanceId());
}

char const* VezaxReadyInterrupt(Player* bot, Unit* target)
{
    if (!bot || !target)
        return nullptr;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
        return nullptr;

    static char const* const interrupts[] = {"kick",        "pummel",         "shield bash",
                                             "counterspell", "wind shear",    "mind freeze",
                                             "silencing shot", "spell lock"};

    for (char const* interrupt : interrupts)
        if (botAI->CanCastSpell(interrupt, target))
            return interrupt;

    return nullptr;
}

bool VezaxIsSearingFlamesInterrupter(Player* bot, Unit* boss)
{
    if (!bot || !boss || !VezaxReadyInterrupt(bot, boss))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return true;

    bool interrupter = true;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() || member->GetMapId() != ULDUAR_MAP_ID)
            continue;

        if (member->GetGUID() < bot->GetGUID() && VezaxReadyInterrupt(member, boss))
        {
            interrupter = false;
            break;
        }
    }

    return interrupter;
}
