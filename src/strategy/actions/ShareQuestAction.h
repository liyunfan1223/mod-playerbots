/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_SHAREQUESTACTION_H
#define _PLAYERBOT_SHAREQUESTACTION_H

#include "Action.h"

class PlayerbotAI;

class ShareQuestAction : public Action
{
public:
    ShareQuestAction(PlayerbotAI* botAI, std::string name = "share quest") : Action(botAI, name) { }
    bool Execute(Event event) override;
};

class AutoShareQuestAction : public ShareQuestAction
{
public:
    AutoShareQuestAction(PlayerbotAI* botAI) : ShareQuestAction(botAI, "auto share quest") {}
    bool Execute(Event event) override;

    bool isUseful() override;
};

#endif
