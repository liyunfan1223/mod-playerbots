/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_GENERICWARLOCKNONCOMBATSTRATEGY_H
#define _PLAYERBOT_GENERICWARLOCKNONCOMBATSTRATEGY_H

#include "NonCombatStrategy.h"

class PlayerbotAI;

class GenericWarlockNonCombatStrategy : public NonCombatStrategy
{
    public:
        GenericWarlockNonCombatStrategy(PlayerbotAI* botAI);

        std::string const getName() override { return "nc"; }
        void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class WarlockPetStrategy : public Strategy
{
    public:
        WarlockPetStrategy(PlayerbotAI* botAI) : Strategy(botAI) { }

        std::string const getName() override { return "pet"; }
        void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
