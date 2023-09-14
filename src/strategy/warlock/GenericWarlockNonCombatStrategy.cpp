/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GenericWarlockNonCombatStrategy.h"
#include "Playerbots.h"

class GenericWarlockNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
    public:
        GenericWarlockNonCombatStrategyActionNodeFactory()
        {
            creators["fel armor"] = &fel_armor;
            creators["demon armor"] = &demon_armor;
            creators["summon voidwalker"] = &summon_voidwalker;
            creators["summon felguard"] = &summon_felguard;
            creators["summon succubus"] = &summon_succubus;
            creators["summon felhunter"] = &summon_felhunter;
        }

    private:
        static ActionNode* fel_armor([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("fel armor",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("demon armor"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* demon_armor([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("demon armor",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("demon skin"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* summon_voidwalker([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode("summon voidwalker",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("summon imp"), nullptr),
                /*C*/ nullptr);
        }
        static ActionNode* summon_succubus(PlayerbotAI* botAI)
        {
            return new ActionNode("summon succubus",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("summon imp"), nullptr),
                /*C*/ nullptr);
        }
        static ActionNode* summon_felhunter([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode("summon felhunter",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("summon voidwalker"), nullptr),
                /*C*/ nullptr);
        }
        static ActionNode* summon_felguard([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode("summon felguard",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("summon succubus"), nullptr),
                /*C*/ nullptr);
        }
};

GenericWarlockNonCombatStrategy::GenericWarlockNonCombatStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericWarlockNonCombatStrategyActionNodeFactory());
}

void GenericWarlockNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("demon armor", NextAction::array(0, new NextAction("fel armor", 21.0f), nullptr)));
    triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("apply oil", 1.0f), nullptr)));
    triggers.push_back(new TriggerNode("has pet", NextAction::array(0, new NextAction("toggle pet spell", 60.0f), nullptr)));
}

void WarlockPetStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("no pet", NextAction::array(0, new NextAction("summon felguard", 60.0f), nullptr)));
    // TODO Warlock pets
}

SummonImpStrategy::SummonImpStrategy(PlayerbotAI* ai): NonCombatStrategy(ai) {}

void SummonImpStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "no pet",
        NextAction::array(0, new NextAction("summon imp", 11.0f), NULL)));
}

SummonFelguardStrategy::SummonFelguardStrategy(PlayerbotAI* ai): NonCombatStrategy(ai) {}

void SummonFelguardStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "no pet",
        NextAction::array(0, new NextAction("summon felguard", 11.0f), NULL)));
}

SummonFelhunterStrategy::SummonFelhunterStrategy(PlayerbotAI* ai): NonCombatStrategy(ai) {}

void SummonFelhunterStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "no pet",
        NextAction::array(0, new NextAction("summon felhunter", 11.0f), NULL)));
}