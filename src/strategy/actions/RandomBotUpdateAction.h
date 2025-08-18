/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RANDOMBOTUPDATEACTION_H
#define _PLAYERBOT_RANDOMBOTUPDATEACTION_H

#include "Action.h"

class PlayerbotAI;

class RandomBotUpdateAction : public Action
{
public:
    RandomBotUpdateAction(PlayerbotAI* botAI) : Action(botAI, "random bot update") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
