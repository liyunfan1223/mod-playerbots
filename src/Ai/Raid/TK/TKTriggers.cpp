/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TKTriggers.h"
#include "EncounterHelpers.h"
#include "MoveSpline.h"
#include "Playerbots.h"
#include "TKHelpers.h"
#include <array>

using namespace TkHelpers;
using namespace EncounterHelpers;

// General

bool TempestKeepNoEncounterInProgressTrigger::IsActive()
{
    if (IsEncounterInProgress(bot, TK_MAP_ID))
        return false;

    return IsMechanicTrackerBot(bot, TK_MAP_ID);
}

bool TempestKeepStuckFallingTrigger::IsActive()
{
    if (!bot->HasUnitMovementFlag(MOVEMENTFLAG_FALLING) || !bot->movespline->Finalized())
        return false;

    if (bot->GetMapId() != TK_MAP_ID)
        return false;

    return !IsEncounterInProgress(bot, TK_MAP_ID);
}

// Trash

bool CrimsonHandCenturionCastsArcaneFlurryTrigger::IsActive()
{
    return bot->getClass() == CLASS_MAGE && GetCenturionCastingArcaneFlurry(botAI);
}

// Al'ar <Phoenix God>

bool AlarPullingBossTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    return alar && alar->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT;
}

bool AlarFliesBetweenPlatformsTrigger::IsActiveInEncounter()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar || IsAlarInPhase2(alar->GetInstanceId()))
        return false;

    int8 locationIndex = GetAlarCurrentLocationIndex(alar);
    if (locationIndex == LOCATION_NONE)
        locationIndex = GetAlarDestinationLocationIndex(alar);

    return locationIndex != POINT_QUILL_OR_DIVE_IDX && locationIndex != POINT_MIDDLE_IDX;
}

bool AlarEmbersExplodeUponDeathTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "ember of al'ar");
}

bool AlarShouldAssignNonTankTargetTrigger::IsActiveInEncounter()
{
    return !PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "al'ar");
}

bool AlarIncomingFlameQuillsTrigger::IsActiveInEncounter()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar || IsAlarInPhase2(alar->GetInstanceId()))
        return false;

    return GetAlarCurrentLocationIndex(alar) == POINT_QUILL_OR_DIVE_IDX ||
        GetAlarDestinationLocationIndex(alar) == POINT_QUILL_OR_DIVE_IDX;
}

bool AlarRisingFromTheAshesTrigger::IsActiveInEncounter()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar || alar->GetHealthPct() > 5.0f)
        return false;

    if (IsAlarInPhase2(alar->GetInstanceId()))
        return false;

    return GetAlarCurrentLocationIndex(alar) != POINT_QUILL_OR_DIVE_IDX &&
        GetAlarDestinationLocationIndex(alar) != POINT_QUILL_OR_DIVE_IDX;
}

bool AlarInPhase2Trigger::IsActiveInEncounter()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    return alar && IsAlarInPhase2(alar->GetInstanceId());
}

bool AlarShouldManagePhaseTrackerTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, TK_MAP_ID) && AI_VALUE2(Unit*, "find target", "al'ar");
}

// Void Reaver

bool VoidReaverShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "void reaver");
}

bool VoidReaverKnockAwayPullsAggroToNonTanksTrigger::IsActiveInEncounter()
{
    if (bot->getClass() == CLASS_DEATH_KNIGHT || bot->getClass() == CLASS_DRUID ||
        bot->getClass() == CLASS_SHAMAN || bot->getClass() == CLASS_WARRIOR)
    {
        return false;
    }

    if (PlayerbotAI::IsTank(bot))
        return false;

    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    return voidReaver && voidReaver->GetVictim() == bot;
}

bool VoidReaverRangedShouldStandBackTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!voidReaver || voidReaver->GetVictim() == bot)
        return false;

    return !IsNearActiveArcaneOrb(bot, ARCANE_ORB_BUFFER_DISTANCE);
}

bool VoidReaverArcaneOrbIsIncomingTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!voidReaver || voidReaver->GetVictim() == bot)
        return false;

    return IsNearActiveArcaneOrb(bot, ARCANE_ORB_SAFE_DISTANCE);
}

// High Astromancer Solarian

bool HighAstromancerSolarianShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMainTank(bot))
        return false;

    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!astromancer)
        return false;

    Creature* astromancerCreature = astromancer->ToCreature();
    return astromancerCreature && astromancerCreature->GetReactState() != REACT_PASSIVE;
}

bool HighAstromancerSolarianWrathOfTheAstromancerTrigger::IsActiveInEncounter()
{
    return HasWrathOfTheAstromancer(bot);
}

bool HighAstromancerSolarianSolariumPriestsSpawnedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot) || PlayerbotAI::IsMainTank(bot))
        return false;

    return AI_VALUE2(Unit*, "find target", "solarium priest");
}

// Kael'thas Sunstrider <Lord of the Blood Elves>

bool KaelthasSunstriderChasedByThaladredTrigger::IsActiveInEncounter()
{
    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");
    if (!thaladred || thaladred->GetVictim() != bot)
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    uint32 const phase = GetKaelthasTkPhase(kaelthas);
    if (PlayerbotAI::IsTank(bot) && phase == PHASE_ALL_ADVISORS)
        return false;

    return phase != PHASE_NONE;
}

bool KaelthasSunstriderPullingTankableAdvisorsTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    uint32 const phase = GetKaelthasTkPhase(kaelthas);
    return phase == PHASE_SINGLE_ADVISOR || phase == PHASE_ALL_ADVISORS;
}

bool KaelthasSunstriderSanguinarOrTelonicusShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (PlayerbotAI::IsMainTank(bot))
        return IsAdvisorActive(AI_VALUE2(Unit*, "find target", "lord sanguinar"));

    if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
        return IsAdvisorActive(AI_VALUE2(Unit*, "find target", "master engineer telonicus"));

    return false;
}

bool KaelthasSunstriderCapernianShouldBeTankedByWarlockTrigger::IsActiveInEncounter()
{
    if (!IsCapernianTank(bot))
        return false;

    return IsAdvisorActive(AI_VALUE2(Unit*, "find target", "grand astromancer capernian"));
}

bool KaelthasSunstriderShouldStandBackFromCapernianTrigger::IsActiveInEncounter()
{
    if (!IsAdvisorActive(AI_VALUE2(Unit*, "find target", "grand astromancer capernian")))
        return false;

    return !IsCapernianTank(bot);
}

bool KaelthasSunstriderShouldHoldPhase3PositionsTrigger::IsActiveInEncounter()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    if (GetKaelthasTkPhase(kaelthas) != PHASE_ALL_ADVISORS)
        return false;

    Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");
    // The designated healer stays in position by the melee tanks while the rest of the raid runs
    // all over the place to kite and kill Thaladred.
    if (PlayerbotAI::IsAssistHealOfIndex(bot, 0, true))
        return sanguinar && sanguinar->IsAlive();

    // The Sanguinar check is a proxy for the revival/Kael talk phase (any non-selectable advisor
    // would do, since all four revive together, but Sanguinar is already needed for the healer).
    if (!sanguinar || !sanguinar->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        return false;

    return PlayerbotAI::IsMainTank(bot) || PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) ||
        IsCapernianTank(bot);
}

bool KaelthasSunstriderDeterminingAdvisorKillOrderTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsMainTank(bot) || PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    uint32 const phase = GetKaelthasTkPhase(kaelthas);
    return phase == PHASE_SINGLE_ADVISOR || phase == PHASE_ALL_ADVISORS;
}

bool KaelthasSunstriderShouldManageAdvisorDpsTimerTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, TK_MAP_ID))
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    return GetKaelthasTkPhase(kaelthas) == PHASE_SINGLE_ADVISOR;
}

bool KaelthasSunstriderLegendaryWeaponsAreAliveTrigger::IsActiveInEncounter()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    if (GetKaelthasTkPhase(kaelthas) != PHASE_WEAPONS)
        return false;

    return !PlayerbotAI::IsMainTank(bot);
}

bool KaelthasSunstriderLegendaryAxeCastsWhirlwindTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMainTank(bot))
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    uint32 const phase = GetKaelthasTkPhase(kaelthas);
    if (phase < PHASE_WEAPONS || phase > PHASE_ALL_ADVISORS)
        return false;

    return GetLegendaryWeapon(botAI, Id(TkNpcs::NPC_DEVASTATION)) != nullptr;
}

bool KaelthasSunstriderLegendaryWeaponsAreDeadTrigger::IsActiveInEncounter()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    uint32 const phase = GetKaelthasTkPhase(kaelthas);
    if (phase < PHASE_WEAPONS || phase > PHASE_ALL_ADVISORS)
        return false;

    Unit* axe = GetLegendaryWeapon(botAI, Id(TkNpcs::NPC_DEVASTATION));
    if (axe && axe->GetVictim() == bot)
        return false;

    return HasDeadLegendaryWeapon(botAI);
}

bool KaelthasSunstriderLegendaryWeaponsAreEquippedTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsHeal(bot))
        return false;

    if (PlayerbotAI::IsMelee(bot) && PlayerbotAI::IsDps(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "kael'thas sunstrider"))
        return false;

    return GetEquippedItemInSlot(
               bot, EQUIPMENT_SLOT_MAINHAND, Id(TkItems::ITEM_STAFF_OF_DISINTEGRATION)) ||
        GetEquippedItemInSlot(
               bot, EQUIPMENT_SLOT_RANGED, Id(TkItems::ITEM_NETHERSTRAND_LONGBOW)) ||
        GetEquippedItemInSlot(
               bot, EQUIPMENT_SLOT_OFFHAND, Id(TkItems::ITEM_PHASESHIFT_BULWARK));
}

bool KaelthasSunstriderLegendaryWeaponsWereLostTrigger::IsActive()
{
    if (bot->GetMapId() != TK_MAP_ID)
        return false;

    if (IsEncounterInProgress(bot, TK_MAP_ID))
        return false;

    if (AI_VALUE2(bool, "combat", "self target"))
        return false;

    auto const& creatureStore = bot->GetMap()->GetCreatureBySpawnIdStore();
    auto it = creatureStore.find(KAELTHAS_DB_GUID);
    if (it == creatureStore.end())
        return false;

    Creature* kaelthas = it->second;
    if (!kaelthas || bot->GetExactDist2d(kaelthas) > KAELTHAS_ROOM_SEARCH_DISTANCE)
        return false;

    static constexpr std::array weaponSlots = {
        EQUIPMENT_SLOT_MAINHAND, EQUIPMENT_SLOT_OFFHAND, EQUIPMENT_SLOT_RANGED, };

    for (uint8 slot : weaponSlots)
    {
        if (!bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot) && HasEquippableItemForSlot(bot, slot))
            return true;
    }

    return false;
}

bool KaelthasSunstriderHasEnteredTheFightTrigger::IsActiveInEncounter()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    return GetKaelthasTkPhase(kaelthas) == PHASE_FINAL;
}

bool KaelthasSunstriderShouldAssignFinalPhaseTargetTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsMainTank(bot))
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas || kaelthas->GetVictim() == bot)
        return false;

    return GetKaelthasTkPhase(kaelthas) == PHASE_FINAL;
}

bool KaelthasSunstriderRaidMemberIsMindControlledTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsCaster(bot))
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    if (PlayerbotAI::IsTank(bot) && kaelthas->GetVictim() == bot)
        return false;

    if (!bot->HasItemCount(Id(TkItems::ITEM_INFINITY_BLADE), 1, true))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->HasAura(Id(TkSpells::SPELL_KAELTHAS_MIND_CONTROL)))
            return true;
    }

    return false;
}

bool KaelthasSunstriderInGravityLapsePhaseTrigger::IsActiveInEncounter()
{
    constexpr float gravityLapseHpThreshold = 50.0f;
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    return kaelthas && kaelthas->GetHealthPct() <= gravityLapseHpThreshold;
}
