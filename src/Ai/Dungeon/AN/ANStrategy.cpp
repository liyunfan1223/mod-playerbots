/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ANStrategy.h"
#include "ANMultipliers.h"

void WotlkDungeonANStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Krik'thir the Gatewatcher
    // TODO: Add CC trigger while web wraps are casting?
    // TODO: Bring healer closer than ranged dps to avoid fixates?
    triggers.push_back(new TriggerNode("krik'thir web wrap",
        { NextAction("attack web wrap", ACTION_RAID + 5) }));
    triggers.push_back(new TriggerNode("krik'thir watchers",
        { NextAction("krik'thir priority", ACTION_RAID + 4) }));

    // Hadronox
    // The core AC triggers are very buggy with this boss, but default strat seems to play correctly

    //Anub'arak
    // TODO: No clear way to track these spikes. They don't seem to appear as gameobjects or triggers,
    // and cast time is instant so no way to check currently casting location.
    // May need to hook boss AI.. might be able to just heal through it for now.
    // triggers.push_back(new TriggerNode("anub'arak impale",
    //     { NextAction("TODO", ACTION_MOVE + 5) }));
    triggers.push_back(new TriggerNode("anub'arak pound",
        { NextAction("dodge pound", ACTION_MOVE + 5) }));
}

void WotlkDungeonANStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new KrikthirMultiplier(botAI));
}
