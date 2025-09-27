/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_INVALIDTARGETVALUE_H
#define _PLAYERBOT_INVALIDTARGETVALUE_H

#include "NamedObjectContext.h"
#include "Value.h"

class PlayerbotAI;

class InvalidTargetValue : public BoolCalculatedValue, public Qualified
{
public:
    InvalidTargetValue(PlayerbotAI* botAI, std::string const name = "invalid target") : BoolCalculatedValue(botAI, name)
    {
    }

    bool Calculate() override;
};

#endif
