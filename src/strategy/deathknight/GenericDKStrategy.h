/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_GENERICDKSTRATEGY_H
#define _PLAYERBOT_GENERICDKSTRATEGY_H

#include "MeleeCombatStrategy.h"

class PlayerbotAI;

class GenericDKStrategy : public MeleeCombatStrategy
{
public:
    GenericDKStrategy(PlayerbotAI* botAI);

    std::string const getName() override { return "DK"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
