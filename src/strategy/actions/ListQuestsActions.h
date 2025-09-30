/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_LISTQUESTSACTIONS_H
#define _PLAYERBOT_LISTQUESTSACTIONS_H

#include "Action.h"

class PlayerbotAI;

enum QuestListFilter
{
    QUEST_LIST_FILTER_SUMMARY = 0,
    QUEST_LIST_FILTER_COMPLETED = 1,
    QUEST_LIST_FILTER_INCOMPLETED = 2,
    QUEST_LIST_FILTER_ALL = QUEST_LIST_FILTER_COMPLETED | QUEST_LIST_FILTER_INCOMPLETED
};

enum QuestTravelDetail
{
    QUEST_TRAVEL_DETAIL_NONE = 0,
    QUEST_TRAVEL_DETAIL_SUMMARY = 1,
    QUEST_TRAVEL_DETAIL_FULL = 2
};

class ListQuestsAction : public Action
{
public:
    ListQuestsAction(PlayerbotAI* botAI) : Action(botAI, "quests") {}

    bool Execute(Event event) override;

private:
    void ListQuests(QuestListFilter filter, QuestTravelDetail travelDetail = QUEST_TRAVEL_DETAIL_NONE);
    uint32 ListQuests(bool completed, bool silent, QuestTravelDetail travelDetail);
};

#endif
