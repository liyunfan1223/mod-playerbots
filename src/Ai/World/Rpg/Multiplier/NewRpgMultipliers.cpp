/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "NewRpgMultipliers.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

float NewRpgQuestGiverMultiplier::GetValue(Action* action)
{
    if (!sPlayerbotAIConfig.PrioritiseQuestGivers || !action)
        return 1.0f;

    // Hold back grinding and loot pickup/chase so a giver with a real accept/reward is not starved;
    // the giver search already counts only accept/reward opportunities.
    std::string const name = action->getName();
    if (name != "attack anything" && name != "move to loot" && name != "loot" && name != "open loot")
        return 1.0f;

    return AI_VALUE(bool, "quest giver nearby") ? 0.0f : 1.0f;
}
