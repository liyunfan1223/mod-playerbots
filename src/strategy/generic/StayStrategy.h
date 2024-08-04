/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_STAYSTRATEGY_H
#define _PLAYERBOT_STAYSTRATEGY_H

#include "NonCombatStrategy.h"

class PlayerbotAI;

class StayStrategy : public NonCombatStrategy
{
public:
    StayStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}

    std::string const getName() override { return "stay"; }
    NextAction** getDefaultActions() override;
};

class SitStrategy : public NonCombatStrategy
{
public:
    SitStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}

    std::string const getName() override { return "sit"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
