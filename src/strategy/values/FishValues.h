/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_FISHVALUES_H
#define _PLAYERBOT_FISHVALUES_H

#include "Value.h"
#include "NamedObjectContext.h"


class PlayerbotAI;

class CanFishValue : public BoolCalculatedValue
{
    public:
        CanFishValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "can fish") {};
        bool Calculate() override;
};
class CanOpenBobberValue : public BoolCalculatedValue
    {
    public:
        CanOpenBobberValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "can open bobber") {};
            bool Calculate() override;
    };

#endif
