/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RandomBotUpdateAction.h"
#include "Event.h"
#include "Playerbots.h"

bool RandomBotUpdateAction::Execute(Event /*event*/)
{
    if (!sRandomPlayerbotMgr.IsRandomBot(bot))
        return false;

    if (bot->GetGroup() && botAI->GetGroupLeader())
    {
        PlayerbotAI* groupLeaderBotAI = GET_PLAYERBOT_AI(botAI->GetGroupLeader());
        if (!groupLeaderBotAI || IsSelfBot(botAI->GetGroupLeader()))
            return true;
    }

    if (botAI->HasPlayerNearby(sPlayerbotAIConfig.GrindDistance))
        return true;

    return sRandomPlayerbotMgr.ProcessBot(bot);
}

bool RandomBotUpdateAction::isUseful() { return AI_VALUE(bool, "random bot update"); }
