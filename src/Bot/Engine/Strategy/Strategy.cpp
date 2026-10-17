/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "Strategy.h"
#include "Playerbots.h"

class ActionNodeFactoryInternal : public NamedObjectFactory<ActionNode>
{
public:
    ActionNodeFactoryInternal()
    {
        creators["melee"] = &melee;
        creators["healthstone"] = &healthstone;
        creators["be near"] = &follow_master_random;
        creators["attack anything"] = &attack_anything;
        creators["move random"] = &move_random;
        creators["move to loot"] = &move_to_loot;
        creators["food"] = &food;
        creators["drink"] = &drink;
        creators["mana potion"] = &mana_potion;
        creators["healing potion"] = &healing_potion;
    }

private:
    static ActionNode* melee([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "melee",
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* healthstone([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "healthstone",
            /*P*/ {},
            /*A*/ { NextAction("healing potion") },
            /*C*/ {}
        );
    }

    static ActionNode* follow_master_random([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "be near",
            /*P*/ {},
            /*A*/ { NextAction("follow") },
            /*C*/ {}
        );
    }

    static ActionNode* attack_anything([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "attack anything",
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* move_random([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "move random",
            /*P*/ {},
            /*A*/ { NextAction("stay line") },
            /*C*/ {}
        );
    }

    static ActionNode* move_to_loot([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "move to loot",
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* food([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "food",
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* drink([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "drink",
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* mana_potion([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "mana potion",
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* healing_potion([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "healing potion",
            /*P*/ {},
            /*A*/ { NextAction("food") },
            /*C*/ {}
        );
    }
};

Strategy::Strategy(PlayerbotAI* botAI) : PlayerbotAIAware(botAI)
{
    actionNodeFactories.Add(new ActionNodeFactoryInternal());
}

ActionNode* Strategy::GetAction(std::string const name) { return actionNodeFactories.GetContextObject(name, botAI); }
