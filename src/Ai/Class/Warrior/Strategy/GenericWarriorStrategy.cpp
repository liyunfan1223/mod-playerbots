/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GenericWarriorStrategy.h"
#include "Playerbots.h"

class GenericWarriorStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericWarriorStrategyActionNodeFactory() { creators["berserker rage"] = &berserker_rage; }

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

GenericWarriorStrategy::GenericWarriorStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericWarriorStrategyActionNodeFactory());
}

void GenericWarriorStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    CombatStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode(
        "enemy out of melee", { NextAction("reach melee", ACTION_HIGH + 1) }));
    triggers.push_back(new TriggerNode(
        "fear sleep sap", { NextAction("berserker rage", ACTION_EMERGENCY + 1) }));
}

class WarriorAoeStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    WarriorAoeStrategyActionNodeFactory()
    {

    }

private:

};

WarriorAoeStrategy::WarriorAoeStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI)
{
    actionNodeFactories.Add(new WarriorAoeStrategyActionNodeFactory());
}

void WarriorAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "light aoe", { NextAction("sweeping strikes", ACTION_HIGH + 7),
                                       NextAction("bladestorm", ACTION_HIGH + 6),
                                       NextAction("thunder clap", ACTION_HIGH + 5),
                                       NextAction("shockwave", ACTION_HIGH + 4),
                                       NextAction("demoralizing shout without life time check", ACTION_HIGH + 1),
                                       NextAction("cleave", ACTION_HIGH) }));
    triggers.push_back(
        new TriggerNode("shockwave on snare target",
                        { NextAction("shockwave on snare target", ACTION_HIGH + 5) }));

}
