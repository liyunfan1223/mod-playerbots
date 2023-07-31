/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_GENERICWARLOCKSTRATEGY_H
#define _PLAYERBOT_GENERICWARLOCKSTRATEGY_H

#include "CombatStrategy.h"
#include "RangedCombatStrategy.h"

class PlayerbotAI;

class GenericWarlockStrategy : public RangedCombatStrategy
{
    public:
        GenericWarlockStrategy(PlayerbotAI* botAI);

        std::string const getName() override { return "warlock"; }
        void InitTriggers(std::vector<TriggerNode*>& triggers) override;
        NextAction** getDefaultActions() override;
};

class WarlockBoostStrategy : public Strategy
{
    public:
        WarlockBoostStrategy(PlayerbotAI* botAI) : Strategy(botAI) { };

        std::string const getName() override { return "boost"; }
        void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class WarlockCcStrategy : public Strategy
{
    public:
        WarlockCcStrategy(PlayerbotAI* botAI) : Strategy(botAI) { };

        std::string const getName() override { return "cc"; }
        void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
