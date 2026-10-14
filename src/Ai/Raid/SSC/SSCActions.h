/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SSCACTIONS_H
#define PLAYERBOTS_SSCACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "Position.h"
#include "SSCHelpers.h"
#include <optional>
#include <string>
#include <vector>

class GameObject;
class Item;

// Shared

class SscResetEncounterStatesAction : public Action
{
public:
    SscResetEncounterStatesAction(PlayerbotAI* botAI)
        : Action(botAI, "ssc reset encounter states") {}
    bool Execute(Event event) override;
};

// Used for Morogrim Tidewalker and Lady Vashj.
class SscMisdirectToMainTankAction : public Action
{
public:
    SscMisdirectToMainTankAction(
        PlayerbotAI* botAI, std::string const& name, std::string const& bossName)
        : Action(botAI, name), _bossName(bossName) {}
    bool Execute(Event event) override;

private:
    std::string const _bossName;
};

// Used for Hydross the Unstable and Leotheras the Blind.
class SscStopAttackingAction : public Action
{
public:
    SscStopAttackingAction(PlayerbotAI* botAI, std::string const& name)
        : Action(botAI, name) {}
    bool Execute(Event event) override;
};

// Used for Hydross the Unstable, Leotheras the Blind, and Fathom-Lord Karathress.
class SscSpreadRangedAction : public MovementAction
{
public:
    SscSpreadRangedAction(PlayerbotAI* botAI, std::string const& name, float distance)
        : MovementAction(botAI, name), _distance(distance) {}
    bool Execute(Event event) override;

private:
    float const _distance;
};

// Trash

class UnderbogColossusEscapeToxicPoolAction : public MovementAction
{
public:
    UnderbogColossusEscapeToxicPoolAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "underbog colossus escape toxic pool") {}
    bool Execute(Event event) override;
};

class GreyheartTidecallerMarkWaterElementalTotemAction : public Action
{
public:
    GreyheartTidecallerMarkWaterElementalTotemAction(PlayerbotAI* botAI)
        : Action(botAI, "greyheart tidecaller mark water elemental totem") {}
    bool Execute(Event event) override;
};

// Hydross the Unstable <Duke of Currents>

class HydrossTheUnstablePositionAndSwapTanksAction : public AttackAction
{
public:
    HydrossTheUnstablePositionAndSwapTanksAction(
        PlayerbotAI* botAI, std::string const& name, bool frostTank)
        : AttackAction(botAI, name), _frostTank(frostTank) {}
    bool Execute(Event event) override;

private:
    bool StepTo(Position const& position, Unit* hydross);
    bool const _frostTank;
};

class HydrossTheUnstableMisdirectToTankAction : public Action
{
public:
    HydrossTheUnstableMisdirectToTankAction(PlayerbotAI* botAI)
        : Action(botAI, "hydross the unstable misdirect to tank") {}
    bool Execute(Event event) override;
};

class HydrossTheUnstableManagePhaseTimersAction : public Action
{
public:
    HydrossTheUnstableManagePhaseTimersAction(PlayerbotAI* botAI)
        : Action(botAI, "hydross the unstable manage phase timers") {}
    bool Execute(Event event) override;
};

// The Lurker Below

class TheLurkerBelowRunAroundBehindBossAction : public MovementAction
{
public:
    TheLurkerBelowRunAroundBehindBossAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "the lurker below run around behind boss") {}
    bool Execute(Event event) override;
};

class TheLurkerBelowPositionMainTankAction : public AttackAction
{
public:
    TheLurkerBelowPositionMainTankAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "the lurker below position main tank") {}
    bool Execute(Event event) override;
};

class TheLurkerBelowSpreadRangedInArcAction : public MovementAction
{
public:
    TheLurkerBelowSpreadRangedInArcAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "the lurker below spread ranged in arc") {}
    bool Execute(Event event) override;
    bool ResetRangedPosition()
    {
        if (!_hasRangedPosition)
            return false;

        _hasRangedPosition = false;
        return true;
    }

private:
    Position _rangedPosition;
    bool _hasRangedPosition = false;
};

class TheLurkerBelowTanksPickUpGuardiansAction : public AttackAction
{
public:
    TheLurkerBelowTanksPickUpGuardiansAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "the lurker below tanks pick up guardians") {}
    bool Execute(Event event) override;

private:
    ObjectGuid ClaimGuardianForTank(std::vector<Unit*> const& guardians, int8 myIndex);
};

class TheLurkerBelowMeleeMoveDirectlyToTargetAction : public MovementAction
{
public:
    TheLurkerBelowMeleeMoveDirectlyToTargetAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "the lurker below melee move directly to target") {}
    bool Execute(Event event) override;
};

class TheLurkerBelowMeleeGetOutOfWaterAction : public MovementAction
{
public:
    TheLurkerBelowMeleeGetOutOfWaterAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "the lurker below melee get out of water") {}
    bool Execute(Event event) override;
};

// Leotheras the Blind

class LeotherasTheBlindWarlockTankAttackDemonFormAction : public AttackAction
{
public:
    LeotherasTheBlindWarlockTankAttackDemonFormAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "leotheras the blind warlock tank attack demon form") {}
    bool Execute(Event event) override;
};

class LeotherasTheBlindTanksBuildRageOnDemonFormAction : public AttackAction
{
public:
    LeotherasTheBlindTanksBuildRageOnDemonFormAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "leotheras the blind tanks build rage on demon form") {}
    bool Execute(Event event) override;
};

class LeotherasTheBlindRangedKeepDistanceAction : public MovementAction
{
public:
    LeotherasTheBlindRangedKeepDistanceAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "leotheras the blind ranged keep distance") {}
    bool Execute(Event event) override;
};

class LeotherasTheBlindRunAwayFromWhirlwindAction : public MovementAction
{
public:
    LeotherasTheBlindRunAwayFromWhirlwindAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "leotheras the blind run away from whirlwind") {}
    bool Execute(Event event) override;
};

class LeotherasTheBlindMeleeRunFromChaosBlastAction : public MovementAction
{
public:
    LeotherasTheBlindMeleeRunFromChaosBlastAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "leotheras the blind melee run from chaos blast") {}
    bool Execute(Event event) override;
};

class LeotherasTheBlindDestroyInnerDemonAction : public AttackAction
{
public:
    LeotherasTheBlindDestroyInnerDemonAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "leotheras the blind destroy inner demon") {}
    bool Execute(Event event) override;

private:
    bool HandleFeralTankStrategy(Unit* innerDemon);
    bool HandleHealerStrategy(Unit* innerDemon);
    bool HandleHunterStrategy(Unit* innerDemon);
};

class LeotherasTheBlindFinalPhaseAttackBossAction : public AttackAction
{
public:
    LeotherasTheBlindFinalPhaseAttackBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "leotheras the blind final phase attack boss") {}
    bool Execute(Event event) override;
};

class LeotherasTheBlindFinalPhaseSeparateBossFromDemonAction : public MovementAction
{
public:
    LeotherasTheBlindFinalPhaseSeparateBossFromDemonAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "leotheras the blind final phase separate boss from demon") {}
    bool Execute(Event event) override;
};

class LeotherasTheBlindMisdirectDemonFormToTankAction : public Action
{
public:
    LeotherasTheBlindMisdirectDemonFormToTankAction(PlayerbotAI* botAI)
        : Action(botAI, "leotheras the blind misdirect demon form to tank") {}
    bool Execute(Event event) override;
};

class LeotherasTheBlindManageDpsWaitTimersAction : public Action
{
public:
    LeotherasTheBlindManageDpsWaitTimersAction(PlayerbotAI* botAI)
        : Action(botAI, "leotheras the blind manage dps wait timers") {}
    bool Execute(Event event) override;

private:
    bool TrackWhirlwindEnd(Unit* leotheras, std::optional<uint32>& whirlwindEnd, uint32 now);
};

// Fathom-Lord Karathress

class FathomLordKarathressTanksPositionTargetsAction : public AttackAction
{
public:
    FathomLordKarathressTanksPositionTargetsAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "fathom-lord karathress tanks position targets") {}
    bool Execute(Event event) override;
};

class FathomLordKarathressPositionCaribdisTankHealerAction : public MovementAction
{
public:
    FathomLordKarathressPositionCaribdisTankHealerAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "fathom-lord karathress position caribdis tank healer") {}
    bool Execute(Event event) override;
};

class FathomLordKarathressMisdirectToTanksAction : public Action
{
public:
    FathomLordKarathressMisdirectToTanksAction(PlayerbotAI* botAI)
        : Action(botAI, "fathom-lord karathress misdirect to tanks") {}
    bool Execute(Event event) override;
};

class FathomLordKarathressAssignDpsPriorityAction : public AttackAction
{
public:
    FathomLordKarathressAssignDpsPriorityAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "fathom-lord karathress assign dps priority") {}
    bool Execute(Event event) override;

private:
    bool ApproachCaribdis(Unit* caribdis);
};

class FathomLordKarathressManageDpsTimerAction : public Action
{
public:
    FathomLordKarathressManageDpsTimerAction(PlayerbotAI* botAI)
        : Action(botAI, "fathom-lord karathress manage dps timer") {}
    bool Execute(Event event) override;
};

class FathomLordKarathressDropToGroundAfterCycloneAction : public MovementAction
{
public:
    FathomLordKarathressDropToGroundAfterCycloneAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "fathom-lord karathress drop to ground after cyclone") {}
    bool Execute(Event event) override;
};

// Morogrim Tidewalker

class MorogrimTidewalkerPositionMainTankAction : public AttackAction
{
public:
    MorogrimTidewalkerPositionMainTankAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "morogrim tidewalker position main tank") {}
    bool Execute(Event event) override;

private:
    bool MoveToPhase1TankPosition(Unit* tidewalker);
    bool MoveToPhase2TankPosition(Unit* tidewalker);
};

class MorogrimTidewalkerStackRangedBehindBossAction : public MovementAction
{
public:
    MorogrimTidewalkerStackRangedBehindBossAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "morogrim tidewalker stack ranged behind boss") {}
    bool Execute(Event event) override;
};

class MorogrimTidewalkerReturnToBossAction : public MovementAction
{
public:
    MorogrimTidewalkerReturnToBossAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "morogrim tidewalker return to boss") {}
    bool Execute(Event event) override;
};

// Lady Vashj <Coilfang Matron>

class LadyVashjMainTankPositionBossAction : public AttackAction
{
public:
    LadyVashjMainTankPositionBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "lady vashj main tank position boss") {}
    bool Execute(Event event) override;

private:
    bool MoveToPhase1TankPosition(Unit* vashj);
    bool MoveAwayFromElementalsAndStriders(Unit* vashj);
};

class LadyVashjPhase1SpreadRangedInArcAction : public MovementAction
{
public:
    LadyVashjPhase1SpreadRangedInArcAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "lady vashj phase 1 spread ranged in arc") {}
    bool Execute(Event event) override;
    bool HasReachedRangedPosition() const { return _reachedRangedPosition; }
    bool ResetRangedPosition()
    {
        if (!_hasRangedPosition)
            return false;

        _hasRangedPosition = false;
        _reachedRangedPosition = false;
        return true;
    }

private:
    Position _rangedPosition;
    bool _hasRangedPosition = false;
    bool _reachedRangedPosition = false;
};

class LadyVashjAssignStationSlotsAction : public Action
{
public:
    LadyVashjAssignStationSlotsAction(PlayerbotAI* botAI)
        : Action(botAI, "lady vashj assign station slots") {}
    bool Execute(Event event) override;
};

class LadyVashjPhase2PositionAtStationAction : public MovementAction
{
public:
    LadyVashjPhase2PositionAtStationAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "lady vashj phase 2 position at station") {}
    bool Execute(Event event) override;
};

class LadyVashjPhase3PositionRangedAction : public MovementAction
{
public:
    LadyVashjPhase3PositionRangedAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "lady vashj phase 3 position ranged") {}
    bool Execute(Event event) override;
};

class LadyVashjPhase3MoveIntoSightAction : public MovementAction
{
public:
    LadyVashjPhase3MoveIntoSightAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "lady vashj phase 3 move into sight") {}
    bool Execute(Event event) override;
};

class LadyVashjAssignGroundingShamanAction : public Action
{
public:
    LadyVashjAssignGroundingShamanAction(PlayerbotAI* botAI)
        : Action(botAI, "lady vashj assign grounding shaman") {}
    bool Execute(Event event) override;
};

class LadyVashjSetGroundingTotemInMainTankGroupAction : public MovementAction
{
public:
    LadyVashjSetGroundingTotemInMainTankGroupAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "lady vashj set grounding totem in main tank group") {}
    bool Execute(Event event) override;
};

class LadyVashjStaticChargeMoveAwayFromGroupAction : public MovementAction
{
public:
    LadyVashjStaticChargeMoveAwayFromGroupAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "lady vashj static charge move away from group") {}
    bool Execute(Event event) override;
};

class LadyVashjAssignTargetPriorityAction : public AttackAction
{
public:
    LadyVashjAssignTargetPriorityAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "lady vashj assign target priority") {}
    bool Execute(Event event) override;
};

class LadyVashjTankApplyFearWardAction : public Action
{
public:
    LadyVashjTankApplyFearWardAction(PlayerbotAI* botAI)
        : Action(botAI, "lady vashj tank apply fear ward") {}
    bool Execute(Event event) override;
};

class LadyVashjPositionCoilfangStriderAction : public MovementAction
{
public:
    LadyVashjPositionCoilfangStriderAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "lady vashj position coilfang strider") {}
    bool Execute(Event event) override;

private:
    bool MoveStriderToHoldPosition(Unit* strider);
    bool MoveStriderAwayFromVashj(Unit* strider, Unit* vashj);
};

class LadyVashjPositionCoilfangEliteAction : public MovementAction
{
public:
    LadyVashjPositionCoilfangEliteAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "lady vashj position coilfang elite") {}
    bool Execute(Event event) override;
};

class LadyVashjTankWaitInTheMiddleAction : public MovementAction
{
public:
    LadyVashjTankWaitInTheMiddleAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "lady vashj tank wait in the middle") {}
    bool Execute(Event event) override;
};

class LadyVashjAssignTaintedCoreLooterAction : public Action
{
public:
    LadyVashjAssignTaintedCoreLooterAction(PlayerbotAI* botAI)
        : Action(botAI, "lady vashj assign tainted core looter") {}
    bool Execute(Event event) override;
};

class LadyVashjAttackTaintedElementalAction : public AttackAction
{
public:
    LadyVashjAttackTaintedElementalAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "lady vashj attack tainted elemental") {}
    bool Execute(Event event) override;
};

class LadyVashjLootTaintedCoreAction : public MovementAction
{
public:
    LadyVashjLootTaintedCoreAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "lady vashj loot tainted core") {}
    bool Execute(Event event) override;
};

class LadyVashjPassTheTaintedCoreAction : public MovementAction
{
public:
    LadyVashjPassTheTaintedCoreAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "lady vashj pass the tainted core") {}
    bool Execute(Event event) override;

private:
    bool MoveToCoreSpot(SscHelpers::VashjCorePassingChain& chain, int8 index);
    bool ThrowCore(
        SscHelpers::VashjCorePassingChain& chain, size_t next, Item* core, GameObject* generator);
    bool UseCoreOnGenerator(Item* core, GameObject* generator);
};

class LadyVashjDestroyTaintedCoreAction : public Action
{
public:
    LadyVashjDestroyTaintedCoreAction(PlayerbotAI* botAI)
        : Action(botAI, "lady vashj destroy tainted core") {}
    bool Execute(Event event) override;
};

class LadyVashjCommandPetTargetAction : public Action
{
public:
    LadyVashjCommandPetTargetAction(PlayerbotAI* botAI)
        : Action(botAI, "lady vashj command pet target") {}
    bool Execute(Event event) override;
};

class LadyVashjReturnToTheGroundAction : public Action
{
public:
    LadyVashjReturnToTheGroundAction(PlayerbotAI* botAI)
        : Action(botAI, "lady vashj return to the ground") {}
    bool Execute(Event event) override;
};

class LadyVashjAvoidToxicSporesAction : public MovementAction
{
public:
    LadyVashjAvoidToxicSporesAction(
        PlayerbotAI* botAI, std::string const& name = "lady vashj avoid toxic spores")
        : MovementAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    bool StepTowardBreakoutSpot(Unit* vashj);
    Position _breakoutSpot;
    bool _hasBreakoutSpot = false;
    uint32 _breakoutStartTime = 0;
};

class LadyVashjMeleeMoveAroundToxicSporesAction : public LadyVashjAvoidToxicSporesAction
{
public:
    LadyVashjMeleeMoveAroundToxicSporesAction(PlayerbotAI* botAI)
        : LadyVashjAvoidToxicSporesAction(botAI, "lady vashj melee move around toxic spores") {}
    bool Execute(Event event) override;
};

class LadyVashjRangedReachAroundToxicSporesAction : public MovementAction
{
public:
    LadyVashjRangedReachAroundToxicSporesAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "lady vashj ranged reach around toxic spores") {}
    bool Execute(Event event) override;
};

class LadyVashjPaladinUseHandOfFreedomAction : public Action
{
public:
    LadyVashjPaladinUseHandOfFreedomAction(PlayerbotAI* botAI)
        : Action(botAI, "lady vashj paladin use hand of freedom") {}
    bool Execute(Event event) override;
};

class LadyVashjRogueUseCloakOfShadowsAction : public Action
{
public:
    LadyVashjRogueUseCloakOfShadowsAction(PlayerbotAI* botAI)
        : Action(botAI, "lady vashj rogue use cloak of shadows") {}
    bool Execute(Event event) override;
};

#endif
