/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "LootStrategyValue.h"

#include "AiObjectContext.h"
#include "ItemUsageValue.h"
#include "LootObjectStack.h"
#include "Playerbots.h"

class NormalLootStrategy : public LootStrategy
{
public:
    bool CanLoot(ItemTemplate const* proto, AiObjectContext* context) override
    {
        std::ostringstream out;
        out << proto->ItemId;
        ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", out.str());
        return usage != ITEM_USAGE_NONE;
    }

    std::string const GetName() override { return "normal"; }
};

class GrayLootStrategy : public NormalLootStrategy
{
public:
    bool CanLoot(ItemTemplate const* proto, AiObjectContext* context) override
    {
        return NormalLootStrategy::CanLoot(proto, context) || proto->Quality == ITEM_QUALITY_POOR;
    }

    std::string const GetName() override { return "gray"; }
};

class DisenchantLootStrategy : public NormalLootStrategy
{
public:
    bool CanLoot(ItemTemplate const* proto, AiObjectContext* context) override
    {
        return NormalLootStrategy::CanLoot(proto, context) ||
               (proto->Quality >= ITEM_QUALITY_UNCOMMON && proto->Bonding != BIND_WHEN_PICKED_UP &&
                (proto->Class == ITEM_CLASS_ARMOR || proto->Class == ITEM_CLASS_WEAPON));
    }

    std::string const GetName() override { return "disenchant"; }
};

class AllLootStrategy : public LootStrategy
{
public:
    bool CanLoot(ItemTemplate const* proto, AiObjectContext* context) override { return true; }

    std::string const GetName() override { return "all"; }
};

LootStrategyValue::~LootStrategyValue()
{
    // delete defaultValue;
}

LootStrategy* LootStrategyValue::normal = new NormalLootStrategy();
LootStrategy* LootStrategyValue::gray = new GrayLootStrategy();
LootStrategy* LootStrategyValue::disenchant = new DisenchantLootStrategy();
LootStrategy* LootStrategyValue::all = new AllLootStrategy();

LootStrategy* LootStrategyValue::instance(std::string const strategy)
{
    if (strategy == "*" || strategy == "all")
        return all;

    if (strategy == "g" || strategy == "gray")
        return gray;

    if (strategy == "d" || strategy == "e" || strategy == "disenchant" || strategy == "enchant")
        return disenchant;

    return normal;
}

std::string const LootStrategyValue::Save() { return value ? value->GetName() : "?"; }

bool LootStrategyValue::Load(std::string const text)
{
    value = LootStrategyValue::instance(text);
    return true;
}
