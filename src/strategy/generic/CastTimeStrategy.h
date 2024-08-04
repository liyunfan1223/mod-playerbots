/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_CASTTIMESTRATEGY_H
#define _PLAYERBOT_CASTTIMESTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

class CastTimeMultiplier : public Multiplier
{
public:
    CastTimeMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "cast time") {}

    float GetValue(Action* action) override;
};

class CastTimeStrategy : public Strategy
{
public:
    CastTimeStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
    std::string const getName() override { return "cast time"; }
};

#endif
