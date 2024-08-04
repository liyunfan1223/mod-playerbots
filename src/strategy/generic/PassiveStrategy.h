/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PASSIVESTRATEGY_H
#define _PLAYERBOT_PASSIVESTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

class PassiveStrategy : public Strategy
{
public:
    PassiveStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
    std::string const getName() override { return "passive"; }
};

#endif
