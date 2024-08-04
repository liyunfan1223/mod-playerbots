/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "TankWarriorStrategy.h"

#include "Playerbots.h"

class TankWarriorStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    TankWarriorStrategyActionNodeFactory()
    {
        creators["charge"] = &charge;
        creators["sunder armor"] = &sunder_armor;
        creators["commanding shout"] = &commanding_shout;
        // creators["shield slam"] = &shield_slam;
        creators["devastate"] = &devastate;
        creators["last stand"] = &last_stand;
        creators["heroic throw on snare target"] = &heroic_throw_on_snare_target;
        creators["heroic throw taunt"] = &heroic_throw_taunt;
        creators["taunt"] = &taunt;
        creators["taunt spell"] = &taunt;
    }

private:
    // ACTION_NODE_A(charge, "charge", "intercept with stance");
    ACTION_NODE_A(charge, "charge", "reach melee");
    ACTION_NODE_A(sunder_armor, "sunder armor", "melee");
    ACTION_NODE_A(commanding_shout, "commanding shout", "battle shout");
    // ACTION_NODE_A(shield_slam, "shield slam", "heroic strike");
    ACTION_NODE_A(devastate, "devastate", "sunder armor");
    ACTION_NODE_A(last_stand, "last stand", "intimidating shout");
    ACTION_NODE_A(heroic_throw_on_snare_target, "heroic throw on snare target", "taunt on snare target");
    ACTION_NODE_A(heroic_throw_taunt, "heroic throw", "shield slam");
    static ActionNode* taunt(PlayerbotAI* botAI)
    {
        return new ActionNode("taunt",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("heroic throw taunt"), nullptr),
                              /*C*/ nullptr);
    }
};

TankWarriorStrategy::TankWarriorStrategy(PlayerbotAI* botAI) : GenericWarriorStrategy(botAI)
{
    actionNodeFactories.Add(new TankWarriorStrategyActionNodeFactory());
}

NextAction** TankWarriorStrategy::getDefaultActions()
{
    return NextAction::array(
        0, new NextAction("devastate", ACTION_DEFAULT + 0.3f), new NextAction("revenge", ACTION_DEFAULT + 0.2f),
        new NextAction("demoralizing shout", ACTION_DEFAULT + 0.1f), new NextAction("melee", ACTION_DEFAULT), NULL);
}

void TankWarriorStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericWarriorStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode("enemy out of melee", NextAction::array(0, new NextAction("heroic throw", ACTION_MOVE + 11),
                                                                new NextAction("charge", ACTION_MOVE + 10), nullptr)));
    // triggers.push_back(new TriggerNode("intercept and rage", NextAction::array(0, new NextAction("berserker stance",
    // ACTION_MOVE + 14), nullptr))); triggers.push_back(new TriggerNode("intercept and rage", NextAction::array(0, new
    // NextAction("intercept", ACTION_MOVE + 13), nullptr))); triggers.push_back(new TriggerNode("thunder clap and
    // rage", NextAction::array(0, new NextAction("battle stance", ACTION_MOVE + 12), nullptr)));
    triggers.push_back(new TriggerNode(
        "thunder clap and rage", NextAction::array(0, new NextAction("thunder clap", ACTION_MOVE + 11), nullptr)));
    triggers.push_back(new TriggerNode(
        "defensive stance", NextAction::array(0, new NextAction("defensive stance", ACTION_HIGH + 9), nullptr)));
    triggers.push_back(new TriggerNode(
        "commanding shout", NextAction::array(0, new NextAction("commanding shout", ACTION_HIGH + 8), nullptr)));
    triggers.push_back(
        new TriggerNode("bloodrage", NextAction::array(0, new NextAction("bloodrage", ACTION_HIGH + 2), nullptr)));
    triggers.push_back(
        new TriggerNode("sunder armor", NextAction::array(0, new NextAction("devastate", ACTION_HIGH + 2), nullptr)));
    triggers.push_back(new TriggerNode("medium rage available",
                                       NextAction::array(0, new NextAction("shield slam", ACTION_HIGH + 1), nullptr)));
    triggers.push_back(new TriggerNode(
        "shield block", NextAction::array(0, new NextAction("shield block", ACTION_INTERRUPT + 1), nullptr)));
    triggers.push_back(
        new TriggerNode("revenge", NextAction::array(0, new NextAction("revenge", ACTION_HIGH + 2), nullptr)));
    triggers.push_back(
        new TriggerNode("disarm", NextAction::array(0, new NextAction("disarm", ACTION_HIGH + 1), nullptr)));
    triggers.push_back(
        new TriggerNode("lose aggro", NextAction::array(0, new NextAction("taunt", ACTION_INTERRUPT + 1), nullptr)));
    triggers.push_back(new TriggerNode(
        "taunt on snare target",
        NextAction::array(0, new NextAction("heroic throw on snare target", ACTION_INTERRUPT), nullptr)));
    triggers.push_back(new TriggerNode(
        "low health", NextAction::array(0, new NextAction("shield wall", ACTION_MEDIUM_HEAL), nullptr)));
    triggers.push_back(new TriggerNode(
        "critical health", NextAction::array(0, new NextAction("last stand", ACTION_EMERGENCY + 3), nullptr)));
    // triggers.push_back(new TriggerNode("medium aoe", NextAction::array(0, new NextAction("battle shout taunt",
    // ACTION_HIGH + 1), nullptr)));
    triggers.push_back(new TriggerNode(
        "high aoe", NextAction::array(0, new NextAction("challenging shout", ACTION_HIGH + 3), nullptr)));
    triggers.push_back(new TriggerNode(
        "concussion blow", NextAction::array(0, new NextAction("concussion blow", ACTION_INTERRUPT), nullptr)));
    triggers.push_back(
        new TriggerNode("shield bash", NextAction::array(0, new NextAction("shield bash", ACTION_INTERRUPT), nullptr)));
    triggers.push_back(new TriggerNode(
        "shield bash on enemy healer",
        NextAction::array(0, new NextAction("shield bash on enemy healer", ACTION_INTERRUPT), nullptr)));
    triggers.push_back(new TriggerNode(
        "spell reflection", NextAction::array(0, new NextAction("spell reflection", ACTION_INTERRUPT + 1), nullptr)));
    triggers.push_back(new TriggerNode(
        "victory rush", NextAction::array(0, new NextAction("victory rush", ACTION_INTERRUPT), nullptr)));
    triggers.push_back(new TriggerNode("sword and board",
                                       NextAction::array(0, new NextAction("shield slam", ACTION_INTERRUPT), nullptr)));
    triggers.push_back(
        new TriggerNode("rend", NextAction::array(0, new NextAction("rend", ACTION_NORMAL + 1), nullptr)));
    triggers.push_back(new TriggerNode(
        "rend on attacker", NextAction::array(0, new NextAction("rend on attacker", ACTION_NORMAL + 1), nullptr)));
    triggers.push_back(new TriggerNode("protect party member",
                                       NextAction::array(0, new NextAction("intervene", ACTION_EMERGENCY), nullptr)));
    triggers.push_back(new TriggerNode(
        "high rage available", NextAction::array(0, new NextAction("heroic strike", ACTION_HIGH + 1), nullptr)));
    triggers.push_back(new TriggerNode("medium rage available",
                                       NextAction::array(0, new NextAction("thunder clap", ACTION_HIGH + 1), nullptr)));
}
