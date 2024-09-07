#/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GenericDKStrategy.h"

#include "DKAiObjectContext.h"
#include "Playerbots.h"

class GenericDKStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericDKStrategyActionNodeFactory()
    {
        // blood
        // creators["rune tap"] = &rune_tap; cd
        // creators["vampiric blood"] = &vampiric_blood;
        // creators["death pact"] = &death_pact;
        // creators["hysteria"] = &hysteria; boost party
        // creators["dancing rune weapon"] = &dancing_rune_weapon; //cd
        // creators["dark command"] = &dark_command; taunt

        // frost
        // creators["chains of ice"] = &chains_of_ice;
        // creators["icy clutch"] = &icy_clutch;
        creators["horn of winter"] = &horn_of_winter;
        creators["killing machine"] = &killing_machine;  // buff
        // creators["deathchill"] = &deathchill;		//boost
        creators["icebound fortitude"] = &icebound_fortitude;
        // creators["mind freeze"] = &mind_freeze; interrupt
        // creators["empower rune weapon"] = &empower_rune_weapon; boost
        // creators["hungering cold"] = &hungering_cold; snare
        // creators["unbreakable armor"] = &unbreakable_armor; boost +cd
        // creators["improved icy talons"] = &improved_icy_talons; boost party

        // unholy
        creators["death and decay"] = &death_and_decay;
        // creators["raise dead"] = &raise_dead;
        // creators["army of the dead"] = &army of the dead;
        // creators["summon gargoyle"] = &army of the dead;
        // creators["anti magic shell"] = &anti_magic_shell; cd
        creators["anti magic zone"] = &anti_magic_zone;
        // creators["ghoul frenzy"] = &ghoul_frenzy;
        creators["corpse explosion"] = &corpse_explosion;
        creators["bone shield"] = &bone_shield;
        creators["heart strike"] = &heart_strike;
        creators["death grip"] = &death_grip;
        creators["plague strike"] = &plague_strike;
        creators["pestilence"] = &pestilence;
        creators["icy touch"] = &icy_touch;
    }

private:
    static ActionNode* death_coil([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("death coil",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* death_grip([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("death grip",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("icy touch"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* plague_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("plague strike",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* icy_touch([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("icy touch",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* heart_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("heart strike",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* pestilence([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("pestilence",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* horn_of_winter([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("horn of winter",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* bone_shield([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("bone shield",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* killing_machine([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("killing machine",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("improved icy talons"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* corpse_explosion([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("corpse explosion",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* death_and_decay([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("death and decay",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* anti_magic_zone([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("anti magic zone",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("anti magic shell"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* icebound_fortitude([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("icebound fortitude",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }
};

GenericDKStrategy::GenericDKStrategy(PlayerbotAI* botAI) : MeleeCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericDKStrategyActionNodeFactory());
}

void GenericDKStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    MeleeCombatStrategy::InitTriggers(triggers);

    // triggers.push_back(new TriggerNode("high aoe", NextAction::array(0, new NextAction("anti magic shell",
    // ACTION_NORMAL + 3), nullptr))); triggers.push_back(new TriggerNode("death coil", NextAction::array(0, new
    // NextAction("death coil", ACTION_NORMAL + 3), nullptr))); triggers.push_back(new TriggerNode("critical aoe heal",
    // NextAction::array(0, new NextAction("anti magic zone", ACTION_EMERGENCY + 1), nullptr)));
    triggers.push_back(
        new TriggerNode("no pet", NextAction::array(0, new NextAction("raise dead", ACTION_NORMAL + 5), nullptr)));
    triggers.push_back(
        new TriggerNode("mind freeze", NextAction::array(0, new NextAction("mind freeze", ACTION_HIGH + 1), nullptr)));
    triggers.push_back(
        new TriggerNode("mind freeze on enemy healer",
                        NextAction::array(0, new NextAction("mind freeze on enemy healer", ACTION_HIGH + 1), nullptr)));
    triggers.push_back(new TriggerNode(
        "horn of winter", NextAction::array(0, new NextAction("horn of winter", ACTION_NORMAL + 1), nullptr)));
    triggers.push_back(new TriggerNode("critical health",
                                       NextAction::array(0, new NextAction("death pact", ACTION_HIGH + 5), nullptr)));
    
    triggers.push_back(
        new TriggerNode("low health", NextAction::array(0, new NextAction("icebound fortitude", ACTION_HIGH + 5),
                                                        new NextAction("rune tap", ACTION_HIGH + 4), nullptr)));
    triggers.push_back(
        new TriggerNode("medium aoe", NextAction::array(0, new NextAction("death and decay", ACTION_HIGH + 9),
                                                        new NextAction("pestilence", ACTION_NORMAL + 4),
                                                        new NextAction("blood boil", ACTION_NORMAL + 3), nullptr)));
    // triggers.push_back(new TriggerNode("light aoe", NextAction::array(0,
    // 	new NextAction("pestilence", ACTION_NORMAL + 4),
    // 	nullptr)));
    triggers.push_back(
        new TriggerNode("pestilence glyph", NextAction::array(0, new NextAction("pestilence", ACTION_HIGH + 9), NULL)));
}
