/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GDACTIONS_H
#define PLAYERBOTS_GDACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "GDTriggers.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

class AvoidPoisonNovaAction : public MovementAction
{
public:
    AvoidPoisonNovaAction(PlayerbotAI* botAI) : MovementAction(botAI, "avoid poison nova") {}
    bool Execute(Event event) override;
};

class AttackSnakeWrapAction : public AttackAction
{
public:
    AttackSnakeWrapAction(PlayerbotAI* botAI) : AttackAction(botAI, "attack snake wrap") {}
    bool Execute(Event event) override;
};

class SladranStackOnTankAction : public MovementAction
{
public:
    SladranStackOnTankAction(PlayerbotAI* botAI) : MovementAction(botAI, "slad'ran stack on tank") {}
    bool Execute(Event event) override;
};

class SladranTankHoldAction : public AttackAction
{
public:
    SladranTankHoldAction(PlayerbotAI* botAI) : AttackAction(botAI, "slad'ran tank hold") {}
    bool Execute(Event event) override;
};

class AvoidWhirlingSlashAction : public MovementAction
{
public:
    AvoidWhirlingSlashAction(PlayerbotAI* botAI) : MovementAction(botAI, "avoid whirling slash") {}
    bool Execute(Event event) override;
};

#endif
