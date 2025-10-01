/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_GUILDACCEPTACTION_H
#define _PLAYERBOT_GUILDACCEPTACTION_H

#include "Action.h"

class PlayerbotAI;

class GuildAcceptAction : public Action
{
public:
    GuildAcceptAction(PlayerbotAI* botAI) : Action(botAI, "guild accept") {}

    bool Execute(Event event) override;
};

#endif
