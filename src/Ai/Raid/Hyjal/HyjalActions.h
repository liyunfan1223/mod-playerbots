/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HYJALACTIONS_H
#define PLAYERBOTS_HYJALACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "Position.h"
#include <string>

// General

class HyjalResetEncounterStatesAction : public Action
{
public:
    HyjalResetEncounterStatesAction(PlayerbotAI* botAI)
        : Action(botAI, "hyjal reset encounter states") {}
    bool Execute(Event event) override;
};

class HyjalMisdirectBossToMainTankAction : public Action
{
public:
    HyjalMisdirectBossToMainTankAction(
        PlayerbotAI* botAI, std::string const& name, std::string const& bossName)
        : Action(botAI, name), _bossName(bossName) {}
    bool Execute(Event event) override;

private:
    std::string const _bossName;
};

// For all five bosses. _bailBelowHealthPct is the tank's own health, below which it stops walking
// the boss.
class HyjalMainTankPositionBossAction : public AttackAction
{
public:
    HyjalMainTankPositionBossAction(
        PlayerbotAI* botAI, std::string const& name, std::string const& bossName,
        Position const& position, float bailBelowHealthPct = 0.0f)
        : AttackAction(botAI, name), _bossName(bossName), _position(position),
          _bailBelowHealthPct(bailBelowHealthPct) {}
    bool Execute(Event event) override;

private:
    std::string const _bossName;
    Position const _position;
    float const _bailBelowHealthPct;
};

// Remove Mark of Kaz'rogal and Doomfire in certain cases
class HyjalRemoveDangerousDotAction : public Action
{
public:
    HyjalRemoveDangerousDotAction(PlayerbotAI* botAI)
        : Action(botAI, "hyjal remove dangerous dot") {}
    bool Execute(Event event) override;
};

// Rage Winterchill

class RageWinterchillRangedGetOutOfDeathAndDecayAction : public MovementAction
{
public:
    RageWinterchillRangedGetOutOfDeathAndDecayAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "rage winterchill ranged get out of death and decay") {}
    bool Execute(Event event) override;
};

class RageWinterchillSpreadRangedInCircleAction : public MovementAction
{
public:
    RageWinterchillSpreadRangedInCircleAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "rage winterchill spread ranged in circle") {}
    bool Execute(Event event) override;
    bool ResetWinterchillPositionReached()
    {
        if (!_winterchillPositionReached)
            return false;
        _winterchillPositionReached = false;
        return true;
    }

private:
    bool _winterchillPositionReached = false;
};

class RageWinterchillMeleeManeuverThroughDeathAndDecayAction : public AttackAction
{
public:
    RageWinterchillMeleeManeuverThroughDeathAndDecayAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "rage winterchill melee maneuver through death and decay") {}
    bool Execute(Event event) override;
};

// Anetheron

class AnetheronMisdirectBossAndInfernalsToTanksAction : public Action
{
public:
    AnetheronMisdirectBossAndInfernalsToTanksAction(PlayerbotAI* botAI)
        : Action(botAI, "anetheron misdirect boss and infernals to tanks") {}
    bool Execute(Event event) override;
};

class AnetheronSpreadRangedInCircleAction : public MovementAction
{
public:
    AnetheronSpreadRangedInCircleAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "anetheron spread ranged in circle") {}
    bool Execute(Event event) override;
    bool ResetAnetheronPositionReached()
    {
        if (!_anetheronPositionReached)
            return false;
        _anetheronPositionReached = false;
        return true;
    }

private:
    bool _anetheronPositionReached = false;
};

class AnetheronMoveAwayFromInfernoTargetAction : public MovementAction
{
public:
    AnetheronMoveAwayFromInfernoTargetAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "anetheron move away from inferno target") {}
    bool Execute(Event event) override;
};

class AnetheronBringInfernalToInfernalTankAction : public MovementAction
{
public:
    AnetheronBringInfernalToInfernalTankAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "anetheron bring infernal to infernal tank") {}
    bool Execute(Event event) override;
};

class AnetheronInfernalTankTakePositionAction : public MovementAction
{
public:
    AnetheronInfernalTankTakePositionAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "anetheron infernal tank take position") {}
    bool Execute(Event event) override;
};

class AnetheronGetOutOfImmolationAction : public MovementAction
{
public:
    AnetheronGetOutOfImmolationAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "anetheron get out of immolation") {}
    bool Execute(Event event) override;
};

class AnetheronAssignDpsPriorityAction : public AttackAction
{
public:
    AnetheronAssignDpsPriorityAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "anetheron assign dps priority") {}
    bool Execute(Event event) override;
};

// Kaz'rogal

class KazrogalAssistTanksMoveInFrontAction : public AttackAction
{
public:
    KazrogalAssistTanksMoveInFrontAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "kaz'rogal assist tanks move in front") {}
    bool Execute(Event event) override;
};

class KazrogalSpreadRangedInArcAction : public MovementAction
{
public:
    KazrogalSpreadRangedInArcAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "kaz'rogal spread ranged in arc") {}
    bool Execute(Event event) override;
};

class KazrogalMoveAwayFromGroupAction : public MovementAction
{
public:
    KazrogalMoveAwayFromGroupAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "kaz'rogal move away from group") {}
    bool Execute(Event event) override;
};

class KazrogalActivateAspectOfTheViperAction : public Action
{
public:
    KazrogalActivateAspectOfTheViperAction(PlayerbotAI* botAI)
        : Action(botAI, "kaz'rogal activate aspect of the viper") {}
    bool Execute(Event event) override;
};

class KazrogalCancelImmunityAction : public Action
{
public:
    KazrogalCancelImmunityAction(PlayerbotAI* botAI) : Action(botAI, "kaz'rogal cancel immunity") {}
    bool Execute(Event event) override;
};

class KazrogalWarlockManageManaAction : public Action
{
public:
    KazrogalWarlockManageManaAction(PlayerbotAI* botAI)
        : Action(botAI, "kaz'rogal warlock manage mana") {}
    bool Execute(Event event) override;
};

// Azgalor

class AzgalorDisperseRangedAction : public MovementAction
{
public:
    AzgalorDisperseRangedAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "azgalor disperse ranged") {}
    bool Execute(Event event) override;
};

class AzgalorMeleeManeuverThroughFireAction : public MovementAction
{
public:
    AzgalorMeleeManeuverThroughFireAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "azgalor melee maneuver through fire") {}
    bool Execute(Event event) override;
};

class AzgalorRangedGetOutOfRainOfFireAction : public MovementAction
{
public:
    AzgalorRangedGetOutOfRainOfFireAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "azgalor ranged get out of rain of fire") {}
    bool Execute(Event event) override;
};

class AzgalorMoveToDoomguardTankAction : public MovementAction
{
public:
    AzgalorMoveToDoomguardTankAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "azgalor move to doomguard tank") {}
    bool Execute(Event event) override;
};

// The Doomguard tank is the first assist tank, or the second when the first is Doomed.
class AzgalorTankPositionDoomguardAction : public AttackAction
{
public:
    AzgalorTankPositionDoomguardAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "azgalor tank position doomguard") {}
    bool Execute(Event event) override;
};

class AzgalorDetermineDpsPriorityAction : public AttackAction
{
public:
    AzgalorDetermineDpsPriorityAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "azgalor determine dps priority") {}
    bool Execute(Event event) override;
};

// Archimonde

class ArchimondeSetTremorTotemAction : public Action
{
public:
    ArchimondeSetTremorTotemAction(PlayerbotAI* botAI)
        : Action(botAI, "archimonde set tremor totem") {}
    bool Execute(Event event) override;
};

class ArchimondeKeepAirBurstAwayFromTankAction : public MovementAction
{
public:
    ArchimondeKeepAirBurstAwayFromTankAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "archimonde keep air burst away from tank") {}
    bool Execute(Event event) override;
};

class ArchimondeSpreadRangedAction : public MovementAction
{
public:
    ArchimondeSpreadRangedAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "archimonde spread ranged") {}
    bool Execute(Event event) override;
};

class ArchimondeAvoidDoomfireAction : public MovementAction
{
public:
    ArchimondeAvoidDoomfireAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "archimonde avoid doomfire") {}
    bool Execute(Event event) override;
};

#endif
