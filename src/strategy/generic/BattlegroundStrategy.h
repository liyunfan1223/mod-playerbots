/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_BATTLEGROUNDSTRATEGY_H
#define _PLAYERBOT_BATTLEGROUNDSTRATEGY_H

#include "PassTroughStrategy.h"

class BGStrategy : public PassTroughStrategy
{
public:
    BGStrategy(PlayerbotAI* botAI);

    uint32 GetType() const override { return STRATEGY_TYPE_NONCOMBAT; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "bg"; }
};

class BattlegroundStrategy : public Strategy
{
public:
    BattlegroundStrategy(PlayerbotAI* botAI) : Strategy(botAI){};

    uint32 GetType() const override { return STRATEGY_TYPE_NONCOMBAT; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "Battleground"; }
};

class WarsongStrategy : public Strategy
{
public:
    WarsongStrategy(PlayerbotAI* botAI) : Strategy(botAI){};

    uint32 GetType() const override { return STRATEGY_TYPE_GENERIC; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "warsong"; }
};

class AlteracStrategy : public Strategy
{
public:
    AlteracStrategy(PlayerbotAI* botAI) : Strategy(botAI){};

    uint32 GetType() const override { return STRATEGY_TYPE_GENERIC; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "alterac"; }
};

class ArathiStrategy : public Strategy
{
public:
    ArathiStrategy(PlayerbotAI* botAI) : Strategy(botAI){};

    uint32 GetType() const override { return STRATEGY_TYPE_GENERIC; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "arathi"; }
};

class EyeStrategy : public Strategy
{
public:
    EyeStrategy(PlayerbotAI* botAI) : Strategy(botAI){};

    uint32 GetType() const override { return STRATEGY_TYPE_GENERIC; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "eye"; }
};

class IsleStrategy : public Strategy
{
public:
    IsleStrategy(PlayerbotAI* botAI) : Strategy(botAI){};

    uint32 GetType() const override { return STRATEGY_TYPE_GENERIC; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "isle"; }
};

class ArenaStrategy : public Strategy
{
public:
    ArenaStrategy(PlayerbotAI* botAI) : Strategy(botAI){};

    uint32 GetType() const override { return STRATEGY_TYPE_GENERIC; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "arena"; }
};

#endif
