/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "FuryWarriorStrategy.h"

#include "Playerbots.h"

class FuryWarriorStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    FuryWarriorStrategyActionNodeFactory()
    {
        creators["charge"] = &charge;
        creators["intercept"] = &intercept;
        // creators["death wish"] = &death_wish;
        creators["piercing howl"] = &piercing_howl;
        // creators["bloodthirst"] = &bloodthirst;
        creators["pummel"] = &pummel;
    }

private:
    ACTION_NODE_A(charge, "charge", "intercept");
    ACTION_NODE_A(intercept, "intercept", "reach melee");
    ACTION_NODE_A(piercing_howl, "piercing howl", "hamstring");
    // ACTION_NODE_A(death_wish, "death wish", "berserker rage");
    // ACTION_NODE_A(bloodthirst, "bloodthirst", "melee");
    ACTION_NODE_A(pummel, "pummel", "intercept");
};

FuryWarriorStrategy::FuryWarriorStrategy(PlayerbotAI* botAI) : GenericWarriorStrategy(botAI)
{
    actionNodeFactories.Add(new FuryWarriorStrategyActionNodeFactory());
}

NextAction** FuryWarriorStrategy::getDefaultActions()
{
    return NextAction::array(
        0, new NextAction("bloodthirst", ACTION_DEFAULT + 0.5f), new NextAction("whirlwind", ACTION_DEFAULT + 0.4f),
        new NextAction("sunder armor", ACTION_DEFAULT + 0.3f), new NextAction("execute", ACTION_DEFAULT + 0.2f),
        // new NextAction("overpower", ACTION_DEFAULT + 0.1f),
        new NextAction("melee", ACTION_DEFAULT), NULL);
}

void FuryWarriorStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericWarriorStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("enemy out of melee",
                                       NextAction::array(0, new NextAction("charge", ACTION_MOVE + 9), nullptr)));
    triggers.push_back(new TriggerNode(
        "berserker stance", NextAction::array(0, new NextAction("berserker stance", ACTION_HIGH + 9), nullptr)));
    triggers.push_back(new TriggerNode("battle shout",
                                       NextAction::array(0, new NextAction("battle shout", ACTION_HIGH + 8), nullptr)));
    // triggers.push_back(new TriggerNode("target critical health", NextAction::array(0, new NextAction("execute",
    // ACTION_HIGH + 4), nullptr))); triggers.push_back(new TriggerNode("sudden death", NextAction::array(0, new
    // NextAction("execute", ACTION_HIGH + 4), nullptr))); triggers.push_back(new TriggerNode("hamstring",
    // NextAction::array(0, new NextAction("piercing howl", ACTION_HIGH + 1), nullptr)));
    triggers.push_back(
        new TriggerNode("pummel on enemy healer",
                        NextAction::array(0, new NextAction("pummel on enemy healer", ACTION_INTERRUPT), nullptr)));
    triggers.push_back(
        new TriggerNode("pummel", NextAction::array(0, new NextAction("pummel", ACTION_INTERRUPT), nullptr)));
    triggers.push_back(new TriggerNode(
        "victory rush", NextAction::array(0, new NextAction("victory rush", ACTION_INTERRUPT), nullptr)));
    // triggers.push_back(new TriggerNode("intercept on snare target", NextAction::array(0, new NextAction("intercept on
    // snare target", ACTION_HIGH), nullptr)));
    triggers.push_back(
        new TriggerNode("bloodthirst", NextAction::array(0, new NextAction("bloodthirst", ACTION_HIGH + 7), nullptr)));
    triggers.push_back(
        new TriggerNode("instant slam", NextAction::array(0, new NextAction("slam", ACTION_HIGH + 5), nullptr)));
    triggers.push_back(
        new TriggerNode("bloodrage", NextAction::array(0, new NextAction("bloodrage", ACTION_HIGH + 2), nullptr)));
    triggers.push_back(new TriggerNode("medium rage available",
                                       NextAction::array(0, new NextAction("heroic strike", ACTION_HIGH + 1), NULL)));
    // triggers.push_back(new TriggerNode("berserker rage", NextAction::array(0, new NextAction("berserker rage",
    // ACTION_HIGH + 2), nullptr))); triggers.push_back(new TriggerNode("light aoe", NextAction::array(0,
    //     new NextAction("whirlwind", ACTION_HIGH + 2),
    //     nullptr)));

    triggers.push_back(
        new TriggerNode("death wish", NextAction::array(0, new NextAction("death wish", ACTION_HIGH), nullptr)));
    triggers.push_back(
        new TriggerNode("recklessness", NextAction::array(0, new NextAction("recklessness", ACTION_HIGH), nullptr)));
}
