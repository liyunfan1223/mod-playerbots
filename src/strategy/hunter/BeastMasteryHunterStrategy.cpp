/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "BeastMasteryHunterStrategy.h"
#include "Playerbots.h"

// ===== Action Node Factory =====
class BeastMasteryHunterStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    BeastMasteryHunterStrategyActionNodeFactory()
    {
        creators["auto shot"] = &auto_shot;
        creators["kill command"] = &kill_command;
        creators["kill shot"] = &kill_shot;
        creators["viper sting"] = &viper_sting;
        creators["serpent sting"] = serpent_sting;
        creators["aimed shot"] = &aimed_shot;
        creators["arcane shot"] = &arcane_shot;
        creators["steady shot"] = &steady_shot;
        creators["multi-shot"] = &multi_shot;
        creators["volley"] = &volley;
    }

private:
    static ActionNode* auto_shot(PlayerbotAI*) { return new ActionNode("auto shot", nullptr, nullptr, nullptr); }
    static ActionNode* kill_command(PlayerbotAI*) { return new ActionNode("kill command", nullptr, nullptr, nullptr); }
    static ActionNode* kill_shot(PlayerbotAI*) { return new ActionNode("kill shot", nullptr, nullptr, nullptr); }
    static ActionNode* viper_sting(PlayerbotAI*) { return new ActionNode("viper sting", nullptr, nullptr, nullptr); }
    static ActionNode* serpent_sting(PlayerbotAI*) { return new ActionNode("serpent sting", nullptr, nullptr, nullptr); }
    static ActionNode* aimed_shot(PlayerbotAI*) { return new ActionNode("aimed shot", nullptr, nullptr, nullptr); }
    static ActionNode* arcane_shot(PlayerbotAI*) { return new ActionNode("arcane shot", nullptr, nullptr, nullptr); }
    static ActionNode* steady_shot(PlayerbotAI*) { return new ActionNode("steady shot", nullptr, nullptr, nullptr); }
    static ActionNode* multi_shot(PlayerbotAI*) { return new ActionNode("multi shot", nullptr, nullptr, nullptr); }
    static ActionNode* volley(PlayerbotAI*) { return new ActionNode("volley", nullptr, nullptr, nullptr); }
};

// ===== Single Target Strategy =====
BeastMasteryHunterStrategy::BeastMasteryHunterStrategy(PlayerbotAI* botAI) : GenericHunterStrategy(botAI)
{
    actionNodeFactories.Add(new BeastMasteryHunterStrategyActionNodeFactory());
}

// ===== Default Actions =====
NextAction** BeastMasteryHunterStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("bestial wrath", 19.0f),
                                new NextAction("kill command", 5.7f),
                                new NextAction("kill shot", 5.6f),
                                new NextAction("serpent sting", 5.5f),
                                new NextAction("aimed shot", 5.4f),
                                new NextAction("arcane shot", 5.3f),
                                new NextAction("steady shot", 5.2f),
                                new NextAction("auto shot", 5.1f), nullptr);
}

// ===== Trigger Initialization ===
void BeastMasteryHunterStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericHunterStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("intimidation", NextAction::array(0, new NextAction("intimidation", 40.0f), nullptr)));
    triggers.push_back(new TriggerNode("kill command", NextAction::array(0, new NextAction("kill command", 18.5f), nullptr)));
    triggers.push_back(new TriggerNode("target critical health", NextAction::array(0, new NextAction("kill shot", 18.0f), nullptr)));
    triggers.push_back(new TriggerNode("low mana", NextAction::array(0, new NextAction("viper sting", 17.5f), nullptr)));
    triggers.push_back(new TriggerNode("no stings", NextAction::array(0, new NextAction("serpent sting", 17.0f), nullptr)));
    triggers.push_back(new TriggerNode("serpent sting on attacker", NextAction::array(0, new NextAction("serpent sting on attacker", 16.5f), nullptr)));
}
