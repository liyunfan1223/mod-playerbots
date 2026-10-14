/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ACACTIONS_H
#define PLAYERBOTS_ACACTIONS_H

#include "ACTriggers.h"
#include "AttackAction.h"
#include "MovementActions.h"

// Shirrak the Dead Watcher

class ShirrakTankPositionBossAction : public AttackAction
{
public:
    ShirrakTankPositionBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "shirrak tank position boss") {}
    bool Execute(Event event) override;
};

class ShirrakFleeFocusFireAction : public MovementAction
{
public:
    ShirrakFleeFocusFireAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "shirrak flee focus fire") {}
    bool Execute(Event event) override;
};

class ShirrakRangedKeepDistanceAction : public MovementAction
{
public:
    ShirrakRangedKeepDistanceAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "shirrak ranged keep distance") {}
    bool Execute(Event event) override;
};

#endif
