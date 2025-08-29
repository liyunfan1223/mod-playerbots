/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RANDOMBOTUPDATEVALUE_H
#define _PLAYERBOT_RANDOMBOTUPDATEVALUE_H

#include "Value.h"

class PlayerbotAI;

class RandomBotUpdateValue : public ManualSetValue<bool>
{
public:
    RandomBotUpdateValue(PlayerbotAI* botAI, std::string const name = "random bot update")
        : ManualSetValue<bool>(botAI, false, name)
    {
    }
};

#endif
