/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "TalkToQuestGiverAction.h"

#include "ChatHelper.h"
#include "Event.h"
#include "ItemUsageValue.h"
#include "Object.h"
#include "Playerbots.h"
#include "QuestDef.h"
#include "StatsWeightCalculator.h"
#include "WorldPacket.h"
#include "BroadcastHelper.h"

bool TalkToQuestGiverAction::ProcessQuest(Quest const* quest, Object* questGiver)
{
    bool isCompleted = false;
    std::ostringstream out;
    out << "Quest ";

    QuestStatus status = bot->GetQuestStatus(quest->GetQuestId());
    Player* master = GetMaster();

    if (sPlayerbotAIConfig->syncQuestForPlayer && master)
    {
        PlayerbotAI* masterBotAI = GET_PLAYERBOT_AI(master);
        if (!masterBotAI || masterBotAI->IsRealPlayer())
        {
            QuestStatus masterStatus = master->GetQuestStatus(quest->GetQuestId());
            if (masterStatus == QUEST_STATUS_INCOMPLETE || masterStatus == QUEST_STATUS_FAILED)
                isCompleted |= CompleteQuest(master, quest->GetQuestId());
        }
    }

    if (sPlayerbotAIConfig->syncQuestWithPlayer)
    {
        if (master && master->GetQuestStatus(quest->GetQuestId()) == QUEST_STATUS_COMPLETE &&
            (status == QUEST_STATUS_INCOMPLETE || status == QUEST_STATUS_FAILED))
        {
            isCompleted |= CompleteQuest(bot, quest->GetQuestId());
            status = bot->GetQuestStatus(quest->GetQuestId());
        }
    }

    switch (status)
    {
    case QUEST_STATUS_COMPLETE:
        isCompleted |= TurnInQuest(quest, questGiver, out);
        break;
    case QUEST_STATUS_INCOMPLETE:
        out << "|cffff0000Incompleted|r";
        break;
    case QUEST_STATUS_NONE:
        out << "|cff00ff00Available|r";
        break;
    case QUEST_STATUS_FAILED:
        out << "|cffff0000Failed|r";
        break;
    default:
        break;
    }

    out << ": " << chat->FormatQuest(quest);
    botAI->TellMaster(out);

    return isCompleted;
}

bool TalkToQuestGiverAction::TurnInQuest(Quest const* quest, Object* questGiver, std::ostringstream& out)
{
    uint32 questID = quest->GetQuestId();

    if (bot->GetQuestRewardStatus(questID))
        return false;

    bot->PlayDistanceSound(621);

    if (quest->GetRewChoiceItemsCount() == 0)
        RewardNoItem(quest, questGiver, out);
    else if (quest->GetRewChoiceItemsCount() == 1)
        RewardSingleItem(quest, questGiver, out);
    else
    {
        RewardMultipleItem(quest, questGiver, out);
    }

    if (botAI->HasStrategy("debug quest", BotState::BOT_STATE_NON_COMBAT) || botAI->HasStrategy("debug rpg", BotState::BOT_STATE_COMBAT))
    {
        const Quest* pQuest = sObjectMgr->GetQuestTemplate(questID);
        const std::string text_quest = ChatHelper::FormatQuest(pQuest);
        LOG_INFO("playerbots", "{} => Quest [ {} ] completed", bot->GetName(), pQuest->GetTitle());
        bot->Say("Quest [ " + text_quest + " ] completed", LANG_UNIVERSAL);
    }

    return true;
}

void TalkToQuestGiverAction::RewardNoItem(Quest const* quest, Object* questGiver, std::ostringstream& out)
{
    std::map<std::string, std::string> args;
    args["%quest"] = chat->FormatQuest(quest);
    
    if (bot->CanRewardQuest(quest, false))
    {
        out << BOT_TEXT2("quest_status_completed", args);
        BroadcastHelper::BroadcastQuestTurnedIn(botAI, bot, quest);

        bot->RewardQuest(quest, 0, questGiver, false);
    }
    else
    {
        out << BOT_TEXT2("quest_status_unable_to_complete", args);
    }
}

void TalkToQuestGiverAction::RewardSingleItem(Quest const* quest, Object* questGiver, std::ostringstream& out)
{
    int index = 0;
    ItemTemplate const* item = sObjectMgr->GetItemTemplate(quest->RewardChoiceItemId[index]);
    std::map<std::string, std::string> args;
    args["%quest"] = chat->FormatQuest(quest);
    args["%item"] = chat->FormatItem(item);

    if (bot->CanRewardQuest(quest, index, false))
    {
        out << BOT_TEXT2("quest_status_complete_single_reward", args);
        BroadcastHelper::BroadcastQuestTurnedIn(botAI, bot, quest);
        bot->RewardQuest(quest, index, questGiver, true);
    }
    else
    {
        out << BOT_TEXT2("quest_status_unable_to_complete", args);
    }
}

ItemIds TalkToQuestGiverAction::BestRewards(Quest const* quest)
{
    ItemIds returnIds;
    ItemUsage bestUsage = ITEM_USAGE_NONE;
    if (quest->GetRewChoiceItemsCount() == 0)
        return returnIds;
    else if (quest->GetRewChoiceItemsCount() == 1)
        return {0};
    else
    {
        for (uint8 i = 0; i < quest->GetRewChoiceItemsCount(); ++i)
        {
            ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", quest->RewardChoiceItemId[i]);
            if (usage == ITEM_USAGE_EQUIP || usage == ITEM_USAGE_REPLACE)
                bestUsage = ITEM_USAGE_EQUIP;
            else if (usage == ITEM_USAGE_BAD_EQUIP && bestUsage != ITEM_USAGE_EQUIP)
                bestUsage = usage;
            else if (usage != ITEM_USAGE_NONE && bestUsage == ITEM_USAGE_NONE)
                bestUsage = usage;
        }
        for (uint8 i = 0; i < quest->GetRewChoiceItemsCount(); ++i)
        {
            ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", quest->RewardChoiceItemId[i]);
            if (usage == bestUsage || usage == ITEM_USAGE_REPLACE)
                returnIds.insert(i);
        }
        return returnIds;
    }
}

void TalkToQuestGiverAction::RewardMultipleItem(Quest const* quest, Object* questGiver, std::ostringstream& out)
{
    std::set<uint32> bestIds;

    std::ostringstream outid;
    if (!botAI->IsAlt() || sPlayerbotAIConfig->autoPickReward == "yes")
    {
        bestIds = BestRewards(quest);
        if (!bestIds.empty())
        {
            StatsWeightCalculator calc(bot);
            uint32 best = 0;
            float bestScore = 0;
            for (uint32 id : bestIds)
            {
                float score = calc.CalculateItem(quest->RewardChoiceItemId[id]);
                if (score > bestScore)
                {
                    bestScore = score;
                    best = id;
                }
            }
            ItemTemplate const* item = sObjectMgr->GetItemTemplate(quest->RewardChoiceItemId[best]);
            bot->RewardQuest(quest, best, questGiver, true);
            out << "Rewarded " << ChatHelper::FormatItem(item);
        }
        else
        {
            out << "Unable to find suitable reward. Asking for help....";
            AskToSelectReward(quest, out, true);
        }
    }
    else if (sPlayerbotAIConfig->autoPickReward == "no")
    {
        // Old functionality, list rewards.
        AskToSelectReward(quest, out, false);
    }
    else
    {
        // Try to pick the usable item. If multiple list usable rewards.
        bestIds = BestRewards(quest);
        if (!bestIds.empty())
        {
            AskToSelectReward(quest, out, true);
        }
        else
        {
            // Pick the first item
            ItemTemplate const* item = sObjectMgr->GetItemTemplate(quest->RewardChoiceItemId[*bestIds.begin()]);
            bot->RewardQuest(quest, *bestIds.begin(), questGiver, true);

            out << "Rewarded " << ChatHelper::FormatItem(item);
        }
    }
}

void TalkToQuestGiverAction::AskToSelectReward(Quest const* quest, std::ostringstream& out, bool forEquip)
{
    std::ostringstream msg;
    msg << "Choose reward: ";

    for (uint8 i = 0; i < quest->GetRewChoiceItemsCount(); ++i)
    {
        ItemTemplate const* item = sObjectMgr->GetItemTemplate(quest->RewardChoiceItemId[i]);
        ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", quest->RewardChoiceItemId[i]);

        if (!forEquip || BestRewards(quest).count(i) > 0)
        {
            msg << chat->FormatItem(item);
        }
    }

    botAI->TellMaster(msg);
    out << "Reward pending";
}

bool TurnInQueryQuestAction::Execute(Event event)
{
    WorldPacket pakcet = event.getPacket();
    ObjectGuid guid;
    uint32 questId;
    ObjectGuid unk1;
    pakcet >> guid >> questId;
    Object* object =
        ObjectAccessor::GetObjectByTypeMask(*bot, guid, TYPEMASK_UNIT | TYPEMASK_GAMEOBJECT | TYPEMASK_ITEM);
    if (!object || (!object->hasQuest(questId) && !object->hasInvolvedQuest(questId)))
    {
        return false;
    }
    Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
    QuestStatus status = bot->GetQuestStatus(quest->GetQuestId());
    Player* master = GetMaster();

    if (sPlayerbotAIConfig->syncQuestForPlayer && master)
    {
        PlayerbotAI* masterBotAI = GET_PLAYERBOT_AI(master);
        if (!masterBotAI || masterBotAI->IsRealPlayer())
        {
            QuestStatus masterStatus = master->GetQuestStatus(quest->GetQuestId());
            if (masterStatus == QUEST_STATUS_INCOMPLETE || masterStatus == QUEST_STATUS_FAILED)
                CompleteQuest(master, quest->GetQuestId());
        }
    }

    if (sPlayerbotAIConfig->syncQuestWithPlayer)
    {
        if (status == QUEST_STATUS_INCOMPLETE || status == QUEST_STATUS_FAILED)
        {
            CompleteQuest(bot, quest->GetQuestId());
            status = bot->GetQuestStatus(quest->GetQuestId());
        }
    }
    std::ostringstream out;
    out << "Quest ";
    switch (status)
    {
    case QUEST_STATUS_COMPLETE:
        TurnInQuest(quest, object, out);
        break;
    case QUEST_STATUS_INCOMPLETE:
        out << "|cffff0000Incompleted|r";
        break;
    case QUEST_STATUS_NONE:
        out << "|cff00ff00Available|r";
        break;
    case QUEST_STATUS_FAILED:
        out << "|cffff0000Failed|r";
        break;
    case QUEST_STATUS_REWARDED:
        out << "|cffff0000Rewarded|r";
        break;
    default:
        break;
    }

    out << ": " << chat->FormatQuest(quest);
    botAI->TellMaster(out);
    return true;
}
