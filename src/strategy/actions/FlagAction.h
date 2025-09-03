/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_FLAGACTION_H
#define _PLAYERBOT_FLAGACTION_H

#include "Action.h"

class PlayerbotAI;

class FlagAction : public Action
{
public:
    FlagAction(PlayerbotAI* botAI) : Action(botAI, "flag") {}

    bool Execute(Event event) override;

private:
    bool TellUsage();
};

#endif
