/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UPStrategy.h"
#include "ChooseTargetActions.h"
#include "UPMultipliers.h"

void WotlkDungeonUPStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Svala Sorrowgrave

    // Gortok Palehoof

    // Skadi the Ruthless
    // TODO: Harpoons launchable via GameObject. For now players should do them
    triggers.push_back(new TriggerNode("freezing cloud",
        { NextAction("avoid freezing cloud", ACTION_RAID + 5) }));
    triggers.push_back(new TriggerNode("skadi whirlwind",
        { NextAction("avoid skadi whirlwind", ACTION_RAID + 4) }));

    // King Ymiron
    // May need to avoid orb.. unclear if the generic avoid AoE does this well
    triggers.push_back(new TriggerNode("ymiron bane",
        { NextAction("drop target", ACTION_RAID + 5) }));
}

void WotlkDungeonUPStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new SkadiMultiplier(botAI));
    multipliers.push_back(new YmironMultiplier(botAI));
}
