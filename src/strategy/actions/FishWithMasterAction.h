/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_FISHWITHMASTERACTION_H
#define _PLAYERBOT_FISHWITHMASTERACTION_H

#include "FishingAction.h"

class PlayerbotAI;

class FishWithMasterAction : public Action
{
public:
    FishWithMasterAction(PlayerbotAI* botAI) : Action(botAI, "fish with master") {}

    bool Execute(Event event) override;
};

#endif