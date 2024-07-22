/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
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

    botAI->TellMaster("Quest removed");
    return true;
}

bool CleanQuestLogAction::Execute(Event event)
{
    std::string const link = event.getParam();
    if (botAI->HasActivePlayerMaster())
        return false;

    uint8 totalQuests = 0;

    DropQuestType(totalQuests); //Count the total quests

    if (MAX_QUEST_LOG_SIZE - totalQuests > 6)
        return true;

    if (AI_VALUE(bool, "can fight equal")) // Only drop gray quests when able to fight proper lvl quests.
    {
        DropQuestType(totalQuests, MAX_QUEST_LOG_SIZE - 6);                    // Drop gray/red quests.
        DropQuestType(totalQuests, MAX_QUEST_LOG_SIZE - 6, false, true);       // Drop gray/red quests with progress.
        DropQuestType(totalQuests, MAX_QUEST_LOG_SIZE - 6, false, true, true); // Drop gray/red completed quests.
    }

    if (MAX_QUEST_LOG_SIZE - totalQuests > 4)
        return true;

    DropQuestType(totalQuests, MAX_QUEST_LOG_SIZE - 4, true); //Drop quests without progress.

    if (MAX_QUEST_LOG_SIZE - totalQuests > 2)
        return true;

    DropQuestType(totalQuests, MAX_QUEST_LOG_SIZE - 2, true, true); //Drop quests with progress.

    if (MAX_QUEST_LOG_SIZE - totalQuests > 0)
        return true;

    DropQuestType(totalQuests, MAX_QUEST_LOG_SIZE - 1, true, true, true); //Drop completed quests.

    if (MAX_QUEST_LOG_SIZE - totalQuests > 0)
        return true;

    return false;
}

void CleanQuestLogAction::DropQuestType(uint8& numQuest, uint8 wantNum, bool isGreen, bool hasProgress, bool isComplete)
{
    for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 questId = bot->GetQuestSlotQuestId(slot);
        if (!questId)
            continue;

        Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
        if (!quest)
            continue;

        if (quest->GetRequiredClasses() && (quest->GetRewSpellCast() || quest->GetRewSpell())) //Do not drop class specific quests that learn spells.
            continue;

        if (quest->GetRequiredClasses() && (quest->GetRewSpellCast() || quest->GetRewSpell())) // Do not drop class specific quests that learn spells.
            continue;

        if (wantNum == 100)
            numQuest++;

        int32 lowLevelDiff = sWorld->getIntConfig(CONFIG_QUEST_LOW_LEVEL_HIDE_DIFF);
        if (lowLevelDiff < 0 || bot->GetLevel() <= bot->GetQuestLevel(quest) + uint32(lowLevelDiff)) // Quest is not gray
        {
            if (bot->GetLevel() + 5 > bot->GetQuestLevel(quest)) // Quest is not red
                if (!isGreen)
                    continue;
        }
        else // Quest is gray
        {
            if (isGreen)
                continue;
        }

        if (HasProgress(bot, quest) && !hasProgress)
            continue;

        if (bot->GetQuestStatus(questId) == QUEST_STATUS_COMPLETE && !isComplete)
            continue;

        if (numQuest <= wantNum && bot->GetQuestStatus(questId) != QUEST_STATUS_FAILED) // Always drop failed quests
            continue;

        //Drop quest.
        bot->SetQuestSlot(slot, 0);

        //We ignore unequippable quest items in this case, its' still be equipped
        bot->TakeQuestSourceItem(questId, false);

        bot->SetQuestStatus(questId, QUEST_STATUS_NONE);
        bot->RemoveRewardedQuest(questId);

        numQuest--;

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
            int required  = quest->RequiredItemCount[i];
            int available = questStatus.ItemCount[i];
            if (available > 0 && required > 0)
                return true;
        }

        if (quest->RequiredNpcOrGo[i])
        {
            int required  = quest->RequiredNpcOrGoCount[i];
            int available = questStatus.CreatureOrGOCount[i];

            if (available > 0 && required > 0)
                return true;
        }
    }

    return false;
}
