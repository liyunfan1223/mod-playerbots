/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_GRINDINGSTRATEGY_H
#define _PLAYERBOT_GRINDINGSTRATEGY_H

#include "NonCombatStrategy.h"

class PlayerbotAI;

class GrindingStrategy : public NonCombatStrategy
{
    public:
        GrindingStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) { }

        std::string const getName() override { return "grind"; }
        uint32 GetType() const override { return STRATEGY_TYPE_DPS; }
        NextAction** getDefaultActions() override;
        void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class MoveRandomStrategy : public NonCombatStrategy
{
public:
    MoveRandomStrategy(PlayerbotAI* ai) : NonCombatStrategy(botAI) {}
    std::string const getName() override { return "move random"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};
#endif
