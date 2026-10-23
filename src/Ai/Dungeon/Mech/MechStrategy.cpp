/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MechStrategy.h"
#include "MechMultipliers.h"
#include "MechTriggers.h"

void TbcDungeonMechanarStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("sepethrea kite flame", {
        NextAction("sepethrea kite flame", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("sepethrea avoid flame", {
        NextAction("sepethrea avoid flame", ACTION_EMERGENCY + 5) }));

    triggers.push_back(new TriggerNode("sepethrea trail", {
        NextAction("sepethrea avoid trail", ACTION_EMERGENCY + 4) }));

    triggers.push_back(new TriggerNode("sepethrea focus boss", {
        NextAction("sepethrea focus boss", ACTION_RAID + 2) }));
}

void TbcDungeonMechanarStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new SepethreaKiteFlameMultiplier(botAI));
    multipliers.push_back(new SepethreaFocusBossMultiplier(botAI));
}
