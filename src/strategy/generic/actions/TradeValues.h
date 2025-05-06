/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_TRADEVALUES_H
#define _PLAYERBOT_TRADEVALUES_H

#include "NamedObjectContext.h"
#include "Value.h"

class Item;
class PlayerbotAI;

class ItemsUsefulToGiveValue : public CalculatedValue<std::vector<Item*>>, public Qualified
{
public:
    ItemsUsefulToGiveValue(PlayerbotAI* botAI, std::string const name = "useful to give") : CalculatedValue(botAI, name)
    {
    }

    std::vector<Item*> Calculate();
};

#endif
