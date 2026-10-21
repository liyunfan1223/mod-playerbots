/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CRAFTVALUE_H
#define PLAYERBOTS_CRAFTVALUE_H

#include "CraftRules.h"
#include "ObjectGuid.h"
#include "Value.h"

class PlayerbotAI;

// A crafting order from the bot's master. The customer supplies every reagent.
class CraftData : public CraftRules::OrderState
{
public:
    ObjectGuid owner;

    void Reset()
    {
        Clear();
        owner.Clear();
    }
    bool IsCustomer(ObjectGuid guid) const { return !IsEmpty() && guid == owner; }
};

class CraftValue : public ManualSetValue<CraftData&>
{
public:
    CraftValue(PlayerbotAI* botAI, std::string const name = "craft") : ManualSetValue<CraftData&>(botAI, data, name) {}

private:
    CraftData data;
};

#endif
