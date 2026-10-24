/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "EoEStrategy.h"
#include "EoEDefinitions.h"

void RaidEoEStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    EoEMalygosDefinition().AddTriggerNodes(triggers);
}

void RaidEoEStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    EoEMalygosDefinition().AddMultipliers(botAI, multipliers);
}
