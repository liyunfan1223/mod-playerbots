/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_REPAIRALLACTION_H
#define _PLAYERBOT_REPAIRALLACTION_H

#include "Action.h"

class PlayerbotAI;

class RepairAllAction : public Action
{
public:
    RepairAllAction(PlayerbotAI* botAI) : Action(botAI, "repair") {}

    bool Execute(Event event) override;
};

#endif
