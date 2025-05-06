/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "SendMailAction.h"

#include "ChatHelper.h"
#include "Event.h"
#include "ItemVisitors.h"
#include "Mail.h"
#include "Playerbots.h"

bool SendMailAction::Execute(Event event)
{
    uint32 account = bot->GetSession()->GetAccountId();
    bool randomBot = sPlayerbotAIConfig->IsInRandomAccountList(account);

    GuidVector gos = *context->GetValue<GuidVector>("nearest game objects");
    bool mailboxFound = false;
    for (ObjectGuid const guid : gos)
    {
        if (GameObject* go = botAI->GetGameObject(guid))
            if (go->GetGoType() == GAMEOBJECT_TYPE_MAILBOX)
            {
                mailboxFound = true;
                break;
            }
    }

    std::string const text = event.getParam();

    Player* receiver = GetMaster();
    Player* tellTo = receiver;

    std::vector<std::string> ss = split(text, ' ');
    if (ss.size() > 1)
    {
        if (Player* p = ObjectAccessor::FindPlayer(ObjectGuid(uint64(ss[ss.size() - 1].c_str()))))
            receiver = p;
    }

    if (!receiver)
        receiver = event.getOwner();

    if (!receiver || receiver == bot)
    {
        return false;
    }

    if (!tellTo)
        tellTo = receiver;

    if (!mailboxFound && !randomBot)
    {
        bot->Whisper("There is no mailbox nearby", LANG_UNIVERSAL, tellTo);
        return false;
    }

    ItemIds ids = chat->parseItems(text);
    if (ids.size() > 1)
    {
        bot->Whisper("You can not request more than one item", LANG_UNIVERSAL, tellTo);
        return false;
    }

    if (ids.empty())
    {
        uint32 money = chat->parseMoney(text);
        if (!money)
            return false;

        if (randomBot)
        {
            bot->Whisper("I cannot send money", LANG_UNIVERSAL, tellTo);
            return false;
        }

        if (bot->GetMoney() < money)
        {
            botAI->TellError("I don't have enough money");
            return false;
        }

        std::ostringstream body;
        body << "Hello, " << receiver->GetName() << ",\n";
        body << "\n";
        body << "Here is the money you asked for";
        body << "\n";
        body << "Thanks,\n";
        body << bot->GetName() << "\n";

        CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();

        MailDraft draft("Money you asked for", body.str());
        draft.AddMoney(money);
        bot->SetMoney(bot->GetMoney() - money);
        draft.SendMailTo(trans, MailReceiver(receiver), MailSender(bot));

        CharacterDatabase.CommitTransaction(trans);

        std::ostringstream out;
        out << "Sending mail to " << receiver->GetName();
        botAI->TellMaster(out.str());
        return true;
    }

    std::ostringstream body;
    body << "Hello, " << receiver->GetName() << ",\n";
    body << "\n";
    body << "Here are the item(s) you asked for";
    body << "\n";
    body << "Thanks,\n";
    body << bot->GetName() << "\n";

    MailDraft draft("Item(s) you asked for", body.str());
    for (ItemIds::iterator i = ids.begin(); i != ids.end(); i++)
    {
        FindItemByIdVisitor visitor(*i);
        IterateItems(&visitor, ITERATE_ITEMS_IN_BAGS);

        std::vector<Item*> items = visitor.GetResult();
        for (Item* item : items)
        {
            if (item->IsSoulBound() || item->IsConjuredConsumable())
            {
                std::ostringstream out;
                out << "Cannot send " << ChatHelper::FormatItem(item->GetTemplate());
                bot->Whisper(out.str(), LANG_UNIVERSAL, tellTo);
                continue;
            }

            ItemTemplate const* proto = item->GetTemplate();
            if (!proto)
                continue;

            if (randomBot)
            {
                uint32 price = item->GetCount() * proto->SellPrice;
                if (!price)
                {
                    std::ostringstream out;
                    out << ChatHelper::FormatItem(item->GetTemplate()) << ": it is not for sale";
                    bot->Whisper(out.str(), LANG_UNIVERSAL, tellTo);
                    return false;
                }

                draft.AddCOD(price);
            }

            CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();

            bot->MoveItemFromInventory(item->GetBagSlot(), item->GetSlot(), true);
            item->DeleteFromInventoryDB(trans);
            item->SetOwnerGUID(receiver->GetGUID());
            item->SaveToDB(trans);
            draft.AddItem(item);
            draft.SendMailTo(trans, MailReceiver(receiver), MailSender(bot));

            CharacterDatabase.CommitTransaction(trans);

            std::ostringstream out;
            out << "Sent mail to " << receiver->GetName();
            bot->Whisper(out.str(), LANG_UNIVERSAL, tellTo);
            return true;
        }
    }

    return false;
}
