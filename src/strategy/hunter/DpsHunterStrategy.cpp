/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "DpsHunterStrategy.h"
#include "Playerbots.h"

class DpsHunterStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
    public:
        DpsHunterStrategyActionNodeFactory()
        {
            creators["aimed shot"] = &aimed_shot;
            creators["chimera shot"] = &chimera_shot;
            creators["explosive shot"] = &explosive_shot;
            creators["concussive shot"] = &concussive_shot;
            creators["viper sting"] = &viper_sting;
        }

    private:
        static ActionNode* viper_sting([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("viper sting",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("mana potion", 10.0f), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* aimed_shot([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("aimed shot",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("chimera shot", 10.0f), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* chimera_shot([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("chimera shot",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("arcane shot", 10.0f), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* explosive_shot([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("explosive shot",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("aimed shot"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* concussive_shot([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("concussive shot",
                /*P*/ nullptr,
                /*A*/ nullptr,
                /*C*/ nullptr);
        }
};

DpsHunterStrategy::DpsHunterStrategy(PlayerbotAI* botAI) : GenericHunterStrategy(botAI)
{
    actionNodeFactories.Add(new DpsHunterStrategyActionNodeFactory());
}

NextAction** DpsHunterStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("explosive shot", 11.0f), new NextAction("auto shot", 10.0f), new NextAction("auto attack", 9.0f), nullptr);
}

void DpsHunterStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericHunterStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("black arrow", NextAction::array(0, new NextAction("black arrow", 15.0f), nullptr)));
    triggers.push_back(new TriggerNode("low mana", NextAction::array(0, new NextAction("viper sting", 23), nullptr)));
    triggers.push_back(new TriggerNode("hunter's mark", NextAction::array(0, new NextAction("hunter's mark", 19.0f), nullptr)));
    triggers.push_back(new TriggerNode("concussive shot on snare target", NextAction::array(0, new NextAction("concussive shot", 20.0f), nullptr)));
    /*triggers.push_back(new TriggerNode("has aggro", NextAction::array(0, new NextAction("concussive shot", 20.0f), nullptr)));*/
}

void DpsAoeHunterStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("light aoe", NextAction::array(0, new NextAction("multi-shot", 20.0f), nullptr)));
	triggers.push_back(new TriggerNode("medium aoe", NextAction::array(0, new NextAction("volley", 20.0f), nullptr)));
	triggers.push_back(new TriggerNode("serpent sting on attacker", NextAction::array(0, new NextAction("serpent sting on attacker", 17.0f), nullptr)));
}

void DpsHunterDebuffStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("no stings", NextAction::array(0, new NextAction("serpent sting", 18.0f), nullptr)));
}
