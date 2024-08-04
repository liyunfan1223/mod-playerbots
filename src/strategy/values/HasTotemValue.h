/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_HASTOTEMVALUE_H
#define _PLAYERBOT_HASTOTEMVALUE_H

#include "NamedObjectContext.h"
#include "Value.h"

class PlayerbotAI;

class HasTotemValue : public BoolCalculatedValue, public Qualified
{
public:
    HasTotemValue(PlayerbotAI* botAI, std::string const name = "has totem") : BoolCalculatedValue(botAI, name) {}

    bool Calculate() override;
};

#endif
