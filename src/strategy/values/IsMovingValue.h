/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ISMOVINGVALUE_H
#define _PLAYERBOT_ISMOVINGVALUE_H

#include "NamedObjectContext.h"
#include "Value.h"

class PlayerbotAI;

class IsMovingValue : public BoolCalculatedValue, public Qualified
{
public:
    IsMovingValue(PlayerbotAI* botAI, std::string const name = "is moving") : BoolCalculatedValue(botAI, name) {}

    bool Calculate() override;
};

class IsSwimmingValue : public BoolCalculatedValue, public Qualified
{
public:
    IsSwimmingValue(PlayerbotAI* botAI, std::string const name = "is swimming") : BoolCalculatedValue(botAI, name) {}

    bool Calculate() override;
};

#endif
