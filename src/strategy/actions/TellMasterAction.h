/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_TELLMASTERACTION_H
#define _PLAYERBOT_TELLMASTERACTION_H

#include "MovementActions.h"

class PlayerbotAI;

class TellMasterAction : public Action
{
public:
    TellMasterAction(PlayerbotAI* botAI, std::string const text) : Action(botAI, "tell master"), text(text) {}

    bool Execute(Event event) override;

private:
    std::string const text;
};

class OutOfReactRangeAction : public MovementAction
{
public:
    OutOfReactRangeAction(PlayerbotAI* botAI) : MovementAction(botAI, "tell out of react range") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
