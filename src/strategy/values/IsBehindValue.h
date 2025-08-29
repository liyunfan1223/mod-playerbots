/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ISBEHINDVALUE_H
#define _PLAYERBOT_ISBEHINDVALUE_H

#include "NamedObjectContext.h"
#include "Value.h"

class PlayerbotAI;

class IsBehindValue : public BoolCalculatedValue, public Qualified
{
public:
    IsBehindValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI) {}

    bool Calculate() override;
};

#endif
