/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "SuggestWhatToDoAction.h"
#include "ChannelMgr.h"
#include "Event.h"
#include "ItemVisitors.h"
#include "AiFactory.h"
#include "ChatHelper.h"
#include "Playerbots.h"
#include "PlayerbotTextMgr.h"

std::map<std::string, uint8> SuggestWhatToDoAction::factions;

SuggestWhatToDoAction::SuggestWhatToDoAction(PlayerbotAI* botAI, std::string const name) : InventoryAction(botAI, name)
{
    suggestions.push_back(&SuggestWhatToDoAction::specificQuest);
    suggestions.push_back(&SuggestWhatToDoAction::grindReputation);
    suggestions.push_back(&SuggestWhatToDoAction::something);
}

bool SuggestWhatToDoAction::Execute(Event event)
{
    if (!sRandomPlayerbotMgr->IsRandomBot(bot) || bot->GetGroup() || bot->GetInstanceId())
        return false;

    uint32 index = rand() % suggestions.size();
    (this->*suggestions[index])();

    std::string const qualifier = "suggest what to do";
    time_t lastSaid = AI_VALUE2(time_t, "last said", qualifier);
    botAI->GetAiObjectContext()->GetValue<time_t>("last said", qualifier)->Set(time(nullptr) + urand(1, 60));

    return true;
}

std::vector<uint32> SuggestWhatToDoAction::GetIncompletedQuests()
{
    std::vector<uint32> result;

    for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 questId = bot->GetQuestSlotQuestId(slot);
        if (!questId)
            continue;

        QuestStatus status = bot->GetQuestStatus(questId);
        if (status == QUEST_STATUS_INCOMPLETE || status == QUEST_STATUS_NONE)
            result.push_back(questId);
    }

    return std::move(result);
}

void SuggestWhatToDoAction::specificQuest()
{
    std::vector<uint32> quests = GetIncompletedQuests();
    if (quests.empty())
        return;

    uint32 index = rand() % quests.size();

    Quest const* quest = sObjectMgr->GetQuestTemplate(quests[index]);

    std::map<std::string, std::string> placeholders;
    placeholders["%role"] = chat->FormatClass(bot, AiFactory::GetPlayerSpecTab(bot));
    placeholders["%quest"] = chat->FormatQuest(quest);

    spam(sPlayerbotTextMgr->Format("suggest_quest", placeholders));
}

void SuggestWhatToDoAction::grindReputation()
{
    if (factions.empty())
    {
        factions["Argent Dawn"] = 60;
        factions["Bloodsail Buccaneers"] = 40;
        factions["Brood of Nozdormu"] = 60;
        factions["Cenarion Circle"] = 55;
        factions["Darkmoon Faire"] = 20;
        factions["Hydraxian Waterlords"] = 60;
        factions["Ravenholdt"] = 20;
        factions["Thorium Brotherhood"] = 40;
        factions["Timbermaw Hold"] = 50;
        factions["Wintersaber Trainers"] = 50;
        factions["Booty Bay"] = 30;
        factions["Everlook"] = 40;
        factions["Gadgetzan"] = 50;
        factions["Ratchet"] = 20;

        factions["Ashtongue Deathsworn"] = 70;
        factions["Cenarion Expedition"] = 62;
        factions["The Consortium"] = 65;
        factions["Honor Hold"] = 66;
        factions["Keepers of Time"] = 68;
        factions["Netherwing"] = 65;
        factions["Ogri'la"] = 65;
        factions["The Scale of the Sands"] = 65;
        factions["Sporeggar"] = 65;
        factions["Tranquillien"] = 10;
        factions["The Violet Eye"] = 70;

        factions["Argent Crusade"] = 75;
        factions["Ashen Verdict"] = 75;
        factions["The Kalu'ak"] = 72;
        factions["Kirin Tor"] = 75;
        factions["Knights of the Ebon Blade"] = 77;
        factions["The Sons of Hodir"] = 78;
        factions["The Wyrmrest Accord"] = 77;
    }

    std::vector<std::string> levels;
    levels.push_back("honored");
    levels.push_back("revered");
    levels.push_back("exalted");

    std::vector<std::string> allowedFactions;
    for (std::map<std::string, uint8>::iterator i = factions.begin(); i != factions.end(); ++i)
    {
        if (bot->getLevel() >= i->second)
            allowedFactions.push_back(i->first);
    }

    if (allowedFactions.empty())
        return;

    std::map<std::string, std::string> placeholders;
    placeholders["%role"] = chat->FormatClass(bot, AiFactory::GetPlayerSpecTab(bot));
    placeholders["%level"] = levels[urand(0, 2)];

    std::ostringstream rnd;
    rnd << urand(1, 5) << "K";
    placeholders["%rndK"] = rnd.str();

    std::ostringstream itemout;
    itemout << "|c004040b0" << allowedFactions[urand(0, allowedFactions.size() - 1)] << "|r";
    placeholders["%faction"] = itemout.str();

    spam(sPlayerbotTextMgr->Format("suggest_faction", placeholders));
}

void SuggestWhatToDoAction::something()
{
    std::map<std::string, std::string> placeholders;
    placeholders["%role"] = chat->FormatClass(bot, AiFactory::GetPlayerSpecTab(bot));

    AreaTableEntry const* entry = sAreaTableStore.LookupEntry(bot->GetAreaId());
    if (!entry)
        return;

    std::ostringstream out;
    out << "|cffb04040" << entry->area_name[0] << "|r";
    placeholders["%zone"] = out.str();

    spam(sPlayerbotTextMgr->Format("suggest_something", placeholders));
}

void SuggestWhatToDoAction::spam(
    std::string msg,
    uint32 channelId,
    bool const isLowerCase
)
{
    std::set<std::string> said;
    for (uint32 i = 0; i < sChatChannelsStore.GetNumRows(); ++i)
    {
        ChatChannelsEntry const* channel = sChatChannelsStore.LookupEntry(i);
        if (!channel || channel->ChannelID != channelId) continue;

        for (AreaTableEntry const* area : sAreaTableStore)
        {
            char channelName[255];
            snprintf(channelName, 255, channel->pattern[0], area->area_name[0]);
            if (said.find(channelName) != said.end())
                continue;

            said.insert(channelName);

            if (ChannelMgr* cMgr = ChannelMgr::forTeam(bot->GetTeamId()))
            {
                if (Channel* chn = cMgr->GetJoinChannel(channelName, channel->ChannelID))
                {
                    chn->JoinChannel(bot, "");
                    if (isLowerCase)
                    {
                        strToLower(msg);
                    }
                    if (chn->GetName().length() > 0)
                    {
                        chn->Say(bot->GetGUID(), msg.c_str(), LANG_UNIVERSAL);
                    }
                }
            }
        }
    }
}

class FindTradeItemsVisitor : public IterateItemsVisitor
{
    public:
        FindTradeItemsVisitor(uint32 quality) : quality(quality), IterateItemsVisitor() { }

        bool Visit(Item* item) override
        {
            ItemTemplate const* proto = item->GetTemplate();
            if (proto->Quality != quality)
                return true;

            if (proto->Class == ITEM_CLASS_TRADE_GOODS && proto->Bonding == NO_BIND)
            {
                if (proto->Quality == ITEM_QUALITY_NORMAL && item->GetCount() > 1 && item->GetCount() == item->GetMaxStackCount())
                    stacks.push_back(proto->ItemId);

                items.push_back(proto->ItemId);
                count[proto->ItemId] += item->GetCount();
            }

            return true;
        }

        std::map<uint32, uint32> count;
        std::vector<uint32> stacks;
        std::vector<uint32> items;

    private:
        uint32 quality;
};

SuggestTradeAction::SuggestTradeAction(PlayerbotAI* botAI) : SuggestWhatToDoAction(botAI, "suggest trade")
{
}

bool SuggestTradeAction::Execute(Event event)
{
    if (!sRandomPlayerbotMgr->IsRandomBot(bot) || bot->GetGroup() || bot->GetInstanceId())
        return false;

    uint32 quality = urand(0, 100);
    if (quality > 95)
        quality = ITEM_QUALITY_LEGENDARY;
    else if (quality > 90)
        quality = ITEM_QUALITY_EPIC;
    else if (quality > 75)
        quality = ITEM_QUALITY_RARE;
    else if (quality > 50)
        quality = ITEM_QUALITY_UNCOMMON;
    else
        quality = ITEM_QUALITY_NORMAL;

    uint32 item = 0, count = 0;
    while (quality-- > ITEM_QUALITY_POOR)
    {
        FindTradeItemsVisitor visitor(quality);
        IterateItems(&visitor);
        if (!visitor.stacks.empty())
        {
            uint32 index = urand(0, visitor.stacks.size() - 1);
            item = visitor.stacks[index];
        }

        if (!item)
        {
            if (!visitor.items.empty())
            {
                uint32 index = urand(0, visitor.items.size() - 1);
                item = visitor.items[index];
            }
        }

        if (item)
        {
            count = visitor.count[item];
            break;
        }
    }

    if (!item || !count)
        return false;

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(item);
    if (!proto)
        return false;

    uint32 price = proto->SellPrice * sRandomPlayerbotMgr->GetSellMultiplier(bot) * count;
    if (!price)
        return false;

    std::map<std::string, std::string> placeholders;
    placeholders["%item"] = chat->FormatItem(proto, count);
    placeholders["%gold"] = chat->formatMoney(price);

    spam(sPlayerbotTextMgr->Format("suggest_sell", placeholders));
    return true;
}

bool SuggestWhatToDoAction::isUseful()
{
    std::string const qualifier = "suggest what to do";
    time_t lastSaid = AI_VALUE2(time_t, "last said", qualifier);
    return (time(nullptr) - lastSaid) > 30;
}
