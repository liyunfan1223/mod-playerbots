/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "BloodDKStrategy.h"

#include "Playerbots.h"

class BloodDKStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    BloodDKStrategyActionNodeFactory()
    {
        // creators["melee"] = &melee;
        // creators["blood strike"] = &blood_strike;
        creators["rune strike"] = &rune_strike;
        creators["heart strike"] = &heart_strike;
        creators["death strike"] = &death_strike;
        // creators["death grip"] = &death_grip;
        // creators["plague strike"] = &plague_strike;
        // creators["pestilence"] = &pestilence;
        creators["icy touch"] = &icy_touch;
        // creators["obliterate"] = &obliterate;
        // creators["blood boil"] = &blood_boil;
        // creators["mark of_blood"] = &mark_of_blood;
        // creators["blood presence"] = &blood_presence;
        // creators["rune tap"] = &rune_tap;
        // creators["vampiric blood"] = &vampiric_blood;
        // creators["death pact"] = &death_pact;
        // creators["death rune_mastery"] = &death_rune_mastery;
        // creators["hysteria"] = &hysteria;
        // creators["dancing weapon"] = &dancing_weapon;
        creators["dark command"] = &dark_command;
        creators["taunt spell"] = &dark_command;
    }

private:
    static ActionNode* rune_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("rune strike",
                              /*P*/ NextAction::array(0, new NextAction("frost presence"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }
    static ActionNode* icy_touch([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("icy touch",
                              /*P*/ NextAction::array(0, new NextAction("frost presence"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }
    static ActionNode* heart_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("heart strike",
                              /*P*/ NextAction::array(0, new NextAction("frost presence"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* death_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("death strike",
                              /*P*/ NextAction::array(0, new NextAction("frost presence"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }
    static ActionNode* dark_command([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("dark command",
                              /*P*/ NextAction::array(0, new NextAction("frost presence"), NULL),
                              /*A*/ NextAction::array(0, new NextAction("death grip"), NULL),
                              /*C*/ NULL);
    }
};

BloodDKStrategy::BloodDKStrategy(PlayerbotAI* botAI) : GenericDKStrategy(botAI)
{
    actionNodeFactories.Add(new BloodDKStrategyActionNodeFactory());
}

NextAction** BloodDKStrategy::getDefaultActions()
{
    return NextAction::array(
        0, new NextAction("rune strike", ACTION_DEFAULT + 0.8f), new NextAction("icy touch", ACTION_DEFAULT + 0.7f),
        new NextAction("heart strike", ACTION_DEFAULT + 0.6f), new NextAction("blood strike", ACTION_DEFAULT + 0.5f),
        new NextAction("dancing rune weapon", ACTION_DEFAULT + 0.4f),
        new NextAction("death coil", ACTION_DEFAULT + 0.3f), new NextAction("plague strike", ACTION_DEFAULT + 0.2f),
        new NextAction("horn of winter", ACTION_DEFAULT + 0.1f), new NextAction("melee", ACTION_DEFAULT), NULL);
}

void BloodDKStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericDKStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "rune strike", NextAction::array(0, new NextAction("rune strike", ACTION_NORMAL + 3), nullptr)));
    triggers.push_back(
        new TriggerNode("blood tap", NextAction::array(0, new NextAction("blood tap", ACTION_HIGH + 5), nullptr)));
    triggers.push_back(
        new TriggerNode("lose aggro", NextAction::array(0, new NextAction("dark command", ACTION_HIGH + 3), nullptr)));
    triggers.push_back(
        new TriggerNode("low health", NextAction::array(0, new NextAction("army of the dead", ACTION_HIGH + 4),
                                                        new NextAction("death strike", ACTION_HIGH + 3), nullptr)));
    triggers.push_back(
        new TriggerNode("critical health", NextAction::array(0, new NextAction("vampiric blood", ACTION_HIGH + 5), nullptr)));
    triggers.push_back(
        new TriggerNode("icy touch", NextAction::array(0, new NextAction("icy touch", ACTION_HIGH + 2), nullptr)));
    triggers.push_back(new TriggerNode(
        "plague strike", NextAction::array(0, new NextAction("plague strike", ACTION_HIGH + 2), nullptr)));
}
