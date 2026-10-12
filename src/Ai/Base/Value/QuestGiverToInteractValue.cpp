/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "QuestGiverToInteractValue.h"
#include "NewRpgBaseAction.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

ObjectGuid QuestGiverToInteractValue::Calculate()
{
    return NewRpgBaseAction::FindNpcOrGameObjectToInteract(botAI, bot, true, QUEST_GIVER_SEARCH_DISTANCE);
}
