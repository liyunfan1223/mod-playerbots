/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_USEPOTIONSSTRATEGY_H
#define _PLAYERBOT_USEPOTIONSSTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

class UsePotionsStrategy : public Strategy
{
public:
    UsePotionsStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "potions"; }
};

#endif
