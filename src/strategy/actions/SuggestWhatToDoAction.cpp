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
#include "GuildMgr.h"

std::map<std::string, uint8> SuggestWhatToDoAction::instances;
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

void SuggestWhatToDoAction::instance()
{
    if (instances.empty())
    {
        instances["Ragefire Chasm"] = 15;
        instances["Deadmines"] = 18;
        instances["Wailing Caverns"] = 18;
        instances["Shadowfang Keep"] = 25;
        instances["Blackfathom Deeps"] = 20;
        instances["Stockade"] = 20;
        instances["Gnomeregan"] = 35;
        instances["Razorfen Kraul"] = 35;
        instances["Maraudon"] = 50;
        instances["Scarlet Monestery"] = 40;
        instances["Uldaman"] = 45;
        instances["Dire Maul"] = 58;
        instances["Scholomance"] = 59;
        instances["Razorfen Downs"] = 40;
        instances["Strathholme"] = 59;
        instances["Zul'Farrak"] = 45;
        instances["Blackrock Depths"] = 55;
        instances["Temple of Atal'Hakkar"] = 55;
        instances["Lower Blackrock Spire"] = 57;

        instances["Hellfire Citidel"] = 65;
        instances["Coilfang Reservoir"] = 65;
        instances["Auchindoun"] = 65;
        instances["Cavens of Time"] = 68;
        instances["Tempest Keep"] = 69;
        instances["Magister's Terrace"] = 70;

        instances["Utgarde Keep"] = 75;
        instances["The Nexus"] = 75;
        instances["Ahn'kahet: The Old Kingdom"] = 75;
        instances["Azjol-Nerub"] = 75;
        instances["Drak'Tharon Keep"] = 75;
        instances["Violet Hold"] = 80;
        instances["Gundrak"] = 77;
        instances["Halls of Stone"] = 77;
        instances["Halls of Lightning"] = 77;
        instances["Oculus"] = 77;
        instances["Utgarde Pinnacle"] = 77;
        instances["Trial of the Champion"] = 80;
        instances["Forge of Souls"] = 80;
        instances["Pit of Saron"] = 80;
        instances["Halls of Reflection"] = 80;
    }

    std::vector<std::string> allowedInstances;
    for (auto & instance : instances)
    {
        if (bot->getLevel() >= instance.second) allowedInstances.push_back(instance.first);
    }

    if (allowedInstances.empty()) return;

    std::map<std::string, std::string> placeholders;
    placeholders["%role"] = ChatHelper::FormatClass(bot, AiFactory::GetPlayerSpecTab(bot));

    std::ostringstream itemout;
    //itemout << "|c00b000b0" << allowedInstances[urand(0, allowedInstances.size() - 1)] << "|r";
    itemout << allowedInstances[urand(0, allowedInstances.size() - 1)];
    placeholders["%instance"] = itemout.str();

    spam(BOT_TEXT2("suggest_instance", placeholders), urand(0, 1) ? 0x50 : 0, urand(0, 2), urand(0, 2));
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

    return result;
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

    spam(BOT_TEXT2("suggest_quest", placeholders), urand(0, 1) ? 0x18 : 0, urand(0, 2), urand(0, 2));
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
//    itemout << "|c004040b0" << allowedFactions[urand(0, allowedFactions.size() - 1)] << "|r";
    itemout << allowedFactions[urand(0, allowedFactions.size() - 1)];
    placeholders["%faction"] = itemout.str();

    spam(BOT_TEXT2("suggest_faction", placeholders), 0x18, true);
}

void SuggestWhatToDoAction::something()
{
    std::map<std::string, std::string> placeholders;
    placeholders["%role"] = chat->FormatClass(bot, AiFactory::GetPlayerSpecTab(bot));

    AreaTableEntry const* entry = sAreaTableStore.LookupEntry(bot->GetAreaId());
    if (!entry)
        return;

    std::ostringstream out;
//    out << "|cffb04040" << entry->area_name[0] << "|r";
    out << entry->area_name[0];
    placeholders["%zone"] = out.str();

    spam(BOT_TEXT2("suggest_something", placeholders), urand(0, 1) ? 0x18 : 0, urand(0, 2), urand(0, 2));
}

void SuggestWhatToDoAction::spam(std::string msg, uint8 flags, bool worldChat, bool guild)
{
    if (msg.empty())
        return;

    std::vector<std::string> channelNames;
    ChannelMgr* cMgr = ChannelMgr::forTeam(bot->GetTeamId());
    if (!cMgr)
        return;


    for (uint32 i = 0; i < sChatChannelsStore.GetNumRows(); ++i)
    {
        ChatChannelsEntry const* channel = sChatChannelsStore.LookupEntry(i);
        if (!channel) continue;

        for (AreaTableEntry const* current_zone : sAreaTableStore)
        {
           if (!current_zone)
               continue;


            // combine full channel name
            char channelName[100];
            Channel* chn = nullptr;
            if ((channel->flags & CHANNEL_DBC_FLAG_LFG) != 0)
            {
                std::string chanName = channel->pattern[0];
                chn = cMgr->GetChannel(chanName, bot);
            }
            else
            {
                snprintf(channelName, 100, channel->pattern[0], current_zone->area_name[0]);
                chn = cMgr->GetChannel(channelName, bot);
            }
            if (!chn)
                continue;
            // skip world chat here
            if (chn->GetName() == "World")
                continue;

            if (flags != 0 && chn->GetFlags() != flags)
                continue;

            // skip local defense
            //if (chn->GetFlags() == 0x18)
            //    continue;

            // no filter, pick several options
            if (flags == CHANNEL_FLAG_NONE)
            {
                channelNames.push_back(chn->GetName());
            }
            else
                chn->Say(bot->GetGUID(), msg.c_str(), LANG_UNIVERSAL);
        }

        if (!channelNames.empty())
        {
            std::string randomName = channelNames[urand(0, channelNames.size() - 1)];
            if (Channel* chn = cMgr->GetChannel(randomName, bot))
                chn->Say(bot->GetGUID(), msg.c_str(), LANG_UNIVERSAL);
        }

        if (worldChat)
        {
            if (Channel* worldChannel = cMgr->GetChannel("World", bot))
                worldChannel->Say(bot->GetGUID(), msg.c_str(), LANG_UNIVERSAL);
        }

        if (guild && bot->GetGuildId())
        {
            Guild* guild = sGuildMgr->GetGuildById(bot->GetGuildId());
            if (guild)
                guild->BroadcastToGuild(bot->GetSession(), false, msg.c_str(), LANG_UNIVERSAL);
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

    spam(BOT_TEXT2("suggest_sell", placeholders), urand(0, 1) ? 0x3C : 0, urand(0, 1), urand(0, 5));
    return true;
}

bool SuggestWhatToDoAction::isUseful()
{
    std::string const qualifier = "suggest what to do";
    time_t lastSaid = AI_VALUE2(time_t, "last said", qualifier);
    return (time(nullptr) - lastSaid) > 30;
}
