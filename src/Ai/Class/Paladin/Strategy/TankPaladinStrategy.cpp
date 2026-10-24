/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TankPaladinStrategy.h"
#include "Playerbots.h"

class TankPaladinStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    TankPaladinStrategyActionNodeFactory()
    {
        creators["seal of corruption"] = &seal_of_corruption;
        creators["seal of vengeance"] = &seal_of_vengeance;
        creators["seal of command"] = &seal_of_command;
        creators["hand of reckoning"] = &hand_of_reckoning;
        creators["taunt spell"] = &hand_of_reckoning;
    }

private:
    static ActionNode* seal_of_command([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "seal of command",
            /*P*/ {},
            /*A*/ { NextAction("seal of corruption") },
            /*C*/ {}
        );
    }
    static ActionNode* seal_of_corruption([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "seal of corruption",
            /*P*/ {},
            /*A*/ { NextAction("seal of vengeance") },
            /*C*/ {}
        );
    }

    static ActionNode* seal_of_vengeance([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "seal of vengeance",
            /*P*/ {},
            /*A*/ { NextAction("seal of righteousness") },
            /*C*/ {}
        );
    }

    static ActionNode* hand_of_reckoning([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "hand of reckoning",
            /*P*/ {},
            /*A*/ { NextAction("righteous defense") },
            /*C*/ {}
        );
    }
};

TankPaladinStrategy::TankPaladinStrategy(PlayerbotAI* botAI) : GenericPaladinStrategy(botAI)
{
    actionNodeFactories.Add(new TankPaladinStrategyActionNodeFactory());
}

std::vector<NextAction> TankPaladinStrategy::getDefaultActions()
{
    return {
        NextAction("shield of righteousness", ACTION_DEFAULT + 0.6f),
        NextAction("hammer of the righteous", ACTION_DEFAULT + 0.5f),
        NextAction("judgement of wisdom", ACTION_DEFAULT + 0.4f),
        NextAction("melee", ACTION_DEFAULT)
    };
}

void TankPaladinStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericPaladinStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "seal",
            {
                NextAction("seal of corruption", ACTION_HIGH)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low mana",
            {
                NextAction("seal of wisdom", ACTION_HIGH + 9)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "light aoe",
            {
                NextAction("avenger's shield", ACTION_HIGH + 5)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium aoe",
            {
                NextAction("consecration", ACTION_HIGH + 7),
                NextAction("avenger's shield", ACTION_HIGH + 6)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "lose aggro",
            {
                NextAction("hand of reckoning", ACTION_HIGH + 7)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "holy shield",
            {
                NextAction("holy shield", ACTION_HIGH + 10)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "avenging wrath",
            {
                NextAction("avenging wrath", ACTION_HIGH + 2)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "target critical health",
            {
                NextAction("hammer of wrath", ACTION_CRITICAL_HEAL)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "righteous fury",
            {
                NextAction("righteous fury", ACTION_HIGH + 8)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium group heal setting",
            {
                NextAction("divine sacrifice", ACTION_HIGH + 5)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "enough mana",
            {
                NextAction("consecration", ACTION_HIGH + 4)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "not facing target",
            {
                NextAction("set facing", ACTION_NORMAL + 7)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "enemy out of melee",
            {
                NextAction("reach melee", ACTION_HIGH + 1)
            }
        )
    );
}
