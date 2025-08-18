/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_GENERICWARLOCKSTRATEGY_H
#define _PLAYERBOT_GENERICWARLOCKSTRATEGY_H

#include "CombatStrategy.h"

class PlayerbotAI;

class GenericWarlockStrategy : public CombatStrategy
{
public:
    GenericWarlockStrategy(PlayerbotAI* botAI);

    std::string const getName() override { return "warlock"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    NextAction** getDefaultActions() override;
    uint32 GetType() const override { return CombatStrategy::GetType() | STRATEGY_TYPE_RANGED | STRATEGY_TYPE_DPS; }
};

class AoEWarlockStrategy : public CombatStrategy
{
public:
    AoEWarlockStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

    std::string const getName() override { return "aoe"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class WarlockBoostStrategy : public Strategy
{
public:
    WarlockBoostStrategy(PlayerbotAI* botAI) : Strategy(botAI){};

    std::string const getName() override { return "boost"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class WarlockPetStrategy : public Strategy
{
public:
    WarlockPetStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "pet"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class WarlockCcStrategy : public Strategy
{
public:
    WarlockCcStrategy(PlayerbotAI* botAI) : Strategy(botAI){};

    std::string const getName() override { return "cc"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class WarlockCurseOfAgonyStrategy : public Strategy
{
public:
    WarlockCurseOfAgonyStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "curse of agony"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class WarlockCurseOfTheElementsStrategy : public Strategy
{
public:
    WarlockCurseOfTheElementsStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "curse of elements"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class WarlockCurseOfDoomStrategy : public Strategy
{
public:
    WarlockCurseOfDoomStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "curse of doom"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class WarlockCurseOfExhaustionStrategy : public Strategy
{
public:
    WarlockCurseOfExhaustionStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "curse of exhaustion"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class WarlockCurseOfTonguesStrategy : public Strategy
{
public:
    WarlockCurseOfTonguesStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "curse of tongues"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class WarlockCurseOfWeaknessStrategy : public Strategy
{
public:
    WarlockCurseOfWeaknessStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "curse of weakness"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
