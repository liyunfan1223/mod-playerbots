/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "HunterBuffStrategies.h"
#include "Playerbots.h"

void HunterBuffDpsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("aspect of the hawk", NextAction::array(0, new NextAction("aspect of the dragonhawk", 90.0f), nullptr)));
	triggers.push_back(new TriggerNode("aspect of the hawk", NextAction::array(0, new NextAction("aspect of the hawk", 90.0f), nullptr)));
}

void HunterNatureResistanceStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
	triggers.push_back(new TriggerNode("aspect of the wild", NextAction::array(0, new NextAction("aspect of the wild", 90.0f), nullptr)));
}

void HunterBuffSpeedStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("aspect of the pack", NextAction::array(0, new NextAction("aspect of the pack", 10.0f), nullptr)));
}

void HunterBuffManaStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("aspect of the viper", NextAction::array(0, new NextAction("aspect of the viper", 10.0f), nullptr)));
}
