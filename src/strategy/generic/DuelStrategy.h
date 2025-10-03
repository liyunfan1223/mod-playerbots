/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_DUELSTRATEGY_H
#define _PLAYERBOT_DUELSTRATEGY_H

#include "PassTroughStrategy.h"

class PlayerbotAI;

class DuelStrategy : public PassTroughStrategy
{
public:
    DuelStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "duel"; }
};

class StartDuelStrategy : public Strategy
{
public:
    StartDuelStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "start duel"; }
};

#endif
