/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPEncounter_Muru.h"
#include "AiObjectContext.h"
#include "CharmInfo.h"
#include "CreatureAI.h"
#include "Playerbots.h"
#include <algorithm>
#include <limits>
#include <list>
#include <mutex>

namespace SwpHelpers
{

namespace
{

void StampMuruDarknessWindow(
    MuruDarknessState& state, uint32 now, uint32 elapsedMs, uint32 remainingMs)
{
    uint32 const startMs = now > elapsedMs ? now - elapsedMs : 0;

    if (!state.startMs || state.expireMs <= now || startMs < state.startMs)
        state.startMs = startMs;

    state.expireMs = std::max(state.expireMs, now + remainingMs);
}

MuruEncounterGuids const& GetCachedMuruEncounterGuids(PlayerbotAI* botAI)
{
    return botAI->GetAiObjectContext()
        ->GetValue<MuruEncounterGuids>("muru encounter targets")->RefGet();
}

Unit* ResolveLivingUnit(PlayerbotAI* botAI, ObjectGuid const& guid)
{
    Unit* unit = botAI->GetUnit(guid);
    return unit && unit->IsAlive() ? unit : nullptr;
}

void ResolveLivingUnits(PlayerbotAI* botAI, GuidVector const& guids, std::vector<Unit*>& units)
{
    units.reserve(guids.size());
    for (ObjectGuid const& guid : guids)
    {
        if (Unit* unit = ResolveLivingUnit(botAI, guid))
            units.push_back(unit);
    }
}

float GetBerserkerStunReach(Player* bot)
{
    switch (bot->getClass())
    {
        case CLASS_DRUID:
        case CLASS_ROGUE:
        case CLASS_WARRIOR:
            return MELEE_ABILITY_REACH;

        case CLASS_PALADIN:
            return HAMMER_OF_JUSTICE_REACH;

        case CLASS_WARLOCK:
            return RANGED_ABILITY_REACH;

        default:
            return bot->getRace() == RACE_TAUREN ? SELF_AOE_RACIAL_RADIUS : 0.0f;
    }
}

float GetFuryMageInterruptReach(Player* bot)
{
    switch (bot->getClass())
    {
        case CLASS_ROGUE:
        case CLASS_WARRIOR:
            return MELEE_ABILITY_REACH;

        case CLASS_SHAMAN:
            return WIND_SHEAR_REACH;

        case CLASS_DEATH_KNIGHT:
        case CLASS_MAGE:
        case CLASS_PALADIN:
        case CLASS_PRIEST:
        case CLASS_WARLOCK:
            return RANGED_ABILITY_REACH;

        case CLASS_HUNTER:
            return SILENCING_SHOT_REACH;

        default:
            return 0.0f;
    }
}

bool IsFlurriedBerserker(Unit* berserker)
{
    return berserker && berserker->HasAura(Id(SwpSpells::SPELL_FLURRY)) &&
        !berserker->HasUnitState(UNIT_STATE_STUNNED);
}

bool IsCastingFelFireball(Unit* furyMage)
{
    return furyMage && furyMage->FindCurrentSpellBySpellId(Id(SwpSpells::SPELL_FEL_FIREBALL));
}

bool IsSpellFuryBuffedFuryMage(Unit* furyMage)
{
    return furyMage && furyMage->HasAura(Id(SwpSpells::SPELL_SPELL_FURY));
}

Unit* SelectNearestQualifying(
    PlayerbotAI* botAI, GuidVector const& candidates, float reach, bool (*qualifies)(Unit*))
{
    Player* bot = botAI->GetBot();
    Unit* currentTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();

    Unit* best = nullptr;
    float bestDistance = std::numeric_limits<float>::max();

    for (ObjectGuid const& guid : candidates)
    {
        Unit* candidate = botAI->GetUnit(guid);
        if (!candidate || !candidate->IsAlive() || !qualifies(candidate))
            continue;

        float const distance = bot->GetExactDist(candidate);
        if (distance > reach)
            continue;

        if (candidate == currentTarget)
            return candidate;

        if (distance < bestDistance)
        {
            best = candidate;
            bestDistance = distance;
        }
    }

    return best;
}

std::mutex muruStateMutex;
std::unordered_map<uint32, MuruEncounterState> muruStates;

MuruDarknessState& GetOrEmplaceDarkness(uint32 instanceId)
{
    std::optional<MuruDarknessState>& darkness = MuruState(instanceId).darkness;
    return darkness ? *darkness : darkness.emplace();
}

} // end anonymous namespace

bool IsMuruPhaseActive(Unit* muru)
{
    // DamageTaken caps M'uru at exactly 1 health when phase 2 activates, and from then on M'uru
    // is invisible but remains on players' threat lists.
    return muru && muru->GetHealth() > 1;
}

bool TryGetMuruDarknessActiveState(Player* bot, Unit* muru)
{
    if (!muru)
        return false;

    uint32 const instanceId = bot->GetInstanceId();
    uint32 const now = getMSTime();

    if (Aura* darkness = muru->GetAura(Id(SwpSpells::SPELL_DARKNESS)))
    {
        int32 const remainingMs = std::max(darkness->GetDuration(), 0);
        int32 const elapsedZoneMs = std::max(darkness->GetMaxDuration() - remainingMs, 0);

        StampMuruDarknessWindow(
            GetOrEmplaceDarkness(instanceId), now,
            MURU_DARKNESS_PRE_EFFECT_MS + static_cast<uint32>(elapsedZoneMs),
            static_cast<uint32>(remainingMs));
    }
    else if (Aura* preEffect = muru->GetAura(Id(SwpSpells::SPELL_DARKNESS_PRE_EFFECT)))
    {
        int32 const duration = preEffect->GetDuration();
        uint32 const remainingPreEffectMs = duration < 0 ?
            MURU_DARKNESS_PRE_EFFECT_MS :
            std::min(static_cast<uint32>(duration), MURU_DARKNESS_PRE_EFFECT_MS);

        StampMuruDarknessWindow(
            GetOrEmplaceDarkness(instanceId), now,
            MURU_DARKNESS_PRE_EFFECT_MS - remainingPreEffectMs,
            remainingPreEffectMs + MURU_DARKNESS_AURA_MS);
    }

    MuruEncounterState* state = FindMuruState(instanceId);
    if (!state || !state->darkness)
        return false;

    uint32 const expireMs = state->darkness->expireMs;
    if (expireMs > now)
        return true;

    if (expireMs <= now)
        state->darkness.reset();

    return false;
}

bool TryGetMuruDarknessEarlyState(Player* bot, Unit* muru, uint32 earlyWindowMs)
{
    if (!TryGetMuruDarknessActiveState(bot, muru))
        return false;

    return PeekMuruDarknessEarlyState(bot, earlyWindowMs);
}

bool PeekMuruDarknessActiveState(Player* bot)
{
    MuruEncounterState const* state = FindMuruState(bot->GetInstanceId());
    return state && state->darkness && state->darkness->expireMs > getMSTime();
}

bool PeekMuruDarknessEarlyState(Player* bot, uint32 earlyWindowMs)
{
    MuruEncounterState const* state = FindMuruState(bot->GetInstanceId());
    if (!state || !state->darkness)
        return false;

    MuruDarknessState const& darkness = *state->darkness;
    uint32 const now = getMSTime();
    if (darkness.expireMs <= now)
        return false;

    return darkness.startMs < now && now - darkness.startMs < earlyWindowMs;
}

MuruEncounterGuids FindMuruEncounterGuids(PlayerbotAI* botAI)
{
    AiObjectContext* context = botAI->GetAiObjectContext();
    auto const& units = context->GetValue<GuidVector>("possible targets no los")->RefGet();

    MuruEncounterGuids guids;
    for (ObjectGuid const& guid : units)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;

        switch (unit->GetEntry())
        {
            case Id(SwpNpcs::NPC_MURU):
                guids.muru = guid;
                break;

            case Id(SwpNpcs::NPC_ENTROPIUS):
                guids.entropius = guid;
                break;

            case Id(SwpNpcs::NPC_VOID_SENTINEL):
                guids.voidSentinels.push_back(guid);
                break;

            case Id(SwpNpcs::NPC_VOID_SPAWN):
                guids.voidSpawns.push_back(guid);
                break;

            case Id(SwpNpcs::NPC_SHADOWSWORD_FURY_MAGE):
                guids.furyMages.push_back(guid);
                break;

            case Id(SwpNpcs::NPC_SHADOWSWORD_BERSERKER):
                guids.berserkers.push_back(guid);
                break;

            default:
                break;
        }
    }

    return guids;
}

void GatherMuruEncounterTargets(PlayerbotAI* botAI, MuruEncounterTargets& targets)
{
    MuruEncounterGuids const& guids = GetCachedMuruEncounterGuids(botAI);

    targets.muru = ResolveLivingUnit(botAI, guids.muru);
    targets.entropius = ResolveLivingUnit(botAI, guids.entropius);
    ResolveLivingUnits(botAI, guids.voidSentinels, targets.voidSentinels);
    ResolveLivingUnits(botAI, guids.voidSpawns, targets.voidSpawns);
    ResolveLivingUnits(botAI, guids.furyMages, targets.furyMages);
    ResolveLivingUnits(botAI, guids.berserkers, targets.berserkers);
}

Unit* SelectNearestMuruTargetByEntry(
    Unit* currentTarget, uint32 entry, std::vector<Unit*> const& candidates, Position const& origin)
{
    Unit* selected = nullptr;
    if (currentTarget && currentTarget->IsAlive() && currentTarget->GetEntry() == entry)
        selected = currentTarget;

    for (Unit* candidate : candidates)
    {
        if (!candidate || selected == candidate)
            continue;

        if (!selected)
        {
            selected = candidate;
            continue;
        }

        if (candidate->GetExactDist2d(origin) + MURU_TARGET_SWITCH_MARGIN <
            selected->GetExactDist2d(origin))
        {
            selected = candidate;
        }
    }

    return selected;
}

bool IsMuruAddInVoidSentinelPulse(Unit* add, std::vector<Unit*> const& voidSentinels)
{
    if (!add)
        return false;

    if (add->GetEntry() != Id(SwpNpcs::NPC_SHADOWSWORD_FURY_MAGE) &&
        add->GetEntry() != Id(SwpNpcs::NPC_SHADOWSWORD_BERSERKER) &&
        add->GetEntry() != Id(SwpNpcs::NPC_VOID_SPAWN))
    {
        return false;
    }

    for (Unit* voidSentinel : voidSentinels)
    {
        if (voidSentinel &&
            add->GetExactDist2d(voidSentinel) < MURU_MELEE_ADD_MIN_DIST_FROM_SENTINEL)
        {
            return true;
        }
    }

    return false;
}

Unit* FindMuruBerserkerToStun(PlayerbotAI* botAI)
{
    float const reach = GetBerserkerStunReach(botAI->GetBot());
    if (reach <= 0.0f)
        return nullptr;

    return SelectNearestQualifying(
        botAI, GetCachedMuruEncounterGuids(botAI).berserkers, reach, &IsFlurriedBerserker);
}

Unit* FindMuruFuryMageToInterrupt(PlayerbotAI* botAI)
{
    float const reach = GetFuryMageInterruptReach(botAI->GetBot());
    if (reach <= 0.0f)
        return nullptr;

    return SelectNearestQualifying(
        botAI, GetCachedMuruEncounterGuids(botAI).furyMages, reach, &IsCastingFelFireball);
}

Unit* FindMuruFuryMageToSpellsteal(PlayerbotAI* botAI)
{
    if (botAI->GetBot()->getClass() != CLASS_MAGE)
        return nullptr;

    return SelectNearestQualifying(
        botAI, GetCachedMuruEncounterGuids(botAI).furyMages, RANGED_ABILITY_REACH,
        &IsSpellFuryBuffedFuryMage);
}

Position const& GetAssignedVoidSentinelTankPosition(Unit* voidSentinel)
{
    ObjectGuid const sentinelGuid = voidSentinel->GetGUID();
    Position const& northPosition = MURU_VOID_SENTINEL_N_TANK_POSITION;
    Position const& eastPosition = MURU_VOID_SENTINEL_E_TANK_POSITION;

    auto& assignments = MuruState(voidSentinel->GetInstanceId()).voidSentinelTankAssignments;
    auto assignmentItr = assignments.find(sentinelGuid);
    if (assignmentItr == assignments.end())
    {
        float const northDistance = voidSentinel->GetExactDist2d(northPosition);
        float const eastDistance = voidSentinel->GetExactDist2d(eastPosition);

        uint8 const assignedIndex = northDistance <= eastDistance ? 0 : 1;
        assignmentItr = assignments.emplace(sentinelGuid, assignedIndex).first;
    }

    return assignmentItr->second == 0 ? northPosition : eastPosition;
}

bool IsTankingMuruVoidSentinel(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    for (ObjectGuid const& guid : GetCachedMuruEncounterGuids(botAI).voidSentinels)
    {
        Unit* voidSentinel = ResolveLivingUnit(botAI, guid);
        if (voidSentinel && voidSentinel->GetVictim() == bot)
            return true;
    }

    return false;
}

ObjectGuid FindMuruSingularityGuid(Player* bot)
{
    Creature* singularity = bot->FindNearestCreature(
        Id(SwpNpcs::NPC_SINGULARITY), SINGULARITY_SEARCH_RADIUS);

    return singularity ? singularity->GetGUID() : ObjectGuid::Empty;
}

GuidVector FindMuruVoidZoneGuids(Player* bot)
{
    std::list<Creature*> voidZones;
    bot->GetCreatureListWithEntryInGrid(
        voidZones, Id(SwpNpcs::NPC_DARKNESS), VOID_ZONE_SEARCH_RADIUS);

    GuidVector guids;
    guids.reserve(voidZones.size());
    for (Creature* voidZone : voidZones)
    {
        if (voidZone && voidZone->IsAlive())
            guids.push_back(voidZone->GetGUID());
    }

    return guids;
}

GuidVector FindMuruDarkFiendGuids(Player* bot)
{
    std::list<Creature*> darkFiends;
    bot->GetCreatureListWithEntryInGrid(
        darkFiends, Id(SwpNpcs::NPC_DARK_FIEND), DARK_FIEND_DISPEL_SEARCH_RADIUS);

    GuidVector guids;
    guids.reserve(darkFiends.size());
    for (Creature* darkFiend : darkFiends)
    {
        if (darkFiend && darkFiend->IsAlive())
            guids.push_back(darkFiend->GetGUID());
    }

    return guids;
}

std::vector<Creature*> GetMuruDarkFiends(PlayerbotAI* botAI)
{
    auto const& guids =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("muru dark fiends")->RefGet();

    std::vector<Creature*> darkFiends;
    darkFiends.reserve(guids.size());
    for (ObjectGuid const& guid : guids)
    {
        Unit* unit = ResolveLivingUnit(botAI, guid);
        if (Creature* darkFiend = unit ? unit->ToCreature() : nullptr)
            darkFiends.push_back(darkFiend);
    }

    return darkFiends;
}

Creature* GetNearestMuruDarkFiend(PlayerbotAI* botAI, float radius)
{
    Player* bot = botAI->GetBot();
    Creature* nearest = nullptr;
    float nearestDistance = radius;

    for (Creature* darkFiend : GetMuruDarkFiends(botAI))
    {
        float const distance = bot->GetDistance(darkFiend);
        if (distance > nearestDistance)
            continue;

        nearest = darkFiend;
        nearestDistance = distance;
    }

    return nearest;
}

Creature* FindMuruVoidZoneToAvoid(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    auto const& guids =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("muru void zones")->RefGet();

    Creature* nearest = nullptr;
    float nearestDistance = VOID_ZONE_SAFE_DISTANCE;

    for (ObjectGuid const& guid : guids)
    {
        Unit* unit = ResolveLivingUnit(botAI, guid);
        if (!unit)
            continue;

        float const distance = bot->GetExactDist2d(unit);
        if (distance >= nearestDistance)
            continue;

        Creature* voidZone = unit->ToCreature();
        if (!voidZone)
            continue;

        nearest = voidZone;
        nearestDistance = distance;
    }

    return nearest;
}

Creature* FindAvailableVoidSpawnForEnslave(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();

    Creature* bestSpawn = nullptr;
    float closestDistance = std::numeric_limits<float>::max();

    for (ObjectGuid const& guid : GetCachedMuruEncounterGuids(botAI).voidSpawns)
    {
        Unit* unit = ResolveLivingUnit(botAI, guid);
        if (!unit || unit->IsCharmed() || unit->GetCharmer())
            continue;

        Creature* creature = unit->ToCreature();
        if (!creature)
            continue;

        float const distance = bot->GetExactDist2d(unit);
        if (distance >= closestDistance)
            continue;

        bestSpawn = creature;
        closestDistance = distance;
    }

    return bestSpawn;
}

bool CommandControlledCreatureToAttack(Unit* controlled, Unit* target)
{
    if (!controlled || !controlled->IsAlive() || !target || controlled->GetVictim() == target)
        return false;

    controlled->ClearUnitState(UNIT_STATE_FOLLOW);
    controlled->AttackStop();
    controlled->SetTarget(target->GetGUID());

    if (CharmInfo* charmInfo = controlled->GetCharmInfo())
    {
        charmInfo->SetIsCommandAttack(true);
        charmInfo->SetIsAtStay(false);
        charmInfo->SetIsFollowing(false);
        charmInfo->SetIsCommandFollow(false);
        charmInfo->SetIsReturning(false);
    }

    if (!controlled->IsPlayer() && controlled->IsCreature() &&
        controlled->ToCreature()->IsAIEnabled)
    {
        controlled->ToCreature()->AI()->AttackStart(target);
    }
    else
    {
        controlled->Attack(target, true);
    }

    return true;
}

// Shared encounter state

MuruEncounterState& MuruState(uint32 instanceId)
{
    std::lock_guard lock(muruStateMutex);
    return muruStates[instanceId];
}

MuruEncounterState* FindMuruState(uint32 instanceId)
{
    std::lock_guard lock(muruStateMutex);
    auto it = muruStates.find(instanceId);
    return it != muruStates.end() ? &it->second : nullptr;
}

bool MuruResetInstance(uint32 instanceId)
{
    std::lock_guard lock(muruStateMutex);
    auto it = muruStates.find(instanceId);
    if (it == muruStates.end())
        return false;

    MuruEncounterState const& state = it->second;
    bool const wasSet = state.darkness || !state.voidSentinelTankAssignments.empty();

    muruStates.erase(it);
    return wasSet;
}

}
