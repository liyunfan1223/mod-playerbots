/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_SKIPSPELLSLISTACTION_H
#define _PLAYERBOT_SKIPSPELLSLISTACTION_H

#include "Action.h"
#include "ChatHelper.h"

class PlayerbotAI;

class SkipSpellsListAction : public Action
{
public:
    SkipSpellsListAction(PlayerbotAI* botAI) : Action(botAI, "ss") {}

    bool Execute(Event event) override;

private:
    SpellIds parseIds(std::string const text);
};

#endif
