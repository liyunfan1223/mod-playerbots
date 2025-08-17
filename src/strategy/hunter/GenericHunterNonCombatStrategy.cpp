/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericHunterNonCombatStrategy.h"

#include "Playerbots.h"

class GenericHunterNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericHunterNonCombatStrategyActionNodeFactory()
    {
        creators["rapid fire"] = &rapid_fire;
        creators["boost"] = &rapid_fire;
        creators["aspect of the pack"] = &aspect_of_the_pack;
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
};

GenericHunterNonCombatStrategy::GenericHunterNonCombatStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericHunterNonCombatStrategyActionNodeFactory());
}

void GenericHunterNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("trueshot aura", NextAction::array(0, new NextAction("trueshot aura", 2.0f), nullptr)));
    triggers.push_back(new TriggerNode("often", NextAction::array(0,
                       new NextAction("apply stone", 1.0f),
                       new NextAction("apply oil", 1.0f),
                       nullptr)));
    triggers.push_back(new TriggerNode("low ammo", NextAction::array(0, new NextAction("say::low ammo", ACTION_NORMAL), nullptr)));
    triggers.push_back(new TriggerNode("no track", NextAction::array(0, new NextAction("track humanoids", ACTION_NORMAL), nullptr)));
    triggers.push_back(new TriggerNode("no ammo", NextAction::array(0, new NextAction("equip upgrades", ACTION_HIGH + 1), nullptr)));
    // triggers.push_back(new TriggerNode("no ammo", NextAction::array(0, new NextAction("switch to melee",
    // ACTION_NORMAL + 1), new NextAction("say::no ammo", ACTION_NORMAL), nullptr))); triggers.push_back(new
    // TriggerNode("has ammo", NextAction::array(0, new NextAction("switch to ranged", ACTION_NORMAL), nullptr)));
}

void HunterPetStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("no pet", NextAction::array(0, new NextAction("call pet", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("has pet", NextAction::array(0, new NextAction("toggle pet spell", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("new pet", NextAction::array(0, new NextAction("set pet stance", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("pet not happy", NextAction::array(0, new NextAction("feed pet", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("hunters pet medium health", NextAction::array(0, new NextAction("mend pet", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("hunters pet dead", NextAction::array(0, new NextAction("revive pet", 60.0f), nullptr)));
}
