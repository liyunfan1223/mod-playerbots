/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "DropQuestAction.h"

#include "ChatHelper.h"
#include "Event.h"
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
        const Quest* pQuest = sObjectMgr->GetQuestTemplate(entry);
        const std::string text_quest = ChatHelper::FormatQuest(pQuest);
        LOG_INFO("playerbots", "{} => Quest [ {} ] removed", bot->GetName(), pQuest->GetTitle());
        bot->Say("Quest [ " + text_quest + " ] removed", LANG_UNIVERSAL);
    }

    botAI->TellMaster("Quest removed");
    return true;
}


bool CleanQuestLogAction::Execute(Event event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    if (!requester)
    {
        botAI->TellMaster("No event owner detected");
        return false;
    }

    if (!sPlayerbotAIConfig->dropObsoleteQuests)
    {
        return false;
    }

    // Only output this message if "debug rpg" strategy is enabled
    if (botAI->HasStrategy("debug rpg", BotState::BOT_STATE_COMBAT))
    {
        botAI->TellMaster("Clean Quest Log command received, removing grey/trivial quests...");
    }

    uint8 botLevel = bot->GetLevel();  // Get bot's level
    uint8 numQuest = 0;
    for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        if (bot->GetQuestSlotQuestId(slot))
        {
            numQuest++;
        }
    }

    for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 questId = bot->GetQuestSlotQuestId(slot);
        if (!questId)
            continue;

        const Quest* quest = sObjectMgr->GetQuestTemplate(questId);
        if (!quest)
            continue;

        // Determine if quest is trivial by comparing levels
        int32 questLevel = quest->GetQuestLevel();
        if (questLevel == -1) // For scaling quests, default to bot level
        {
            questLevel = botLevel;
        }

        // Set the level difference for when a quest becomes trivial
        // This was determined by using the Lua code the client uses
        int32 trivialLevel = 5;
        if (botLevel >= 40)
        {
            trivialLevel = 8;
        }
        else if (botLevel >= 30)
        {
            trivialLevel = 7;
        }
        else if (botLevel >= 20)
        {
            trivialLevel = 6;
        }

        // Check if the quest is trivial (grey) for the bot
        if ((botLevel - questLevel) > trivialLevel)
        {
            // Output only if "debug rpg" strategy is enabled
            if (botAI->HasStrategy("debug rpg", BotState::BOT_STATE_COMBAT))
            {
                botAI->TellMaster("Quest [ " + quest->GetTitle() + " ] will be removed because it is trivial (grey).");
            }

            // Remove quest
            botAI->rpgStatistic.questDropped++;
            bot->SetQuestSlot(slot, 0);
            bot->TakeQuestSourceItem(questId, false);
            bot->SetQuestStatus(questId, QUEST_STATUS_NONE);
            bot->RemoveRewardedQuest(questId);

            numQuest--;

            if (botAI->HasStrategy("debug rpg", BotState::BOT_STATE_COMBAT))
            {
                const std::string text_quest = ChatHelper::FormatQuest(quest);
                LOG_INFO("playerbots", "{} => Quest [ {} ] removed", bot->GetName(), quest->GetTitle());
                bot->Say("Quest [ " + text_quest + " ] removed", LANG_UNIVERSAL);
            }

            if (botAI->HasStrategy("debug rpg", BotState::BOT_STATE_COMBAT))
            {
                botAI->TellMaster("Quest [ " + quest->GetTitle() + " ] has been removed.");
            }
        }
        else
        {
            // Only output if "debug rpg" strategy is enabled
            if (botAI->HasStrategy("debug rpg", BotState::BOT_STATE_COMBAT))
            {
                botAI->TellMaster("Quest [ " + quest->GetTitle() + " ] is not trivial and will be kept.");
            }
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
                if (!isGreen)
                    continue;
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
            bot->Say("Quest [ " + text_quest + " ] removed", LANG_UNIVERSAL);
        }
        botAI->TellMaster("Quest removed" + chat->FormatQuest(quest));
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
