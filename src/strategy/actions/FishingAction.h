/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_FISHINGACTION_H
#define _PLAYERBOT_FISHINGACTION_H

#include "Action.h"
#include "Event.h"
#include "Playerbots.h"

class PlayerbotAI;

class FishingAction : public Action, public Qualified
{
public:
    FishingAction(PlayerbotAI* botAI) : Action(botAI, "fishing action"){}
    bool Execute(Event event) override;
    bool isUseful() override;

private:
    WorldPosition FindWater(Player* bot, float distance = 5.0f);
};


#endif
