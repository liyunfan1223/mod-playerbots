/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "HireAction.h"

#include "Event.h"
#include "Playerbots.h"

bool HireAction::Execute(Event event)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    if (!sRandomPlayerbotMgr->IsRandomBot(bot))
        return false;

    uint32 account = master->GetSession()->GetAccountId();
    QueryResult results = CharacterDatabase.Query("SELECT COUNT(*) FROM characters WHERE account = {}", account);

    uint32 charCount = 10;
    if (results)
    {
        Field* fields = results->Fetch();
        charCount = uint32(fields[0].Get<uint64>());
    }

    if (charCount >= 10)
    {
        botAI->TellMaster("You already have the maximum number of characters");
        return false;
    }

    if (bot->GetLevel() > master->GetLevel())
    {
        botAI->TellMaster("You cannot hire higher level characters than you");
        return false;
    }

    uint32 discount = sRandomPlayerbotMgr->GetTradeDiscount(bot, master);
    uint32 m = 1 + (bot->GetLevel() / 10);
    uint32 moneyReq = m * 5000 * bot->GetLevel();
    if (discount < moneyReq)
    {
        std::ostringstream out;
        out << "You cannot hire me - I barely know you. Make sure you have at least " << chat->formatMoney(moneyReq)
            << " as a trade discount";
        botAI->TellMaster(out.str());
        return false;
    }

    botAI->TellMaster("I will join you at your next relogin");

    bot->SetMoney(moneyReq);
    sRandomPlayerbotMgr->Remove(bot);
    CharacterDatabase.Execute("UPDATE characters SET account = {} WHERE guid = {}", account,
                              bot->GetGUID().GetCounter());

    return true;
}
