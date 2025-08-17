/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericHunterStrategy.h"

#include "Playerbots.h"
#include "Strategy.h"

class GenericHunterStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericHunterStrategyActionNodeFactory()
    {
        creators["rapid fire"] = &rapid_fire;
        creators["boost"] = &rapid_fire;
        creators["aspect of the pack"] = &aspect_of_the_pack;
        creators["aspect of the dragonhawk"] = &aspect_of_the_dragonhawk;
        creators["feign death"] = &feign_death;
        creators["wing clip"] = &wing_clip;
        creators["mongoose bite"] = &mongoose_bite;
        creators["raptor strike"] = &raptor_strike;
        creators["explosive trap"] = &explosive_trap;
    }

private:
    static ActionNode* rapid_fire([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("rapid fire",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("readiness"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* aspect_of_the_pack([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("aspect of the pack",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("aspect of the cheetah"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* aspect_of_the_dragonhawk([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("aspect of the dragonhawk",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("aspect of the hawk"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* feign_death([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("feign death",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* wing_clip([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("wing clip",
                              /*P*/ nullptr,
                              // /*A*/ NextAction::array(0, new NextAction("mongoose bite"), nullptr),
                              nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* mongoose_bite([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("mongoose bite",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("raptor strike"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* raptor_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("raptor strike",
                              /*P*/ NextAction::array(0, new NextAction("melee"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* explosive_trap([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("explosive trap",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("immolation trap"), nullptr),
                              /*C*/ nullptr);
    }
};

GenericHunterStrategy::GenericHunterStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericHunterStrategyActionNodeFactory());
}

void GenericHunterStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    CombatStrategy::InitTriggers(triggers);

    // Mark/Ammo/Mana Triggers
    triggers.push_back(new TriggerNode("no ammo", NextAction::array(0, new NextAction("equip upgrades", 30.0f), nullptr)));
    triggers.push_back(new TriggerNode("hunter's mark", NextAction::array(0, new NextAction("hunter's mark", 29.5f), nullptr)));
    triggers.push_back(new TriggerNode("rapid fire", NextAction::array(0, new NextAction("rapid fire", 29.0f), nullptr)));
    triggers.push_back(new TriggerNode("aspect of the viper", NextAction::array(0, new NextAction("aspect of the viper", 28.0f), NULL)));
    triggers.push_back(new TriggerNode("aspect of the hawk", NextAction::array(0, new NextAction("aspect of the dragonhawk", 27.5f), nullptr)));

    // Aggro/Threat/Defensive Triggers
    triggers.push_back(new TriggerNode("has aggro", NextAction::array(0, new NextAction("concussive shot", 20.0f), nullptr)));
    triggers.push_back(new TriggerNode("low tank threat", NextAction::array(0, new NextAction("misdirection on main tank", 27.0f), NULL)));
    triggers.push_back(new TriggerNode("low health", NextAction::array(0, new NextAction("deterrence", 35.0f), nullptr)));
    triggers.push_back(new TriggerNode("concussive shot on snare target", NextAction::array(0, new NextAction("concussive shot", 20.0f), nullptr)));
    triggers.push_back(new TriggerNode("medium threat", NextAction::array(0, new NextAction("feign death", 35.0f), nullptr)));
    triggers.push_back(new TriggerNode("hunters pet medium health", NextAction::array(0, new NextAction("mend pet", 22.0f), nullptr)));
    triggers.push_back(new TriggerNode("hunters pet low health", NextAction::array(0, new NextAction("mend pet", 21.0f), nullptr)));

    // Dispel Triggers
    triggers.push_back(new TriggerNode("tranquilizing shot enrage", NextAction::array(0, new NextAction("tranquilizing shot", 61.0f), NULL)));
    triggers.push_back(new TriggerNode("tranquilizing shot magic", NextAction::array(0, new NextAction("tranquilizing shot", 61.0f), NULL)));

    // Ranged-based Triggers
    triggers.push_back(new TriggerNode("enemy within melee", NextAction::array(0,
                                                         new NextAction("explosive trap", 37.0f),
                                                         new NextAction("mongoose bite", 22.0f),
                                                         new NextAction("wing clip", 21.0f), nullptr)));

    triggers.push_back(new TriggerNode("enemy too close for auto shot", NextAction::array(0,
                                                                    new NextAction("disengage", 35.0f),
                                                                    new NextAction("flee", 34.0f), nullptr)));
}

// ===== AoE Strategy, 2/3+ enemies =====
AoEHunterStrategy::AoEHunterStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

void AoEHunterStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("volley channel check", NextAction::array(0, new NextAction("cancel channel", 23.0f), nullptr)));
    triggers.push_back(new TriggerNode("medium aoe", NextAction::array(0, new NextAction("volley", 22.0f), nullptr)));
    triggers.push_back(new TriggerNode("light aoe", NextAction::array(0, new NextAction("multi-shot", 21.0f), nullptr)));
}

void HunterBoostStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
}

void HunterCcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("scare beast", NextAction::array(0, new NextAction("scare beast on cc", 23.0f), nullptr)));
    triggers.push_back(new TriggerNode("freezing trap", NextAction::array(0, new NextAction("freezing trap on cc", 23.0f), nullptr)));
}

void HunterTrapWeaveStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("immolation trap no cd", NextAction::array(0, new NextAction("reach melee", 23.0f), nullptr)));
}
