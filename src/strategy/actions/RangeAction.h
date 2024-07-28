/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RANGEACTION_H
#define _PLAYERBOT_RANGEACTION_H

#include "Action.h"

class PlayerbotAI;

class RangeAction : public Action
{
public:
    RangeAction(PlayerbotAI* botAI) : Action(botAI, "range") {}

    bool Execute(Event event) override;

private:
    void PrintRange(std::string const type);
};

#endif
