/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "QueryQuestAction.h"

#include "ChatHelper.h"
#include "Event.h"
#include "Playerbots.h"

void QueryQuestAction::TellObjective(std::string const name, uint32 available, uint32 required)
{
    botAI->TellMaster(chat->FormatQuestObjective(name, available, required));
}

bool QueryQuestAction::Execute(Event event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    Player* bot = botAI->GetBot();
    WorldPosition botPos(bot);
    WorldPosition* ptr_botpos = &botPos;
    std::string text = event.getParam();
    bool travel = false;

    if (text.find("travel") != std::string::npos)
    {
        travel = true;
        chat->eraseAllSubStr(text, " travel");
    }

    PlayerbotChatHandler ch(bot);
    uint32 questId = ch.extractQuestId(text);
    if (!questId)
    {
        for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
        {
            uint32 logQuest = bot->GetQuestSlotQuestId(slot);

            Quest const* quest = sObjectMgr->GetQuestTemplate(logQuest);
            if (!quest)
                continue;

            if (text.find(quest->GetTitle()) != std::string::npos)
            {
                questId = quest->GetQuestId();
                break;
            }
        }
    }

    for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        if (questId != bot->GetQuestSlotQuestId(slot))
            continue;

        std::ostringstream out;
        out << "--- " << chat->FormatQuest(sObjectMgr->GetQuestTemplate(questId)) << " ";

        if (bot->GetQuestStatus(questId) == QUEST_STATUS_COMPLETE)
        {
            out << "|c0000FF00completed|r ---";
            botAI->TellMaster(out);
        }
        else
        {
            out << "|c00FF0000not completed|r ---";
            botAI->TellMaster(out);
            TellObjectives(questId);
        }

        if (travel)
        {
            uint32 limit = 0;
            std::vector<TravelDestination*> allDestinations =
                sTravelMgr->getQuestTravelDestinations(bot, questId, true, true, -1);

            std::sort(allDestinations.begin(), allDestinations.end(), [ptr_botpos](TravelDestination* i, TravelDestination* j) {return i->distanceTo(ptr_botpos) < j->distanceTo(ptr_botpos); });
            for (auto dest : allDestinations)
            {
                if (limit > 50)
                    continue;

                std::ostringstream out;

                uint32 tpoints = dest->getPoints(true).size();
                uint32 apoints = dest->getPoints().size();

                out << round(dest->distanceTo(&botPos));
                out << " to " << dest->getTitle();
                out << " " << apoints;

                if (apoints < tpoints)
                    out << "/" << tpoints;

                out << " points.";

                if (!dest->isActive(bot))
                    out << " not active";

                botAI->TellMaster(out);

                limit++;
            }
        }

        return true;
    }

    return false;
}

void QueryQuestAction::TellObjectives(uint32 questId)
{
    Quest const* questTemplate = sObjectMgr->GetQuestTemplate(questId);

    // Checks if the questTemplate is valid
    if (!questTemplate)
    {
        botAI->TellMaster("Quest template not found.");
        return;
    }

    QuestStatusData questStatus = bot->getQuestStatusMap()[questId];

    for (uint32 i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
    {
        // Checks for objective text
        if (!questTemplate->ObjectiveText[i].empty())
            botAI->TellMaster(questTemplate->ObjectiveText[i]);

        // Checks for required items
        if (questTemplate->RequiredItemId[i])
        {
            uint32 required = questTemplate->RequiredItemCount[i];
            uint32 available = questStatus.ItemCount[i];
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(questTemplate->RequiredItemId[i]);
            if (proto)
                TellObjective(chat->FormatItem(proto), available, required);
        }

        // Checks for required NPCs or GOs
        if (questTemplate->RequiredNpcOrGo[i])
        {
            uint32 required = questTemplate->RequiredNpcOrGoCount[i];
            uint32 available = questStatus.CreatureOrGOCount[i];

            if (questTemplate->RequiredNpcOrGo[i] < 0)
            {
                if (GameObjectTemplate const* info = sObjectMgr->GetGameObjectTemplate(-questTemplate->RequiredNpcOrGo[i]))
                    TellObjective(info->name, available, required);
            }
            else
            {
                if (CreatureTemplate const* info = sObjectMgr->GetCreatureTemplate(questTemplate->RequiredNpcOrGo[i]))
                    TellObjective(info->Name, available, required);
            }
        }
    }
}
