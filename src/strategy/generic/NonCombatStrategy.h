/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_NONCOMBATSTRATEGY_H
#define _PLAYERBOT_NONCOMBATSTRATEGY_H

#include "PassTroughStrategy.h"

class PlayerbotAI;

class NonCombatStrategy : public Strategy
{
public:
    NonCombatStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    uint32 GetType() const override { return STRATEGY_TYPE_NONCOMBAT; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class CollisionStrategy : public Strategy
{
public:
    CollisionStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    uint32 GetType() const override { return STRATEGY_TYPE_NONCOMBAT; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "collision"; }
};

class MountStrategy : public Strategy
{
public:
    MountStrategy(PlayerbotAI* botAI) : Strategy(botAI){};

    uint32 GetType() const override { return STRATEGY_TYPE_NONCOMBAT; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "mount"; }
};

class AttackTaggedStrategy : public Strategy
{
public:
    AttackTaggedStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    uint32 GetType() const override { return STRATEGY_TYPE_NONCOMBAT; }
    std::string const getName() override { return "attack tagged"; }
};

class WorldBuffStrategy : public Strategy
{
public:
    WorldBuffStrategy(PlayerbotAI* ai) : Strategy(ai) {}

    uint32 GetType() const override { return STRATEGY_TYPE_NONCOMBAT; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "worldbuff"; }
};

#endif
