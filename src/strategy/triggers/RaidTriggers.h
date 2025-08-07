/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RAIDTRIGGERS_H
#define _PLAYERBOT_RAIDTRIGGERS_H

#include "Trigger.h"

#include "PlayerbotAI.h"
#include "Playerbots.h"

class IsBotMainTankTrigger : public Trigger
{
public:
    IsBotMainTankTrigger(PlayerbotAI* ai) : Trigger(ai, "is bot main tank trigger") {}
    bool IsActive() override;
};

class RaidTrigger : public Trigger
{
public:
    RaidTrigger(PlayerbotAI* ai, std::string const name = "raid trigger") : Trigger(ai, name) {}


};


#endif
