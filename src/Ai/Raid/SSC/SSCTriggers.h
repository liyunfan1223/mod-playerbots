/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SSCTRIGGERS_H
#define PLAYERBOTS_SSCTRIGGERS_H

#include "EncounterHelpers.h"
#include "SSCHelpers.h"
#include "Trigger.h"
#include <string>

// Shared

class SscEncounterTrigger : public Trigger
{
public:
    SscEncounterTrigger(PlayerbotAI* botAI, std::string const name, int32 checkInterval = 1)
        : Trigger(botAI, name, checkInterval) {}

    bool IsActive() final
    {
        return EncounterHelpers::IsEncounterInProgress(bot, SscHelpers::SSC_MAP_ID) &&
            IsActiveInEncounter();
    }

protected:
    virtual bool IsActiveInEncounter() = 0;
};

class SscNoEncounterInProgressTrigger : public Trigger
{
public:
    SscNoEncounterInProgressTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "ssc no encounter in progress", 1000) {}
    bool IsActive() override;
};

// Used for Fathom-Lord Karathress, Morogrim Tidewalker, and Lady Vashj.
class SscHunterShouldMisdirectTrigger : public SscEncounterTrigger
{
public:
    SscHunterShouldMisdirectTrigger(
        PlayerbotAI* botAI, std::string const& name, std::string const& bossName)
        : SscEncounterTrigger(botAI, name), _bossName(bossName) {}

protected:
    bool IsActiveInEncounter() override;

private:
    std::string const _bossName;
};

// Trash

class UnderbogColossusInToxicPoolTrigger : public Trigger
{
public:
    UnderbogColossusInToxicPoolTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "underbog colossus in toxic pool") {}
    bool IsActive() override;
};

class GreyheartTidecallerWaterElementalTotemSpawnedTrigger : public Trigger
{
public:
    GreyheartTidecallerWaterElementalTotemSpawnedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "greyheart tidecaller water elemental totem spawned") {}
    bool IsActive() override;
};

// Hydross the Unstable <Duke of Currents>

class HydrossTheUnstableShouldBeTankedByFrostTankTrigger : public SscEncounterTrigger
{
public:
    HydrossTheUnstableShouldBeTankedByFrostTankTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "hydross the unstable should be tanked by frost tank") {}

protected:
    bool IsActiveInEncounter() override;
};

class HydrossTheUnstableShouldBeTankedByNatureTankTrigger : public SscEncounterTrigger
{
public:
    HydrossTheUnstableShouldBeTankedByNatureTankTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "hydross the unstable should be tanked by nature tank") {}

protected:
    bool IsActiveInEncounter() override;
};

class HydrossTheUnstableRangedShouldSpreadInFrostPhaseTrigger : public SscEncounterTrigger
{
public:
    HydrossTheUnstableRangedShouldSpreadInFrostPhaseTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "hydross the unstable ranged should spread in frost phase") {}

protected:
    bool IsActiveInEncounter() override;
};

class HydrossTheUnstableShouldMisdirectUponPhaseChangeTrigger : public SscEncounterTrigger
{
public:
    HydrossTheUnstableShouldMisdirectUponPhaseChangeTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "hydross the unstable should misdirect upon phase change") {}

protected:
    bool IsActiveInEncounter() override;
};

class HydrossTheUnstableAggroResetsUponPhaseChangeTrigger : public SscEncounterTrigger
{
public:
    HydrossTheUnstableAggroResetsUponPhaseChangeTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "hydross the unstable aggro resets upon phase change") {}

protected:
    bool IsActiveInEncounter() override;
};

class HydrossTheUnstableOffPhaseTankAttackingTrigger : public SscEncounterTrigger
{
public:
    HydrossTheUnstableOffPhaseTankAttackingTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "hydross the unstable off phase tank attacking") {}

protected:
    bool IsActiveInEncounter() override;
};

class HydrossTheUnstableShouldManagePhaseTimersTrigger : public SscEncounterTrigger
{
public:
    HydrossTheUnstableShouldManagePhaseTimersTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "hydross the unstable should manage phase timers") {}

protected:
    bool IsActiveInEncounter() override;
};

// The Lurker Below

class TheLurkerBelowSpoutIsActiveTrigger : public SscEncounterTrigger
{
public:
    TheLurkerBelowSpoutIsActiveTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "the lurker below spout is active") {}

protected:
    bool IsActiveInEncounter() override;
};

class TheLurkerBelowShouldBeTankedTrigger : public SscEncounterTrigger
{
public:
    TheLurkerBelowShouldBeTankedTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "the lurker below should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class TheLurkerBelowRangedShouldSpreadTrigger : public SscEncounterTrigger
{
public:
    TheLurkerBelowRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "the lurker below ranged should spread") {}

protected:
    bool IsActiveInEncounter() override;
};

class TheLurkerBelowGuardiansShouldBeTankedTrigger : public SscEncounterTrigger
{
public:
    TheLurkerBelowGuardiansShouldBeTankedTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "the lurker below guardians should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class TheLurkerBelowMeleeCannotReachTargetTrigger : public SscEncounterTrigger
{
public:
    TheLurkerBelowMeleeCannotReachTargetTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "the lurker below melee cannot reach target") {}

protected:
    bool IsActiveInEncounter() override;
};

class TheLurkerBelowMeleeInWaterTrigger : public SscEncounterTrigger
{
public:
    TheLurkerBelowMeleeInWaterTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "the lurker below melee in water") {}

protected:
    bool IsActiveInEncounter() override;
};

// Leotheras the Blind

// Not encounter gated because the encounter does not start during the Spellbinder phase unless he
// is hit by something (it's likely he'll get hit by an AoE spell in practice).
class LeotherasTheBlindRangedShouldSpreadUponPullTrigger : public Trigger
{
public:
    LeotherasTheBlindRangedShouldSpreadUponPullTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "leotheras the blind ranged should spread upon pull", 1000) {}
    bool IsActive() override;
};

class LeotherasTheBlindWarlockShouldTankDemonFormTrigger : public SscEncounterTrigger
{
public:
    LeotherasTheBlindWarlockShouldTankDemonFormTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "leotheras the blind warlock should tank demon form") {}

protected:
    bool IsActiveInEncounter() override;
};

class LeotherasTheBlindTanksShouldAutoAttackDemonFormTrigger : public SscEncounterTrigger
{
public:
    LeotherasTheBlindTanksShouldAutoAttackDemonFormTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "leotheras the blind tanks should auto-attack demon form") {}

protected:
    bool IsActiveInEncounter() override;
};

class LeotherasTheBlindRangedShouldKeepDistanceTrigger : public SscEncounterTrigger
{
public:
    LeotherasTheBlindRangedShouldKeepDistanceTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "leotheras the blind ranged should keep distance") {}

protected:
    bool IsActiveInEncounter() override;
};

class LeotherasTheBlindChannelingWhirlwindTrigger : public SscEncounterTrigger
{
public:
    LeotherasTheBlindChannelingWhirlwindTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "leotheras the blind channeling whirlwind") {}

protected:
    bool IsActiveInEncounter() override;
};

class LeotherasTheBlindTooManyChaosBlastStacksTrigger : public SscEncounterTrigger
{
public:
    LeotherasTheBlindTooManyChaosBlastStacksTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "leotheras the blind too many chaos blast stacks") {}

protected:
    bool IsActiveInEncounter() override;
};

class LeotherasTheBlindInnerDemonHasAwakenedTrigger : public SscEncounterTrigger
{
public:
    LeotherasTheBlindInnerDemonHasAwakenedTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "leotheras the blind inner demon has awakened") {}

protected:
    bool IsActiveInEncounter() override;
};

class LeotherasTheBlindInFinalPhaseTrigger : public SscEncounterTrigger
{
public:
    LeotherasTheBlindInFinalPhaseTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "leotheras the blind in final phase") {}

protected:
    bool IsActiveInEncounter() override;
};

class LeotherasTheBlindShouldSeparateBossFromDemonTrigger : public SscEncounterTrigger
{
public:
    LeotherasTheBlindShouldSeparateBossFromDemonTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "leotheras the blind should separate boss from demon") {}

protected:
    bool IsActiveInEncounter() override;
};

class LeotherasTheBlindHunterShouldMisdirectDemonFormTrigger : public SscEncounterTrigger
{
public:
    LeotherasTheBlindHunterShouldMisdirectDemonFormTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "leotheras the blind hunter should misdirect demon form") {}

protected:
    bool IsActiveInEncounter() override;
};

class LeotherasTheBlindAggroResetsTrigger : public SscEncounterTrigger
{
public:
    LeotherasTheBlindAggroResetsTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "leotheras the blind aggro resets") {}

protected:
    bool IsActiveInEncounter() override;
};

class LeotherasTheBlindShouldManageDpsWaitTimersTrigger : public SscEncounterTrigger
{
public:
    LeotherasTheBlindShouldManageDpsWaitTimersTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "leotheras the blind should manage dps wait timers") {}

protected:
    bool IsActiveInEncounter() override;
};

// Fathom-Lord Karathress

class FathomLordKarathressTargetsShouldBeTankedTrigger : public SscEncounterTrigger
{
public:
    FathomLordKarathressTargetsShouldBeTankedTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "fathom-lord karathress targets should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class FathomLordKarathressShouldHealCaribdisTankTrigger : public SscEncounterTrigger
{
public:
    FathomLordKarathressShouldHealCaribdisTankTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(
            botAI, "fathom-lord karathress should heal caribdis tank") {}

protected:
    bool IsActiveInEncounter() override;
};

class FathomLordKarathressShouldAssignDpsPriorityTrigger : public SscEncounterTrigger
{
public:
    FathomLordKarathressShouldAssignDpsPriorityTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "fathom-lord karathress should assign dps priority") {}

protected:
    bool IsActiveInEncounter() override;
};

class FathomLordKarathressShouldManageDpsTimerTrigger : public SscEncounterTrigger
{
public:
    FathomLordKarathressShouldManageDpsTimerTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "fathom-lord karathress should manage dps timer") {}

protected:
    bool IsActiveInEncounter() override;
};

class FathomLordKarathressRangedShouldSpreadTrigger : public SscEncounterTrigger
{
public:
    FathomLordKarathressRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "fathom-lord karathress ranged should spread") {}

protected:
    bool IsActiveInEncounter() override;
};

class FathomLordKarathressStuckMidairAfterCycloneTrigger : public SscEncounterTrigger
{
public:
    FathomLordKarathressStuckMidairAfterCycloneTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "fathom-lord karathress stuck midair after cyclone", 1000) {}

protected:
    bool IsActiveInEncounter() override;
};

// Morogrim Tidewalker

class MorogrimTidewalkerShouldBeTankedTrigger : public SscEncounterTrigger
{
public:
    MorogrimTidewalkerShouldBeTankedTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "morogrim tidewalker should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class MorogrimTidewalkerRangedShouldStackTrigger : public SscEncounterTrigger
{
public:
    MorogrimTidewalkerRangedShouldStackTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "morogrim tidewalker ranged should stack") {}

protected:
    bool IsActiveInEncounter() override;
};

class MorogrimTidewalkerTooFarFromBossTrigger : public SscEncounterTrigger
{
public:
    MorogrimTidewalkerTooFarFromBossTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "morogrim tidewalker too far from boss") {}

protected:
    bool IsActiveInEncounter() override;
};

// Lady Vashj <Coilfang Matron>

class LadyVashjShouldBeTankedTrigger : public SscEncounterTrigger
{
public:
    LadyVashjShouldBeTankedTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjRangedShouldSpreadInPhase1Trigger : public SscEncounterTrigger
{
public:
    LadyVashjRangedShouldSpreadInPhase1Trigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj ranged should spread in phase 1") {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjStationSlotsNeedHoldersTrigger : public SscEncounterTrigger
{
public:
    LadyVashjStationSlotsNeedHoldersTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj station slots need holders", 1000) {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjShouldHoldStationInPhase2Trigger : public SscEncounterTrigger
{
public:
    LadyVashjShouldHoldStationInPhase2Trigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj should hold station in phase 2") {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjRangedShouldPositionInPhase3Trigger : public SscEncounterTrigger
{
public:
    LadyVashjRangedShouldPositionInPhase3Trigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj ranged should position in phase 3") {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjOutOfSightInPhase3Trigger : public SscEncounterTrigger
{
public:
    LadyVashjOutOfSightInPhase3Trigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj out of sight in phase 3") {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjMainTankNeedsGroundingShamanTrigger : public SscEncounterTrigger
{
public:
    LadyVashjMainTankNeedsGroundingShamanTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj main tank needs grounding shaman", 1000) {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjShamanShouldGroundShockBlastTrigger : public SscEncounterTrigger
{
public:
    LadyVashjShamanShouldGroundShockBlastTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj shaman should ground shock blast") {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjStaticChargeOnGroupMemberTrigger : public SscEncounterTrigger
{
public:
    LadyVashjStaticChargeOnGroupMemberTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj static charge on group member") {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjShouldAssignTargetPriorityTrigger : public SscEncounterTrigger
{
public:
    LadyVashjShouldAssignTargetPriorityTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj should assign target priority") {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjTankNeedsFearWardTrigger : public SscEncounterTrigger
{
public:
    LadyVashjTankNeedsFearWardTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj tank needs fear ward") {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjCoilfangStriderShouldBeTankedTrigger : public SscEncounterTrigger
{
public:
    LadyVashjCoilfangStriderShouldBeTankedTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj coilfang strider should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjCoilfangEliteShouldBeTankedTrigger : public SscEncounterTrigger
{
public:
    LadyVashjCoilfangEliteShouldBeTankedTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj coilfang elite should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjTankIsIdleAwayFromTheMiddleTrigger : public SscEncounterTrigger
{
public:
    LadyVashjTankIsIdleAwayFromTheMiddleTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj tank is idle away from the middle") {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjTaintedElementalNeedsLooterTrigger : public SscEncounterTrigger
{
public:
    LadyVashjTaintedElementalNeedsLooterTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj tainted elemental needs looter") {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjShouldAttackTaintedElementalTrigger : public SscEncounterTrigger
{
public:
    LadyVashjShouldAttackTaintedElementalTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj should attack tainted elemental") {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjTaintedCoreLooterTrigger : public SscEncounterTrigger
{
public:
    LadyVashjTaintedCoreLooterTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj tainted core looter") {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjCorePassingChainMemberTrigger : public SscEncounterTrigger
{
public:
    LadyVashjCorePassingChainMemberTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj core passing chain member") {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjShouldDestroyTaintedCoreTrigger : public SscEncounterTrigger
{
public:
    LadyVashjShouldDestroyTaintedCoreTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj should destroy tainted core") {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjPetShouldSwitchTargetTrigger : public SscEncounterTrigger
{
public:
    LadyVashjPetShouldSwitchTargetTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj pet should switch target") {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjBotAboveTheGroundTrigger : public SscEncounterTrigger
{
public:
    LadyVashjBotAboveTheGroundTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj bot above the ground") {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjBotInToxicSporesTrigger : public SscEncounterTrigger
{
public:
    LadyVashjBotInToxicSporesTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj bot in toxic spores") {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjMeleeNearToxicSporesTrigger : public SscEncounterTrigger
{
public:
    LadyVashjMeleeNearToxicSporesTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj melee near toxic spores") {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjRangedReachBlockedByToxicSporesTrigger : public SscEncounterTrigger
{
public:
    LadyVashjRangedReachBlockedByToxicSporesTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj ranged reach blocked by toxic spores") {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjEntangleOnMeleeTrigger : public SscEncounterTrigger
{
public:
    LadyVashjEntangleOnMeleeTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj entangle on melee") {}

protected:
    bool IsActiveInEncounter() override;
};

class LadyVashjStaticChargeOnRogueTrigger : public SscEncounterTrigger
{
public:
    LadyVashjStaticChargeOnRogueTrigger(PlayerbotAI* botAI)
        : SscEncounterTrigger(botAI, "lady vashj static charge on rogue") {}

protected:
    bool IsActiveInEncounter() override;
};

#endif
