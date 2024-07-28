/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "HunterBuffStrategies.h"

#include "Playerbots.h"

class BuffHunterStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    BuffHunterStrategyActionNodeFactory() { creators["aspect of the hawk"] = &aspect_of_the_hawk; }

private:
    static ActionNode* aspect_of_the_hawk([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("aspect of the hawk",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("aspect of the monkey"), nullptr),
                              /*C*/ nullptr);
    }
};

HunterBuffDpsStrategy::HunterBuffDpsStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new BuffHunterStrategyActionNodeFactory());
}

void HunterBuffDpsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("aspect of the hawk", NextAction::array(0, new NextAction("aspect of the dragonhawk", 20.1f),
                                                                new NextAction("aspect of the hawk", 20.0f), nullptr)));
}

void HunterNatureResistanceStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("aspect of the wild",
                                       NextAction::array(0, new NextAction("aspect of the wild", 20.0f), nullptr)));
}

void HunterBuffSpeedStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("aspect of the pack",
                                       NextAction::array(0, new NextAction("aspect of the pack", 20.0f), nullptr)));
}

void HunterBuffManaStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("aspect of the viper",
                                       NextAction::array(0, new NextAction("aspect of the viper", 20.0f), nullptr)));
}
