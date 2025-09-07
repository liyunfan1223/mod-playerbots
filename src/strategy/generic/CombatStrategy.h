/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_COMBATSTRATEGY_H
#define _PLAYERBOT_COMBATSTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

class CombatStrategy : public Strategy
{
public:
    CombatStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    uint32 GetType() const override { return STRATEGY_TYPE_COMBAT; }
};

class AvoidAoeStrategy : public Strategy
{
public:
    explicit AvoidAoeStrategy(PlayerbotAI* ai);
    const std::string getName() override { return "avoid aoe"; }
    NextAction** getDefaultActions() override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class TankFaceStrategy : public Strategy
{
public:
    explicit TankFaceStrategy(PlayerbotAI* ai);
    const std::string getName() override { return "tank face"; }
    NextAction** getDefaultActions() override;
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class CombatFormationStrategy : public Strategy
{
public:
    CombatFormationStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    const std::string getName() override { return "formation"; }
    NextAction** getDefaultActions() override;
};

#endif
