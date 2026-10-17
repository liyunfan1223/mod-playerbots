/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_COSACTIONS_H
#define PLAYERBOTS_COSACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "CoSTriggers.h"
#include "GenericSpellActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

class ExplodeGhoulSpreadAction : public MovementAction
{
public:
    ExplodeGhoulSpreadAction(PlayerbotAI* botAI) : MovementAction(botAI, "explode ghoul spread") {}
    bool Execute(Event event) override;
};

class EpochStackAction : public MovementAction
{
public:
    EpochStackAction(PlayerbotAI* botAI) : MovementAction(botAI, "epoch stack") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
