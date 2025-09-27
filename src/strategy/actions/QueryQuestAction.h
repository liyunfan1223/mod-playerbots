/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_QUERYQUESTACTION_H
#define _PLAYERBOT_QUERYQUESTACTION_H

#include "Action.h"

class PlayerbotAI;

class QueryQuestAction : public Action
{
public:
    QueryQuestAction(PlayerbotAI* botAI) : Action(botAI, "query quest") {}

    bool Execute(Event event) override;

private:
    void TellObjectives(uint32 questId);
    void TellObjective(std::string const name, uint32 available, uint32 required);
};

#endif
