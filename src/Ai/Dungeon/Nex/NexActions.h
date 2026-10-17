/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_NEXACTIONS_H
#define PLAYERBOTS_NEXACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "NexTriggers.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

class MoveFromWhirlwindAction : public MovementAction
{
public:
    MoveFromWhirlwindAction(PlayerbotAI* botAI) : MovementAction(botAI, "move from whirlwind") {}
    bool Execute(Event event) override;
};

class FirebombSpreadAction : public MovementAction
{
public:
    FirebombSpreadAction(PlayerbotAI* botAI) : MovementAction(botAI, "firebomb spread") {}
    bool Execute(Event event) override;
};

class TelestraSplitTargetAction : public AttackAction
{
public:
    TelestraSplitTargetAction(PlayerbotAI* botAI) : AttackAction(botAI, "telestra split target") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class ChaoticRiftTargetAction : public AttackAction
{
public:
    ChaoticRiftTargetAction(PlayerbotAI* botAI) : AttackAction(botAI, "chaotic rift target") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class DodgeSpikesAction : public MovementAction
{
public:
    DodgeSpikesAction(PlayerbotAI* botAI) : MovementAction(botAI, "dodge spikes") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class IntenseColdJumpAction : public MovementAction
{
public:
    IntenseColdJumpAction(PlayerbotAI* botAI) : MovementAction(botAI, "intense cold jump") {}
    bool Execute(Event event) override;
};

#endif
