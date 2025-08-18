/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
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

class SummonImpStrategy : public NonCombatStrategy
{
public:
    SummonImpStrategy(PlayerbotAI* ai);
    virtual std::string const getName() override { return "imp"; }

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class SummonVoidwalkerStrategy : public NonCombatStrategy
{
public:
    SummonVoidwalkerStrategy(PlayerbotAI* ai);
    virtual std::string const getName() override { return "voidwalker"; }

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class SummonSuccubusStrategy : public NonCombatStrategy
{
public:
    SummonSuccubusStrategy(PlayerbotAI* ai);
    virtual std::string const getName() override { return "succubus"; }

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class SummonFelhunterStrategy : public NonCombatStrategy
{
public:
    SummonFelhunterStrategy(PlayerbotAI* ai);
    virtual std::string const getName() override { return "felhunter"; }

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class SummonFelguardStrategy : public NonCombatStrategy
{
public:
    SummonFelguardStrategy(PlayerbotAI* ai);
    virtual std::string const getName() override { return "felguard"; }

public:
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

class UseSpellstoneStrategy : public NonCombatStrategy
{
public:
    UseSpellstoneStrategy(PlayerbotAI* ai);
    virtual std::string const getName() override { return "spellstone"; }

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class UseFirestoneStrategy : public NonCombatStrategy
{
public:
    UseFirestoneStrategy(PlayerbotAI* ai);
    virtual std::string const getName() override { return "firestone"; }

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
