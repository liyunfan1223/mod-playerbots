/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_FROSTFIREMAGESTRATEGY_H
#define _PLAYERBOT_FROSTFIREMAGESTRATEGY_H

#include "GenericMageStrategy.h"

class PlayerbotAI;

class FrostFireMageStrategy : public GenericMageStrategy
{
public:
    FrostFireMageStrategy(PlayerbotAI* botAI) : GenericMageStrategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "frostfire"; }
    NextAction** getDefaultActions() override;
};

class FrostFireMageAoeStrategy : public CombatStrategy
{
public:
    FrostFireMageAoeStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "frostfire aoe"; }
};

#endif
