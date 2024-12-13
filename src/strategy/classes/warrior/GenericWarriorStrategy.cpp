/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GenericWarriorStrategy.h"

#include "Playerbots.h"

GenericWarriorStrategy::GenericWarriorStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI)
{
    // actionNodeFactories.Add(new WarriorStanceRequirementActionNodeFactory());
}

void GenericWarriorStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    CombatStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode(
        "enemy out of melee", NextAction::array(0, new NextAction("reach melee", ACTION_NORMAL + 8), nullptr)));
    /*triggers.push_back(new TriggerNode("bloodrage", NextAction::array(0, new NextAction("bloodrage", ACTION_HIGH + 1),
    nullptr))); triggers.push_back(new TriggerNode("shield bash", NextAction::array(0, new NextAction("shield bash",
    ACTION_INTERRUPT + 4), nullptr))); triggers.push_back(new TriggerNode("shield bash on enemy healer",
    NextAction::array(0, new NextAction("shield bash on enemy healer", ACTION_INTERRUPT + 3), nullptr)));
    triggers.push_back(new TriggerNode("critical health", NextAction::array(0, new NextAction("intimidating shout",
    ACTION_EMERGENCY), nullptr)));*/
}

class WarrirorAoeStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    WarrirorAoeStrategyActionNodeFactory()
    {
        // creators["whirlwind"] = &whirlwind;
    }

private:
    // ACTION_NODE_A(whirlwind, "whirlwind", "cleave");
};

WarrirorAoeStrategy::WarrirorAoeStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI)
{
    actionNodeFactories.Add(new WarrirorAoeStrategyActionNodeFactory());
}

void WarrirorAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // triggers.push_back(new TriggerNode("thunder clap on snare target", NextAction::array(0, new NextAction("thunder
    // clap on snare target", ACTION_HIGH + 3), nullptr))); triggers.push_back(new TriggerNode("thunder clap",
    // NextAction::array(0, new NextAction("thunder clap", ACTION_HIGH + 10), nullptr)));
    triggers.push_back(new TriggerNode(
        "light aoe", NextAction::array(0, new NextAction("thunder clap", ACTION_HIGH + 5),
                                       new NextAction("shockwave", ACTION_HIGH + 4),
                                       new NextAction("sweeping strikes", ACTION_HIGH + 3),
                                       new NextAction("bladestorm", ACTION_HIGH + 3),
                                       // new NextAction("whirlwind", ACTION_HIGH + 2),
                                       new NextAction("demoralizing shout without life time check", ACTION_HIGH + 1),
                                       new NextAction("cleave", ACTION_HIGH), nullptr)));
    triggers.push_back(
        new TriggerNode("shockwave on snare target",
                        NextAction::array(0, new NextAction("shockwave on snare target", ACTION_HIGH + 5), nullptr)));
    // triggers.push_back(new TriggerNode("high rage available", NextAction::array(0, new NextAction("whirlwind",
    // ACTION_HIGH + 10), nullptr)));
}
