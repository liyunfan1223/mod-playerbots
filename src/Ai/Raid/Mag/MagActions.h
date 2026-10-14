/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MAGACTIONS_H
#define PLAYERBOTS_MAGACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MagHelpers.h"
#include "MovementActions.h"

class MagtheridonResetEncounterStatesAction : public Action
{
public:
    MagtheridonResetEncounterStatesAction(PlayerbotAI* botAI)
        : Action(botAI, "magtheridon reset encounter states") {}
    bool Execute(Event event) override;
};

class MagtheridonMainTankAttackFirstThreeChannelersAction : public AttackAction
{
public:
    MagtheridonMainTankAttackFirstThreeChannelersAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "magtheridon main tank attack first three channelers") {}
    bool Execute(Event event) override;
};

class MagtheridonAssistTanksAttackLastTwoChannelersAction : public AttackAction
{
public:
    MagtheridonAssistTanksAttackLastTwoChannelersAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "magtheridon assist tanks attack last two channelers") {}
    bool Execute(Event event) override;
};

class MagtheridonMisdirectHellfireChannelersToMainTankAction : public Action
{
public:
    MagtheridonMisdirectHellfireChannelersToMainTankAction(PlayerbotAI* botAI)
        : Action(botAI, "magtheridon misdirect hellfire channelers to main tank") {}
    bool Execute(Event event) override;
};

class MagtheridonAssignDpsPriorityAction : public AttackAction
{
public:
    MagtheridonAssignDpsPriorityAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "magtheridon assign dps priority") {}
    bool Execute(Event event) override;
};

class MagtheridonWarlockCcBurningAbyssalAction : public Action
{
public:
    MagtheridonWarlockCcBurningAbyssalAction(PlayerbotAI* botAI)
        : Action(botAI, "magtheridon warlock cc burning abyssal") {}
    bool Execute(Event event) override;
};

class MagtheridonMainTankPositionBossAction : public AttackAction
{
public:
    MagtheridonMainTankPositionBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "magtheridon main tank position boss") {}
    bool Execute(Event event) override;
};

class MagtheridonSpreadRangedAction : public MovementAction
{
public:
    MagtheridonSpreadRangedAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "magtheridon spread ranged") {}
    bool Execute(Event event) override;
};

class MagtheridonMoveOutOfDebrisAction : public MovementAction
{
public:
    MagtheridonMoveOutOfDebrisAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "magtheridon move out of debris") {}
    bool Execute(Event event) override;

private:
    bool FindSafePosition(Position& outPos);
};

class MagtheridonUseManticronCubeAction : public MovementAction
{
public:
    MagtheridonUseManticronCubeAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "magtheridon use manticron cube") {}
    bool Execute(Event event) override;

private:
    MagHelpers::CubeInfo const* GetAssignedCube();
    bool HandleCubeRelease(Unit* magtheridon);
    bool HandleCubeInteraction(GameObject* cube);
    bool HandleWaitingPhase(MagHelpers::CubeInfo const& cubeInfo);
    bool FindSafePositionNearCube(
        MagHelpers::CubeInfo const& cubeInfo, float preferredDistance, Position& outPos);
};

class MagtheridonUpdateTimersAndAssignmentsAction : public Action
{
public:
    MagtheridonUpdateTimersAndAssignmentsAction(PlayerbotAI* botAI)
        : Action(botAI, "magtheridon update timers and assignments") {}
    bool Execute(Event event) override;

private:
    bool AssignCubeClickers(uint32 instanceId, Unit* magtheridon);
    bool NeedsCubeReassignment(uint32 instanceId);
};

#endif
