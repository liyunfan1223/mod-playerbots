/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "NewRpgMultipliers.h"
#include "Playerbots.h"

float NewRpgQuestGiverMultiplier::GetValue(Action* action)
{
    if (!action || action->getName() != "attack anything")
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

    // The giver visit runs first in the status action (3.0), below "attack anything" (4.0). Raising the status
    // actions would also put their movement above grinding, and a multiplier above 1 cannot reorder the queue:
    // the engine orders by base relevance and applies multipliers after the pop. So grinding is dropped for the
    // tick instead. SearchQuestGiverAndAcceptOrReward gives up on a giver after a minute, which empties the value.
    return AI_VALUE(ObjectGuid, "new rpg quest giver").IsEmpty() ? 1.0f : 0.0f;
}
