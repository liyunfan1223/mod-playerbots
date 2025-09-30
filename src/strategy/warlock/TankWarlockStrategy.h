/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_TANKWARLOCKSTRATEGY_H
#define _PLAYERBOT_TANKWARLOCKSTRATEGY_H

#include "GenericWarlockStrategy.h"

class PlayerbotAI;

class TankWarlockStrategy : public GenericWarlockStrategy
{
public:
    TankWarlockStrategy(PlayerbotAI* botAI);

    std::string const getName() override { return "tank"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    NextAction** getDefaultActions() override;
};

#endif
