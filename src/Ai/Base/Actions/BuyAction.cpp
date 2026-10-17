/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BuyAction.h"
#include "BudgetValues.h"
#include "DBCStores.h"
#include "Event.h"
#include "ItemCountValue.h"
#include "ItemUsageValue.h"
#include "ItemVisitors.h"
#include "Playerbots.h"
#include "StatsWeightCalculator.h"

bool BuyAction::Execute(Event event)
{
    bool buyUseful = false;
    ItemIds itemIds;
    std::string const link = event.getParam();

    if (link == "vendor")
        buyUseful = true;
    else
    {
        itemIds = chat->parseItems(link);
    }

    GuidVector vendors = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();

    bool vendored = false;
    bool result = false;
    for (GuidVector::iterator i = vendors.begin(); i != vendors.end(); ++i)
    {
        ObjectGuid vendorguid = *i;
        Creature* pCreature = bot->GetNPCIfCanInteractWith(vendorguid, UNIT_NPC_FLAG_VENDOR);
        if (!pCreature)
            continue;

        vendored = true;

        if (buyUseful)
        {
            // Items are evaluated from high-level to low level.
            // For each item the bot checks again if an item is usefull.
            // Bot will buy until no usefull items are left.

            VendorItemData const* tItems = pCreature->GetVendorItems();
            if (!tItems)
                continue;

            VendorItemList m_items_sorted = tItems->m_items;

            m_items_sorted.erase(std::remove_if(m_items_sorted.begin(), m_items_sorted.end(),
                                                [](VendorItem* i)
                                                {
                                                    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(i->item);
                                                    return !proto;
                                                }),
                                 m_items_sorted.end());

            if (m_items_sorted.empty())
                continue;

            StatsWeightCalculator calculator(bot);
            calculator.SetItemSetBonus(false);
            calculator.SetOverflowPenalty(false);

            std::sort(m_items_sorted.begin(), m_items_sorted.end(),
                [&calculator](VendorItem* i, VendorItem* j)
                {
                    ItemTemplate const* item1 = sObjectMgr->GetItemTemplate(i->item);
                    ItemTemplate const* item2 = sObjectMgr->GetItemTemplate(j->item);

                    if (!item1 || !item2)
                        return false;

                    float score1 = calculator.CalculateItem(item1->ItemId);
                    float score2 = calculator.CalculateItem(item2->ItemId);

                    // Fallback to itemlevel if either score is 0
                    if (score1 == 0 || score2 == 0)
                    {
                        score1 = item1->ItemLevel;
                        score2 = item2->ItemLevel;
                    }
                    return score1 > score2; // Sort in descending order (highest score first)
                });

            std::unordered_map<uint32, float> bestPurchasedItemScore;  // Track best item score per InventoryType

            for (auto& tItem : m_items_sorted)
            {
                uint32 maxPurchases = 1;  // Default to buying once
                ItemTemplate const* proto = sObjectMgr->GetItemTemplate(tItem->item);
                if (!proto)
                    continue;

                // Skip items that are out of stock or that cost currencies (honor/arena/tokens) the bot lacks.
                if (!CanAfford(tItem, proto, pCreature))
                    continue;

                if (proto->Class == ITEM_CLASS_CONSUMABLE || proto->Class == ITEM_CLASS_PROJECTILE)
                {
                    maxPurchases = 10;  // Allow up to 10 purchases if it's a consumable or projectile
                }

                for (uint32 i = 0; i < maxPurchases; i++)
                {
                    ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", tItem->item);

                    uint32 invType = proto->InventoryType;

                    // Calculate item score
                    float newScore = calculator.CalculateItem(proto->ItemId);

                    // Skip if we already bought a better item for this slot
                    if (bestPurchasedItemScore.find(invType) != bestPurchasedItemScore.end() &&
                        bestPurchasedItemScore[invType] > newScore)
                    {
                        break;  // Skip lower-scoring items
                    }

                    // Check the bot's currently equipped item for this slot
                    uint8 dstSlot = botAI->FindEquipSlot(proto, NULL_SLOT, true);
                    Item* oldItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, dstSlot);

                    float oldScore = 0.0f;
                    if (oldItem)
                    {
                        ItemTemplate const* oldItemProto = oldItem->GetTemplate();
                        if (oldItemProto)
                            oldScore = calculator.CalculateItem(oldItemProto->ItemId);
                    }

                    // Skip if the bot already has a better or equal item equipped
                    if (oldScore > newScore)
                        break;

                    uint32 price = proto->BuyPrice;
                    price = uint32(floor(price * bot->GetReputationPriceDiscount(pCreature)));

                    NeedMoneyFor needMoneyFor = NeedMoneyFor::none;
                    switch (usage)
                    {
                        case ITEM_USAGE_REPLACE:
                        case ITEM_USAGE_EQUIP:
                        case ITEM_USAGE_BAD_EQUIP:
                        case ITEM_USAGE_BROKEN_EQUIP:
                            needMoneyFor = NeedMoneyFor::gear;
                            break;
                        case ITEM_USAGE_AMMO:
                            needMoneyFor = NeedMoneyFor::ammo;
                            break;
                        case ITEM_USAGE_QUEST:
                            needMoneyFor = NeedMoneyFor::anything;
                            break;
                        case ITEM_USAGE_USE:
                            needMoneyFor = NeedMoneyFor::consumables;
                            break;
                        case ITEM_USAGE_SKILL:
                            needMoneyFor = NeedMoneyFor::tradeskill;
                            break;
                        default:
                            break;
                    }

                    if (needMoneyFor == NeedMoneyFor::none)
                        break;

                    if (AI_VALUE2(uint32, "free money for", uint32(needMoneyFor)) < price)
                        break;

                    if (!BuyItem(tItems, vendorguid, proto))
                        break;

                    // Store the best item score per InventoryType
                    bestPurchasedItemScore[invType] = newScore;

                    if (needMoneyFor == NeedMoneyFor::gear)
                    {
                        botAI->DoSpecificAction("equip upgrades packet action");
                    }
                }
            }
        }
        else
        {
            if (itemIds.empty())
                return false;

            for (ItemIds::iterator i = itemIds.begin(); i != itemIds.end(); i++)
            {
                uint32 itemId = *i;
                ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                if (!proto)
                    continue;

                result |= BuyItem(pCreature->GetVendorItems(), vendorguid, proto);

                if (!result)
                {
                    std::ostringstream out;
                    out << "Nobody sells " << ChatHelper::FormatItem(proto) << " nearby";
                    botAI->TellMaster(out.str());
                    continue;
                }

                ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", itemId);
                if (usage == ITEM_USAGE_REPLACE || usage == ITEM_USAGE_EQUIP ||
                    usage == ITEM_USAGE_BAD_EQUIP || usage == ITEM_USAGE_BROKEN_EQUIP)
                {
                    botAI->DoSpecificAction("equip upgrades packet action");
                    break;
                }
            }
        }
    }

    return vendored;
}

bool BuyAction::CanAfford(VendorItem const* tItem, ItemTemplate const* proto, Creature* vendor) const
{
    // Limited stock: skip if the vendor has fewer than one purchase remaining.
    if (tItem->maxcount != 0 && vendor->GetVendorItemCurrentCount(tItem) < proto->BuyCount)
        return false;

    // Non-gold costs (honor / arena points, token items, personal arena rating).
    if (tItem->ExtendedCost)
    {
        ItemExtendedCostEntry const* iece = sItemExtendedCostStore.LookupEntry(tItem->ExtendedCost);
        if (!iece)
            return false;

        if (bot->GetHonorPoints() < iece->reqhonorpoints)
            return false;
        if (bot->GetArenaPoints() < iece->reqarenapoints)
            return false;

        for (uint8 i = 0; i < MAX_ITEM_EXTENDED_COST_REQUIREMENTS; ++i)
            if (iece->reqitem[i] && !bot->HasItemCount(iece->reqitem[i], iece->reqitemcount[i]))
                return false;

        if (bot->GetMaxPersonalArenaRatingRequirement(iece->reqarenaslot) < iece->reqpersonalarenarating)
            return false;
    }

    return true;
}

bool BuyAction::BuyItem(VendorItemData const* tItems, ObjectGuid vendorguid, ItemTemplate const* proto)
{
    if (!tItems || !proto)
        return false;

    uint32 itemId = proto->ItemId;
    uint32 oldCount = bot->GetItemCount(itemId, false);

    for (uint32 slot = 0; slot < tItems->GetItemCount(); ++slot)
    {
        if (tItems->GetItem(slot)->item != itemId)
            continue;

        uint32 botMoney = bot->GetMoney();
        if (botAI->HasCheat(BotCheatMask::gold))
            bot->SetMoney(10000000);

        bot->BuyItemFromVendorSlot(vendorguid, slot, itemId, 1, NULL_BAG, NULL_SLOT);

        if (botAI->HasCheat(BotCheatMask::gold))
            bot->SetMoney(botMoney);

        uint32 newCount = bot->GetItemCount(itemId, false);
        if (newCount > oldCount)
        {
            std::ostringstream out;
            out << "Buying " << ChatHelper::FormatItem(proto);
            botAI->TellMaster(out.str());
            return true;
        }

        return false;
    }

    return false;
}
