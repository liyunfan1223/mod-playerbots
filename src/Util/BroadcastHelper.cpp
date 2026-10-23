/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BroadcastHelper.h"
#include "AiFactory.h"
#include "Channel.h"
#include "Playerbots.h"
#include "ServerFacade.h"

BroadcastHelper::BroadcastHelper() {}

uint8 BroadcastHelper::GetLocale()
{
    uint8 locale = sWorld->GetDefaultDbcLocale();
    // -- In case we're using auto detect on config file^M
    if (locale >= TOTAL_LOCALES)
        locale = LocaleConstant::LOCALE_enUS;
    return locale;
}

bool BroadcastHelper::BroadcastTest(PlayerbotAI* botAI, Player* /* bot */)
{
    //return something to ignore the logic
    return false;

    std::map<std::string, std::string> placeholders;
    placeholders["%rand1"] = std::to_string(urand(0, 1));
    placeholders["%rand2"] = std::to_string(urand(0, 1));
    placeholders["%rand3"] = std::to_string(urand(0, 1));

    int32 rand = urand(0, 1);

    if (rand == 1 && botAI->SayToChannel(PlayerbotTextMgr::instance().GetBotText("Posted to trade, %rand1, %rand2, %rand3", placeholders), ChatChannelId::TRADE))
        return true;
    else if (botAI->SayToChannel(PlayerbotTextMgr::instance().GetBotText("Posted to GuildRecruitment, %rand1, %rand2, %rand3", placeholders), ChatChannelId::GUILD_RECRUITMENT))
        return true;

    return botAI->SayToChannel(PlayerbotTextMgr::instance().GetBotText("Posted to trade, %rand1, %rand2, %rand3", placeholders), ChatChannelId::TRADE);

    //int32 rand = urand(1, 8);
    if (rand == 1 && botAI->SayToGuild(PlayerbotTextMgr::instance().GetBotText("Posted to guild, %rand1, %rand2, %rand3", placeholders)))
        return true;
    else if (rand == 2 && botAI->SayToWorld(PlayerbotTextMgr::instance().GetBotText("Posted to world, %rand1, %rand2, %rand3", placeholders)))
        return true;
    else if (rand == 3 && botAI->SayToChannel(PlayerbotTextMgr::instance().GetBotText("Posted to general, %rand1, %rand2, %rand3", placeholders), ChatChannelId::GENERAL))
        return true;
    else if (rand == 4 && botAI->SayToChannel(PlayerbotTextMgr::instance().GetBotText("Posted to trade, %rand1, %rand2, %rand3", placeholders), ChatChannelId::TRADE))
        return true;
    else if (rand == 5 && botAI->SayToChannel(PlayerbotTextMgr::instance().GetBotText("Posted to LFG, %rand1, %rand2, %rand3", placeholders), ChatChannelId::LOOKING_FOR_GROUP))
        return true;
    else if (rand == 6 && botAI->SayToChannel(PlayerbotTextMgr::instance().GetBotText("Posted to LocalDefense, %rand1, %rand2, %rand3", placeholders), ChatChannelId::LOCAL_DEFENSE))
        return true;
    else if (rand == 7 && botAI->SayToChannel(PlayerbotTextMgr::instance().GetBotText("Posted to WorldDefense, %rand1, %rand2, %rand3", placeholders), ChatChannelId::WORLD_DEFENSE))
        return true;
    else if (rand == 8 && botAI->SayToChannel(PlayerbotTextMgr::instance().GetBotText("Posted to GuildRecruitment, %rand1, %rand2, %rand3", placeholders), ChatChannelId::GUILD_RECRUITMENT))
        return true;

    return false;
}

/**
@param toChannels - map of (ToChannel, chance), where chance is in range 0-100 as uint32 (unless global chance is not 100%)

@return true if said to the channel, false otherwise
*/
bool BroadcastHelper::BroadcastToChannelWithGlobalChance(PlayerbotAI* botAI, std::string message, std::list<std::pair<ToChannel, uint32>> toChannels)
{
    if (!sPlayerbotAIConfig.EnableBroadcasts)
        return false;
    if (message.empty())
    {
        return false;
    }

    for (auto const& pair : toChannels)
    {
        uint32 roll = urand(1, 100);
        uint32 chance = pair.second;
        uint32 broadcastRoll = urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue);

        switch (pair.first)
        {
            case TO_GUILD:
            {
                if (roll <= chance
                    && broadcastRoll <= sPlayerbotAIConfig.BroadcastToGuildGlobalChance
                    && botAI->SayToGuild(message))
                {
                    return true;
                }
                break;
            }
            case TO_WORLD:
            {
                if (roll <= chance
                    && broadcastRoll <= sPlayerbotAIConfig.BroadcastToWorldGlobalChance
                    && botAI->SayToWorld(message))
                {
                    return true;
                }
                break;
            }
            case TO_GENERAL:
            {
                if (roll <= chance
                    && broadcastRoll <= sPlayerbotAIConfig.BroadcastToGeneralGlobalChance
                    && botAI->SayToChannel(message, ChatChannelId::GENERAL))
                {
                    return true;
                }
                break;
            }
            case TO_TRADE:
            {
                if (roll <= chance
                    && broadcastRoll <= sPlayerbotAIConfig.BroadcastToTradeGlobalChance
                    && botAI->SayToChannel(message, ChatChannelId::TRADE))
                {
                    return true;
                }
                break;
            }
            case TO_LOOKING_FOR_GROUP:
            {
                if (roll <= chance
                    && broadcastRoll <= sPlayerbotAIConfig.BroadcastToLFGGlobalChance
                    && botAI->SayToChannel(message, ChatChannelId::LOOKING_FOR_GROUP))
                {
                    return true;
                }
                break;
            }
            case TO_LOCAL_DEFENSE:
            {
                if (roll <= chance
                    && broadcastRoll <= sPlayerbotAIConfig.BroadcastToLocalDefenseGlobalChance
                    && botAI->SayToChannel(message, ChatChannelId::LOCAL_DEFENSE))
                {
                    return true;
                }
                break;
            }
            case TO_WORLD_DEFENSE:
            {
                if (roll <= chance
                    && broadcastRoll <= sPlayerbotAIConfig.BroadcastToWorldDefenseGlobalChance
                    && botAI->SayToChannel(message, ChatChannelId::WORLD_DEFENSE))
                {
                    return true;
                }
                break;
            }
            case TO_GUILD_RECRUITMENT:
            {
                if (roll <= chance
                    && broadcastRoll <= sPlayerbotAIConfig.BroadcastToGuildRecruitmentGlobalChance
                    && botAI->SayToChannel(message, ChatChannelId::GUILD_RECRUITMENT))
                {
                    return true;
                }
                break;
            }
            default:
                break;
        }
    }

    return false;
}

bool BroadcastHelper::BroadcastLootingItem(PlayerbotAI* botAI, Player* bot, ItemTemplate const* proto)
{
    if (!sPlayerbotAIConfig.EnableBroadcasts)
        return false;
    std::map<std::string, std::string> placeholders;
    placeholders["%item_link"] = botAI->GetChatHelper()->FormatItem(proto);
    AreaTableEntry const* current_area = botAI->GetCurrentArea();
    AreaTableEntry const* current_zone = botAI->GetCurrentZone();
    placeholders["%area_name"] = current_area ? PlayerbotAI::GetLocalizedAreaName(current_area) : PlayerbotTextMgr::instance().GetBotText("string_unknown_area");
    placeholders["%zone_name"] = current_zone ? PlayerbotAI::GetLocalizedAreaName(current_zone) : PlayerbotTextMgr::instance().GetBotText("string_unknown_area");
    placeholders["%my_class"] = botAI->GetChatHelper()->FormatClass(bot->getClass());
    placeholders["%my_race"] = botAI->GetChatHelper()->FormatRace(bot->getRace());
    placeholders["%my_level"] = std::to_string(bot->GetLevel());

    switch (proto->Quality)
    {
        case ITEM_QUALITY_POOR:
            if (urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceLootingItemPoor)
            {
                return BroadcastToChannelWithGlobalChance(
                    botAI,
                    PlayerbotTextMgr::instance().GetBotText("broadcast_looting_item_poor", placeholders),
                    { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
                );
            }
            break;
        case ITEM_QUALITY_NORMAL:
            if (urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceLootingItemNormal)
            {
                return BroadcastToChannelWithGlobalChance(
                    botAI,
                    PlayerbotTextMgr::instance().GetBotText("broadcast_looting_item_normal", placeholders),
                    { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
                );
            }
            break;
        case ITEM_QUALITY_UNCOMMON:
            if (urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceLootingItemUncommon)
            {
                return BroadcastToChannelWithGlobalChance(
                    botAI,
                    PlayerbotTextMgr::instance().GetBotText("broadcast_looting_item_uncommon", placeholders),
                    { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
                );
            }
            break;
        case ITEM_QUALITY_RARE:
            if (urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceLootingItemRare)
            {
                return BroadcastToChannelWithGlobalChance(
                    botAI,
                    PlayerbotTextMgr::instance().GetBotText("broadcast_looting_item_rare", placeholders),
                    { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
                );
            }
            break;
        case ITEM_QUALITY_EPIC:
            if (urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceLootingItemEpic)
            {
                return BroadcastToChannelWithGlobalChance(
                    botAI,
                    PlayerbotTextMgr::instance().GetBotText("broadcast_looting_item_epic", placeholders),
                    { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
                );
            }
            break;
        case ITEM_QUALITY_LEGENDARY:
            if (urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceLootingItemLegendary)
            {
                return BroadcastToChannelWithGlobalChance(
                    botAI,
                    PlayerbotTextMgr::instance().GetBotText("broadcast_looting_item_legendary", placeholders),
                    { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
                );
            }
            break;
        case ITEM_QUALITY_ARTIFACT:
            if (urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceLootingItemArtifact)
            {
                return BroadcastToChannelWithGlobalChance(
                    botAI,
                    PlayerbotTextMgr::instance().GetBotText("broadcast_looting_item_artifact", placeholders),
                    { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
                );
            }
            break;
        default:
            break;
    }

    return false;
}

bool BroadcastHelper::BroadcastQuestAccepted(PlayerbotAI* botAI, Player* bot, Quest const* quest)
{
    if (!sPlayerbotAIConfig.EnableBroadcasts)
        return false;
    if (urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceQuestAccepted)
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%quest_link"] = botAI->GetChatHelper()->FormatQuest(quest);
        AreaTableEntry const* current_area = botAI->GetCurrentArea();
        AreaTableEntry const* current_zone = botAI->GetCurrentZone();
        placeholders["%area_name"] = current_area ? PlayerbotAI::GetLocalizedAreaName(current_area) : PlayerbotTextMgr::instance().GetBotText("string_unknown_area");
        placeholders["%zone_name"] = current_zone ? PlayerbotAI::GetLocalizedAreaName(current_zone) : PlayerbotTextMgr::instance().GetBotText("string_unknown_area");
        placeholders["%my_class"] = botAI->GetChatHelper()->FormatClass(bot->getClass());
        placeholders["%my_race"] = botAI->GetChatHelper()->FormatRace(bot->getRace());
        placeholders["%my_level"] = std::to_string(bot->GetLevel());

        return BroadcastToChannelWithGlobalChance(
            botAI,
            PlayerbotTextMgr::instance().GetBotText("broadcast_quest_accepted_generic", placeholders),
            { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

bool BroadcastHelper::BroadcastQuestUpdateAddKill(PlayerbotAI* botAI, Player* bot, Quest const* quest, uint32 availableCount, uint32 requiredCount, std::string obectiveName)
{
    if (!sPlayerbotAIConfig.EnableBroadcasts)
        return false;
    std::map<std::string, std::string> placeholders;
    AreaTableEntry const* current_area = botAI->GetCurrentArea();
    AreaTableEntry const* current_zone = botAI->GetCurrentZone();
    placeholders["%area_name"] = current_area ? PlayerbotAI::GetLocalizedAreaName(current_area) : PlayerbotTextMgr::instance().GetBotText("string_unknown_area");
    placeholders["%zone_name"] = current_zone ? PlayerbotAI::GetLocalizedAreaName(current_zone) : PlayerbotTextMgr::instance().GetBotText("string_unknown_area");
    placeholders["%quest_link"] = botAI->GetChatHelper()->FormatQuest(quest);
    placeholders["%quest_obj_name"] = obectiveName;
    placeholders["%my_class"] = botAI->GetChatHelper()->FormatClass(bot->getClass());
    placeholders["%my_race"] = botAI->GetChatHelper()->FormatRace(bot->getRace());
    placeholders["%my_level"] = std::to_string(bot->GetLevel());
    placeholders["%quest_obj_available"] = std::to_string(availableCount);
    placeholders["%quest_obj_required"] = std::to_string(requiredCount);
    placeholders["%quest_obj_missing"] = std::to_string(requiredCount - std::min(availableCount, requiredCount));
    placeholders["%quest_obj_full_formatted"] = botAI->GetChatHelper()->FormatQuestObjective(obectiveName, availableCount, requiredCount);

    if (availableCount < requiredCount
        && urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceQuestUpdateObjectiveProgress)
    {
        return BroadcastToChannelWithGlobalChance(
            botAI,
            PlayerbotTextMgr::instance().GetBotText("broadcast_quest_update_add_kill_objective_progress", placeholders),
            { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }
    else if (availableCount == requiredCount
        && urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceQuestUpdateObjectiveCompleted)
    {
        return BroadcastToChannelWithGlobalChance(
            botAI,
            PlayerbotTextMgr::instance().GetBotText("broadcast_quest_update_add_kill_objective_completed", placeholders),
            { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

bool BroadcastHelper::BroadcastQuestUpdateAddItem(PlayerbotAI* botAI, Player* bot, Quest const* quest, uint32 availableCount, uint32 requiredCount, ItemTemplate const* proto)
{
    if (!sPlayerbotAIConfig.EnableBroadcasts)
        return false;
    std::map<std::string, std::string> placeholders;
    AreaTableEntry const* current_area = botAI->GetCurrentArea();
    AreaTableEntry const* current_zone = botAI->GetCurrentZone();
    placeholders["%area_name"] = current_area ? PlayerbotAI::GetLocalizedAreaName(current_area) : PlayerbotTextMgr::instance().GetBotText("string_unknown_area");
    placeholders["%zone_name"] = current_zone ? PlayerbotAI::GetLocalizedAreaName(current_zone) : PlayerbotTextMgr::instance().GetBotText("string_unknown_area");
    placeholders["%quest_link"] = botAI->GetChatHelper()->FormatQuest(quest);
    std::string itemLinkFormatted = botAI->GetChatHelper()->FormatItem(proto);
    placeholders["%item_link"] = itemLinkFormatted;
    placeholders["%my_class"] = botAI->GetChatHelper()->FormatClass(bot->getClass());
    placeholders["%my_race"] = botAI->GetChatHelper()->FormatRace(bot->getRace());
    placeholders["%my_level"] = std::to_string(bot->GetLevel());
    placeholders["%quest_obj_available"] = std::to_string(availableCount);
    placeholders["%quest_obj_required"] = std::to_string(requiredCount);
    placeholders["%quest_obj_missing"] = std::to_string(requiredCount - std::min(availableCount, requiredCount));
    placeholders["%quest_obj_full_formatted"] = botAI->GetChatHelper()->FormatQuestObjective(itemLinkFormatted, availableCount, requiredCount);

    if (availableCount < requiredCount
        && urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceQuestUpdateObjectiveProgress)
    {
        return BroadcastToChannelWithGlobalChance(
            botAI,
            PlayerbotTextMgr::instance().GetBotText("broadcast_quest_update_add_item_objective_progress", placeholders),
            { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }
    else if (availableCount == requiredCount
        && urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceQuestUpdateObjectiveCompleted)
    {
        return BroadcastToChannelWithGlobalChance(
            botAI,
            PlayerbotTextMgr::instance().GetBotText("broadcast_quest_update_add_item_objective_completed", placeholders),
            { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

bool BroadcastHelper::BroadcastQuestUpdateFailedTimer(PlayerbotAI* botAI, Player* bot, Quest const* quest)
{
    if (!sPlayerbotAIConfig.EnableBroadcasts)
        return false;
    if (urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceQuestUpdateFailedTimer)
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%quest_link"] = botAI->GetChatHelper()->FormatQuest(quest);
        AreaTableEntry const* current_area = botAI->GetCurrentArea();
        AreaTableEntry const* current_zone = botAI->GetCurrentZone();
        placeholders["%area_name"] = current_area ? PlayerbotAI::GetLocalizedAreaName(current_area) : PlayerbotTextMgr::instance().GetBotText("string_unknown_area");
        placeholders["%zone_name"] = current_zone ? PlayerbotAI::GetLocalizedAreaName(current_zone) : PlayerbotTextMgr::instance().GetBotText("string_unknown_area");
        placeholders["%my_class"] = botAI->GetChatHelper()->FormatClass(bot->getClass());
        placeholders["%my_race"] = botAI->GetChatHelper()->FormatRace(bot->getRace());
        placeholders["%my_level"] = std::to_string(bot->GetLevel());

        return BroadcastToChannelWithGlobalChance(
            botAI,
            PlayerbotTextMgr::instance().GetBotText("broadcast_quest_update_failed_timer", placeholders),
            { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

bool BroadcastHelper::BroadcastQuestUpdateComplete(PlayerbotAI* botAI, Player* bot, Quest const* quest)
{
    if (!sPlayerbotAIConfig.EnableBroadcasts)
        return false;
    if (urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceQuestUpdateComplete)
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%quest_link"] = botAI->GetChatHelper()->FormatQuest(quest);
        AreaTableEntry const* current_area = botAI->GetCurrentArea();
        AreaTableEntry const* current_zone = botAI->GetCurrentZone();
        placeholders["%area_name"] = current_area ? PlayerbotAI::GetLocalizedAreaName(current_area) : PlayerbotTextMgr::instance().GetBotText("string_unknown_area");
        placeholders["%zone_name"] = current_zone ? PlayerbotAI::GetLocalizedAreaName(current_zone) : PlayerbotTextMgr::instance().GetBotText("string_unknown_area");
        placeholders["%my_class"] = botAI->GetChatHelper()->FormatClass(bot->getClass());
        placeholders["%my_race"] = botAI->GetChatHelper()->FormatRace(bot->getRace());
        placeholders["%my_level"] = std::to_string(bot->GetLevel());

        return BroadcastToChannelWithGlobalChance(
            botAI,
            PlayerbotTextMgr::instance().GetBotText("broadcast_quest_update_complete", placeholders),
            { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

bool BroadcastHelper::BroadcastQuestTurnedIn(PlayerbotAI* botAI, Player* bot, Quest const* quest)
{
    if (!sPlayerbotAIConfig.EnableBroadcasts)
        return false;
    if (urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceQuestTurnedIn)
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%quest_link"] = botAI->GetChatHelper()->FormatQuest(quest);
        AreaTableEntry const* current_area = botAI->GetCurrentArea();
        AreaTableEntry const* current_zone = botAI->GetCurrentZone();
        placeholders["%area_name"] = current_area ? PlayerbotAI::GetLocalizedAreaName(current_area) : PlayerbotTextMgr::instance().GetBotText("string_unknown_area");
        placeholders["%zone_name"] = current_zone ? PlayerbotAI::GetLocalizedAreaName(current_zone) : PlayerbotTextMgr::instance().GetBotText("string_unknown_area");
        placeholders["%my_class"] = botAI->GetChatHelper()->FormatClass(bot->getClass());
        placeholders["%my_race"] = botAI->GetChatHelper()->FormatRace(bot->getRace());
        placeholders["%my_level"] = std::to_string(bot->GetLevel());

        return BroadcastToChannelWithGlobalChance(
            botAI,
            PlayerbotTextMgr::instance().GetBotText("broadcast_quest_turned_in", placeholders),
            { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

bool BroadcastHelper::BroadcastKill(PlayerbotAI* botAI, Player* bot, Creature *creature)
{
    if (!sPlayerbotAIConfig.EnableBroadcasts)
        return false;
    std::map<std::string, std::string> placeholders;
    placeholders["%victim_name"] = creature->GetName();
    AreaTableEntry const* current_area = botAI->GetCurrentArea();
    AreaTableEntry const* current_zone = botAI->GetCurrentZone();
    placeholders["%area_name"] = current_area ? PlayerbotAI::GetLocalizedAreaName(current_area) : PlayerbotTextMgr::instance().GetBotText("string_unknown_area");
    placeholders["%zone_name"] = current_zone ? PlayerbotAI::GetLocalizedAreaName(current_zone) : PlayerbotTextMgr::instance().GetBotText("string_unknown_area");
    placeholders["%victim_level"] = creature->GetLevel();
    placeholders["%my_class"] = botAI->GetChatHelper()->FormatClass(bot->getClass());
    placeholders["%my_race"] = botAI->GetChatHelper()->FormatRace(bot->getRace());
    placeholders["%my_level"] = std::to_string(bot->GetLevel());

    //if ((creature->IsElite() && !creature->GetMap()->IsDungeon())
    //if creature->IsWorldBoss()
    //if creature->GetLevel() > DEFAULT_MAX_LEVEL + 1
    //if creature->GetLevel() > bot->GetLevel() + 4

    if (creature->IsPet())
    {
        if (urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceKillPet)
        {
            return BroadcastToChannelWithGlobalChance(
                botAI,
                PlayerbotTextMgr::instance().GetBotText("broadcast_killed_pet", placeholders),
                { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
            );
        }
    }
    else if (creature->IsPlayer())
    {
        if (urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceKillPlayer)
        {
            placeholders["%victim_class"] = botAI->GetChatHelper()->FormatClass(creature->getClass());

            return BroadcastToChannelWithGlobalChance(
                botAI,
                PlayerbotTextMgr::instance().GetBotText("broadcast_killed_player", placeholders),
                { {TO_WORLD_DEFENSE, 50}, {TO_LOCAL_DEFENSE, 50}, {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
            );
        }
    }
    else
    {
        switch (creature->GetCreatureTemplate()->rank)
        {
            case CREATURE_ELITE_NORMAL:
                if (urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceKillNormal)
                {
                    return BroadcastToChannelWithGlobalChance(
                        botAI,
                        PlayerbotTextMgr::instance().GetBotText("broadcast_killed_normal", placeholders),
                        { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
                    );
                }
                break;
            case CREATURE_ELITE_ELITE:
                if (urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceKillElite)
                {
                    return BroadcastToChannelWithGlobalChance(
                        botAI,
                        PlayerbotTextMgr::instance().GetBotText("broadcast_killed_elite", placeholders),
                        { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
                    );
                }
                break;
            case CREATURE_ELITE_RAREELITE:
                if (urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceKillRareelite)
                {
                    return BroadcastToChannelWithGlobalChance(
                        botAI,
                        PlayerbotTextMgr::instance().GetBotText("broadcast_killed_rareelite", placeholders),
                        { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
                    );
                }
                break;
            case CREATURE_ELITE_WORLDBOSS:
                if (urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceKillWorldboss)
                {
                    return BroadcastToChannelWithGlobalChance(
                        botAI,
                        PlayerbotTextMgr::instance().GetBotText("broadcast_killed_worldboss", placeholders),
                        { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
                    );
                }
                break;
            case CREATURE_ELITE_RARE:
                if (urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceKillRare)
                {
                    return BroadcastToChannelWithGlobalChance(
                        botAI,
                        PlayerbotTextMgr::instance().GetBotText("broadcast_killed_rare", placeholders),
                        { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
                    );
                }
                break;
            case CREATURE_UNKNOWN:
                if (urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceKillUnknown)
                {
                    return BroadcastToChannelWithGlobalChance(
                        botAI,
                        PlayerbotTextMgr::instance().GetBotText("broadcast_killed_unknown", placeholders),
                        { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
                    );
                }
                break;
            default:
                break;
        }
    }

    return false;
}

bool BroadcastHelper::BroadcastLevelup(PlayerbotAI* botAI, Player* bot)
{
    if (!sPlayerbotAIConfig.EnableBroadcasts)
        return false;
    uint32 level = bot->GetLevel();

    std::map<std::string, std::string> placeholders;
    AreaTableEntry const* current_area = botAI->GetCurrentArea();
    AreaTableEntry const* current_zone = botAI->GetCurrentZone();
    placeholders["%area_name"] = current_area ? PlayerbotAI::GetLocalizedAreaName(current_area) : PlayerbotTextMgr::instance().GetBotText("string_unknown_area");
    placeholders["%zone_name"] = current_zone ? PlayerbotAI::GetLocalizedAreaName(current_zone) : PlayerbotTextMgr::instance().GetBotText("string_unknown_area");
    placeholders["%my_class"] = botAI->GetChatHelper()->FormatClass(bot->getClass());
    placeholders["%my_race"] = botAI->GetChatHelper()->FormatRace(bot->getRace());
    placeholders["%my_level"] = std::to_string(level);

    if (level == sPlayerbotAIConfig.RandomBotMaxLevel
        && urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceLevelupMaxLevel)
    {
        return BroadcastToChannelWithGlobalChance(
            botAI,
            PlayerbotTextMgr::instance().GetBotText("broadcast_levelup_max_level", placeholders),
            { {TO_GUILD, 30}, {TO_WORLD, 90}, {TO_GENERAL, 100} }
        );
    }
    // It's divisible by 10
    else if (level % 10 == 0
        && urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceLevelupTenX)
    {
        return BroadcastToChannelWithGlobalChance(
            botAI,
            PlayerbotTextMgr::instance().GetBotText("broadcast_levelup_10x", placeholders),
            { {TO_GUILD, 50}, {TO_WORLD, 90}, {TO_GENERAL, 100} }
        );
    }
    else if (urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceLevelupGeneric)
    {
        return BroadcastToChannelWithGlobalChance(
            botAI,
            PlayerbotTextMgr::instance().GetBotText("broadcast_levelup_generic", placeholders),
            { {TO_GUILD, 90}, {TO_WORLD, 90}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

bool BroadcastHelper::BroadcastGuildMemberPromotion(PlayerbotAI* botAI, Player* /* bot */, Player* player)
{
    if (!sPlayerbotAIConfig.EnableBroadcasts)
        return false;
    if (urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceGuildManagement)
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%other_name"] = player->GetName();
        placeholders["%other_class"] = botAI->GetChatHelper()->FormatClass(player->getClass());
        placeholders["%other_race"] = botAI->GetChatHelper()->FormatRace(player->getRace());
        placeholders["%other_level"] = std::to_string(player->GetLevel());

        return botAI->SayToGuild(PlayerbotTextMgr::instance().GetBotText("broadcast_guild_promotion", placeholders));
    }

    return false;
}

bool BroadcastHelper::BroadcastGuildMemberDemotion(PlayerbotAI* botAI, Player* /* bot */, Player* player)
{
    if (urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceGuildManagement)
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%other_name"] = player->GetName();
        placeholders["%other_class"] = botAI->GetChatHelper()->FormatClass(player->getClass());
        placeholders["%other_race"] = botAI->GetChatHelper()->FormatRace(player->getRace());
        placeholders["%other_level"] = std::to_string(player->GetLevel());

        return botAI->SayToGuild(PlayerbotTextMgr::instance().GetBotText("broadcast_guild_demotion", placeholders));
    }

    return false;
}

bool BroadcastHelper::BroadcastGuildGroupOrRaidInvite(PlayerbotAI* botAI, Player* /* bot */, Player* player, Group* group)
{
    if (!sPlayerbotAIConfig.EnableBroadcasts)
        return false;
    std::map<std::string, std::string> placeholders;
    placeholders["%name"] = player->GetName();
    AreaTableEntry const* current_area = botAI->GetCurrentArea();
    AreaTableEntry const* current_zone = botAI->GetCurrentZone();
    placeholders["%area_name"] = current_area ? PlayerbotAI::GetLocalizedAreaName(current_area) : PlayerbotTextMgr::instance().GetBotText("string_unknown_area");
    placeholders["%zone_name"] = current_zone ? PlayerbotAI::GetLocalizedAreaName(current_zone) : PlayerbotTextMgr::instance().GetBotText("string_unknown_area");

    //TODO move texts to sql!
    if (group && group->isRaidGroup())
    {
        if (urand(0, 3))
        {
            return botAI->SayToGuild(PlayerbotTextMgr::instance().GetBotText("Hey anyone want to raid in %zone_name", placeholders));
        }
        else
        {
            return botAI->SayToGuild(PlayerbotTextMgr::instance().GetBotText("Hey %name I'm raiding in %zone_name do you wan to join me?", placeholders));
        }
    }
    else
    {
        //(bot->GetTeam() == ALLIANCE ? LANG_COMMON : LANG_ORCISH)
        if (urand(0, 3))
        {
            return botAI->SayToGuild(PlayerbotTextMgr::instance().GetBotText("Hey anyone wanna group up in %zone_name?", placeholders));
        }
        else
        {
            return botAI->SayToGuild(PlayerbotTextMgr::instance().GetBotText("Hey %name do you want join my group? I'm heading for %zone_name", placeholders));
        }
    }

    return false;
}

bool BroadcastHelper::BroadcastSuggestInstance(PlayerbotAI* botAI, std::vector<std::string>& allowedInstances, Player* bot)
{
    if (!sPlayerbotAIConfig.EnableBroadcasts)
        return false;
    if (urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceSuggestInstance)
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%my_role"] = ChatHelper::FormatClass(bot, AiFactory::GetPlayerSpecTab(bot));

        std::ostringstream itemout;
        //itemout << "|c00b000b0" << allowedInstances[urand(0, allowedInstances.size() - 1)] << "|r";
        itemout << allowedInstances[urand(0, allowedInstances.size() - 1)];
        placeholders["%instance_name"] = itemout.str();

        placeholders["%my_class"] = botAI->GetChatHelper()->FormatClass(bot->getClass());
        placeholders["%my_race"] = botAI->GetChatHelper()->FormatRace(bot->getRace());
        placeholders["%my_level"] = std::to_string(bot->GetLevel());

        return BroadcastToChannelWithGlobalChance(
            botAI,
            PlayerbotTextMgr::instance().GetBotText("suggest_instance", placeholders),
            { {TO_LOOKING_FOR_GROUP, 50}, {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

bool BroadcastHelper::BroadcastSuggestQuest(PlayerbotAI* botAI, std::vector<uint32>& quests, Player* bot)
{
    if (!sPlayerbotAIConfig.EnableBroadcasts)
        return false;
    if (urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceSuggestQuest)
    {

        int index = rand() % quests.size();

        Quest const* quest = sObjectMgr->GetQuestTemplate(quests[index]);

        std::map<std::string, std::string> placeholders;
        placeholders["%my_role"] = ChatHelper::FormatClass(bot, AiFactory::GetPlayerSpecTab(bot));
        placeholders["%quest_link"] = botAI->GetChatHelper()->FormatQuest(quest);
        placeholders["%quest_level"] = std::to_string(quest->GetQuestLevel());
        placeholders["%my_class"] = botAI->GetChatHelper()->FormatClass(bot->getClass());
        placeholders["%my_race"] = botAI->GetChatHelper()->FormatRace(bot->getRace());
        placeholders["%my_level"] = std::to_string(bot->GetLevel());

        return BroadcastToChannelWithGlobalChance(
            botAI,
            PlayerbotTextMgr::instance().GetBotText("suggest_quest", placeholders),
            { {TO_LOOKING_FOR_GROUP, 50}, {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

bool BroadcastHelper::BroadcastSuggestGrindMaterials(PlayerbotAI* botAI, std::string item, Player* bot)
{
    if (!sPlayerbotAIConfig.EnableBroadcasts)
        return false;
    if (urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceSuggestGrindMaterials)
    {

        std::map<std::string, std::string> placeholders;
        placeholders["%my_role"] = ChatHelper::FormatClass(bot, AiFactory::GetPlayerSpecTab(bot));
        placeholders["%category"] = item;

        placeholders["%my_class"] = botAI->GetChatHelper()->FormatClass(bot->getClass());
        placeholders["%my_race"] = botAI->GetChatHelper()->FormatRace(bot->getRace());
        placeholders["%my_level"] = std::to_string(bot->GetLevel());

        return BroadcastToChannelWithGlobalChance(
            botAI,
            PlayerbotTextMgr::instance().GetBotText("suggest_trade", placeholders),
            { {TO_TRADE, 50}, {TO_LOOKING_FOR_GROUP, 50}, {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

bool BroadcastHelper::BroadcastSuggestGrindReputation(PlayerbotAI* botAI, std::vector<std::string> levels, std::vector<std::string> allowedFactions, Player* bot)
{
    if (!sPlayerbotAIConfig.EnableBroadcasts)
        return false;
    if (urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceSuggestGrindReputation)
    {

        std::map<std::string, std::string> placeholders;
        placeholders["%my_role"] = ChatHelper::FormatClass(bot, AiFactory::GetPlayerSpecTab(bot));
        placeholders["%rep_level"] = levels[urand(0, 2)];
        std::ostringstream rnd; rnd << urand(1, 5) << "K";
        placeholders["%rndK"] = rnd.str();

        std::ostringstream itemout;
        //itemout << "|c004040b0" << allowedFactions[urand(0, allowedFactions.size() - 1)] << "|r";
        itemout << allowedFactions[urand(0, allowedFactions.size() - 1)];
        placeholders["%faction"] = itemout.str();

        placeholders["%my_class"] = botAI->GetChatHelper()->FormatClass(bot->getClass());
        placeholders["%my_race"] = botAI->GetChatHelper()->FormatRace(bot->getRace());
        placeholders["%my_level"] = std::to_string(bot->GetLevel());

        return BroadcastToChannelWithGlobalChance(
            botAI,
            PlayerbotTextMgr::instance().GetBotText("suggest_faction", placeholders),
            { {TO_LOOKING_FOR_GROUP, 50}, {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

bool BroadcastHelper::BroadcastSuggestSell(PlayerbotAI* botAI, ItemTemplate const* proto, uint32 count, uint32 price, Player* bot)
{
    if (!sPlayerbotAIConfig.EnableBroadcasts)
        return false;
    if (urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceSuggestSell)
    {

        std::map<std::string, std::string> placeholders;
        placeholders["%item_link"] = botAI->GetChatHelper()->FormatItem(proto, 0);
        placeholders["%item_formatted_link"] = botAI->GetChatHelper()->FormatItem(proto, count);
        placeholders["%item_count"] = std::to_string(count);
        placeholders["%cost_gold"] = botAI->GetChatHelper()->formatMoney(price);

        placeholders["%my_class"] = botAI->GetChatHelper()->FormatClass(bot->getClass());
        placeholders["%my_race"] = botAI->GetChatHelper()->FormatRace(bot->getRace());
        placeholders["%my_level"] = std::to_string(bot->GetLevel());

        return BroadcastToChannelWithGlobalChance(
            botAI,
            PlayerbotTextMgr::instance().GetBotText("suggest_sell", placeholders),
            { {TO_TRADE, 90}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

bool BroadcastHelper::BroadcastSuggestSomething(PlayerbotAI* botAI, Player* bot)
{
    if (!sPlayerbotAIConfig.EnableBroadcasts)
        return false;
    if (urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceSuggestSomething)
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%my_role"] = ChatHelper::FormatClass(bot, AiFactory::GetPlayerSpecTab(bot));

        AreaTableEntry const* current_area = botAI->GetCurrentArea();
        AreaTableEntry const* current_zone = botAI->GetCurrentZone();
        placeholders["%area_name"] = current_area ? PlayerbotAI::GetLocalizedAreaName(current_area) : PlayerbotTextMgr::instance().GetBotText("string_unknown_area");
        placeholders["%zone_name"] = current_zone ? PlayerbotAI::GetLocalizedAreaName(current_zone) : PlayerbotTextMgr::instance().GetBotText("string_unknown_area");
        placeholders["%my_class"] = botAI->GetChatHelper()->FormatClass(bot->getClass());
        placeholders["%my_race"] = botAI->GetChatHelper()->FormatRace(bot->getRace());
        placeholders["%my_level"] = std::to_string(bot->GetLevel());

        return BroadcastToChannelWithGlobalChance(
            botAI,
            PlayerbotTextMgr::instance().GetBotText("suggest_something", placeholders),
            { {TO_GUILD, 10}, {TO_WORLD, 70}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

bool BroadcastHelper::BroadcastSuggestSomethingToxic(PlayerbotAI* botAI, Player* bot)
{
    if (!sPlayerbotAIConfig.EnableBroadcasts)
        return false;
    if (urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceSuggestSomethingToxic)
    {
        //items
        std::vector<Item*> botItems = botAI->GetInventoryAndEquippedItems();

        std::map<std::string, std::string> placeholders;

        placeholders["%random_inventory_item_link"] = botItems.size() > 0 ? botAI->GetChatHelper()->FormatItem(botItems[rand() % botItems.size()]->GetTemplate()) : PlayerbotTextMgr::instance().GetBotText("string_empty_link");

        placeholders["%my_role"] = ChatHelper::FormatClass(bot, AiFactory::GetPlayerSpecTab(bot));
        AreaTableEntry const* current_area = botAI->GetCurrentArea();
        AreaTableEntry const* current_zone = botAI->GetCurrentZone();
        placeholders["%area_name"] = current_area ? PlayerbotAI::GetLocalizedAreaName(current_area) : PlayerbotTextMgr::instance().GetBotText("string_unknown_area");
        placeholders["%zone_name"] = current_zone ? PlayerbotAI::GetLocalizedAreaName(current_zone) : PlayerbotTextMgr::instance().GetBotText("string_unknown_area");
        placeholders["%my_class"] = botAI->GetChatHelper()->FormatClass(bot->getClass());
        placeholders["%my_race"] = botAI->GetChatHelper()->FormatRace(bot->getRace());
        placeholders["%my_level"] = std::to_string(bot->GetLevel());

        return BroadcastToChannelWithGlobalChance(
            botAI,
            PlayerbotTextMgr::instance().GetBotText("suggest_something_toxic", placeholders),
            { {TO_GUILD, 10}, {TO_WORLD, 70}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

bool BroadcastHelper::BroadcastSuggestToxicLinks(PlayerbotAI* botAI, Player* bot)
{
    if (!sPlayerbotAIConfig.EnableBroadcasts)
        return false;
    if (urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceSuggestToxicLinks)
    {
        //quests
        std::vector<uint32> incompleteQuests;
        for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
        {
            uint32 questId = bot->GetQuestSlotQuestId(slot);
            if (!questId)
                continue;

            QuestStatus status = bot->GetQuestStatus(questId);
            if (status == QUEST_STATUS_INCOMPLETE || status == QUEST_STATUS_NONE)
                incompleteQuests.push_back(questId);
        }

        //items
        std::vector<Item*> botItems = botAI->GetInventoryAndEquippedItems();

        //spells
        //?

        std::map<std::string, std::string> placeholders;

        placeholders["%random_inventory_item_link"] = botItems.size() > 0 ? botAI->GetChatHelper()->FormatItem(botItems[rand() % botItems.size()]->GetTemplate()) : PlayerbotTextMgr::instance().GetBotText("string_empty_link");
        placeholders["%prefix"] = sPlayerbotAIConfig.ToxicLinksPrefix;

        if (incompleteQuests.size() > 0)
        {
            Quest const* quest = sObjectMgr->GetQuestTemplate(incompleteQuests[rand() % incompleteQuests.size()]);
            placeholders["%random_taken_quest_or_item_link"] = botAI->GetChatHelper()->FormatQuest(quest);
        }
        else
        {
            placeholders["%random_taken_quest_or_item_link"] = placeholders["%random_inventory_item_link"];
        }

        placeholders["%my_role"] = ChatHelper::FormatClass(bot, AiFactory::GetPlayerSpecTab(bot));
        AreaTableEntry const* current_area = botAI->GetCurrentArea();
        AreaTableEntry const* current_zone = botAI->GetCurrentZone();
        placeholders["%area_name"] = current_area ? PlayerbotAI::GetLocalizedAreaName(current_area) : PlayerbotTextMgr::instance().GetBotText("string_unknown_area");
        placeholders["%zone_name"] = current_zone ? PlayerbotAI::GetLocalizedAreaName(current_zone) : PlayerbotTextMgr::instance().GetBotText("string_unknown_area");
        placeholders["%my_class"] = botAI->GetChatHelper()->FormatClass(bot->getClass());
        placeholders["%my_race"] = botAI->GetChatHelper()->FormatRace(bot->getRace());
        placeholders["%my_level"] = std::to_string(bot->GetLevel());

        return BroadcastToChannelWithGlobalChance(
            botAI,
            PlayerbotTextMgr::instance().GetBotText("suggest_toxic_links", placeholders),
            { {TO_GUILD, 10}, {TO_WORLD, 70}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

bool BroadcastHelper::BroadcastSuggestThunderfury(PlayerbotAI* botAI, Player* bot)
{
    if (urand(1, sPlayerbotAIConfig.BroadcastChanceMaxValue) <= sPlayerbotAIConfig.BroadcastChanceSuggestThunderfury)
    {
        std::map<std::string, std::string> placeholders;
        ItemTemplate const* thunderfuryProto = sObjectMgr->GetItemTemplate(19019);
        placeholders["%thunderfury_link"] = GET_PLAYERBOT_AI(bot)->GetChatHelper()->FormatItem(thunderfuryProto);

        return BroadcastToChannelWithGlobalChance(
            botAI,
            PlayerbotTextMgr::instance().GetBotText("thunderfury_spam", placeholders),
            { {TO_WORLD, 70}, {TO_GENERAL, 100} }
        );
    }

    return false;
}
