/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TKTRIGGERS_H
#define PLAYERBOTS_TKTRIGGERS_H

#include "EncounterHelpers.h"
#include "TKHelpers.h"
#include "Trigger.h"
#include <string>

// Shared

class TempestKeepEncounterTrigger : public Trigger
{
public:
    TempestKeepEncounterTrigger(PlayerbotAI* botAI, std::string const name, int32 checkInterval = 1)
        : Trigger(botAI, name, checkInterval) {}

    bool IsActive() final
    {
        return EncounterHelpers::IsEncounterInProgress(bot, TkHelpers::TK_MAP_ID) &&
            IsActiveInEncounter();
    }

protected:
    virtual bool IsActiveInEncounter() = 0;
};

class TempestKeepNoEncounterInProgressTrigger : public Trigger
{
public:
    TempestKeepNoEncounterInProgressTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "tempest keep no encounter in progress", 1000) {}
    bool IsActive() override;
};

class TempestKeepStuckFallingTrigger : public Trigger
{
public:
    // This is to address a fringe wipe scenario during Kael'thas.
    TempestKeepStuckFallingTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "tempest keep stuck falling", 1000) {}
    bool IsActive() override;
};

// Trash

class CrimsonHandCenturionCastsArcaneFlurryTrigger : public Trigger
{
public:
    CrimsonHandCenturionCastsArcaneFlurryTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "crimson hand centurion casts arcane flurry") {}
    bool IsActive() override;
};

// Al'ar <Phoenix God>

class AlarHunterShouldMisdirectTrigger : public TempestKeepEncounterTrigger
{
public:
    AlarHunterShouldMisdirectTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(botAI, "al'ar hunter should misdirect") {}

protected:
    bool IsActiveInEncounter() override;
};

class AlarFliesBetweenPlatformsTrigger : public TempestKeepEncounterTrigger
{
public:
    AlarFliesBetweenPlatformsTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(botAI, "al'ar flies between platforms") {}

protected:
    bool IsActiveInEncounter() override;
};

class AlarEmbersShouldBeTankedTrigger : public TempestKeepEncounterTrigger
{
public:
    AlarEmbersShouldBeTankedTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(botAI, "al'ar embers should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class AlarShouldAssignNonTankTargetTrigger : public TempestKeepEncounterTrigger
{
public:
    AlarShouldAssignNonTankTargetTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(botAI, "al'ar should assign non-tank target") {}

protected:
    bool IsActiveInEncounter() override;
};

class AlarIncomingFlameQuillsTrigger : public TempestKeepEncounterTrigger
{
public:
    AlarIncomingFlameQuillsTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(botAI, "al'ar incoming flame quills") {}

protected:
    bool IsActiveInEncounter() override;
};

class AlarRisingFromTheAshesTrigger : public TempestKeepEncounterTrigger
{
public:
    AlarRisingFromTheAshesTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(botAI, "al'ar rising from the ashes") {}

protected:
    bool IsActiveInEncounter() override;
};

class AlarInPhase2Trigger : public TempestKeepEncounterTrigger
{
public:
    AlarInPhase2Trigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(botAI, "al'ar in phase 2") {}

protected:
    bool IsActiveInEncounter() override;
};

class AlarShouldManagePhaseTrackerTrigger : public TempestKeepEncounterTrigger
{
public:
    AlarShouldManagePhaseTrackerTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(botAI, "al'ar should manage phase tracker") {}

protected:
    bool IsActiveInEncounter() override;
};

// Void Reaver

class VoidReaverShouldBeTankedTrigger : public TempestKeepEncounterTrigger
{
public:
    VoidReaverShouldBeTankedTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(botAI, "void reaver should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class VoidReaverKnockAwayPullsAggroToNonTanksTrigger : public TempestKeepEncounterTrigger
{
public:
    VoidReaverKnockAwayPullsAggroToNonTanksTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(botAI, "void reaver knock away pulls aggro to non-tanks") {}

protected:
    bool IsActiveInEncounter() override;
};

class VoidReaverRangedShouldStandBackTrigger : public TempestKeepEncounterTrigger
{
public:
    VoidReaverRangedShouldStandBackTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(botAI, "void reaver ranged should stand back") {}

protected:
    bool IsActiveInEncounter() override;
};

class VoidReaverArcaneOrbIsIncomingTrigger : public TempestKeepEncounterTrigger
{
public:
    VoidReaverArcaneOrbIsIncomingTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(botAI, "void reaver arcane orb is incoming") {}

protected:
    bool IsActiveInEncounter() override;
};

// High Astromancer Solarian

class HighAstromancerSolarianShouldBeTankedTrigger : public TempestKeepEncounterTrigger
{
public:
    HighAstromancerSolarianShouldBeTankedTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(botAI, "high astromancer solarian should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class HighAstromancerSolarianWrathOfTheAstromancerTrigger : public TempestKeepEncounterTrigger
{
public:
    HighAstromancerSolarianWrathOfTheAstromancerTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "high astromancer solarian wrath of the astromancer") {}

protected:
    bool IsActiveInEncounter() override;
};

class HighAstromancerSolarianSolariumPriestsSpawnedTrigger : public TempestKeepEncounterTrigger
{
public:
    HighAstromancerSolarianSolariumPriestsSpawnedTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "high astromancer solarian solarium priests spawned") {}

protected:
    bool IsActiveInEncounter() override;
};

// Kael'thas Sunstrider <Lord of the Blood Elves>

class KaelthasSunstriderChasedByThaladredTrigger : public TempestKeepEncounterTrigger
{
public:
    KaelthasSunstriderChasedByThaladredTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(botAI, "kael'thas sunstrider chased by thaladred") {}

protected:
    bool IsActiveInEncounter() override;
};

class KaelthasSunstriderHunterShouldMisdirectAdvisorsTrigger : public TempestKeepEncounterTrigger
{
public:
    KaelthasSunstriderHunterShouldMisdirectAdvisorsTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "kael'thas sunstrider hunter should misdirect advisors") {}

protected:
    bool IsActiveInEncounter() override;
};

class KaelthasSunstriderSanguinarOrTelonicusShouldBeTankedTrigger
    : public TempestKeepEncounterTrigger
{
public:
    KaelthasSunstriderSanguinarOrTelonicusShouldBeTankedTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "kael'thas sunstrider sanguinar or telonicus should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class KaelthasSunstriderCapernianShouldBeTankedByWarlockTrigger : public TempestKeepEncounterTrigger
{
public:
    KaelthasSunstriderCapernianShouldBeTankedByWarlockTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "kael'thas sunstrider capernian should be tanked by warlock") {}

protected:
    bool IsActiveInEncounter() override;
};

class KaelthasSunstriderShouldStandBackFromCapernianTrigger : public TempestKeepEncounterTrigger
{
public:
    KaelthasSunstriderShouldStandBackFromCapernianTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "kael'thas sunstrider should stand back from capernian") {}

protected:
    bool IsActiveInEncounter() override;
};

class KaelthasSunstriderShouldHoldPhase3PositionsTrigger : public TempestKeepEncounterTrigger
{
public:
    KaelthasSunstriderShouldHoldPhase3PositionsTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "kael'thas sunstrider should hold phase 3 positions") {}

protected:
    bool IsActiveInEncounter() override;
};

class KaelthasSunstriderShouldAssignAdvisorDpsPriorityTrigger : public TempestKeepEncounterTrigger
{
public:
    KaelthasSunstriderShouldAssignAdvisorDpsPriorityTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "kael'thas sunstrider should assign advisor dps priority") {}

protected:
    bool IsActiveInEncounter() override;
};

class KaelthasSunstriderShouldManageAdvisorDpsTimerTrigger : public TempestKeepEncounterTrigger
{
public:
    KaelthasSunstriderShouldManageAdvisorDpsTimerTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "kael'thas sunstrider should manage advisor dps timer") {}

protected:
    bool IsActiveInEncounter() override;
};

class KaelthasSunstriderLegendaryWeaponsAreAliveTrigger : public TempestKeepEncounterTrigger
{
public:
    KaelthasSunstriderLegendaryWeaponsAreAliveTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(botAI, "kael'thas sunstrider legendary weapons are alive") {}

protected:
    bool IsActiveInEncounter() override;
};

class KaelthasSunstriderLegendaryAxeCastsWhirlwindTrigger : public TempestKeepEncounterTrigger
{
public:
    KaelthasSunstriderLegendaryAxeCastsWhirlwindTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "kael'thas sunstrider legendary axe casts whirlwind") {}

protected:
    bool IsActiveInEncounter() override;
};

class KaelthasSunstriderLegendaryWeaponsAreDeadTrigger : public TempestKeepEncounterTrigger
{
public:
    KaelthasSunstriderLegendaryWeaponsAreDeadTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(botAI, "kael'thas sunstrider legendary weapons are dead") {}

protected:
    bool IsActiveInEncounter() override;
};

class KaelthasSunstriderLegendaryWeaponsAreEquippedTrigger : public TempestKeepEncounterTrigger
{
public:
    KaelthasSunstriderLegendaryWeaponsAreEquippedTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "kael'thas sunstrider legendary weapons are equipped") {}

protected:
    bool IsActiveInEncounter() override;
};

class KaelthasSunstriderLegendaryWeaponsWereLostTrigger : public Trigger
{
public:
    KaelthasSunstriderLegendaryWeaponsWereLostTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "kael'thas sunstrider legendary weapons were lost") {}
    bool IsActive() override;
};

class KaelthasSunstriderHasEnteredTheFightTrigger : public TempestKeepEncounterTrigger
{
public:
    KaelthasSunstriderHasEnteredTheFightTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(botAI, "kael'thas sunstrider has entered the fight") {}

protected:
    bool IsActiveInEncounter() override;
};

class KaelthasSunstriderShouldAssignFinalPhaseTargetTrigger : public TempestKeepEncounterTrigger
{
public:
    KaelthasSunstriderShouldAssignFinalPhaseTargetTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
              botAI, "kael'thas sunstrider should assign final phase target") {}

protected:
    bool IsActiveInEncounter() override;
};

class KaelthasSunstriderRaidMemberIsMindControlledTrigger : public TempestKeepEncounterTrigger
{
public:
    KaelthasSunstriderRaidMemberIsMindControlledTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(
            botAI, "kael'thas sunstrider raid member is mind controlled") {}

protected:
    bool IsActiveInEncounter() override;
};

class KaelthasSunstriderInGravityLapsePhaseTrigger : public TempestKeepEncounterTrigger
{
public:
    KaelthasSunstriderInGravityLapsePhaseTrigger(PlayerbotAI* botAI)
        : TempestKeepEncounterTrigger(botAI, "kael'thas sunstrider in gravity lapse phase") {}

protected:
    bool IsActiveInEncounter() override;
};

#endif
