/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ITEMVISITORS_H
#define _PLAYERBOT_ITEMVISITORS_H

#include "ChatHelper.h"
#include "Common.h"
#include "Item.h"
#include "ItemUsageValue.h"

class AiObjectContext;
class Player;

char* strstri(char const* str1, char const* str2);

enum IterateItemsMask : uint32
{
    ITERATE_ITEMS_IN_BAGS = 1,
    ITERATE_ITEMS_IN_EQUIP = 2,
    ITERATE_ITEMS_IN_BANK = 4,
    ITERATE_ALL_ITEMS = 255
};

class IterateItemsVisitor
{
public:
    IterateItemsVisitor() {}

    virtual bool Visit(Item* item) = 0;
};

class FindItemVisitor : public IterateItemsVisitor
{
public:
    FindItemVisitor() : IterateItemsVisitor(), result() {}

    bool Visit(Item* item) override
    {
        if (!Accept(item->GetTemplate()))
            return true;

        result.push_back(item);
        return true;
    }

    std::vector<Item*>& GetResult() { return result; }

protected:
    virtual bool Accept(ItemTemplate const* proto) = 0;

private:
    std::vector<Item*> result;
};

class FindUsableItemVisitor : public FindItemVisitor
{
public:
    FindUsableItemVisitor(Player* bot) : FindItemVisitor(), bot(bot) {}

    bool Visit(Item* item) override;

private:
    Player* bot;
};

class FindItemsByQualityVisitor : public IterateItemsVisitor
{
public:
    FindItemsByQualityVisitor(uint32 quality, uint32 count) : IterateItemsVisitor(), quality(quality), count(count) {}

    bool Visit(Item* item) override
    {
        if (item->GetTemplate()->Quality != quality)
            return true;

        if (result.size() >= (size_t)count)
            return false;

        result.push_back(item);
        return true;
    }

    std::vector<Item*>& GetResult() { return result; }

private:
    uint32 quality;
    uint32 count;
    std::vector<Item*> result;
};

class FindItemsToTradeByQualityVisitor : public FindItemsByQualityVisitor
{
public:
    FindItemsToTradeByQualityVisitor(uint32 quality, uint32 count) : FindItemsByQualityVisitor(quality, count) {}

    bool Visit(Item* item) override
    {
        if (item->IsSoulBound())
            return true;

        return FindItemsByQualityVisitor::Visit(item);
    }
};

class FindItemsToTradeByClassVisitor : public IterateItemsVisitor
{
public:
    FindItemsToTradeByClassVisitor(uint32 itemClass, uint32 itemSubClass, uint32 count)
        : IterateItemsVisitor(), itemClass(itemClass), itemSubClass(itemSubClass), count(count)
    {
    }  // reorder args - whipowill

    bool Visit(Item* item) override
    {
        if (item->IsSoulBound())
            return true;

        if (item->GetTemplate()->Class != itemClass || item->GetTemplate()->SubClass != itemSubClass)
            return true;

        if (result.size() >= (size_t)count)
            return false;

        result.push_back(item);
        return true;
    }

    std::vector<Item*>& GetResult() { return result; }

private:
    uint32 itemClass;
    uint32 itemSubClass;
    uint32 count;
    std::vector<Item*> result;
};

class QueryItemCountVisitor : public IterateItemsVisitor
{
public:
    QueryItemCountVisitor(uint32 itemId) : count(0), itemId(itemId) {}

    bool Visit(Item* item) override
    {
        if (item->GetTemplate()->ItemId == itemId)
            count += item->GetCount();

        return true;
    }

    uint32 GetCount() { return count; }

protected:
    uint32 count;
    uint32 itemId;
};

class QueryNamedItemCountVisitor : public QueryItemCountVisitor
{
public:
    QueryNamedItemCountVisitor(std::string const name) : QueryItemCountVisitor(0), name(name) {}

    bool Visit(Item* item) override
    {
        ItemTemplate const* proto = item->GetTemplate();
        if (proto && proto->Name1.c_str() && strstri(proto->Name1.c_str(), name.c_str()))
            count += item->GetCount();

        return true;
    }

private:
    std::string const name;
};

class FindNamedItemVisitor : public FindItemVisitor
{
public:
    FindNamedItemVisitor([[maybe_unused]] Player* bot, std::string const name) : FindItemVisitor(), name(name) {}

    bool Accept(ItemTemplate const* proto) override
    {
        return proto && proto->Name1.c_str() && strstri(proto->Name1.c_str(), name.c_str());
    }

private:
    std::string const name;
};

class FindItemByIdVisitor : public FindItemVisitor
{
public:
    FindItemByIdVisitor(uint32 id) : FindItemVisitor(), id(id) {}

    bool Accept(ItemTemplate const* proto) override { return proto->ItemId == id; }

private:
    uint32 id;
};

class FindItemByIdsVisitor : public FindItemVisitor
{
public:
    FindItemByIdsVisitor(ItemIds ids) : FindItemVisitor(), ids(ids) {}

    bool Accept(ItemTemplate const* proto) override { return ids.find(proto->ItemId) != ids.end(); }

private:
    ItemIds ids;
};

class ListItemsVisitor : public IterateItemsVisitor
{
public:
    ListItemsVisitor() : IterateItemsVisitor() {}

    std::map<uint32, uint32> items;
    std::map<uint32, bool> soulbound;

    bool Visit(Item* item) override
    {
        uint32 id = item->GetTemplate()->ItemId;

        if (items.find(id) == items.end())
            items[id] = 0;

        items[id] += item->GetCount();
        soulbound[id] = item->IsSoulBound();
        return true;
    }
};

class CollectItemsVisitor : public IterateItemsVisitor
{
public:
    CollectItemsVisitor() : IterateItemsVisitor() {}

    std::vector<Item*> items;

    bool Visit(Item* item) override
    {
        items.push_back(item);
        return true;
    }
};

class ItemCountByQuality : public IterateItemsVisitor
{
public:
    ItemCountByQuality() : IterateItemsVisitor()
    {
        for (uint32 i = 0; i < MAX_ITEM_QUALITY; ++i)
            count[i] = 0;
    }

    bool Visit(Item* item) override
    {
        ++count[item->GetTemplate()->Quality];
        return true;
    }

public:
    std::map<uint32, uint32> count;
};

class FindPotionVisitor : public FindUsableItemVisitor
{
public:
    FindPotionVisitor(Player* bot, uint32 effectId) : FindUsableItemVisitor(bot), effectId(effectId) {}

    bool Accept(ItemTemplate const* proto) override;

private:
    uint32 effectId;
};

class FindFoodVisitor : public FindUsableItemVisitor
{
public:
    FindFoodVisitor(Player* bot, uint32 spellCategory, bool conjured = false)
        : FindUsableItemVisitor(bot), spellCategory(spellCategory), conjured(conjured)
    {
    }

    bool Accept(ItemTemplate const* proto) override
    {
        return proto->Class == ITEM_CLASS_CONSUMABLE &&
               (proto->SubClass == ITEM_SUBCLASS_CONSUMABLE || proto->SubClass == ITEM_SUBCLASS_FOOD) &&
               proto->Spells[0].SpellCategory == spellCategory && (!conjured || proto->IsConjuredConsumable());
    }

private:
    uint32 spellCategory;
    bool conjured;
};

class FindMountVisitor : public FindUsableItemVisitor
{
public:
    FindMountVisitor(Player* bot) : FindUsableItemVisitor(bot) {}

    bool Accept(ItemTemplate const* proto) override;

private:
    uint32 effectId;
};

class FindPetVisitor : public FindUsableItemVisitor
{
public:
    FindPetVisitor(Player* bot) : FindUsableItemVisitor(bot) {}

    bool Accept(ItemTemplate const* proto) override;
};

class FindAmmoVisitor : public FindUsableItemVisitor
{
public:
    FindAmmoVisitor(Player* bot, uint32 weaponType) : FindUsableItemVisitor(bot), weaponType(weaponType) {}

    bool Accept(ItemTemplate const* proto) override
    {
        if (proto->Class == ITEM_CLASS_PROJECTILE)
        {
            uint32 subClass = 0;
            switch (weaponType)
            {
                case ITEM_SUBCLASS_WEAPON_GUN:
                    subClass = ITEM_SUBCLASS_BULLET;
                    break;
                case ITEM_SUBCLASS_WEAPON_BOW:
                case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                    subClass = ITEM_SUBCLASS_ARROW;
                    break;
            }

            if (!subClass)
                return false;

            if (proto->SubClass == subClass)
                return true;
        }

        return false;
    }

private:
    uint32 weaponType;
};

class FindQuestItemVisitor : public FindUsableItemVisitor
{
public:
    FindQuestItemVisitor(Player* bot) : FindUsableItemVisitor(bot) {}

    bool Accept(ItemTemplate const* proto) override
    {
        if (proto->Class == ITEM_CLASS_QUEST)
        {
            return true;
        }
        return false;
    }
};

class FindRecipeVisitor : public FindUsableItemVisitor
{
public:
    FindRecipeVisitor(Player* bot, SkillType skill = SKILL_NONE) : FindUsableItemVisitor(bot), skill(skill){};

    bool Accept(ItemTemplate const* proto) override
    {
        if (proto->Class == ITEM_CLASS_RECIPE)
        {
            if (skill == SKILL_NONE)
                return true;

            switch (proto->SubClass)
            {
                case ITEM_SUBCLASS_LEATHERWORKING_PATTERN:
                    return skill == SKILL_LEATHERWORKING;
                case ITEM_SUBCLASS_TAILORING_PATTERN:
                    return skill == SKILL_TAILORING;
                case ITEM_SUBCLASS_ENGINEERING_SCHEMATIC:
                    return skill == SKILL_ENGINEERING;
                case ITEM_SUBCLASS_BLACKSMITHING:
                    return skill == SKILL_BLACKSMITHING;
                case ITEM_SUBCLASS_COOKING_RECIPE:
                    return skill == SKILL_COOKING;
                case ITEM_SUBCLASS_ALCHEMY_RECIPE:
                    return skill == SKILL_ALCHEMY;
                case ITEM_SUBCLASS_FIRST_AID_MANUAL:
                    return skill == SKILL_FIRST_AID;
                case ITEM_SUBCLASS_ENCHANTING_FORMULA:
                    return skill == SKILL_ENCHANTING;
                case ITEM_SUBCLASS_FISHING_MANUAL:
                    return skill == SKILL_FISHING;
            }
        }

        return false;
    }

private:
    SkillType skill;
};

class FindItemUsageVisitor : public FindUsableItemVisitor
{
public:
    FindItemUsageVisitor(Player* bot, ItemUsage usage = ITEM_USAGE_NONE);

    bool Accept(ItemTemplate const* proto) override;

private:
    AiObjectContext* context;
    ItemUsage usage;
};

class FindUsableNamedItemVisitor : public FindUsableItemVisitor
{
public:
    FindUsableNamedItemVisitor(Player* bot, std::string name) : FindUsableItemVisitor(bot), name(name) {}

    bool Accept(ItemTemplate const* proto) override;

private:
    std::string name;
};
#endif
