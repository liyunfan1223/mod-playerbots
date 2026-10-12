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
    if (!sPlayerbotAIConfig.PrioritiseQuestGivers || !action || action->getName() != "attack anything")
        return 1.0f;

    switch (botAI->rpgInfo.GetStatus())
    {
        case RPG_GO_GRIND:
        case RPG_GO_CAMP:
        case RPG_WANDER_RANDOM:
        case RPG_WANDER_NPC:
        case RPG_DO_QUEST:
            break;
        default:
            return 1.0f;  // this status's action never visits a giver
    }

    return AI_VALUE(ObjectGuid, "quest giver to interact").IsEmpty() ? 1.0f : 0.0f;
}
