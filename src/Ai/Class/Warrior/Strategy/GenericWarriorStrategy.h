/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GENERICWARRIORSTRATEGY_H
#define PLAYERBOTS_GENERICWARRIORSTRATEGY_H

#include "Action.h"
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
        creators["retaliation"] = &retaliation;
        creators["shattering throw"] = &shattering_throw;

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

    static ActionNode* charge([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "charge",
            /*P*/ { NextAction("battle stance") },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* mocking_blow([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "mocking blow",
            /*P*/ { NextAction("battle stance") },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* overpower([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "overpower",
            /*P*/ { NextAction("battle stance") },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* berserker_rage([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "berserker rage",
            /*P*/ { NextAction("berserker stance") },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* recklessness([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "recklessness",
            /*P*/ { NextAction("berserker stance") },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* whirlwind([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "whirlwind",
            /*P*/ { NextAction("berserker stance") },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* pummel([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "pummel",
            /*P*/ { NextAction("berserker stance") },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* intercept([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "intercept",
            /*P*/ { NextAction("berserker stance") },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* taunt([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "taunt",
            /*P*/ { NextAction("defensive stance") },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* revenge([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "revenge",
            /*P*/ { NextAction("defensive stance") },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* shield_block([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "shield block",
            /*P*/ { NextAction("defensive stance") },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* disarm([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "disarm",
            /*P*/ { NextAction("defensive stance") },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* shield_wall([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "shield wall",
            /*P*/ { NextAction("defensive stance") },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* intervene([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "intervene",
            /*P*/ { NextAction("defensive stance") },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* mortal_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "mortal strike",
            /*P*/ { NextAction("battle stance") },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* retaliation([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "retaliation",
            /*P*/ { NextAction("battle stance") },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* shattering_throw([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "shattering throw",
            /*P*/ { NextAction("battle stance") },
            /*A*/ {},
            /*C*/ {}
        );
    }
};

class GenericWarriorStrategy : public CombatStrategy
{
public:
    GenericWarriorStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "warrior"; }
};

class WarriorAoeStrategy : public CombatStrategy
{
public:
    WarriorAoeStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "aoe"; }
};

#endif
