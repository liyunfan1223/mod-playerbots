/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_AOEHEALVALUES_H
#define _PLAYERBOT_AOEHEALVALUES_H

#include "NamedObjectContext.h"
#include "Value.h"

class PlayerbotAI;

class AoeHealValue : public Uint8CalculatedValue, public Qualified
{
public:
    AoeHealValue(PlayerbotAI* botAI, std::string const name = "aoe heal") : Uint8CalculatedValue(botAI, name) {}

    uint8 Calculate() override;
};

#endif
