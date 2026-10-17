/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GENERICROGUESTRATEGY_H
#define PLAYERBOTS_GENERICROGUESTRATEGY_H

#include "CombatStrategy.h"

class PlayerbotAI;

class GenericRogueStrategy : public CombatStrategy
{
public:
    GenericRogueStrategy(PlayerbotAI* botAI);

    std::string const getName() override { return "rogue"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    uint32 GetType() const override
    {
        return CombatStrategy::GetType() | STRATEGY_TYPE_DPS | STRATEGY_TYPE_MELEE;
    }
};

class StealthedRogueStrategy : public Strategy
{
public:
    StealthedRogueStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "stealthed"; }
    std::vector<NextAction> getDefaultActions() override;
};

class StealthStrategy : public Strategy
{
public:
    StealthStrategy(PlayerbotAI* botAI) : Strategy(botAI){};

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "stealth"; }
};

class RogueAoeStrategy : public Strategy
{
public:
    RogueAoeStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "aoe"; }
};

class RogueBoostStrategy : public Strategy
{
public:
    RogueBoostStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "boost"; }
};

class RogueCcStrategy : public Strategy
{
public:
    RogueCcStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "cc"; }
};

#endif
