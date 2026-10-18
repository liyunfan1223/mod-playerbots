/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldEncounter_Ignis.h"

#include "CellImpl.h"
#include "Creature.h"
#include "EncounterHelpers.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Group.h"
#include "InstanceScript.h"
#include "Map.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RaidInstanceState.h"
#include "UldEncounterGate.h"
#include "UldScripts.h"
#include "Unit.h"

#include <cmath>
#include <functional>
#include <unordered_map>
#include <vector>

using namespace EncounterHelpers;

// Room centre, midway between the two water pools. The whole fight is fought here: it is the only
// spot from which the patch fan clears both pools by the 25 yd the core needs to light them.
const Position ULDUAR_IGNIS_BOSS_ANCHOR = Position(587.5f, 277.8f, 360.8f);
const Position ULDUAR_IGNIS_WATER_POOL_WEST = Position(526.771f, 277.796f, 360.802f);
const Position ULDUAR_IGNIS_WATER_POOL_EAST = Position(646.771f, 277.796f, 360.802f);

// Each assist tank's committed construct. Keyed by instance first: the same tank GUID comes back on a re-pull
// and in a second raid running the fight at the same time.
static RaidInstanceState<std::unordered_map<ObjectGuid, ObjectGuid>> ignisTankDrivenConstructGuid;

static std::unordered_map<ObjectGuid, ObjectGuid>& IgnisDrivenConstructsFor(Player* tank)
{
    return ignisTankDrivenConstructGuid.For(tank->GetInstanceId());
}

// The test FindNearestCreature puts each candidate through: the searcher's phase filter from the
// creature's side, then NearestCreatureEntryWithLiveStateInObjectRangeCheck from the bot's.
static bool IgnisPassesSearchCheck(Player* bot, Creature const* ignis)
{
    return ignis->InSamePhase(bot->GetPhaseMask()) && ignis->GetEntry() == NPC_IGNIS && ignis->IsAlive() &&
           bot->IsWithinDist(ignis, ULDUAR_IGNIS_ROOM_SEARCH_RADIUS) && bot->InSamePhase(ignis);
}

Unit* GetIgnisIf(PlayerbotAI* botAI, bool (*wanted)(Creature const*))
{
    Player* bot = botAI->GetBot();

    InstanceScript* instance = bot->GetInstanceScript();
    if (!instance)
    {
        Creature* ignis = bot->FindNearestCreature(NPC_IGNIS, ULDUAR_IGNIS_ROOM_SEARCH_RADIUS, true);
        return ignis && wanted(ignis) ? ignis : nullptr;
    }

    // One DB spawn, never summoned, tracked by the instance script from create to remove: the grid search could
    // only return him or nothing, and IgnisPassesSearchCheck is every test it would run.
    Creature* tracked = instance->GetCreature(ULD_BOSS_IGNIS);
    if (!tracked || !wanted(tracked) || !IgnisPassesSearchCheck(bot, tracked))
        return nullptr;

    return tracked;
}

Unit* GetIgnis(PlayerbotAI* botAI)
{
    return GetIgnisIf(botAI, [](Creature const*) { return true; });
}

Unit* GetEngagedIgnis(PlayerbotAI* botAI)
{
    return GetIgnisIf(botAI, [](Creature const* ignis) { return ignis->IsInCombat(); });
}

bool IsIgnisEngaged(PlayerbotAI* botAI) { return GetEngagedIgnis(botAI) != nullptr; }

// GetCreatureListWithEntryInGrid's own body, filling a vector instead of a list: one allocation per
// scan rather than one per match, and the same visit order, so ties still break the same way.
static void CollectIgnisRoomCreatures(WorldObject const* from, uint32 entry, std::vector<Creature*>& out)
{
    constexpr size_t expectedMatches = 32;
    out.reserve(expectedMatches);

    Acore::AllCreaturesOfEntryInRange check(from, entry, ULDUAR_IGNIS_ROOM_SEARCH_RADIUS);
    Acore::CreatureListSearcher<Acore::AllCreaturesOfEntryInRange> searcher(from, out, check);
    Cell::VisitObjects(from, searcher, ULDUAR_IGNIS_ROOM_SEARCH_RADIUS);
}

bool IsIgnisConstructActivated(Unit const* construct)
{
    if (!construct || !construct->IsAlive() || construct->GetEntry() != NPC_IGNIS_IRON_CONSTRUCT)
        return false;

    return !construct->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) &&
           !construct->HasAura(SPELL_IGNIS_CONSTRUCT_INACTIVE);
}

bool IsIgnisConstructMolten(Unit const* construct)
{
    return construct && construct->HasAura(SPELL_IGNIS_MOLTEN);
}

bool IsIgnisConstructBrittle(Unit const* construct)
{
    return construct && (construct->HasAura(SPELL_IGNIS_BRITTLE_10) || construct->HasAura(SPELL_IGNIS_BRITTLE_25));
}

// Grid search, not the cached "nearest npcs": that list is LOS-filtered and the walk to the water leaves
// SightDistance.
static Unit* GetNearestIgnisConstructMatching(WorldObject const* from,
                                              std::function<bool(Unit const*)> const& predicate)
{
    if (!from)
        return nullptr;

    std::vector<Creature*> constructs;
    CollectIgnisRoomCreatures(from, NPC_IGNIS_IRON_CONSTRUCT, constructs);

    Unit* best = nullptr;
    float bestDistance = ULDUAR_IGNIS_ROOM_SEARCH_RADIUS;

    for (Creature* construct : constructs)
    {
        if (!IsIgnisConstructActivated(construct) || !predicate(construct))
            continue;

        float const distance = from->GetExactDist2d(construct);
        if (distance > bestDistance)
            continue;

        best = construct;
        bestDistance = distance;
    }

    return best;
}

Unit* GetIgnisBrittleConstruct(PlayerbotAI* botAI)
{
    std::vector<Creature*> constructs;
    CollectIgnisRoomCreatures(botAI->GetBot(), NPC_IGNIS_IRON_CONSTRUCT, constructs);

    // Lowest GUID, the same pick on every bot: two can be Brittle at once, and a split raid wastes both windows.
    Unit* best = nullptr;
    for (Creature* construct : constructs)
    {
        if (!IsIgnisConstructActivated(construct) || !IsIgnisConstructBrittle(construct))
            continue;

        if (!best || construct->GetGUID() < best->GetGUID())
            best = construct;
    }

    return best;
}

Unit* GetIgnisNearestMoltenConstruct(PlayerbotAI* /*botAI*/, WorldObject const* from)
{
    return GetNearestIgnisConstructMatching(from, &IsIgnisConstructMolten);
}

Unit* GetIgnisHeldConstruct(PlayerbotAI* botAI, Player* tank)
{
    if (!tank)
        return nullptr;

    uint32 const instanceId = tank->GetInstanceId();
    if (!instanceId)
        return nullptr;

    // Find rather than For: a read must not create the instance entry.
    auto* driven = ignisTankDrivenConstructGuid.Find(instanceId);
    if (!driven)
        return nullptr;

    auto const held = driven->find(tank->GetGUID());
    if (held == driven->end())
        return nullptr;

    Unit* construct = botAI->GetUnit(held->second);

    // Molten wiped this construct's threat table, so handing it to a closer new one would release it
    // into the raid. Only Brittle (job done) or death lets the tank move on.
    return IsIgnisConstructActivated(construct) && !IsIgnisConstructBrittle(construct) ? construct : nullptr;
}

Unit* GetIgnisDrivenConstruct(PlayerbotAI* botAI, Player* tank)
{
    if (!tank)
        return nullptr;

    if (Unit* held = GetIgnisHeldConstruct(botAI, tank))
        return held;

    auto& driven = IgnisDrivenConstructsFor(tank);

    ObjectGuid const tankGuid = tank->GetGUID();
    driven.erase(tankGuid);

    Unit* construct = GetNearestIgnisConstructMatching(tank, [&driven, &tankGuid](Unit const* candidate)
    {
        if (IsIgnisConstructBrittle(candidate))
            return false;

        // Whatever the other tank already holds is off limits for the same reason: its threat table
        // is gone, so a tank swapping onto it hands it to the raid rather than to a tank.
        for (auto const& entry : driven)
            if (entry.first != tankGuid && entry.second == candidate->GetGUID())
                return false;

        return true;
    });

    if (!construct)
        return nullptr;

    driven[tankGuid] = construct->GetGUID();

    return construct;
}

Unit* GetIgnisNearestScorchedGround(PlayerbotAI* /*botAI*/, WorldObject const* from)
{
    if (!from)
        return nullptr;

    std::vector<Creature*> patches;
    CollectIgnisRoomCreatures(from, NPC_IGNIS_SCORCHED_GROUND, patches);

    Unit* best = nullptr;
    float bestDistance = ULDUAR_IGNIS_ROOM_SEARCH_RADIUS;

    for (Creature* patch : patches)
    {
        if (!patch->IsAlive())
            continue;

        // Never lit next to the water, so a construct parked there would wait for Molten forever.
        Position const& pool = GetIgnisNearestWaterPool(patch);
        if (patch->GetExactDist2d(&pool) <= ULDUAR_IGNIS_SCORCHED_GROUND_INERT_WATER_RADIUS)
            continue;

        float const distance = from->GetExactDist2d(patch);
        if (distance > bestDistance)
            continue;

        best = patch;
        bestDistance = distance;
    }

    return best;
}

Position const& GetIgnisNearestWaterPool(WorldObject const* from)
{
    if (!from)
        return ULDUAR_IGNIS_WATER_POOL_WEST;

    return from->GetExactDist2d(&ULDUAR_IGNIS_WATER_POOL_EAST) <
                   from->GetExactDist2d(&ULDUAR_IGNIS_WATER_POOL_WEST)
               ? ULDUAR_IGNIS_WATER_POOL_EAST
               : ULDUAR_IGNIS_WATER_POOL_WEST;
}

Unit* GetIgnisAssignedScorchedGround(PlayerbotAI* /*botAI*/, WorldObject const* from, int8 tankIndex)
{
    if (!from)
        return nullptr;

    std::vector<Creature*> patches;
    CollectIgnisRoomCreatures(from, NPC_IGNIS_SCORCHED_GROUND, patches);

    Position const& assigned = GetIgnisAssignedWaterPool(tankIndex);

    Unit* best = nullptr;
    float bestDistance = 0.0f;

    for (Creature* patch : patches)
    {
        if (!patch->IsAlive())
            continue;

        Position const& pool = GetIgnisNearestWaterPool(patch);
        if (patch->GetExactDist2d(&pool) <= ULDUAR_IGNIS_SCORCHED_GROUND_INERT_WATER_RADIUS)
            continue;

        // Sorted towards this tank's own pool rather than towards the tank, so the two of them work
        // opposite ends of the patch fan and their constructs never end up in the same water.
        float const distance = patch->GetExactDist2d(&assigned);
        if (best && distance >= bestDistance)
            continue;

        best = patch;
        bestDistance = distance;
    }

    return best;
}

int8 GetIgnisConstructTankIndex(PlayerbotAI* botAI, Player* bot)
{
    // GetGroupAssistTank only ever hands back a member that passes IsTank, so a non-tank can skip its
    // group walk, and with it an IsTank talent scan of every human in the raid.
    if (!PlayerbotAI::IsTank(bot))
        return -1;

    if (GetGroupAssistTank(bot, 0) == bot)
        return 0;

    if (GetGroupAssistTank(bot, 1) == bot)
        return 1;

    return -1;
}

Position const& GetIgnisAssignedWaterPool(int8 tankIndex)
{
    return tankIndex == 1 ? ULDUAR_IGNIS_WATER_POOL_EAST : ULDUAR_IGNIS_WATER_POOL_WEST;
}

Position GetIgnisWaterApproach(Player* tank, Position const& pool)
{
    float const distance = tank->GetExactDist2d(&pool);
    if (distance <= ULDUAR_IGNIS_WATER_STANDOFF)
        return tank->GetPosition();

    float const ratio = ULDUAR_IGNIS_WATER_STANDOFF / distance;
    float const x = pool.GetPositionX() + (tank->GetPositionX() - pool.GetPositionX()) * ratio;
    float const y = pool.GetPositionY() + (tank->GetPositionY() - pool.GetPositionY()) * ratio;

    float z = tank->GetMapWaterOrGroundLevel(x, y, pool.GetPositionZ());
    if (z <= INVALID_HEIGHT)
        z = pool.GetPositionZ();

    return Position(x, y, z);
}

// Which of the three arc slots the main tank holds, and whether Scorch was already up last time we
// looked. Latched per instance so a wipe or a second raid does not inherit a stale rotation.
struct IgnisTankArcState
{
    uint8 slot = 0;
    bool scorchUp = false;
};

static RaidInstanceState<IgnisTankArcState> _ignisTankArcStates;

static IgnisTankArcState& IgnisTankArcStateFor(Player* bot) { return _ignisTankArcStates.For(bot->GetInstanceId()); }

Position GetIgnisMainTankPosition(PlayerbotAI* botAI, Player* bot, Unit* boss)
{
    IgnisTankArcState& state = IgnisTankArcStateFor(bot);

    // Rising edge, so one step per Scorch, at the start of the root. Only the main tank consumes it: anyone else
    // asking would eat the transition.
    if (PlayerbotAI::IsMainTank(bot))
    {
        bool const scorchUp = IsIgnisScorchWindow(boss);
        if (scorchUp && !state.scorchUp)
            state.slot = (state.slot + 1) % ULDUAR_IGNIS_TANK_ARC_SLOTS;

        state.scorchUp = scorchUp;
    }

    float const angle = Position::NormalizeOrientation(
        ULDUAR_IGNIS_TANK_BEARING + (static_cast<float>(state.slot) - 1.0f) * ULDUAR_IGNIS_TANK_ARC_STEP);

    float x = ULDUAR_IGNIS_BOSS_ANCHOR.GetPositionX() + std::cos(angle) * ULDUAR_IGNIS_TANK_RADIUS;
    float y = ULDUAR_IGNIS_BOSS_ANCHOR.GetPositionY() + std::sin(angle) * ULDUAR_IGNIS_TANK_RADIUS;

    float z = bot->GetMapWaterOrGroundLevel(x, y, ULDUAR_IGNIS_BOSS_ANCHOR.GetPositionZ());
    if (z <= INVALID_HEIGHT)
        z = ULDUAR_IGNIS_BOSS_ANCHOR.GetPositionZ();

    bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                   bot->GetPositionZ(), x, y, z, false);

    return Position(x, y, z);
}

// Both ids: spelldifficulty_dbc remaps these on 25-man, so the 10-man id alone never matches.
bool IsIgnisScorchWindow(Unit const* boss)
{
    return boss && (boss->HasAura(SPELL_IGNIS_SCORCH) || boss->HasAura(SPELL_IGNIS_SCORCH_25));
}

bool IsIgnisFlameJetsCasting(Unit const* boss)
{
    if (!boss || !boss->HasUnitState(UNIT_STATE_CASTING))
        return false;

    Spell* spell = boss->GetCurrentSpell(CURRENT_GENERIC_SPELL);

    return spell && (spell->m_spellInfo->Id == SPELL_IGNIS_FLAME_JETS ||
                     spell->m_spellInfo->Id == SPELL_IGNIS_FLAME_JETS_25);
}

bool IsIgnisSlagPotVictim(Player* bot)
{
    return bot && (bot->HasAura(SPELL_IGNIS_SLAG_POT_10) || bot->HasAura(SPELL_IGNIS_SLAG_POT_25));
}

Player* GetIgnisSlagPotVictim(PlayerbotAI* botAI)
{
    Group* group = botAI->GetBot()->GetGroup();
    if (!group)
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && IsIgnisSlagPotVictim(member))
            return member;
    }

    return nullptr;
}
