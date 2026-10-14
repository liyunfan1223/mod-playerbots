/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GenericWarriorNonCombatStrategy.h"
#include "Playerbots.h"

class GenericWarriorNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericWarriorNonCombatStrategyActionNodeFactory() { creators["berserker rage"] = &berserker_rage; }

private:
    static ActionNode* berserker_rage([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "berserker rage",
            /*P*/ { NextAction("berserker stance") },
            /*A*/ {},
            /*C*/ {}
        );
    }
};

GenericWarriorNonCombatStrategy::GenericWarriorNonCombatStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericWarriorNonCombatStrategyActionNodeFactory());
}

void GenericWarriorNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("often", { NextAction("apply stone", 1.0f) }));
    triggers.push_back(new TriggerNode("vigilance", { NextAction("vigilance", 10.0f) }));
    triggers.push_back(new TriggerNode(
        "fear sleep sap", { NextAction("berserker rage", ACTION_EMERGENCY + 1) }));
}
