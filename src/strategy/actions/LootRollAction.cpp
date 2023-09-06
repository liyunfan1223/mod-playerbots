/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "LootRollAction.h"
#include "Event.h"
#include "ItemUsageValue.h"
#include "Playerbots.h"
#include "LootAction.h"

bool LootRollAction::Execute(Event event)
{
    Player* bot = QueryItemUsageAction::botAI->GetBot();

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    WorldPacket p(event.getPacket()); //WorldPacket packet for CMSG_LOOT_ROLL, (8+4+1)
    ObjectGuid guid;
    uint32 slot;
    uint8 rollType;
    uint8 blevel;
    p.rpos(0); //reset packet pointer
    p >> guid; //guid of the item rolled
    p >> slot; //number of players invited to roll
    p >> rollType; //need,greed or pass on roll

    RollVote vote;

    if (!sPlayerbotAIConfig->autoRollForLoot)
        vote = PASS;
    else
    {
        blevel = bot->getLevel();
        std::vector<Roll*> rolls = group->GetRolls();
        for (std::vector<Roll*>::iterator i = rolls.begin(); i != rolls.end(); ++i)
        {
            if ((*i)->isValid() && (*i)->itemGUID == guid && (*i)->itemSlot == slot)
            {
                uint32 itemId = (*i)->itemid;
                ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                bot->Say("roll for :" + std::string(chat->FormatItem(proto) + ", entry:") + std::to_string(itemId), LANG_UNIVERSAL);
                if (!proto)
                    continue;
                uint32 protoClass = proto->Class;
                if (protoClass == ITEM_CLASS_WEAPON || protoClass == ITEM_CLASS_ARMOR)
                {
                    for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
                    {
                        if (Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                        {
                            ItemTemplate const* equipItem = item->GetTemplate();
                            if (equipItem->InventoryType == proto->InventoryType)
                            {
                                uint32 equipItemValue = 0;
                                uint32 protoValue = 0;
                                switch (protoClass)
                                {
                                case ITEM_CLASS_WEAPON:
                                    equipItemValue = equipItem->GetItemLevelIncludingQuality(blevel) + equipItem->getDPS();
                                    protoValue = proto->GetItemLevelIncludingQuality(blevel) + proto->getDPS();
                                    break;
                                case ITEM_CLASS_ARMOR:
                                    if(proto->SubClass == equipItem->SubClass)
                                    {
                                        equipItemValue = equipItem->GetItemLevelIncludingQuality(blevel) + equipItem->Armor;
                                        protoValue = proto->GetItemLevelIncludingQuality(blevel) + proto->Armor;
                                    }
                                    break;
                                default:
                                    equipItemValue = equipItem->GetItemLevelIncludingQuality(blevel) * 1000.0f + equipItem->BuyPrice;
                                    protoValue = proto->GetItemLevelIncludingQuality(blevel) * 1000.0f + equipItem->BuyPrice;
                                    break;
                                }
                                if (protoValue > equipItemValue)
                                {
                                    bot->Say(chat->FormatItem(proto) + " is better than my " + chat->FormatItem(equipItem) + ", i want it!", LANG_UNIVERSAL);
                                    vote = NEED;
                                    break;
                                }
                                else
                                {
                                    if(randomRoll() == 0)
                                        vote = PASS;
                                    else
                                        vote = GREED;
                                }
                            }
                            else
                            {
                                if(randomRoll() == 0)
                                    vote = PASS;
                                else
                                    vote = GREED;
                            }
                        }
                    }
                }
                else
                {
                    bot->Say("Money~Money~Money!", LANG_UNIVERSAL);
                    vote = GREED;
                }
            }
        }
    }

    // std::vector<Roll*> rolls = group->GetRolls();
    // bot->Say("guid:" + std::to_string(guid.GetCounter()) + 
    //     "item entry:" + std::to_string(guid.GetEntry()), LANG_UNIVERSAL);
    // for (std::vector<Roll*>::iterator i = rolls.begin(); i != rolls.end(); ++i)
    // {
    //     if ((*i)->isValid() && (*i)->itemGUID == guid && (*i)->itemSlot == slot)
    //     {
    //         uint32 itemId = (*i)->itemid;
    //         bot->Say("item entry2:" + std::to_string(itemId), LANG_UNIVERSAL);
    //         ItemTemplate const *proto = sObjectMgr->GetItemTemplate(itemId);
    //         if (!proto)
    //             continue;

    //         switch (proto->Class)
    //         {
    //         case ITEM_CLASS_WEAPON:
    //         case ITEM_CLASS_ARMOR:
    //             if (!QueryItemUsage(proto).empty())
    //                 vote = NEED;
    //             else if (bot->HasSkill(SKILL_ENCHANTING))
    //                 vote = DISENCHANT;
    //             break;
    //         default:
    //             if (StoreLootAction::IsLootAllowed(itemId, botAI))
    //                 vote = NEED;
    //             break;
    //         }
    //         break;
    //     }
    // }

    // if (ItemTemplate const* proto = sObjectMgr->GetItemTemplate(guid.GetEntry()))
    // {
    //     switch (proto->Class)
    //     {
    //         case ITEM_CLASS_WEAPON:
    //         case ITEM_CLASS_ARMOR:
    //             if (!QueryItemUsage(proto).empty())
    //                 vote = NEED;
    //             break;
    //         default:
    //             if (StoreLootAction::IsLootAllowed(guid.GetEntry(), botAI))
    //                 vote = NEED;
    //             break;
    //     }
    // }

    switch (group->GetLootMethod())
    {
        case MASTER_LOOT:
        case GROUP_LOOT:    
            group->CountRollVote(bot->GetGUID(), guid, vote);
            break;
        default:
            group->CountRollVote(bot->GetGUID(), guid, PASS);
            break;
    }

    return true;
}

inline uint8 LootRollAction::randomRoll()
{
    std::srand(std::time(0));
    return std::rand() % 2;
}

RollVote LootRollAction::CalculateRollVote(ItemTemplate const* proto)
{
    std::ostringstream out;
    out << proto->ItemId;
    ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", out.str());

    RollVote needVote = PASS;
    switch (usage)
    {
        case ITEM_USAGE_EQUIP:
        case ITEM_USAGE_REPLACE:
        case ITEM_USAGE_GUILD_TASK:
        case ITEM_USAGE_BAD_EQUIP:
            needVote = NEED;
            break;
        case ITEM_USAGE_SKILL:
        case ITEM_USAGE_USE:
        case ITEM_USAGE_DISENCHANT:
        case ITEM_USAGE_AH:
        case ITEM_USAGE_VENDOR:
            needVote = GREED;
            break;
    }

    return StoreLootAction::IsLootAllowed(proto->ItemId, GET_PLAYERBOT_AI(bot)) ? needVote : PASS;
}

bool MasterLootRollAction::isUseful()
{
    return !botAI->HasActivePlayerMaster();
};

bool MasterLootRollAction::Execute(Event event)
{
    Player* bot = QueryItemUsageAction::botAI->GetBot();

    WorldPacket p(event.getPacket()); //WorldPacket packet for CMSG_LOOT_ROLL, (8+4+1)
    ObjectGuid creatureGuid;
    uint32 mapId;
    uint32 itemSlot;
    uint32 itemId;
    uint32 randomSuffix;
    uint32 randomPropertyId;
    uint32 count;
    uint32 timeout;

    p.rpos(0); //reset packet pointer
    p >> creatureGuid; //creature guid what we're looting
    p >> mapId; /// 3.3.3 mapid
    p >> itemSlot; // the itemEntryId for the item that shall be rolled for
    p >> itemId; // the itemEntryId for the item that shall be rolled for
    p >> randomSuffix; // randomSuffix
    p >> randomPropertyId; // item random property ID
    p >> count; // items in stack
    p >> timeout;  // the countdown time to choose "need" or "greed"

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
    if (!proto)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    RollVote vote = CalculateRollVote(proto);
    group->CountRollVote(bot->GetGUID(), creatureGuid, CalculateRollVote(proto));

    return true;
}
