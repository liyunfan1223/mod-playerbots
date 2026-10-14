/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "OCStrategy.h"
#include "OCMultipliers.h"

void WotlkDungeonOccStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Drakos the Interrogator
    // TODO: May need work, TBA.
    triggers.push_back(new TriggerNode("unstable sphere",
        { NextAction("avoid unstable sphere", ACTION_MOVE + 5) }));

    // DRAKES
    triggers.push_back(new TriggerNode("drake mount",
        { NextAction("mount drake", ACTION_RAID + 5) }));
    triggers.push_back(new TriggerNode("drake dismount",
        { NextAction("dismount drake", ACTION_RAID + 5) }));
    triggers.push_back(new TriggerNode("group flying",
        { NextAction("occ fly drake", ACTION_NORMAL + 1) }));
    triggers.push_back(new TriggerNode("drake combat",
        { NextAction("occ drake attack", ACTION_NORMAL + 5) }));

    // Varos Cloudstrider
    // Seems to be no way to identify the marked cores, may need to hook boss AI..
    // triggers.push_back(new TriggerNode("varos cloudstrider",
    //     { NextAction("avoid energize cores", ACTION_RAID + 5) }));

    // Mage-Lord Urom
    triggers.push_back(new TriggerNode("arcane explosion",
        { NextAction("avoid arcane explosion", ACTION_MOVE + 5) }));
    triggers.push_back(new TriggerNode("time bomb",
        { NextAction("time bomb spread", ACTION_MOVE + 4) }));

    // Ley-Guardian Eregos
}

void WotlkDungeonOccStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new MountingDrakeMultiplier(botAI));
    multipliers.push_back(new OccFlyingMultiplier(botAI));
    multipliers.push_back(new UromMultiplier(botAI));
    multipliers.push_back(new EregosMultiplier(botAI));
}
