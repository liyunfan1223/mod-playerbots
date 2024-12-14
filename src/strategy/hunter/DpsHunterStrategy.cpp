/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "DpsHunterStrategy.h"

#include "Playerbots.h"

class DpsHunterStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    DpsHunterStrategyActionNodeFactory()
    {
        creators["aimed shot"] = &aimed_shot; 
        creators["steady shot"] = &steady_shot;
    }

private:
    static ActionNode* aimed_shot([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("aimed shot",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("multi-shot"), nullptr),
                              /*C*/ nullptr);
    }
    static ActionNode* steady_shot([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("steady shot",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("arcane shot"), nullptr),
                              /*C*/ nullptr);
    }
};

DpsHunterStrategy::DpsHunterStrategy(PlayerbotAI* botAI) : GenericHunterStrategy(botAI)
{
    actionNodeFactories.Add(new DpsHunterStrategyActionNodeFactory());
}

NextAction** DpsHunterStrategy::getDefaultActions()
{
    return NextAction::array(
        0, new NextAction("kill shot", ACTION_DEFAULT + 0.8f), new NextAction("chimera shot", ACTION_DEFAULT + 0.7f),
        new NextAction("explosive shot", ACTION_DEFAULT + 0.6f), new NextAction("aimed shot", ACTION_DEFAULT + 0.5f),
        new NextAction("silencing shot", ACTION_DEFAULT + 0.4f),
        new NextAction("kill command", ACTION_DEFAULT + 0.3f),
        // new NextAction("arcane shot", ACTION_DEFAULT + 0.2f),
        new NextAction("steady shot", ACTION_DEFAULT + 0.1f),
        new NextAction("auto shot", ACTION_DEFAULT), nullptr);
}

void DpsHunterStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericHunterStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode("black arrow", NextAction::array(0, new NextAction("black arrow", 15.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("low mana", NextAction::array(0, new NextAction("viper sting", 23.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("hunter's mark", NextAction::array(0, new NextAction("hunter's mark", 31.0f), nullptr)));
    triggers.push_back(new TriggerNode("concussive shot on snare target",
                                       NextAction::array(0, new NextAction("concussive shot", 20.0f), nullptr)));
    // triggers.push_back(new TriggerNode("no pet", NextAction::array(0, new NextAction("call pet", 21.0f), NULL)));
    // triggers.push_back(new TriggerNode("hunters pet low health", NextAction::array(0, new NextAction("mend
    // pet", 21.0f), NULL)));
    triggers.push_back(
        new TriggerNode("has aggro", NextAction::array(0, new NextAction("concussive shot", 20.0f), nullptr)));
}

void DpsAoeHunterStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("light aoe", NextAction::array(0, new NextAction("multi-shot", 20.0f), nullptr)));
    triggers.push_back(new TriggerNode("medium aoe", NextAction::array(0, new NextAction("volley", 21.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("serpent sting on attacker",
                        NextAction::array(0, new NextAction("serpent sting on attacker", 17.0f), nullptr)));
}

void DpsHunterDebuffStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("no stings", NextAction::array(0, new NextAction("serpent sting", 18.0f), nullptr)));
}
