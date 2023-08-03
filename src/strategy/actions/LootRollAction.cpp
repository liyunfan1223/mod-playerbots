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
    p.rpos(0); //reset packet pointer
    p >> guid; //guid of the item rolled
    p >> slot; //number of players invited to roll
    p >> rollType; //need,greed or pass on roll

    RollVote vote = PASS;

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
        case FREE_FOR_ALL:
            group->CountRollVote(bot->GetGUID(), guid, PASS);
            break;
        default:
            group->CountRollVote(bot->GetGUID(), guid, vote);
            break;
    }

    return true;
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
