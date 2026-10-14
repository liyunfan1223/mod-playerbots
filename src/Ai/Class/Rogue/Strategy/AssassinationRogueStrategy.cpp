/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AssassinationRogueStrategy.h"
#include "Playerbots.h"

class AssassinationRogueStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    AssassinationRogueStrategyActionNodeFactory()
    {
        creators["mutilate"] = &mutilate;
        creators["envenom"] = &envenom;
        creators["backstab"] = &backstab;
        creators["rupture"] = &rupture;
    }

private:
    static ActionNode* mutilate([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "mutilate",
            /*P*/ {},
            /*A*/ { NextAction("backstab") },
            /*C*/ {}
        );
    }
    static ActionNode* envenom([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "envenom",
            /*P*/ {},
            /*A*/ { NextAction("eviscerate") },
            /*C*/ {}
        );
    }
    static ActionNode* backstab([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "backstab",
            /*P*/ {},
            /*A*/ { NextAction("sinister strike") },
            /*C*/ {}
        );
    }
    static ActionNode* rupture([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "rupture",
            /*P*/ {},
            /*A*/ { NextAction("eviscerate") },
            /*C*/ {}
        );
    }
};

AssassinationRogueStrategy::AssassinationRogueStrategy(PlayerbotAI* botAI)
    : GenericRogueStrategy(botAI)
{
    actionNodeFactories.Add(new AssassinationRogueStrategyActionNodeFactory());
}

std::vector<NextAction> AssassinationRogueStrategy::getDefaultActions()
{
    return {
        NextAction("melee", ACTION_DEFAULT)
    };
}

void AssassinationRogueStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericRogueStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "high energy available",
            {
                NextAction("garrote", ACTION_HIGH + 7),
                NextAction("ambush", ACTION_HIGH + 6)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "high energy available",
            {
                NextAction("mutilate", ACTION_NORMAL + 3)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "hunger for blood",
            {
                NextAction("hunger for blood", ACTION_HIGH + 6),
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "slice and dice",
            {
                NextAction("slice and dice", ACTION_HIGH + 5),
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "combo points 4 available",
            {
                NextAction("cold blood", ACTION_HIGH + 6),
                NextAction("envenom", ACTION_HIGH + 5)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "target with combo points almost dead",
            {
                NextAction("envenom", ACTION_HIGH + 4)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "expose armor",
            {
                NextAction("expose armor", ACTION_HIGH + 3),
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "medium threat",
            {
                NextAction("vanish", ACTION_HIGH),
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "low health",
            {
                NextAction("evasion", ACTION_HIGH + 9),
                NextAction("feint", ACTION_HIGH + 8)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "critical health",
            {
                NextAction("cloak of shadows", ACTION_HIGH + 7)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "kick",
            {
                NextAction("kick", ACTION_INTERRUPT + 2),
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "kick on enemy healer",
            {
                NextAction("kick on enemy healer", ACTION_INTERRUPT + 1),
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "medium aoe",
            {
                NextAction("fan of knives", ACTION_NORMAL + 5),
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "low tank threat",
            {
                NextAction("tricks of the trade on main tank", ACTION_HIGH + 7),
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "enemy out of melee",
            {
                NextAction("stealth", ACTION_HIGH + 3),
                NextAction("sprint", ACTION_HIGH + 2),
                NextAction("reach melee", ACTION_HIGH + 1),
            }
        )
    );
}
