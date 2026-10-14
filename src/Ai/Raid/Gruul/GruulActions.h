/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GRUULACTIONS_H
#define PLAYERBOTS_GRUULACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "Position.h"

// General

class GruulsLairResetEncounterStatesAction : public Action
{
public:
    GruulsLairResetEncounterStatesAction(PlayerbotAI* botAI)
        : Action(botAI, "gruul's lair reset encounter states") {}
    bool Execute(Event event) override;
};

// High King Maulgar <Lord of the Ogres>

class HighKingMaulgarMeleeTanksPositionBossesAction : public AttackAction
{
public:
    HighKingMaulgarMeleeTanksPositionBossesAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "high king maulgar melee tanks position bosses") {}
    bool Execute(Event event) override;
};

class HighKingMaulgarMageTankAttackKroshAction : public AttackAction
{
public:
    HighKingMaulgarMageTankAttackKroshAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "high king maulgar mage tank attack krosh") {}
    bool Execute(Event event) override;

private:
    bool AttackAndCast(Unit* krosh);
    bool MoveToDesiredDistance(Unit* krosh);
};

class HighKingMaulgarMoonkinTankAttackKigglerAction : public AttackAction
{
public:
    HighKingMaulgarMoonkinTankAttackKigglerAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "high king maulgar moonkin tank attack kiggler") {}
    bool Execute(Event event) override;
};

class HighKingMaulgarAssignDpsPriorityAction : public AttackAction
{
public:
    HighKingMaulgarAssignDpsPriorityAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "high king maulgar assign dps priority") {}
    bool Execute(Event event) override;
};

class HighKingMaulgarRunAwayFromWhirlwindAction : public MovementAction
{
public:
    HighKingMaulgarRunAwayFromWhirlwindAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "high king maulgar run away from whirlwind") {}
    bool Execute(Event event) override;
};

class HighKingMaulgarBackAwayFromKroshAction : public MovementAction
{
public:
    HighKingMaulgarBackAwayFromKroshAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "high king maulgar back away from krosh") {}
    bool Execute(Event event) override;
};

class HighKingMaulgarBanishFelStalkerAction : public Action
{
public:
    HighKingMaulgarBanishFelStalkerAction(PlayerbotAI* botAI)
        : Action(botAI, "high king maulgar banish fel stalker") {}
    bool Execute(Event event) override;
};

class HighKingMaulgarMisdirectOgresToTanksAction : public Action
{
public:
    HighKingMaulgarMisdirectOgresToTanksAction(PlayerbotAI* botAI)
        : Action(botAI, "high king maulgar misdirect ogres to tanks") {}
    bool Execute(Event event) override;
};

// Gruul the Dragonkiller

class GruulTheDragonkillerTanksPositionBossAction : public AttackAction
{
public:
    GruulTheDragonkillerTanksPositionBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "gruul the dragonkiller tanks position boss") {}
    bool Execute(Event event) override;
};

class GruulTheDragonkillerSpreadRangedAction : public MovementAction
{
public:
    GruulTheDragonkillerSpreadRangedAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "gruul the dragonkiller spread ranged") {}
    bool Execute(Event event) override;
    bool ResetInitialPosition()
    {
        if (!_hasReachedInitialPosition && !_hasInitialPosition)
            return false;

        _hasReachedInitialPosition = false;
        _hasInitialPosition = false;
        return true;
    }

private:
    Position _initialPosition;
    bool _hasInitialPosition = false;
    bool _hasReachedInitialPosition = false;
};

class GruulTheDragonkillerGetOutOfCaveInAction : public MovementAction
{
public:
    GruulTheDragonkillerGetOutOfCaveInAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "gruul the dragonkiller get out of cave in") {}
    bool Execute(Event event) override;
};

class GruulTheDragonkillerShatterSpreadAction : public MovementAction
{
public:
    GruulTheDragonkillerShatterSpreadAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "gruul the dragonkiller shatter spread") {}
    bool Execute(Event event) override;
};

#endif
