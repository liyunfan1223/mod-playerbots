/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "SuggestWhatToDoAction.h"
#include "ServerFacade.h"
#include "ChannelMgr.h"
#include "Event.h"
#include "ItemVisitors.h"
#include "AiFactory.h"
#include "ChatHelper.h"
#include "Playerbots.h"
#include "PlayerbotTextMgr.h"
#include "GuildMgr.h"
#include "Config.h"

#include <functional>

enum eTalkType
{
    General             = ChannelFlags::CHANNEL_FLAG_GENERAL  | ChannelFlags::CHANNEL_FLAG_NOT_LFG,
    Trade               = ChannelFlags::CHANNEL_FLAG_CITY     | ChannelFlags::CHANNEL_FLAG_GENERAL  | ChannelFlags::CHANNEL_FLAG_NOT_LFG | ChannelFlags::CHANNEL_FLAG_TRADE,
    LocalDefence        = ChannelFlags::CHANNEL_FLAG_GENERAL  | ChannelFlags::CHANNEL_FLAG_NOT_LFG,
    GuildRecruitment    = ChannelFlags::CHANNEL_FLAG_CITY     | ChannelFlags::CHANNEL_FLAG_GENERAL  | ChannelFlags::CHANNEL_FLAG_NOT_LFG,
    LookingForGroup     = ChannelFlags::CHANNEL_FLAG_LFG      | ChannelFlags::CHANNEL_FLAG_GENERAL
};

std::map<std::string, uint8> SuggestDungeonAction::instances;
std::map<std::string, uint8> SuggestWhatToDoAction::factions;

SuggestWhatToDoAction::SuggestWhatToDoAction(PlayerbotAI* botAI, std::string const name)
    : InventoryAction{ botAI, name }
    , _dbc_locale{ sWorld->GetDefaultDbcLocale() }
{
    suggestions.push_back(std::bind(&SuggestWhatToDoAction::specificQuest, this));
    suggestions.push_back(std::bind(&SuggestWhatToDoAction::grindReputation, this));
    suggestions.push_back(std::bind(&SuggestWhatToDoAction::something, this));
    suggestions.push_back(std::bind(&SuggestWhatToDoAction::grindMaterials, this));
}

bool SuggestWhatToDoAction::isUseful()
{
    if (!sRandomPlayerbotMgr->IsRandomBot(bot) || bot->GetGroup() || bot->GetInstanceId())
        return false;

    std::string qualifier = "suggest what to do";
    time_t lastSaid = AI_VALUE2(time_t, "last said", qualifier);
    return (time(0) - lastSaid) > 30;
}

bool SuggestWhatToDoAction::Execute(Event event)
{
    uint32 index = rand() % suggestions.size();
    auto fnct_ptr = suggestions[index];
    fnct_ptr();

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

    spam(BOT_TEXT2("suggest_quest", placeholders), urand(0, 1) ? eTalkType::General : 0, urand(0, 2), urand(0, 2));
}

void SuggestWhatToDoAction::grindMaterials()
{
    /*if (bot->GetLevel() <= 5)
        return;

    auto result = CharacterDatabase.Query("SELECT distinct category, multiplier FROM ahbot_category where category not in ('other', 'quest', 'trade', 'reagent') and multiplier > 3 order by multiplier desc limit 10");
    if (!result)
        return;

    std::map<std::string, double> categories;
    do
    {
        Field* fields = result->Fetch();
        categories[fields[0].Get<std::string>()] = fields[1].Get<float>();
    } while (result->NextRow());

    for (std::map<std::string, double>::iterator i = categories.begin(); i != categories.end(); ++i)
    {
        if (urand(0, 10) < 3) {
            std::string name = i->first;
            double multiplier = i->second;

            for (int j = 0; j < ahbot::CategoryList::instance.size(); j++)
            {
                ahbot::Category* category = ahbot::CategoryList::instance[j];
                if (name == category->GetName())
                {
                    std::string item = category->GetLabel();
                    transform(item.begin(), item.end(), item.begin(), ::tolower);
                    std::ostringstream itemout;
                    itemout << "|c0000b000" << item << "|r";
                    item = itemout.str();

                    std::map<std::string, std::string> placeholders;
                    placeholders["%role"] = chat->formatClass(bot, AiFactory::GetPlayerSpecTab(bot));
                    placeholders["%category"] = item;

                    spam(BOT_TEXT2("suggest_trade", placeholders), urand(0, 1) ? 0x3C : 0x18, !urand(0, 2), !urand(0, 3));
                    return;
                }
            }
        }
    }*/
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
    for (const auto& i : factions)
    {
        if (bot->GetLevel() >= i.second)
            allowedFactions.push_back(i.first);
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

    spam(BOT_TEXT2("suggest_faction", placeholders), eTalkType::General, true);
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
    out << entry->area_name[_dbc_locale];
    placeholders["%zone"] = out.str();

    spam(BOT_TEXT2("suggest_something", placeholders), urand(0, 1) ? eTalkType::General : 0, urand(0, 2), urand(0, 2));
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

        AreaTableEntry const* current_zone = GetAreaEntryByAreaID(bot->GetAreaId());
        if (!current_zone)
            continue;

        // combine full channel name
        char channelName[100];
        Channel* chn = nullptr;
        if ((channel->flags & CHANNEL_DBC_FLAG_LFG) != 0)
        {
            std::string chanName = channel->pattern[_dbc_locale];
            chn = cMgr->GetChannel(chanName, bot);
        }
        else
        {
            snprintf(channelName, 100, channel->pattern[_dbc_locale], current_zone->area_name[_dbc_locale]);
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
        if (chn->GetChannelId() == 22)
            continue;

        // no filter, pick several options
        if (flags == CHANNEL_FLAG_NONE)
        {
            channelNames.push_back(chn->GetName());
        }
        else
        {
            if (!bot->IsInChannel(chn))
                chn->JoinChannel(bot, "");
            chn->Say(bot->GetGUID(), msg.c_str(), LANG_UNIVERSAL);
        }

        if (!channelNames.empty())
        {
            std::string randomName = channelNames[urand(0, channelNames.size() - 1)];
            if (Channel* chn = cMgr->GetChannel(randomName, bot))
            {
                if (!bot->IsInChannel(chn))
                    chn->JoinChannel(bot, "");

                chn->Say(bot->GetGUID(), msg.c_str(), LANG_UNIVERSAL);
            }
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

SuggestDungeonAction::SuggestDungeonAction(PlayerbotAI* botAI) : SuggestWhatToDoAction(botAI, "suggest dungeon")
{
}

bool SuggestDungeonAction::Execute(Event event)
{
    // TODO: use sPlayerbotDungeonSuggestionMgr

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
    for (const auto& instance : instances)
    {
        if (bot->GetLevel() >= instance.second)
            allowedInstances.push_back(instance.first);
    }
    if (allowedInstances.empty()) return false;

    std::map<std::string, std::string> placeholders;
    placeholders["%role"] = ChatHelper::FormatClass(bot, AiFactory::GetPlayerSpecTab(bot));

    std::ostringstream itemout;
    //itemout << "|c00b000b0" << allowedInstances[urand(0, allowedInstances.size() - 1)] << "|r";
    itemout << allowedInstances[urand(0, allowedInstances.size() - 1)];
    placeholders["%instance"] = itemout.str();

    spam(BOT_TEXT2("suggest_instance", placeholders), urand(0, 1) ? eTalkType::LookingForGroup : 0, urand(0, 2), urand(0, 2));
    return true;
}

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

    spam(BOT_TEXT2("suggest_sell", placeholders), urand(0, 1) ? eTalkType::Trade : 0, urand(0, 1), urand(0, 5));
    return true;
}
