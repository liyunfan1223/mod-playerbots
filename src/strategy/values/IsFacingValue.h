/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ISFACINGVALUE_H
#define _PLAYERBOT_ISFACINGVALUE_H

#include "NamedObjectContext.h"
#include "Value.h"

class PlayerbotAI;

class IsFacingValue : public BoolCalculatedValue, public Qualified
{
public:
    IsFacingValue(PlayerbotAI* botAI, std::string const name = "is facing") : BoolCalculatedValue(botAI, name) {}

    bool Calculate() override;
};

#endif
