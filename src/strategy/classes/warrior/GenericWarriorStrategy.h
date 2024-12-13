/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_GENERICWARRIORSTRATEGY_H
#define _PLAYERBOT_GENERICWARRIORSTRATEGY_H

#include "CombatStrategy.h"

class PlayerbotAI;

// Stance requirements
class WarriorStanceRequirementActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    WarriorStanceRequirementActionNodeFactory()
    {
        // battle only
        creators["charge"] = &charge;
        creators["mocking blow"] = &mocking_blow;
        creators["overpower"] = &overpower;

        // temp
        creators["mortal strike"] = &mortal_strike;

        // berserker only
        creators["berserker rage"] = &berserker_rage;
        creators["recklessness"] = &recklessness;
        creators["whirlwind"] = &whirlwind;
        creators["pummel"] = &pummel;
        creators["intercept"] = &intercept;

        // defensive only
        creators["taunt"] = &taunt;
        creators["revenge"] = &revenge;
        creators["shield block"] = &shield_block;
        creators["disarm"] = &disarm;
        creators["shield wall"] = &shield_wall;
        creators["intervene"] = &intervene;
    }

private:
    ACTION_NODE_P(charge, "charge", "battle stance");
    ACTION_NODE_P(mocking_blow, "mocking blow", "battle stance");
    ACTION_NODE_P(overpower, "overpower", "battle stance");
    ACTION_NODE_P(berserker_rage, "berserker rage", "berserker stance");
    ACTION_NODE_P(recklessness, "recklessness", "berserker stance");
    ACTION_NODE_P(whirlwind, "whirlwind", "berserker stance");
    ACTION_NODE_P(pummel, "pummel", "berserker stance");
    ACTION_NODE_P(intercept, "intercept", "berserker stance");
    ACTION_NODE_P(taunt, "taunt", "defensive stance");
    ACTION_NODE_P(revenge, "revenge", "defensive stance");
    ACTION_NODE_P(shield_block, "shield block", "defensive stance");
    ACTION_NODE_P(disarm, "disarm", "defensive stance");
    ACTION_NODE_P(shield_wall, "shield wall", "defensive stance");
    ACTION_NODE_P(intervene, "intervene", "defensive stance");
    // temp
    ACTION_NODE_P(mortal_strike, "mortal strike", "battle stance");
};

class GenericWarriorStrategy : public CombatStrategy
{
public:
    GenericWarriorStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "warrior"; }
};

class WarrirorAoeStrategy : public CombatStrategy
{
public:
    WarrirorAoeStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "aoe"; }
};

#endif
