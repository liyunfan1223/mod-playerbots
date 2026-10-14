/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TalkToQuestGiverAction.h"
#include "BroadcastHelper.h"
#include "ChatHelper.h"
#include "Event.h"
#include "ItemUsageValue.h"
#include "Object.h"
#include "Playerbots.h"
#include "QuestDef.h"
#include "StatsWeightCalculator.h"
#include "WorldPacket.h"

bool TalkToQuestGiverAction::ProcessQuest(Quest const* quest, Object* questGiver)
{
    bool isCompleted = false;
    std::ostringstream out;
    out << "Quest ";

    QuestStatus status = bot->GetQuestStatus(quest->GetQuestId());
    Player* master = GetMaster();

    if (sPlayerbotAIConfig.SyncQuestForPlayer && master)
    {
        PlayerbotAI* masterBotAI = GET_PLAYERBOT_AI(master);
        if (!masterBotAI || IsSelfBot(master))
        {
            QuestStatus masterStatus = master->GetQuestStatus(quest->GetQuestId());
            if (masterStatus == QUEST_STATUS_INCOMPLETE || masterStatus == QUEST_STATUS_FAILED)
                isCompleted |= CompleteQuest(master, quest->GetQuestId());
        }
    }

    if (sPlayerbotAIConfig.SyncQuestWithPlayer)
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
        Quest const* pQuest = sObjectMgr->GetQuestTemplate(questID);
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
        out << PlayerbotTextMgr::instance().GetBotText("quest_status_completed", args);
        BroadcastHelper::BroadcastQuestTurnedIn(botAI, bot, quest);

        bot->RewardQuest(quest, 0, questGiver, false);
    }
    else
    {
        out << PlayerbotTextMgr::instance().GetBotText("quest_status_unable_to_complete", args);
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
        out << PlayerbotTextMgr::instance().GetBotText("quest_status_complete_single_reward", args);
        BroadcastHelper::BroadcastQuestTurnedIn(botAI, bot, quest);
        bot->RewardQuest(quest, index, questGiver, true);
    }
    else
    {
        out << PlayerbotTextMgr::instance().GetBotText("quest_status_unable_to_complete", args);
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
    if (!botAI->IsAltBot() || sPlayerbotAIConfig.AutoPickReward == "yes")
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
    else if (sPlayerbotAIConfig.AutoPickReward == "no")
    {
        // Old functionality, list rewards.
        AskToSelectReward(quest, out, false);
    }
    else
    {
        // Try to pick the usable item. If multiple, list usable rewards.
        bestIds = BestRewards(quest);

        if (bestIds.size() > 1)
            AskToSelectReward(quest, out, true);

        else if (!bestIds.empty())
        {
            // Pick the first item
            uint32 firstId = *bestIds.begin();
            ItemTemplate const* item = sObjectMgr->GetItemTemplate(quest->RewardChoiceItemId[firstId]);
            bot->RewardQuest(quest, firstId, questGiver, true);

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

    if (sPlayerbotAIConfig.SyncQuestForPlayer && master)
    {
        PlayerbotAI* masterBotAI = GET_PLAYERBOT_AI(master);
        if (!masterBotAI || IsSelfBot(master))
        {
            QuestStatus masterStatus = master->GetQuestStatus(quest->GetQuestId());
            if (masterStatus == QUEST_STATUS_INCOMPLETE || masterStatus == QUEST_STATUS_FAILED)
                CompleteQuest(master, quest->GetQuestId());
        }
    }

    if (sPlayerbotAIConfig.SyncQuestWithPlayer)
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
