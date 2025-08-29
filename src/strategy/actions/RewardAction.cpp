/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RewardAction.h"

#include "ChatHelper.h"
#include "Event.h"
#include "Playerbots.h"

bool RewardAction::Execute(Event event)
{
    std::string const link = event.getParam();

    ItemIds itemIds = chat->parseItems(link);
    if (itemIds.empty())
        return false;

    uint32 itemId = *itemIds.begin();

    GuidVector npcs = AI_VALUE(GuidVector, "nearest npcs");
    for (ObjectGuid const guid : npcs)
    {
        if (Unit* npc = botAI->GetUnit(guid))
            if (Reward(itemId, npc))
                return true;
    }

    GuidVector gos = AI_VALUE(GuidVector, "nearest game objects");
    for (ObjectGuid const guid : gos)
    {
        if (GameObject* go = botAI->GetGameObject(guid))
            if (Reward(itemId, go))
                return true;
    }

    Unit* mtar = AI_VALUE(Unit*, "master target");
    if (mtar && Reward(itemId, mtar))
        return true;

    botAI->TellError("Cannot talk to quest giver");
    return false;
}

bool RewardAction::Reward(uint32 itemId, Object* questGiver)
{
    QuestMenu& questMenu = bot->PlayerTalkClass->GetQuestMenu();
    for (uint32 iI = 0; iI < questMenu.GetMenuItemCount(); ++iI)
    {
        QuestMenuItem const& qItem = questMenu.GetItem(iI);

        uint32 questID = qItem.QuestId;
        Quest const* pQuest = sObjectMgr->GetQuestTemplate(questID);
        QuestStatus status = bot->GetQuestStatus(questID);

        // if quest is complete, turn it in
        if (status == QUEST_STATUS_COMPLETE && !bot->GetQuestRewardStatus(questID) &&
            pQuest->GetRewChoiceItemsCount() > 1 && bot->CanRewardQuest(pQuest, false))
        {
            for (uint8 rewardIdx = 0; rewardIdx < pQuest->GetRewChoiceItemsCount(); ++rewardIdx)
            {
                ItemTemplate const* pRewardItem = sObjectMgr->GetItemTemplate(pQuest->RewardChoiceItemId[rewardIdx]);
                if (itemId == pRewardItem->ItemId)
                {
                    bot->RewardQuest(pQuest, rewardIdx, questGiver, false);

                    std::string const questTitle = pQuest->GetTitle();
                    std::string const itemName = pRewardItem->Name1;

                    std::ostringstream out;
                    out << chat->FormatItem(pRewardItem) << " rewarded";
                    botAI->TellMaster(out);

                    return true;
                }
            }
        }
    }

    return false;
}
