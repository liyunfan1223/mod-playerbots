/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_NONCOMBATSTRATEGY_H
#define PLAYERBOTS_NONCOMBATSTRATEGY_H

#include "PassThroughStrategy.h"

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
    WorldBuffStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    uint32 GetType() const override { return STRATEGY_TYPE_NONCOMBAT; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "worldbuff"; }
};

class MasterFishingStrategy : public Strategy
{
public:
    MasterFishingStrategy(PlayerbotAI* botAI) : Strategy(botAI){}
    uint32 GetType() const override { return STRATEGY_TYPE_NONCOMBAT; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override {return "master fishing";}
};
#endif
