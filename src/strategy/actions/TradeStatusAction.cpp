/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "TradeStatusAction.h"

#include "CraftValue.h"
#include "Event.h"
#include "GuildTaskMgr.h"
#include "ItemUsageValue.h"
#include "ItemVisitors.h"
#include "PlayerbotMgr.h"
#include "PlayerbotSecurity.h"
#include "Playerbots.h"
#include "RandomPlayerbotMgr.h"
#include "SetCraftAction.h"

bool TradeStatusAction::Execute(Event event)
{
    Player* trader = bot->GetTrader();
    Player* master = GetMaster();
    if (!trader)
        return false;

    PlayerbotAI* traderBotAI = GET_PLAYERBOT_AI(trader);

    // Allow the master and group members to trade
    if (trader != master && !traderBotAI && (!bot->GetGroup() || !bot->GetGroup()->IsMember(trader->GetGUID())))
    {
        bot->Whisper("I'm kind of busy now", LANG_UNIVERSAL, trader);
        return false;
    }

    if (sPlayerbotAIConfig->enableRandomBotTrading == 0 && (sRandomPlayerbotMgr->IsRandomBot(bot)|| sRandomPlayerbotMgr->IsAddclassBot(bot)))
    {
        bot->Whisper("Trading is disabled", LANG_UNIVERSAL, trader);
        return false;
    }

    // Allow trades from group members or bots
    if ((!bot->GetGroup() || !bot->GetGroup()->IsMember(trader->GetGUID())) &&
        (trader != master || !botAI->GetSecurity()->CheckLevelFor(PLAYERBOT_SECURITY_ALLOW_ALL, true, master)) &&
        !traderBotAI)
    {
        WorldPacket p;
        uint32 status = 0;
        p << status;
        bot->GetSession()->HandleCancelTradeOpcode(p);
        return false;
    }

    WorldPacket p(event.getPacket());
    p.rpos(0);
    uint32 status;
    p >> status;

    if (status == TRADE_STATUS_TRADE_ACCEPT || (status == TRADE_STATUS_BACK_TO_TRADE && trader->GetTradeData() && trader->GetTradeData()->IsAccepted()))
    {
        WorldPacket p;
        uint32 status = 0;
        p << status;

        uint32 discount = sRandomPlayerbotMgr->GetTradeDiscount(bot, trader);
        if (CheckTrade())
        {
            int32 botMoney = CalculateCost(bot, true);

            std::map<uint32, uint32> givenItemIds, takenItemIds;
            for (uint32 slot = 0; slot < TRADE_SLOT_TRADED_COUNT; ++slot)
            {
                Item* item = trader->GetTradeData()->GetItem((TradeSlots)slot);
                if (item)
                    givenItemIds[item->GetTemplate()->ItemId] += item->GetCount();

                item = bot->GetTradeData()->GetItem((TradeSlots)slot);
                if (item)
                    takenItemIds[item->GetTemplate()->ItemId] += item->GetCount();
            }

            bot->GetSession()->HandleAcceptTradeOpcode(p);
            if (bot->GetTradeData())
            {
                sRandomPlayerbotMgr->SetTradeDiscount(bot, trader, discount);
                return false;
            }

            for (std::map<uint32, uint32>::iterator i = givenItemIds.begin(); i != givenItemIds.end(); ++i)
            {
                uint32 itemId = i->first;
                uint32 count = i->second;

                CraftData& craftData = AI_VALUE(CraftData&, "craft");
                if (!craftData.IsEmpty() && craftData.IsRequired(itemId))
                {
                    craftData.AddObtained(itemId, count);
                }

                sGuildTaskMgr->CheckItemTask(itemId, count, trader, bot);
            }

            for (std::map<uint32, uint32>::iterator i = takenItemIds.begin(); i != takenItemIds.end(); ++i)
            {
                uint32 itemId = i->first;
                uint32 count = i->second;

                CraftData& craftData = AI_VALUE(CraftData&, "craft");
                if (!craftData.IsEmpty() && craftData.itemId == itemId)
                {
                    craftData.Crafted(count);
                }
            }

            return true;
        }
    }
    else if (status == TRADE_STATUS_BEGIN_TRADE)
    {
        if (!bot->HasInArc(CAST_ANGLE_IN_FRONT, trader, sPlayerbotAIConfig->sightDistance))
            bot->SetFacingToObject(trader);

        BeginTrade();

        return true;
    }
    return false;
}

void TradeStatusAction::BeginTrade()
{
    Player* trader = bot->GetTrader();
    if (!trader || GET_PLAYERBOT_AI(bot->GetTrader()))
        return;

    WorldPacket p;
    bot->GetSession()->HandleBeginTradeOpcode(p);

    ListItemsVisitor visitor;
    IterateItems(&visitor);

    botAI->TellMaster("=== Inventory ===");
    TellItems(visitor.items, visitor.soulbound);

    if (sRandomPlayerbotMgr->IsRandomBot(bot))
    {
        uint32 discount = sRandomPlayerbotMgr->GetTradeDiscount(bot, botAI->GetMaster());
        if (discount)
        {
            std::ostringstream out;
            out << "Discount up to: " << chat->formatMoney(discount);
            botAI->TellMaster(out);
        }
    }
}

bool TradeStatusAction::CheckTrade()
{
    Player* trader = bot->GetTrader();
    if (!bot->GetTradeData() || !trader || !trader->GetTradeData())
        return false;

    if (!botAI->HasActivePlayerMaster() && GET_PLAYERBOT_AI(bot->GetTrader()))
    {
        bool isGivingItem = false;
        for (uint32 slot = 0; slot < TRADE_SLOT_TRADED_COUNT; ++slot)
        {
            Item* item = bot->GetTradeData()->GetItem((TradeSlots)slot);
            if (item)
            {
                isGivingItem = true;
                break;
            }
        }
        bool isGettingItem = false;
        for (uint32 slot = 0; slot < TRADE_SLOT_TRADED_COUNT; ++slot)
        {
            Item* item = trader->GetTradeData()->GetItem((TradeSlots)slot);
            if (item)
            {
                isGettingItem = true;
                break;
            }
        }

        if (isGettingItem)
        {
            if (bot->GetGroup() && bot->GetGroup()->IsMember(bot->GetTrader()->GetGUID()) &&
                botAI->HasRealPlayerMaster())
                botAI->TellMasterNoFacing("Thank you " + chat->FormatWorldobject(bot->GetTrader()));
            else
                bot->Say("Thank you " + chat->FormatWorldobject(bot->GetTrader()),
                         (bot->GetTeamId() == TEAM_ALLIANCE ? LANG_COMMON : LANG_ORCISH));
        }
        return isGettingItem;
    }
    if (!bot->GetSession())
    {
        return false;
    }
    uint32 accountId = bot->GetSession()->GetAccountId();
    if (!sPlayerbotAIConfig->IsInRandomAccountList(accountId))
    {
        int32 botItemsMoney = CalculateCost(bot, true);
        int32 botMoney = bot->GetTradeData()->GetMoney() + botItemsMoney;
        int32 playerItemsMoney = CalculateCost(trader, false);
        int32 playerMoney = trader->GetTradeData()->GetMoney() + playerItemsMoney;
        if (playerMoney || botMoney)
            botAI->PlaySound(playerMoney < botMoney ? TEXT_EMOTE_SIGH : TEXT_EMOTE_THANK);

        return true;
    }

    int32 botItemsMoney = CalculateCost(bot, true);
    int32 botMoney = bot->GetTradeData()->GetMoney() + botItemsMoney;
    int32 playerItemsMoney = CalculateCost(trader, false);
    int32 playerMoney = trader->GetTradeData()->GetMoney() + playerItemsMoney;
    if (botItemsMoney > 0 && sPlayerbotAIConfig->enableRandomBotTrading == 2 && (sRandomPlayerbotMgr->IsRandomBot(bot)|| sRandomPlayerbotMgr->IsAddclassBot(bot)))
    {
        bot->Whisper("Selling is disabled.", LANG_UNIVERSAL, trader);
        return false;
    }
    if (playerItemsMoney && sPlayerbotAIConfig->enableRandomBotTrading == 3 && (sRandomPlayerbotMgr->IsRandomBot(bot)|| sRandomPlayerbotMgr->IsAddclassBot(bot)))
    {
        bot->Whisper("Buying is disabled.", LANG_UNIVERSAL, trader);
        return false;
    }
    for (uint32 slot = 0; slot < TRADE_SLOT_TRADED_COUNT; ++slot)
    {
        Item* item = bot->GetTradeData()->GetItem((TradeSlots)slot);
        if (item && !item->GetTemplate()->SellPrice && !item->GetTemplate()->IsConjuredConsumable())
        {
            std::ostringstream out;
            out << chat->FormatItem(item->GetTemplate()) << " - This is not for sale";
            botAI->TellMaster(out);
            botAI->PlaySound(TEXT_EMOTE_NO);
            return false;
        }

        item = trader->GetTradeData()->GetItem((TradeSlots)slot);
        if (item)
        {
            std::ostringstream out;
            out << item->GetTemplate()->ItemId;
            ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", out.str());
            if ((botMoney && !item->GetTemplate()->BuyPrice) || usage == ITEM_USAGE_NONE)
            {
                std::ostringstream out;
                out << chat->FormatItem(item->GetTemplate()) << " - I don't need this";
                botAI->TellMaster(out);
                botAI->PlaySound(TEXT_EMOTE_NO);
                return false;
            }
        }
    }

    if (!botMoney && !playerMoney)
        return true;

    if (!botItemsMoney && !playerItemsMoney)
    {
        botAI->TellError("There are no items to trade");
        return false;
    }

    int32 discount = (int32)sRandomPlayerbotMgr->GetTradeDiscount(bot, trader);
    int32 delta = playerMoney - botMoney;
    int32 moneyDelta = (int32)trader->GetTradeData()->GetMoney() - (int32)bot->GetTradeData()->GetMoney();
    bool success = false;
    if (delta < 0)
    {
        if (delta + discount >= 0)
        {
            if (moneyDelta < 0)
            {
                botAI->TellError("You can use discount to buy items only");
                botAI->PlaySound(TEXT_EMOTE_NO);
                return false;
            }

            success = true;
        }
    }
    else
    {
        success = true;
    }

    if (success)
    {
        sRandomPlayerbotMgr->AddTradeDiscount(bot, trader, delta);
        switch (urand(0, 4))
        {
            case 0:
                botAI->TellMaster("A pleasure doing business with you");
                break;
            case 1:
                botAI->TellMaster("Fair trade");
                break;
            case 2:
                botAI->TellMaster("Thanks");
                break;
            case 3:
                botAI->TellMaster("Off with you");
                break;
        }

        botAI->PlaySound(TEXT_EMOTE_THANK);
        return true;
    }

    std::ostringstream out;
    out << "I want " << chat->formatMoney(-(delta + discount)) << " for this";
    botAI->TellMaster(out);
    botAI->PlaySound(TEXT_EMOTE_NO);
    return false;
}

int32 TradeStatusAction::CalculateCost(Player* player, bool sell)
{
    Player* trader = bot->GetTrader();
    TradeData* data = player->GetTradeData();
    if (!data)
        return 0;

    uint32 sum = 0;
    for (uint32 slot = 0; slot < TRADE_SLOT_TRADED_COUNT; ++slot)
    {
        Item* item = data->GetItem((TradeSlots)slot);
        if (!item)
            continue;

        ItemTemplate const* proto = item->GetTemplate();
        if (!proto)
            continue;

        if (proto->Quality < ITEM_QUALITY_NORMAL)
            return 0;

        CraftData& craftData = AI_VALUE(CraftData&, "craft");
        if (!craftData.IsEmpty())
        {
            if (player == trader && !sell && craftData.IsRequired(proto->ItemId))
            {
                continue;
            }

            if (player == bot && sell && craftData.itemId == proto->ItemId && craftData.IsFulfilled())
            {
                sum += item->GetCount() * SetCraftAction::GetCraftFee(craftData);
                continue;
            }
        }

        if (sell)
        {
            sum += item->GetCount() * proto->SellPrice * sRandomPlayerbotMgr->GetSellMultiplier(bot);
        }
        else
        {
            sum += item->GetCount() * proto->BuyPrice * sRandomPlayerbotMgr->GetBuyMultiplier(bot);
        }
    }

    return sum;
}
