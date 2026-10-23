/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_KARAACTIONS_H
#define PLAYERBOTS_KARAACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include <vector>

class KarazhanResetEncounterStatesAction : public Action
{
public:
    KarazhanResetEncounterStatesAction(PlayerbotAI* botAI)
        : Action(botAI, "karazhan reset encounter states") {}
    bool Execute(Event event) override;
};

class KarazhanSetTremorTotemAction : public Action
{
public:
    KarazhanSetTremorTotemAction(PlayerbotAI* botAI)
        : Action(botAI, "karazhan set tremor totem") {}
    bool Execute(Event event) override;
};

class ManaWarpStunCreatureBeforeWarpBreachAction : public AttackAction
{
public:
    ManaWarpStunCreatureBeforeWarpBreachAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "mana warp stun creature before warp breach") {}
    bool Execute(Event event) override;
};

class AttumenTheHuntsmanHandlePhaseOneAction : public AttackAction
{
public:
    AttumenTheHuntsmanHandlePhaseOneAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "attumen the huntsman handle phase one") {}
    bool Execute(Event event) override;

private:
    bool AssistTankMoveAttumenFromGroup(Unit* midnight, Unit* attumen);
};

class AttumenTheHuntsmanHandlePhaseTwoAction : public AttackAction
{
public:
    AttumenTheHuntsmanHandlePhaseTwoAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "attumen the huntsman handle phase two") {}
    bool Execute(Event event) override;

private:
    bool CurrentTankPositionAttumen(Unit* attumen);
    bool StackBehindAttumen(Unit* attumen);
};

class AttumenTheHuntsmanSetDpsTimerAction : public Action
{
public:
    AttumenTheHuntsmanSetDpsTimerAction(PlayerbotAI* botAI)
        : Action(botAI, "attumen the huntsman set dps timer") {}
    bool Execute(Event event) override;
};

class MoroesMarkTargetAction : public Action
{
public:
    MoroesMarkTargetAction(PlayerbotAI* botAI)
        : Action(botAI, "moroes mark target") {}
    bool Execute(Event event) override;
};

class MaidenOfVirtueTankPositionBossAction : public AttackAction
{
public:
    MaidenOfVirtueTankPositionBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "maiden of virtue tank position boss") {}
    bool Execute(Event event) override;

private:
    bool MoveBossToStunnedHealer(Player* healer);
};

class MaidenOfVirtuePositionRangedBetweenPillarsAction : public MovementAction
{
public:
    MaidenOfVirtuePositionRangedBetweenPillarsAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "maiden of virtue position ranged between pillars") {}
    bool Execute(Event event) override;
};

class MaidenOfVirtueSetGroundingTotemAction : public Action
{
public:
    MaidenOfVirtueSetGroundingTotemAction(PlayerbotAI* botAI)
        : Action(botAI, "maiden of virtue set grounding totem") {}
    bool Execute(Event event) override;
};

class BigBadWolfPositionBossAction : public AttackAction
{
public:
    BigBadWolfPositionBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "big bad wolf position boss") {}
    bool Execute(Event event) override;
};

class BigBadWolfLittleRedRidingHoodRunAwayAction : public MovementAction
{
public:
    BigBadWolfLittleRedRidingHoodRunAwayAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "big bad wolf little red riding hood run away") {}
    bool Execute(Event event) override;
    bool ResetRunIndex()
    {
        if (!_runIndex)
            return false;
        _runIndex = 0;
        return true;
    }

private:
    uint8 _runIndex = 0;
};

class RomuloAndJulianneMarkTargetAction : public Action
{
public:
    RomuloAndJulianneMarkTargetAction(PlayerbotAI* botAI)
        : Action(botAI, "romulo and julianne mark target") {}
    bool Execute(Event event) override;
};

class WizardOfOzMarkTargetAction : public Action
{
public:
    WizardOfOzMarkTargetAction(PlayerbotAI* botAI)
        : Action(botAI, "wizard of oz mark target") {}
    bool Execute(Event event) override;
};

class WizardOfOzScorchStrawmanAction : public Action
{
public:
    WizardOfOzScorchStrawmanAction(PlayerbotAI* botAI)
        : Action(botAI, "wizard of oz scorch strawman") {}
    bool Execute(Event event) override;
};

class TheCuratorMarkAstralFlareAction : public Action
{
public:
    TheCuratorMarkAstralFlareAction(PlayerbotAI* botAI)
        : Action(botAI, "the curator mark astral flare") {}
    bool Execute(Event event) override;
};

class TheCuratorPositionBossAction : public AttackAction
{
public:
    TheCuratorPositionBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "the curator position boss") {}
    bool Execute(Event event) override;
};

class TheCuratorSpreadRangedAction : public MovementAction
{
public:
    TheCuratorSpreadRangedAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "the curator spread ranged") {}
    bool Execute(Event event) override;
};

class TerestianIllhoofMarkTargetAction : public Action
{
public:
    TerestianIllhoofMarkTargetAction(PlayerbotAI* botAI)
        : Action(botAI, "terestian illhoof mark target") {}
    bool Execute(Event event) override;
};

class ShadeOfAranRunAwayFromArcaneExplosionAction : public MovementAction
{
public:
    ShadeOfAranRunAwayFromArcaneExplosionAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "shade of aran run away from arcane explosion") {}
    bool Execute(Event event) override;
};

class ShadeOfAranStopMovingDuringFlameWreathAction : public MovementAction
{
public:
    ShadeOfAranStopMovingDuringFlameWreathAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "shade of aran stop moving during flame wreath") {}
    bool Execute(Event event) override;
};

class ShadeOfAranMarkConjuredElementalAction : public Action
{
public:
    ShadeOfAranMarkConjuredElementalAction(PlayerbotAI* botAI)
        : Action(botAI, "shade of aran mark conjured elemental") {}
    bool Execute(Event event) override;
};

class ShadeOfAranRangedMaintainDistanceAction : public MovementAction
{
public:
    ShadeOfAranRangedMaintainDistanceAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "shade of aran ranged maintain distance") {}
    bool Execute(Event event) override;
};

class NetherspiteBlockRedBeamAction : public MovementAction
{
public:
    NetherspiteBlockRedBeamAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "netherspite block red beam") {}
    bool Execute(Event event) override;
    bool ResetRedBeamState()
    {
        if (!_redBeamTimerWasSet)
            return false;
        _redBeamMoveTimer = 0;
        _lastBeamMoveSideways = false;
        _wasBlockingRedBeam = false;
        _redBeamTimerWasSet = false;
        return true;
    }

private:
    time_t _redBeamMoveTimer = 0;
    bool _lastBeamMoveSideways = false;
    bool _wasBlockingRedBeam = false;
    bool _redBeamTimerWasSet = false;
};

class NetherspiteBlockBlueBeamAction : public MovementAction
{
public:
    NetherspiteBlockBlueBeamAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "netherspite block blue beam") {}
    bool Execute(Event event) override;
    bool ResetBlueBeamState()
    {
        if (!_wasBlockingBlueBeam)
            return false;
        _wasBlockingBlueBeam = false;
        return true;
    }

private:
    bool _wasBlockingBlueBeam = false;
};

class NetherspiteBlockGreenBeamAction : public MovementAction
{
public:
    NetherspiteBlockGreenBeamAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "netherspite block green beam") {}
    bool Execute(Event event) override;
    bool ResetGreenBeamState()
    {
        if (!_wasBlockingGreenBeam)
            return false;
        _wasBlockingGreenBeam = false;
        return true;
    }

private:
    bool _wasBlockingGreenBeam = false;
};

class NetherspiteAvoidBeamAndVoidZoneAction : public MovementAction
{
public:
    NetherspiteAvoidBeamAndVoidZoneAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "netherspite avoid beam and void zone") {}
    bool Execute(Event event) override;

private:
    struct BeamAvoid
    {
        float minDist;
        float maxDist;
        float dirX;
        float dirY;
    };
    bool IsAwayFromBeams(
        float x, float y, float botX, float botY, std::vector<BeamAvoid> const& beams);
};

class NetherspiteBanishPhaseAvoidVoidZoneAction : public MovementAction
{
public:
    NetherspiteBanishPhaseAvoidVoidZoneAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "netherspite banish phase avoid void zone") {}
    bool Execute(Event event) override;
};

class NetherspiteManageTimersAndTrackersAction : public Action
{
public:
    NetherspiteManageTimersAndTrackersAction(PlayerbotAI* botAI)
        : Action(botAI, "netherspite manage timers and trackers") {}
    bool Execute(Event event) override;
};

class PrinceMalchezaarEnfeebledBotAvoidHazardAction : public MovementAction
{
public:
    PrinceMalchezaarEnfeebledBotAvoidHazardAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "prince malchezaar enfeebled bot avoid hazard") {}
    bool Execute(Event event) override;
};

class PrinceMalchezaarNonTankAvoidInfernalAction : public MovementAction
{
public:
    PrinceMalchezaarNonTankAvoidInfernalAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "prince malchezaar non tank avoid infernal") {}
    bool Execute(Event event) override;
};

class PrinceMalchezaarTanksPositionBossAction : public AttackAction
{
public:
    PrinceMalchezaarTanksPositionBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "prince malchezaar tanks position boss") {}
    bool Execute(Event event) override;
};

class NightbaneGroundPhaseTanksPositionBossAction : public AttackAction
{
public:
    NightbaneGroundPhaseTanksPositionBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "nightbane ground phase tanks position boss") {}
    bool Execute(Event event) override;
};

class NightbaneGroundPhaseCoordinateRangedMovementAction : public MovementAction
{
public:
    NightbaneGroundPhaseCoordinateRangedMovementAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "nightbane ground phase coordinate ranged movement") {}
    bool Execute(Event event) override;

private:
    bool MoveRangedLeaderToSafeSpot();
    bool StackOnRangedLeader(Player* rangedLeader);
};

class NightbaneControlPetAggressionAction : public Action
{
public:
    NightbaneControlPetAggressionAction(PlayerbotAI* botAI)
        : Action(botAI, "nightbane control pet aggression") {}
    bool Execute(Event event) override;
};

class NightbaneFlightPhaseStackAndMoveAction : public MovementAction
{
public:
    NightbaneFlightPhaseStackAndMoveAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "nightbane flight phase stack and move") {}
    bool Execute(Event event) override;
    bool ResetRainOfBonesHit()
    {
        if (!_rainOfBonesHit)
            return false;
        _rainOfBonesHit = false;
        return true;
    }

private:
    bool _rainOfBonesHit = false;
};

class NightbaneTeleportBackToTerraceAction : public MovementAction
{
public:
    NightbaneTeleportBackToTerraceAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "nightbane teleport back to terrace") {}
    bool Execute(Event event) override;
};

class NightbaneManageTimersAndTrackersAction : public Action
{
public:
    NightbaneManageTimersAndTrackersAction(PlayerbotAI* botAI)
        : Action(botAI, "nightbane manage timers and trackers") {}
    bool Execute(Event event) override;
};

#endif
