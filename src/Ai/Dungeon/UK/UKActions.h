/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_UKACTIONS_H
#define PLAYERBOTS_UKACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "UKTriggers.h"

class AttackFrostTombAction : public AttackAction
{
public:
    AttackFrostTombAction(PlayerbotAI* botAI) : AttackAction(botAI, "attack frost tomb") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class AttackDalronnAction : public AttackAction
{
public:
    AttackDalronnAction(PlayerbotAI* botAI) : AttackAction(botAI, "attack dalronn") {}
    bool Execute(Event event) override;
};

class IngvarDodgeSmashAction : public MovementAction
{
public:
    IngvarDodgeSmashAction(PlayerbotAI* botAI) : MovementAction(botAI, "ingvar dodge smash") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class IngvarSmashReturnAction : public MovementAction
{
public:
    IngvarSmashReturnAction(PlayerbotAI* botAI) : MovementAction(botAI, "ingvar smash return") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
