/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_CRAFTVALUE_H
#define _PLAYERBOT_CRAFTVALUE_H

#include <map>

#include "Value.h"

class PlayerbotAI;

class CraftData
{
public:
    CraftData() : itemId(0) {}
    CraftData(CraftData const& other) : itemId(other.itemId)
    {
        required.insert(other.required.begin(), other.required.end());
        obtained.insert(other.obtained.begin(), other.obtained.end());
    }

    uint32 itemId;
    std::map<uint32, uint32> required, obtained;

    bool IsEmpty() { return itemId == 0; }
    void Reset() { itemId = 0; }
    bool IsRequired(uint32 item) { return required.find(item) != required.end(); }
    bool IsFulfilled()
    {
        for (std::map<uint32, uint32>::iterator i = required.begin(); i != required.end(); ++i)
        {
            uint32 item = i->first;
            if (obtained[item] < i->second)
                return false;
        }

        return true;
    }
    void AddObtained(uint32 itemId, uint32 count)
    {
        if (IsRequired(itemId))
        {
            obtained[itemId] += count;
        }
    }

    void Crafted(uint32 count)
    {
        for (std::map<uint32, uint32>::iterator i = required.begin(); i != required.end(); ++i)
        {
            uint32 item = i->first;
            if (obtained[item] >= required[item] * count)
            {
                obtained[item] -= required[item] * count;
            }
        }
    }
};

class CraftValue : public ManualSetValue<CraftData&>
{
public:
    CraftValue(PlayerbotAI* botAI, std::string const name = "craft") : ManualSetValue<CraftData&>(botAI, data, name) {}

private:
    CraftData data;
};

#endif
