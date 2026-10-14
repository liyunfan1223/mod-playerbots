/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ZAACTIONS_H
#define PLAYERBOTS_ZAACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include <string>

// General

class ZulAmanResetEncounterStatesAction : public Action
{
public:
    ZulAmanResetEncounterStatesAction(PlayerbotAI* botAI)
        : Action(botAI, "zul'aman reset encounter states") {}
    bool Execute(Event event) override;
};

class ZulAmanMisdirectBossToMainTankAction : public Action
{
public:
    ZulAmanMisdirectBossToMainTankAction(
        PlayerbotAI* botAI, std::string const& name, std::string const& bossName)
        : Action(botAI, name), _bossName(bossName) {}
    bool Execute(Event event) override;

private:
    std::string const _bossName;
};

class ZulAmanTanksPositionBossAction : public AttackAction
{
public:
    ZulAmanTanksPositionBossAction(
        PlayerbotAI* botAI, std::string const& name, std::string const& bossName,
        Position const& position)
        : AttackAction(botAI, name), _bossName(bossName), _position(position) {}
    bool Execute(Event event) override;

private:
    std::string const _bossName;
    Position const _position;
};

class ZulAmanSpreadRangedAction : public MovementAction
{
public:
    ZulAmanSpreadRangedAction(
        PlayerbotAI* botAI, std::string const& name, float minDistance)
        : MovementAction(botAI, name), _minDistance(minDistance) {}
    bool Execute(Event event) override;

private:
    float const _minDistance;
};

class ZulAmanRunAwayFromWhirlwindAction : public MovementAction
{
public:
    ZulAmanRunAwayFromWhirlwindAction(
        PlayerbotAI* botAI, std::string const& name, std::string const& bossName)
        : MovementAction(botAI, name), _bossName(bossName) {}
    bool Execute(Event event) override;

private:
    std::string const _bossName;
};

// Trash

class AmanishiMedicineManMarkWardAction : public Action
{
public:
    AmanishiMedicineManMarkWardAction(PlayerbotAI* botAI)
        : Action(botAI, "amani'shi medicine man mark ward") {}
    bool Execute(Event event) override;
};

// Akil'zon <Eagle Avatar>

class AkilzonMoveToEyeOfTheStormAction : public MovementAction
{
public:
    AkilzonMoveToEyeOfTheStormAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "akil'zon move to eye of the storm") {}
    bool Execute(Event event) override;
};

class AkilzonStartElectricalStormTimerAction : public Action
{
public:
    AkilzonStartElectricalStormTimerAction(PlayerbotAI* botAI)
        : Action(botAI, "akil'zon start electrical storm timer") {}
    bool Execute(Event event) override;
};

// Nalorakk <Bear Avatar>

class NalorakkTanksPositionBossAction : public AttackAction
{
public:
    NalorakkTanksPositionBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "nalorakk tanks position boss") {}
    bool Execute(Event event) override;
};

// Jan'alai <Dragonhawk Avatar>

class JanalaiSpreadRangedInCircleAction : public MovementAction
{
public:
    JanalaiSpreadRangedInCircleAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "jan'alai spread ranged in circle") {}
    bool Execute(Event event) override;
};

class JanalaiAvoidFireBombsAction : public MovementAction
{
public:
    JanalaiAvoidFireBombsAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "jan'alai avoid fire bombs") {}
    bool Execute(Event event) override;
};

class JanalaiMarkAmanishiHatchersAction : public Action
{
public:
    JanalaiMarkAmanishiHatchersAction(PlayerbotAI* botAI)
        : Action(botAI, "jan'alai mark amani'shi hatchers") {}
    bool Execute(Event event) override;
};

// Halazzi <Lynx Avatar>

class HalazziFirstAssistTankAttackSpiritLynxAction : public AttackAction
{
public:
    HalazziFirstAssistTankAttackSpiritLynxAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "halazzi first assist tank attack spirit lynx") {}
    bool Execute(Event event) override;
};

class HalazziDpsAttackTotemAndBossAction : public AttackAction
{
public:
    HalazziDpsAttackTotemAndBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "halazzi dps attack totem and boss") {}
    bool Execute(Event event) override;
};

// Hex Lord Malacrass

class HexLordMalacrassAssignDpsPriorityAction : public AttackAction
{
public:
    HexLordMalacrassAssignDpsPriorityAction(PlayerbotAI* botAI)
        : AttackAction(botAI,  "hex lord malacrass assign dps priority") {}
    bool Execute(Event event) override;
};

class HexLordMalacrassMoveAwayFromFreezingTrapAction : public MovementAction
{
public:
    HexLordMalacrassMoveAwayFromFreezingTrapAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "hex lord malacrass move away from freezing trap") {}
    bool Execute(Event event) override;
};

// Zul'jin

class ZuljinMassDispelCreepingParalysisAction : public Action
{
public:
    ZuljinMassDispelCreepingParalysisAction(PlayerbotAI* botAI)
        : Action(botAI, "zul'jin mass dispel creeping paralysis") {}
    bool Execute(Event event) override;
};

class ZuljinPositionRangedForCyclonesAction : public MovementAction
{
public:
    ZuljinPositionRangedForCyclonesAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "zul'jin position ranged for cyclones") {}
    bool Execute(Event event) override;
};

#endif
