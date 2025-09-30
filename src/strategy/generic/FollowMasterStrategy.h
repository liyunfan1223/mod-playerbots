/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_FOLLOWMASTERSTRATEGY_H
#define _PLAYERBOT_FOLLOWMASTERSTRATEGY_H

#include "NonCombatStrategy.h"

class PlayerbotAI;

class FollowMasterStrategy : public NonCombatStrategy
{
public:
    FollowMasterStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}

    std::string const getName() override { return "follow"; }
    NextAction** getDefaultActions() override;
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
