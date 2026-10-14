/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "HolyPriestStrategy.h"
#include "Playerbots.h"

class HolyDpsPriestStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    HolyDpsPriestStrategyActionNodeFactory() { creators["smite"] = &smite; }

private:
    static ActionNode* smite([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "smite",
            /*P*/ {},
            /*A*/ { NextAction("shoot") },
            /*C*/ {}
        );
    }
};

HolyDpsPriestStrategy::HolyDpsPriestStrategy(PlayerbotAI* botAI) : GenericPriestStrategy(botAI)
{
    actionNodeFactories.Add(new HolyDpsPriestStrategyActionNodeFactory());
}

std::vector<NextAction> HolyDpsPriestStrategy::getDefaultActions()
{
    return {
        NextAction("smite", ACTION_DEFAULT + 0.2f),
        NextAction("mana burn", ACTION_DEFAULT + 0.1f),
        NextAction("starshards", ACTION_DEFAULT)
    };
}

void HolyDpsPriestStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericPriestStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "holy fire",
            {
                NextAction("holy fire", ACTION_NORMAL + 9)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "shadowfiend",
            {
                NextAction("shadowfiend", ACTION_HIGH)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium mana",
            {
                NextAction("shadowfiend", ACTION_HIGH)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low mana",
            {
                NextAction("mana burn", ACTION_HIGH)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member low health",
            {
                NextAction("power word: shield on party", ACTION_MEDIUM_HEAL + 3),
                NextAction("flash heal on party", ACTION_MEDIUM_HEAL + 2)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member medium health",
            {
                NextAction("renew on party", ACTION_LIGHT_HEAL + 2)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member to heal out of spell range",
            {
                NextAction("reach party member to heal", ACTION_CRITICAL_HEAL + 10)
            }
        )
    );
}

HolyHealPriestStrategy::HolyHealPriestStrategy(PlayerbotAI* botAI) : GenericPriestStrategy(botAI)
{
    // No custom ActionNodeFactory needed
}

std::vector<NextAction> HolyHealPriestStrategy::getDefaultActions()
{
    return { NextAction("shoot", ACTION_DEFAULT) };
}

void HolyHealPriestStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericPriestStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "group heal setting",
            {
                NextAction("prayer of mending on party", ACTION_MEDIUM_HEAL + 9),
                NextAction("circle of healing on party", ACTION_MEDIUM_HEAL + 8)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium group heal setting",
            {
                NextAction("divine hymn", ACTION_CRITICAL_HEAL + 7),
                NextAction("prayer of mending on party", ACTION_CRITICAL_HEAL + 6),
                NextAction("circle of healing on party", ACTION_CRITICAL_HEAL + 5),
                NextAction("prayer of healing on party", ACTION_CRITICAL_HEAL + 4)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member critical health",
            {
                NextAction("guardian spirit on party", ACTION_CRITICAL_HEAL + 6),
                NextAction("power word: shield on party", ACTION_CRITICAL_HEAL + 5),
                NextAction("prayer of mending on party", ACTION_CRITICAL_HEAL + 3),
                NextAction("greater heal on party", ACTION_MEDIUM_HEAL + 2),
                NextAction("flash heal on party", ACTION_CRITICAL_HEAL + 1),
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member low health",
            {
                NextAction("circle of healing on party", ACTION_MEDIUM_HEAL + 4),
                NextAction("prayer of mending on party", ACTION_MEDIUM_HEAL + 3),
                NextAction("greater heal on party", ACTION_MEDIUM_HEAL + 2),
                NextAction("flash heal on party", ACTION_MEDIUM_HEAL + 1)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member medium health",
            {
                NextAction("circle of healing on party", ACTION_LIGHT_HEAL + 7),
                NextAction("prayer of mending on party", ACTION_LIGHT_HEAL + 6),
                NextAction("greater heal on party", ACTION_MEDIUM_HEAL + 5),
                NextAction("flash heal on party", ACTION_LIGHT_HEAL + 4),
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member almost full health",
            {
                NextAction("renew on party", ACTION_LIGHT_HEAL + 2),
                NextAction("prayer of mending on party", ACTION_LIGHT_HEAL + 1),
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "party member to heal out of spell range",
            {
                NextAction("reach party member to heal", ACTION_CRITICAL_HEAL + 10)
            }
        )
    );
}
