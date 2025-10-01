/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_TELLCASTFAILEDACTION_H
#define _PLAYERBOT_TELLCASTFAILEDACTION_H

#include "Action.h"

class PlayerbotAI;

class TellSpellAction : public Action
{
public:
    TellSpellAction(PlayerbotAI* botAI) : Action(botAI, "spell") {}

    bool Execute(Event event) override;
};

class TellCastFailedAction : public Action
{
public:
    TellCastFailedAction(PlayerbotAI* botAI) : Action(botAI, "tell cast failed") {}

    bool Execute(Event event) override;
};

#endif
