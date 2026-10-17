/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GENERICWARLOCKNONCOMBATSTRATEGY_H
#define PLAYERBOTS_GENERICWARLOCKNONCOMBATSTRATEGY_H

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
    SummonImpStrategy(PlayerbotAI* botAI);
    virtual std::string const getName() override { return "imp"; }

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class SummonVoidwalkerStrategy : public NonCombatStrategy
{
public:
    SummonVoidwalkerStrategy(PlayerbotAI* botAI);
    virtual std::string const getName() override { return "voidwalker"; }

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class SummonSuccubusStrategy : public NonCombatStrategy
{
public:
    SummonSuccubusStrategy(PlayerbotAI* botAI);
    virtual std::string const getName() override { return "succubus"; }

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class SummonFelhunterStrategy : public NonCombatStrategy
{
public:
    SummonFelhunterStrategy(PlayerbotAI* botAI);
    virtual std::string const getName() override { return "felhunter"; }

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class SummonFelguardStrategy : public NonCombatStrategy
{
public:
    SummonFelguardStrategy(PlayerbotAI* botAI);
    virtual std::string const getName() override { return "felguard"; }

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class SoulstoneSelfStrategy : public NonCombatStrategy
{
public:
    SoulstoneSelfStrategy(PlayerbotAI* botAI);
    virtual std::string const getName() override { return "ss self"; }

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class SoulstoneMasterStrategy : public NonCombatStrategy
{
public:
    SoulstoneMasterStrategy(PlayerbotAI* botAI);
    virtual std::string const getName() override { return "ss master"; }

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class SoulstoneTankStrategy : public NonCombatStrategy
{
public:
    SoulstoneTankStrategy(PlayerbotAI* botAI);
    virtual std::string const getName() override { return "ss tank"; }

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class SoulstoneHealerStrategy : public NonCombatStrategy
{
public:
    SoulstoneHealerStrategy(PlayerbotAI* botAI);
    virtual std::string const getName() override { return "ss healer"; }

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class UseSpellstoneStrategy : public NonCombatStrategy
{
public:
    UseSpellstoneStrategy(PlayerbotAI* botAI);
    virtual std::string const getName() override { return "spellstone"; }

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class UseFirestoneStrategy : public NonCombatStrategy
{
public:
    UseFirestoneStrategy(PlayerbotAI* botAI);
    virtual std::string const getName() override { return "firestone"; }

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
