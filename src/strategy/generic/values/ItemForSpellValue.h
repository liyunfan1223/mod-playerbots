/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ITEMFORSPELLVALUE_H
#define _PLAYERBOT_ITEMFORSPELLVALUE_H

#include "NamedObjectContext.h"
#include "Value.h"

class Item;
class PlayerbotAI;
class SpellInfo;

class ItemForSpellValue : public CalculatedValue<Item*>, public Qualified
{
public:
    ItemForSpellValue(PlayerbotAI* botAI, std::string const name = "item for spell")
        : CalculatedValue<Item*>(botAI, name, 1)
    {
    }

    Item* Calculate() override;

private:
    Item* GetItemFitsToSpellRequirements(uint8 slot, SpellInfo const* spellInfo);
};

#endif
