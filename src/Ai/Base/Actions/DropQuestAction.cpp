/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "DropQuestAction.h"
#include "ChatHelper.h"
#include "Event.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"

bool DropQuestAction::Execute(Event event)
{
    std::string const link = event.getParam();

    Player* master = GetMaster();
    if (!master)
        return false;

    PlayerbotChatHandler handler(master);
    uint32 entry = handler.extractQuestId(link);

    // remove all quest entries for 'entry' from quest log
    for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 logQuest = bot->GetQuestSlotQuestId(slot);

        Quest const* quest = sObjectMgr->GetQuestTemplate(logQuest);
        if (!quest)
            continue;

        if (logQuest == entry || link.find(quest->GetTitle()) != std::string::npos)
        {
            bot->SetQuestSlot(slot, 0);

            // we ignore unequippable quest items in this case, its' still be equipped
            bot->TakeQuestSourceItem(logQuest, false);
            entry = logQuest;
            break;
        }
    }

    if (!entry)
        return false;

    bot->RemoveRewardedQuest(entry);
    bot->RemoveActiveQuest(entry, false);

    if (botAI->HasStrategy("debug quest", BotState::BOT_STATE_NON_COMBAT) || botAI->HasStrategy("debug rpg", BotState::BOT_STATE_COMBAT))
    {
        Quest const* pQuest = sObjectMgr->GetQuestTemplate(entry);
        const std::string text_quest = ChatHelper::FormatQuest(pQuest);
        LOG_INFO("playerbots", "{} => Quest [ {} ] removed", bot->GetName(), pQuest->GetTitle());
        std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "quest_removed_debug",
            "Quest [%quest] removed",
            {{"%quest", text_quest}});
        bot->Say(text, LANG_UNIVERSAL);
    }

    botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "quest_remove", "Quest removed", {}));
    return true;
}

bool CleanQuestLogAction::Execute(Event event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    if (!requester)
        return false;

    if (!sPlayerbotAIConfig.DropObsoleteQuests)
        return false;

    // Only output this message if "debug rpg" strategy is enabled
    if (botAI->HasStrategy("debug rpg", BotState::BOT_STATE_COMBAT))
        botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "clean_quest_log_started",
            "Clean Quest Log command received, removing grey/trivial quests...",
            {}));

    uint8 botLevel = bot->GetLevel();  // Get bot's level

    for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 questId = bot->GetQuestSlotQuestId(slot);
        if (!questId)
            continue;

        Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
        if (!quest)
            continue;

        // Determine if quest is trivial by comparing levels
        int32 questLevel = quest->GetQuestLevel();
        if (questLevel == -1) // For scaling quests, default to bot level
            questLevel = botLevel;

        // Set the level difference for when a quest becomes trivial
        // This was determined by using the Lua code the client uses
        int32 trivialLevel = 5;
        if (botLevel >= 40)
            trivialLevel = 8;
        else if (botLevel >= 30)
            trivialLevel = 7;
        else if (botLevel >= 20)
            trivialLevel = 6;

        // Check if the quest is trivial (grey) for the bot
        if ((botLevel - questLevel) > trivialLevel)
        {
            // Output only if "debug rpg" strategy is enabled
            if (botAI->HasStrategy("debug rpg", BotState::BOT_STATE_COMBAT))
                botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                    "quest_trivial_will_remove",
                    "Quest [%title] will be removed because it is trivial (grey).",
                    {{"%title", quest->GetTitle()}}));

            // Remove quest
            botAI->rpgStatistic.questDropped++;
            bot->SetQuestSlot(slot, 0);
            bot->TakeQuestSourceItem(questId, false);
            bot->SetQuestStatus(questId, QUEST_STATUS_NONE);
            bot->RemoveRewardedQuest(questId);

            if (botAI->HasStrategy("debug rpg", BotState::BOT_STATE_COMBAT))
            {
                const std::string text_quest = ChatHelper::FormatQuest(quest);
                LOG_INFO("playerbots", "{} => Quest [ {} ] removed", bot->GetName(), quest->GetTitle());
                std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                    "quest_removed_debug",
                    "Quest [%quest] removed",
                    {{"%quest", text_quest}});
                bot->Say(text, LANG_UNIVERSAL);
            }

            if (botAI->HasStrategy("debug rpg", BotState::BOT_STATE_COMBAT))
                botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                    "quest_has_been_removed",
                    "Quest [%title] has been removed.",
                    {{"%title", quest->GetTitle()}}));
        }
        else
        {
            // Only output if "debug rpg" strategy is enabled
            if (botAI->HasStrategy("debug rpg", BotState::BOT_STATE_COMBAT))
                botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                    "quest_not_trivial_kept",
                    "Quest [%title] is not trivial and will be kept.",
                    {{"%title", quest->GetTitle()}}));
        }
    }

    return true;
}

void CleanQuestLogAction::DropQuestType(uint8& numQuest, uint8 wantNum, bool isGreen, bool hasProgress, bool isComplete)
{
    std::vector<uint8> slots;
    for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
        slots.push_back(slot);

    if (wantNum < 100)
    {
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(slots.begin(), slots.end(), g);
    }

    for (uint8 slot : slots)
    {
        uint32 questId = bot->GetQuestSlotQuestId(slot);
        if (!questId)
            continue;

        Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
        if (!quest)
            continue;

        // Do not drop class quest, may be not rewarding gold but important spells
        if (quest->GetRequiredClasses())
            continue;

        if (wantNum == 100)
            numQuest++;

        int32 lowLevelDiff = sWorld->getIntConfig(CONFIG_QUEST_LOW_LEVEL_HIDE_DIFF);
        if (lowLevelDiff < 0 ||
            bot->GetLevel() <= bot->GetQuestLevel(quest) + uint32(lowLevelDiff))  // Quest is not gray
        {
            if (bot->GetLevel() + 5 > bot->GetQuestLevel(quest))  // Quest is not red
            {
                if (!isGreen)
                    continue;
            }
        }
        else  // Quest is gray
        {
            if (isGreen)
                continue;
        }

        if (HasProgress(bot, quest) && !hasProgress && bot->GetQuestStatus(questId) != QUEST_STATUS_FAILED)
            continue;

        if (bot->GetQuestStatus(questId) == QUEST_STATUS_COMPLETE && !isComplete)
            continue;

        // Always drop failed quests
        if (numQuest <= wantNum && bot->GetQuestStatus(questId) != QUEST_STATUS_FAILED)
            continue;

        // Drop quest.
        bot->SetQuestSlot(slot, 0);

        // We ignore unequippable quest items in this case, its' still be equipped
        bot->TakeQuestSourceItem(questId, false);

        bot->SetQuestStatus(questId, QUEST_STATUS_NONE);
        bot->RemoveRewardedQuest(questId);

        numQuest--;

        if (botAI->HasStrategy("debug quest", BotState::BOT_STATE_NON_COMBAT) || botAI->HasStrategy("debug rpg", BotState::BOT_STATE_COMBAT))
        {
            const std::string text_quest = ChatHelper::FormatQuest(quest);
            LOG_INFO("playerbots", "{} => Quest [ {} ] removed", bot->GetName(), quest->GetTitle());
            std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "quest_removed_debug",
                "Quest [%quest] removed",
                {{"%quest", text_quest}});
            bot->Say(text, LANG_UNIVERSAL);
        }
        botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "quest_removed_with_name",
            "Quest removed %quest",
            {{"%quest", chat->FormatQuest(quest)}}));
    }
}

bool CleanQuestLogAction::HasProgress(Player* bot, Quest const* quest)
{
    uint32 questId = quest->GetQuestId();

    if (bot->GetQuestStatus(questId) == QUEST_STATUS_COMPLETE)
        return true;

    QuestStatusData questStatus = bot->getQuestStatusMap()[questId];

    for (uint32 i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
    {
        if (!quest->ObjectiveText[i].empty())
            return true;

        if (quest->RequiredItemId[i])
        {
            int required = quest->RequiredItemCount[i];
            int available = questStatus.ItemCount[i];
            if (available > 0 && required > 0)
                return true;
        }

        if (quest->RequiredNpcOrGo[i])
        {
            int required = quest->RequiredNpcOrGoCount[i];
            int available = questStatus.CreatureOrGOCount[i];

            if (available > 0 && required > 0)
                return true;
        }
    }

    return false;
}
