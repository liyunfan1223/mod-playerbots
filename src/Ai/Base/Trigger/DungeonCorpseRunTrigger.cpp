/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "DungeonCorpseRunTrigger.h"

#include "DungeonCorpseRunValue.h"
#include "Playerbots.h"

bool DungeonCorpseRunTrigger::IsActive()
{
    return sPlayerbotAIConfig.DungeonCorpseRunEnabled && !bot->IsAlive() && bot->HasPlayerFlag(PLAYER_FLAGS_GHOST) &&
           bot->GetGroup() && IsRealPlayer(botAI->GetMaster()) &&
           AI_VALUE(DungeonCorpseRunState, "dungeon corpse run").MasterGuid;
}

bool DungeonCorpseRunCatchUpTrigger::IsActive()
{
    return sPlayerbotAIConfig.DungeonCorpseRunEnabled && bot->IsAlive() && bot->GetGroup() &&
           IsRealPlayer(botAI->GetMaster()) && bot->GetMap() && bot->GetMap()->IsNonRaidDungeon() &&
           AI_VALUE(DungeonCorpseRunState, "dungeon corpse run").MasterGuid;
}
