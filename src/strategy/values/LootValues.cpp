/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "LootValues.h"

#include "Playerbots.h"
#include "SharedValueContext.h"

LootTemplateAccess const* DropMapValue::GetLootTemplate(ObjectGuid guid, LootType type)
{
    LootTemplate const* lTemplate = nullptr;

    if (guid.IsCreature())
    {
        CreatureTemplate const* info = sObjectMgr->GetCreatureTemplate(guid.GetEntry());

        if (info)
        {
            if (type == LOOT_CORPSE)
                lTemplate = LootTemplates_Creature.GetLootFor(info->lootid);
            else if (type == LOOT_PICKPOCKETING && info->pickpocketLootId)
                lTemplate = LootTemplates_Pickpocketing.GetLootFor(info->pickpocketLootId);
            else if (type == LOOT_SKINNING && info->SkinLootId)
                lTemplate = LootTemplates_Skinning.GetLootFor(info->SkinLootId);
        }
    }
    else if (guid.IsGameObject())
    {
        GameObjectTemplate const* info = sObjectMgr->GetGameObjectTemplate(guid.GetEntry());
        if (info && info->GetLootId() != 0)
        {
            if (type == LOOT_CORPSE)
                lTemplate = LootTemplates_Gameobject.GetLootFor(info->GetLootId());
            else if (type == LOOT_FISHINGHOLE)
                lTemplate = LootTemplates_Fishing.GetLootFor(info->GetLootId());
        }
    }
    else if (guid.IsItem())
    {
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(guid.GetEntry());

        if (proto)
        {
            if (type == LOOT_CORPSE)
                lTemplate = LootTemplates_Item.GetLootFor(proto->ItemId);
            else if (type == LOOT_DISENCHANTING && proto->DisenchantID)
                lTemplate = LootTemplates_Disenchant.GetLootFor(proto->DisenchantID);
            if (type == LOOT_MILLING)
                lTemplate = LootTemplates_Milling.GetLootFor(proto->ItemId);
            if (type == LOOT_PROSPECTING)
                lTemplate = LootTemplates_Prospecting.GetLootFor(proto->ItemId);
        }
    }

    LootTemplateAccess const* lTemplateA = reinterpret_cast<LootTemplateAccess const*>(lTemplate);

    return lTemplateA;
}

DropMap* DropMapValue::Calculate()
{
    DropMap* dropMap = new DropMap;

    int32 sEntry = 0;

    if (CreatureTemplateContainer const* creatures = sObjectMgr->GetCreatureTemplates())
    {
        for (CreatureTemplateContainer::const_iterator itr = creatures->begin(); itr != creatures->end(); ++itr)
        {
            sEntry = itr->first;

            if (LootTemplateAccess const* lTemplateA =
                    GetLootTemplate(ObjectGuid::Create<HighGuid::Unit>(sEntry, uint32(1)), LOOT_CORPSE))
                for (auto const& lItem : lTemplateA->Entries)
                    dropMap->insert(std::make_pair(lItem->itemid, sEntry));
        }
    }

    if (GameObjectTemplateContainer const* gameobjects = sObjectMgr->GetGameObjectTemplates())
    {
        for (auto const& itr : *gameobjects)
        {
            sEntry = itr.first;

            if (LootTemplateAccess const* lTemplateA =
                    GetLootTemplate(ObjectGuid::Create<HighGuid::GameObject>(sEntry, uint32(1)), LOOT_CORPSE))
                for (auto const& lItem : lTemplateA->Entries)
                    dropMap->insert(std::make_pair(lItem->itemid, -sEntry));
        }
    }

    return dropMap;
}

// What items does this entry have in its loot list?
std::vector<int32> ItemDropListValue::Calculate()
{
    uint32 itemId = stoi(getQualifier());

    DropMap* dropMap = GAI_VALUE(DropMap*, "drop map");

    std::vector<int32> entries;

    auto range = dropMap->equal_range(itemId);

    for (auto itr = range.first; itr != range.second; ++itr)
        entries.push_back(itr->second);

    return entries;
}

// What items does this entry have in its loot list?
std::vector<uint32> EntryLootListValue::Calculate()
{
    int32 entry = stoi(getQualifier());

    std::vector<uint32> items;

    LootTemplateAccess const* lTemplateA;

    if (entry > 0)
        lTemplateA = DropMapValue::GetLootTemplate(ObjectGuid::Create<HighGuid::Unit>(entry, uint32(1)), LOOT_CORPSE);
    else
        lTemplateA =
            DropMapValue::GetLootTemplate(ObjectGuid::Create<HighGuid::GameObject>(entry, uint32(1)), LOOT_CORPSE);

    if (lTemplateA)
        for (auto const& lItem : lTemplateA->Entries)
            items.push_back(lItem->itemid);

    return items;
}

itemUsageMap EntryLootUsageValue::Calculate()
{
    itemUsageMap items;

    for (auto itemId : GAI_VALUE2(std::vector<uint32>, "entry loot list", getQualifier()))
    {
        items[AI_VALUE2(ItemUsage, "item usage", itemId)].push_back(itemId);
    }

    return items;
};

bool HasUpgradeValue::Calculate()
{
    itemUsageMap uMap = AI_VALUE2(itemUsageMap, "entry loot usage", getQualifier());
    return uMap.find(ITEM_USAGE_EQUIP) != uMap.end() || uMap.find(ITEM_USAGE_REPLACE) != uMap.end();
}
