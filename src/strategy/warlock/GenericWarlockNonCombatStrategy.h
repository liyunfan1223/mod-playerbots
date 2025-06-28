/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
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

class SoulstoneSelfStrategy : public NonCombatStrategy
{
public:
    SoulstoneSelfStrategy(PlayerbotAI* ai);
    virtual std::string const getName() override { return "ss self"; }

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class SoulstoneMasterStrategy : public NonCombatStrategy
{
public:
    SoulstoneMasterStrategy(PlayerbotAI* ai);
    virtual std::string const getName() override { return "ss master"; }

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class SoulstoneTankStrategy : public NonCombatStrategy
{
public:
    SoulstoneTankStrategy(PlayerbotAI* ai);
    virtual std::string const getName() override { return "ss tank"; }

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class SoulstoneHealerStrategy : public NonCombatStrategy
{
public:
    SoulstoneHealerStrategy(PlayerbotAI* ai);
    virtual std::string const getName() override { return "ss healer"; }

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
