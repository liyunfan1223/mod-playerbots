/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_LOOTOBJECTSTACK_H
#define _PLAYERBOT_LOOTOBJECTSTACK_H

#include "ObjectGuid.h"

class AiObjectContext;
class Player;
class WorldObject;

struct ItemTemplate;

class LootStrategy
{
public:
    LootStrategy() {}
    virtual ~LootStrategy(){};
    virtual bool CanLoot(ItemTemplate const* proto, AiObjectContext* context) = 0;
    virtual std::string const GetName() = 0;
};

class LootObject
{
public:
    LootObject() : skillId(0), reqSkillValue(0), reqItem(0) {}
    LootObject(Player* bot, ObjectGuid guid);
    LootObject(LootObject const& other);

    bool IsEmpty() { return !guid; }
    bool IsLootPossible(Player* bot);
    void Refresh(Player* bot, ObjectGuid guid);
    WorldObject* GetWorldObject(Player* bot);
    ObjectGuid guid;

    uint32 skillId;
    uint32 reqSkillValue;
    uint32 reqItem;

private:
    static bool IsNeededForQuest(Player* bot, uint32 itemId);
};

class LootTarget
{
public:
    LootTarget(ObjectGuid guid);
    LootTarget(LootTarget const& other);

public:
    LootTarget& operator=(LootTarget const& other);
    bool operator<(LootTarget const& other) const;

public:
    ObjectGuid guid;
    time_t asOfTime;
};

class LootTargetList : public std::set<LootTarget>
{
public:
    void shrink(time_t fromTime);
};

class LootObjectStack
{
public:
    LootObjectStack(Player* bot) : bot(bot) {}

    bool Add(ObjectGuid guid);
    void Remove(ObjectGuid guid);
    void Clear();
    bool CanLoot(float maxDistance);
    LootObject GetLoot(float maxDistance = 0);

private:
    std::vector<LootObject> OrderByDistance(float maxDistance = 0);

    Player* bot;
    LootTargetList availableLoot;
};

#endif
