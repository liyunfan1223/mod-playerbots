/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_AFFLICTIONWARLOCKSTRATEGY_H
#define _PLAYERBOT_AFFLICTIONWARLOCKSTRATEGY_H

#include "GenericWarlockStrategy.h"
#include "CombatStrategy.h"

class PlayerbotAI;

class AfflictionWarlockStrategy : public GenericWarlockStrategy
{
public:
    AfflictionWarlockStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "affli"; }
    NextAction** getDefaultActions() override;
};

#endif
