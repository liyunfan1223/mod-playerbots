/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CombatRogueStrategy.h"
#include "Playerbots.h"

class CombatRogueStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    CombatRogueStrategyActionNodeFactory()
    {
        creators["sinister strike"] = &sinister_strike;
        creators["kick"] = &kick;
        creators["backstab"] = &backstab;
        creators["rupture"] = &rupture;
    }

private:
    static ActionNode* sinister_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "sinister strike",
            /*P*/ {},
            /*A*/ {
                NextAction("melee") },
            /*C*/ {}
        );
    }
    static ActionNode* kick([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "kick",
            /*P*/ {},
            /*A*/ {
                NextAction("kidney shot") },
            /*C*/ {}
        );
    }
    static ActionNode* backstab([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "backstab",
            /*P*/ {},
            /*A*/ {
                NextAction("sinister strike") },
            /*C*/ {}
        );
    }
    static ActionNode* rupture([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "rupture",
            /*P*/ {},
            /*A*/ {
                NextAction("eviscerate") },
            /*C*/ {}
        );
    }
};

CombatRogueStrategy::CombatRogueStrategy(PlayerbotAI* botAI) : GenericRogueStrategy(botAI)
{
    actionNodeFactories.Add(new CombatRogueStrategyActionNodeFactory());
}

std::vector<NextAction> CombatRogueStrategy::getDefaultActions()
{
    return {
        NextAction("killing spree", ACTION_DEFAULT + 0.1f),
        NextAction("melee", ACTION_DEFAULT)
    };
}

void CombatRogueStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
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
                NextAction("sinister strike", ACTION_NORMAL + 3)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "slice and dice",
            {
                NextAction("slice and dice", ACTION_HIGH + 5)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "riposte",
            {
                NextAction("riposte", ACTION_HIGH + 4)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "combo points 5 available",
            {
                NextAction("rupture", ACTION_HIGH + 1),
                NextAction("eviscerate", ACTION_HIGH)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "target with combo points almost dead",
            {
                NextAction("eviscerate", ACTION_HIGH + 2)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "medium threat",
            {
                NextAction("vanish", ACTION_HIGH)
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
                NextAction("kick", ACTION_INTERRUPT + 2)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "kick on enemy healer",
            {
                NextAction("kick on enemy healer", ACTION_INTERRUPT + 1)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "enemy out of melee",
            {
                NextAction("stealth", ACTION_HIGH + 3),
                NextAction("sprint", ACTION_HIGH + 2),
                NextAction("reach melee", ACTION_HIGH + 1)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "expose armor",
            {
                NextAction("expose armor", ACTION_HIGH + 3)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "low tank threat",
            {
                NextAction("tricks of the trade on main tank", ACTION_HIGH + 7)
            }
        )
    );
}

class StealthedRogueStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    StealthedRogueStrategyActionNodeFactory()
    {
        creators["ambush"] = &ambush;
        creators["sinister strike"] = &sinister_strike;
    }

private:
    static ActionNode* ambush([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "ambush",
            /*P*/ {},
            /*A*/ { NextAction("garrote") },
            /*C*/ {}
        );
    }

    static ActionNode* sinister_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "sinister strike",
            /*P*/ {},
            /*A*/ { NextAction("cheap shot") },
            /*C*/ {}
        );
    }
};

StealthedRogueStrategy::StealthedRogueStrategy(PlayerbotAI* botAI) : Strategy(botAI)
{
    actionNodeFactories.Add(new StealthedRogueStrategyActionNodeFactory());
}

std::vector<NextAction> StealthedRogueStrategy::getDefaultActions()
{
    return {
        NextAction("ambush", ACTION_NORMAL + 4),
        NextAction("backstab", ACTION_NORMAL + 3),
        NextAction("cheap shot", ACTION_NORMAL + 2),
        NextAction("sinister strike", ACTION_NORMAL + 1),
        NextAction("melee", ACTION_NORMAL)
    };
}

void StealthedRogueStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "combo points 5 available",
            {
                NextAction("eviscerate", ACTION_HIGH)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "kick",
            {
                NextAction("cheap shot", ACTION_INTERRUPT)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "kick on enemy healer",
            {
                NextAction("cheap shot", ACTION_INTERRUPT)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "behind target",
            {
                NextAction("ambush", ACTION_HIGH)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "not behind target",
            {
                NextAction("cheap shot", ACTION_HIGH)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "enemy flagcarrier near",
            {
                NextAction("sprint", ACTION_EMERGENCY + 1)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "unstealth",
            {
                NextAction("unstealth", ACTION_NORMAL)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "no stealth",
            {
                NextAction("check stealth", ACTION_EMERGENCY)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "sprint",
            {
                NextAction("sprint", ACTION_INTERRUPT)
            }
        )
    );
}

void StealthStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "stealth",
            {
                NextAction("stealth", ACTION_INTERRUPT)
            }
        )
    );
}

void RogueAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "light aoe",
            {
                NextAction("blade flurry", ACTION_HIGH + 4)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium aoe",
            {
                NextAction("fan of knives", ACTION_NORMAL + 5)
            }
        )
    );
}

void RogueBoostStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "adrenaline rush",
            {
                NextAction("adrenaline rush", ACTION_HIGH + 2)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "blade flurry",
            {
                NextAction("blade flurry", ACTION_HIGH + 4)
            }
        )
    );
}

void RogueCcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "sap",
            {
                NextAction("stealth", ACTION_INTERRUPT),
                NextAction("sap", ACTION_INTERRUPT)
            }
        )
    );
}
