/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "InventoryAction.h"
#include "Event.h"
#include "ItemCountValue.h"
#include "ItemVisitors.h"
#include "Playerbots.h"

namespace
{
bool isReservedQualifier(std::string const& text)
{
    static std::array<std::string_view, 14> const exactQualifiers = {
        "ammo",
        "conjured drink",
        "conjured food",
        "conjured water",
        "drink",
        "food",
        "healing potion",
        "materials",
        "mount",
        "mana potion",
        "pet",
        "quest",
        "recipe",
        "water"
    };

    return std::find(exactQualifiers.begin(), exactQualifiers.end(), text) != exactQualifiers.end() ||
        text.rfind("usage ", 0) == 0;
}
}

void InventoryAction::IterateItems(IterateItemsVisitor* visitor, IterateItemsMask mask)
{
    if (mask & ITERATE_ITEMS_IN_BAGS)
        IterateItemsInBags(visitor);

    if (mask & ITERATE_ITEMS_IN_EQUIP)
        IterateItemsInEquip(visitor);

    if (mask == ITERATE_ITEMS_IN_BANK)
        IterateItemsInBank(visitor);
}

void InventoryAction::IterateItemsInBags(IterateItemsVisitor* visitor)
{
    for (uint32 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
        if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            if (!visitor->Visit(pItem))
                return;

    for (uint32 i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; ++i)
        if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            if (!visitor->Visit(pItem))
                return;

    for (uint32 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
        if (Bag* pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
                if (Item* pItem = pBag->GetItemByPos(j))
                    if (!visitor->Visit(pItem))
                        return;
}

void InventoryAction::IterateItemsInEquip(IterateItemsVisitor* visitor)
{
    for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; slot++)
    {
        Item* const pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (!pItem)
            continue;

        if (!visitor->Visit(pItem))
            return;
    }
}

void InventoryAction::IterateItemsInBank(IterateItemsVisitor* visitor)
{
    for (uint8 slot = BANK_SLOT_ITEM_START; slot < BANK_SLOT_ITEM_END; slot++)
    {
        Item* const pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (!pItem)
            continue;

        if (!visitor->Visit(pItem))
            return;
    }

    for (uint32 i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
    {
        if (Bag* pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            if (pBag)
            {
                for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
                {
                    if (Item* pItem = pBag->GetItemByPos(j))
                    {
                        if (!pItem)
                            continue;

                        if (!visitor->Visit(pItem))
                            return;
                    }
                }
            }
        }
    }
}

bool compare_items(ItemTemplate const* proto1, ItemTemplate const* proto2)
{
    if (proto1->Class != proto2->Class)
        return proto1->Class > proto2->Class;

    if (proto1->SubClass != proto2->SubClass)
        return proto1->SubClass < proto2->SubClass;

    if (proto1->Quality != proto2->Quality)
        return proto1->Quality < proto2->Quality;

    if (proto1->ItemLevel != proto2->ItemLevel)
        return proto1->ItemLevel > proto2->ItemLevel;

    return false;
}

bool compare_items_by_level(Item const* item1, Item const* item2)
{
    return compare_items(item1->GetTemplate(), item2->GetTemplate());
}

void InventoryAction::TellItems(std::map<uint32, uint32> itemMap, std::map<uint32, bool> soulbound)
{
    std::vector<ItemTemplate const*> items;
    for (std::map<uint32, uint32>::iterator i = itemMap.begin(); i != itemMap.end(); i++)
    {
        items.push_back(sObjectMgr->GetItemTemplate(i->first));
    }

    std::sort(items.begin(), items.end(), compare_items);

    uint32 oldClass = -1;
    for (ItemTemplate const* proto : items)
    {
        if (proto->Class != oldClass)
        {
            oldClass = proto->Class;
            switch (proto->Class)
            {
                case ITEM_CLASS_CONSUMABLE:
                    botAI->TellMaster("--- consumable ---");
                    break;
                case ITEM_CLASS_CONTAINER:
                    botAI->TellMaster("--- container ---");
                    break;
                case ITEM_CLASS_WEAPON:
                    botAI->TellMaster("--- weapon ---");
                    break;
                case ITEM_CLASS_ARMOR:
                    botAI->TellMaster("--- armor ---");
                    break;
                case ITEM_CLASS_REAGENT:
                    botAI->TellMaster("--- reagent ---");
                    break;
                case ITEM_CLASS_PROJECTILE:
                    botAI->TellMaster("--- projectile ---");
                    break;
                case ITEM_CLASS_TRADE_GOODS:
                    botAI->TellMaster("--- trade goods ---");
                    break;
                case ITEM_CLASS_RECIPE:
                    botAI->TellMaster("--- recipe ---");
                    break;
                case ITEM_CLASS_QUIVER:
                    botAI->TellMaster("--- quiver ---");
                    break;
                case ITEM_CLASS_QUEST:
                    botAI->TellMaster("--- quest items ---");
                    break;
                case ITEM_CLASS_KEY:
                    botAI->TellMaster("--- keys ---");
                    break;
                case ITEM_CLASS_MISC:
                    botAI->TellMaster("--- other ---");
                    break;
            }
        }

        TellItem(proto, itemMap[proto->ItemId], soulbound[proto->ItemId]);
    }
}

void InventoryAction::TellItem(ItemTemplate const* proto, uint32 count, bool soulbound)
{
    std::ostringstream out;
    out << chat->FormatItem(proto, count);
    if (soulbound)
        out << " (soulbound)";

    botAI->TellMaster(out.str());
}

std::vector<Item*> InventoryAction::parseItems(std::string const text, IterateItemsMask mask)
{
    std::set<Item*> found;
    size_t pos = text.find(" ");

    int count = pos != std::string::npos ? atoi(text.substr(pos + 1).c_str()) : TRADE_SLOT_TRADED_COUNT;
    if (count < 1)
        count = 1;
    else if (count > TRADE_SLOT_TRADED_COUNT)
        count = TRADE_SLOT_TRADED_COUNT;

    ItemIds ids = chat->parseItems(text);
    if (!ids.empty())
    {
        for (ItemIds::iterator i = ids.begin(); i != ids.end(); i++)
        {
            FindItemByIdVisitor visitor(*i);
            IterateItems(&visitor, mask);
            found.insert(visitor.GetResult().begin(), visitor.GetResult().end());
        }

        std::vector<Item*> result;
        for (std::set<Item*>::iterator i = found.begin(); i != found.end(); ++i)
            result.push_back(*i);

        std::sort(result.begin(), result.end(), compare_items_by_level);

        return result;
    }

    if (text == "food" || text == "conjured food")
    {
        FindFoodVisitor visitor(bot, 11, text == "conjured food");
        IterateItems(&visitor, ITERATE_ITEMS_IN_BAGS);
        found.insert(visitor.GetResult().begin(), visitor.GetResult().end());
    }

    if (text == "drink" || text == "water" || text == "conjured drink" || text == "conjured water")
    {
        FindFoodVisitor visitor(bot, 59, text == "conjured drink" || text == "conjured water");
        IterateItems(&visitor, ITERATE_ITEMS_IN_BAGS);
        found.insert(visitor.GetResult().begin(), visitor.GetResult().end());

        if (found.empty())
        {
            FindFoodVisitor visitor(bot, 11);
            IterateItems(&visitor, ITERATE_ITEMS_IN_BAGS);
            found.insert(visitor.GetResult().begin(), visitor.GetResult().end());
        }
    }

    if (text == "mana potion")
    {
        FindPotionVisitor visitor(bot, SPELL_EFFECT_ENERGIZE);
        IterateItems(&visitor, ITERATE_ITEMS_IN_BAGS);
        found.insert(visitor.GetResult().begin(), visitor.GetResult().end());
    }

    if (text == "healing potion")
    {
        FindPotionVisitor visitor(bot, SPELL_EFFECT_HEAL);
        IterateItems(&visitor, ITERATE_ITEMS_IN_BAGS);
        found.insert(visitor.GetResult().begin(), visitor.GetResult().end());
    }

    if (text == "mount")
    {
        FindMountVisitor visitor(bot);
        IterateItems(&visitor, ITERATE_ITEMS_IN_BAGS);
        found.insert(visitor.GetResult().begin(), visitor.GetResult().end());
    }

    if (text == "pet")
    {
        FindPetVisitor visitor(bot);
        IterateItems(&visitor, ITERATE_ITEMS_IN_BAGS);
        found.insert(visitor.GetResult().begin(), visitor.GetResult().end());
    }

    if (text == "ammo")
    {
        if (Item* const pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED))
        {
            FindAmmoVisitor visitor(bot, pItem->GetTemplate()->SubClass);
            IterateItems(&visitor, ITERATE_ITEMS_IN_BAGS);
            found.insert(visitor.GetResult().begin(), visitor.GetResult().end());
        }
    }

    if (text == "recipe")
    {
        FindRecipeVisitor visitor(bot);
        IterateItems(&visitor, ITERATE_ITEMS_IN_BAGS);
        found.insert(visitor.GetResult().begin(), visitor.GetResult().end());
    }

    // "recipe" keeps the usable-only filter (used by the bot's own recipe-learning);
    // "recipe all" matches every recipe in the bags, for moving/trading them in bulk.
    if (text == "recipe all")
    {
        FindAnyRecipeVisitor visitor;
        IterateItems(&visitor, ITERATE_ITEMS_IN_BAGS);
        found.insert(visitor.GetResult().begin(), visitor.GetResult().end());
    }

    if (text == "quest")
    {
        FindQuestItemVisitor visitor(bot);
        IterateItems(&visitor, ITERATE_ITEMS_IN_BAGS);
        found.insert(visitor.GetResult().begin(), visitor.GetResult().end());
    }

    if (text == "materials")
    {
        FindTradeMaterialsVisitor visitor(count);
        IterateItems(&visitor, mask);
        found.insert(visitor.GetResult().begin(), visitor.GetResult().end());
    }

    // "usage <n>" is only meaningful at the start of the qualifier, which is how
    // isReservedQualifier() reads it. Matching it anywhere handed stoi() the text at a fixed
    // offset instead of the number, so "bandage usage 3" ended up in stoi("ge usage 3").
    // At most nine digits so the value always fits an int.
    if (text.rfind("usage ", 0) == 0)
    {
        std::string const usage = text.substr(6, text.find(' ', 6) - 6);
        if (!usage.empty() && usage.size() <= 9 && usage.find_first_not_of("0123456789") == std::string::npos)
        {
            FindItemUsageVisitor visitor(bot, ItemUsage(stoi(usage)));
            IterateItems(&visitor, ITERATE_ITEMS_IN_BAGS);
            found.insert(visitor.GetResult().begin(), visitor.GetResult().end());
        }
    }

    if (!isReservedQualifier(text))
    {
        FindNamedItemVisitor visitor(bot, text);
        IterateItems(&visitor, ITERATE_ITEMS_IN_BAGS);
        found.insert(visitor.GetResult().begin(), visitor.GetResult().end());
    }

    uint32 quality = chat->parseItemQuality(text);
    if (quality != MAX_ITEM_QUALITY)
    {
        FindItemsToTradeByQualityVisitor visitor(quality, count);
        IterateItems(&visitor, mask);
        found.insert(visitor.GetResult().begin(), visitor.GetResult().end());
    }

    uint32 itemClass = MAX_ITEM_CLASS, itemSubClass = 0;
    if (chat->parseItemClass(text, &itemClass, &itemSubClass))
    {
        FindItemsToTradeByClassVisitor visitor(itemClass, itemSubClass, count);
        IterateItems(&visitor, mask);
        found.insert(visitor.GetResult().begin(), visitor.GetResult().end());
    }

    uint32 fromSlot = chat->parseSlot(text);
    if (fromSlot != EQUIPMENT_SLOT_END)
    {
        if (Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, fromSlot))
            found.insert(item);
    }

    ItemIds outfit = FindOutfitItems(text);
    if (!outfit.empty())
    {
        FindItemByIdsVisitor visitor(outfit);
        IterateItems(&visitor, mask);
        found.insert(visitor.GetResult().begin(), visitor.GetResult().end());
    }

    ids = chat->parseItems(text);
    for (ItemIds::iterator i = ids.begin(); i != ids.end(); i++)
    {
        FindItemByIdVisitor visitor(*i);
        IterateItems(&visitor, mask);
        found.insert(visitor.GetResult().begin(), visitor.GetResult().end());
    }

    std::vector<Item*> result;
    for (std::set<Item*>::iterator i = found.begin(); i != found.end(); ++i)
        result.push_back(*i);

    std::sort(result.begin(), result.end(), compare_items_by_level);

    return result;
}

uint32 InventoryAction::GetItemCount(FindItemVisitor* visitor, IterateItemsMask mask)
{
    IterateItems(visitor, mask);
    uint32 count = 0;

    std::vector<Item*>& items = visitor->GetResult();
    for (Item* item : items)
        count += item->GetCount();

    return count;
}

ItemIds InventoryAction::FindOutfitItems(std::string const name)
{
    std::vector<std::string>& outfits = AI_VALUE(std::vector<std::string>&, "outfit list");
    for (std::vector<std::string>::iterator i = outfits.begin(); i != outfits.end(); ++i)
    {
        std::string const outfit = *i;
        if (name == parseOutfitName(outfit))
            return parseOutfitItems(outfit);
    }

    return std::set<uint32>();
}

std::string const InventoryAction::parseOutfitName(std::string const outfit)
{
    uint32 pos = outfit.find("=");
    if (pos == uint32(-1))
        return "";

    return outfit.substr(0, pos);
}

ItemIds InventoryAction::parseOutfitItems(std::string const text)
{
    ItemIds itemIds;

    uint8 pos = text.find("=") + 1;
    while (pos < text.size())
    {
        uint32 endPos = text.find(',', pos);
        if (endPos == uint32(-1))
            endPos = text.size();

        std::string const idC = text.substr(pos, endPos - pos);
        uint32 id = atol(idC.c_str());
        pos = endPos + 1;
        if (id)
            itemIds.insert(id);
    }

    return itemIds;
}
