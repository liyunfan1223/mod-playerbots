/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_POSITIONACTION_H
#define _PLAYERBOT_POSITIONACTION_H

#include "MovementActions.h"

class PlayerbotAI;

class PositionAction : public Action
{
public:
    PositionAction(PlayerbotAI* botAI) : Action(botAI, "position") {}

    bool Execute(Event event) override;
};

class MoveToPositionAction : public MovementAction
{
public:
    MoveToPositionAction(PlayerbotAI* botAI, std::string const name, std::string const qualifier, bool idle = false)
        : MovementAction(botAI, name), qualifier(qualifier), idle(idle)
    {
    }

    bool Execute(Event event) override;
    bool isUseful() override;

protected:
    std::string const qualifier;
    bool idle;
};

class GuardAction : public MoveToPositionAction
{
public:
    GuardAction(PlayerbotAI* botAI) : MoveToPositionAction(botAI, "move to position", "guard") {}
};

class SetReturnPositionAction : public Action
{
public:
    SetReturnPositionAction(PlayerbotAI* botAI) : Action(botAI, "set return position") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class ReturnAction : public MoveToPositionAction
{
public:
    ReturnAction(PlayerbotAI* botAI) : MoveToPositionAction(botAI, "return", "return", true) {}

    bool isUseful() override;
};

#endif
