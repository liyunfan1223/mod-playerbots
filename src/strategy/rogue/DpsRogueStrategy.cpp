/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "DpsRogueStrategy.h"

#include "Playerbots.h"

class DpsRogueStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    DpsRogueStrategyActionNodeFactory()
    {
        creators["mutilate"] = &mutilate;
        creators["sinister strike"] = &sinister_strike;
        creators["kick"] = &kick;
        creators["kidney shot"] = &kidney_shot;
        creators["backstab"] = &backstab;
        creators["melee"] = &melee;
        creators["rupture"] = &rupture;
    }

private:
    static ActionNode* melee(PlayerbotAI* botAI)
    {
        return new ActionNode("melee",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("mutilate"), nullptr),
                              /*C*/ nullptr);
    }
    static ActionNode* mutilate(PlayerbotAI* botAI)
    {
        return new ActionNode("mutilate",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("sinister strike"), nullptr),
                              /*C*/ nullptr);
    }
    static ActionNode* sinister_strike(PlayerbotAI* botAI)
    {
        return new ActionNode("sinister strike",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("melee"), nullptr),
                              /*C*/ nullptr);
    }
    static ActionNode* kick(PlayerbotAI* botAI)
    {
        return new ActionNode("kick",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("kidney shot"), nullptr),
                              /*C*/ nullptr);
    }
    static ActionNode* kidney_shot(PlayerbotAI* botAI)
    {
        return new ActionNode("kidney shot",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }
    static ActionNode* backstab(PlayerbotAI* botAI)
    {
        return new ActionNode("backstab",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("mutilate"), nullptr),
                              /*C*/ nullptr);
    }
    static ActionNode* rupture(PlayerbotAI* botAI)
    {
        return new ActionNode("rupture",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("eviscerate"), nullptr),
                              /*C*/ nullptr);
    }
};

DpsRogueStrategy::DpsRogueStrategy(PlayerbotAI* botAI) : MeleeCombatStrategy(botAI)
{
    actionNodeFactories.Add(new DpsRogueStrategyActionNodeFactory());
}

NextAction** DpsRogueStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("killing spree", ACTION_DEFAULT + 0.1f),
                             new NextAction("melee", ACTION_DEFAULT), nullptr);
}

void DpsRogueStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    MeleeCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("high energy available",
                                       NextAction::array(0, new NextAction("garrote", ACTION_HIGH + 7),
                                                         new NextAction("ambush", ACTION_HIGH + 6), nullptr)));

    triggers.push_back(new TriggerNode(
        "high energy available", NextAction::array(0, new NextAction("sinister strike", ACTION_NORMAL + 3), nullptr)));

    triggers.push_back(new TriggerNode(
        "slice and dice", NextAction::array(0, new NextAction("slice and dice", ACTION_HIGH + 2), nullptr)));

    triggers.push_back(new TriggerNode("combo points available",
                                       NextAction::array(0, new NextAction("rupture", ACTION_HIGH + 1),
                                                         new NextAction("eviscerate", ACTION_HIGH), nullptr)));

    triggers.push_back(new TriggerNode("target with combo points almost dead",
                                       NextAction::array(0, new NextAction("eviscerate", ACTION_HIGH + 2), nullptr)));

    triggers.push_back(
        new TriggerNode("medium threat", NextAction::array(0, new NextAction("vanish", ACTION_HIGH), nullptr)));

    triggers.push_back(
        new TriggerNode("low health", NextAction::array(0, new NextAction("evasion", ACTION_HIGH + 9),
                                                        new NextAction("feint", ACTION_HIGH + 8), nullptr)));

    triggers.push_back(new TriggerNode(
        "critical health", NextAction::array(0, new NextAction("cloak of shadows", ACTION_HIGH + 7), nullptr)));

    triggers.push_back(
        new TriggerNode("kick", NextAction::array(0, new NextAction("kick", ACTION_INTERRUPT + 2), nullptr)));

    triggers.push_back(
        new TriggerNode("kick on enemy healer",
                        NextAction::array(0, new NextAction("kick on enemy healer", ACTION_INTERRUPT + 1), nullptr)));

    // triggers.push_back(new TriggerNode(
    //     "behind target",
    //     NextAction::array(0, new NextAction("backstab", ACTION_NORMAL), nullptr)));

    triggers.push_back(
        new TriggerNode("light aoe", NextAction::array(0, new NextAction("blade flurry", ACTION_HIGH + 3), nullptr)));

    triggers.push_back(new TriggerNode("blade flurry",
                                       NextAction::array(0, new NextAction("blade flurry", ACTION_HIGH + 2), nullptr)));

    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        NextAction::array(0, new NextAction("stealth", ACTION_HIGH + 3), new NextAction("sprint", ACTION_HIGH + 2),
                          new NextAction("reach melee", ACTION_HIGH + 1), nullptr)));

    triggers.push_back(new TriggerNode("expose armor",
                                       NextAction::array(0, new NextAction("expose armor", ACTION_HIGH + 3), nullptr)));

    triggers.push_back(new TriggerNode(
        "low tank threat",
        NextAction::array(0, new NextAction("tricks of the trade on main tank", ACTION_HIGH + 7), nullptr)));
}

class StealthedRogueStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    StealthedRogueStrategyActionNodeFactory()
    {
        creators["ambush"] = &ambush;
        creators["cheap shot"] = &cheap_shot;
        creators["garrote"] = &garrote;
        creators["sap"] = &sap;
        creators["sinister strike"] = &sinister_strike;
    }

private:
    static ActionNode* ambush(PlayerbotAI* botAI)
    {
        return new ActionNode("ambush",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("garrote"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* cheap_shot(PlayerbotAI* botAI)
    {
        return new ActionNode("cheap shot",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* garrote(PlayerbotAI* botAI)
    {
        return new ActionNode("garrote",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* sap(PlayerbotAI* botAI)
    {
        return new ActionNode("sap",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* sinister_strike(PlayerbotAI* botAI)
    {
        return new ActionNode("sinister strike",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("cheap shot"), nullptr),
                              /*C*/ nullptr);
    }
};

StealthedRogueStrategy::StealthedRogueStrategy(PlayerbotAI* botAI) : Strategy(botAI)
{
    actionNodeFactories.Add(new StealthedRogueStrategyActionNodeFactory());
}

NextAction** StealthedRogueStrategy::getDefaultActions()
{
    return NextAction::array(
        0, new NextAction("ambush", ACTION_NORMAL + 4), new NextAction("backstab", ACTION_NORMAL + 3),
        new NextAction("cheap shot", ACTION_NORMAL + 2), new NextAction("sinister strike", ACTION_NORMAL + 1),
        new NextAction("melee", ACTION_NORMAL), nullptr);
}

void StealthedRogueStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("combo points available",
                                       NextAction::array(0, new NextAction("eviscerate", ACTION_HIGH), nullptr)));
    triggers.push_back(
        new TriggerNode("kick", NextAction::array(0, new NextAction("cheap shot", ACTION_INTERRUPT), nullptr)));
    triggers.push_back(new TriggerNode("kick on enemy healer",
                                       NextAction::array(0, new NextAction("cheap shot", ACTION_INTERRUPT), nullptr)));
    triggers.push_back(
        new TriggerNode("behind target", NextAction::array(0, new NextAction("ambush", ACTION_HIGH), nullptr)));
    triggers.push_back(
        new TriggerNode("not behind target", NextAction::array(0, new NextAction("cheap shot", ACTION_HIGH), nullptr)));
    triggers.push_back(new TriggerNode("enemy flagcarrier near",
                                       NextAction::array(0, new NextAction("sprint", ACTION_EMERGENCY + 1), nullptr)));
    triggers.push_back(
        new TriggerNode("unstealth", NextAction::array(0, new NextAction("unstealth", ACTION_NORMAL), nullptr)));
    /*triggers.push_back(new TriggerNode("low health", NextAction::array(0, new NextAction("food", ACTION_EMERGENCY +
     * 1), nullptr)));*/
    triggers.push_back(new TriggerNode(
        "no stealth", NextAction::array(0, new NextAction("check stealth", ACTION_EMERGENCY), nullptr)));
    triggers.push_back(
        new TriggerNode("sprint", NextAction::array(0, new NextAction("sprint", ACTION_INTERRUPT), nullptr)));
}

void StealthStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("stealth", NextAction::array(0, new NextAction("stealth", ACTION_INTERRUPT), nullptr)));
}

void RogueAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("light aoe", NextAction::array(0, new NextAction("blade flurry", ACTION_HIGH), nullptr)));
    triggers.push_back(new TriggerNode(
        "medium aoe", NextAction::array(0, new NextAction("fan of knives", ACTION_NORMAL + 5), nullptr)));
}

void RogueBoostStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "adrenaline rush", NextAction::array(0, new NextAction("adrenaline rush", ACTION_HIGH + 2), nullptr)));
}

void RogueCcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("sap", NextAction::array(0, new NextAction("stealth", ACTION_INTERRUPT),
                                                                new NextAction("sap", ACTION_INTERRUPT), nullptr)));
}
