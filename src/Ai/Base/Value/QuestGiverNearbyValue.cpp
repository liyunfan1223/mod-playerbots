/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "QuestGiverNearbyValue.h"
#include "NewRpgBaseAction.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

bool QuestGiverNearbyValue::Calculate()
{
    return !NewRpgBaseAction::FindQuestGiverToInteract(botAI, bot, true,
                                                       NewRpgBaseAction::GetQuestGiverSearchDistance(bot))
                .IsEmpty();
}
