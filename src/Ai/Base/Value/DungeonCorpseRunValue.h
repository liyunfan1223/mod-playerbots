/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_DUNGEONCORPSERUNVALUE_H
#define PLAYERBOTS_DUNGEONCORPSERUNVALUE_H

#include "Value.h"

struct DungeonCorpseRunState
{
    ObjectGuid MasterGuid;
    ObjectGuid GroupGuid;
    uint32 MapId = 0;
    uint32 InstanceId = 0;
    uint32 StartedAt = 0;
    uint32 LastAttemptAt = 0;
    uint32 ResurrectedAt = 0;
    uint8 Attempts = 0;
    float EntranceX = 0.0f;
    float EntranceY = 0.0f;
    float EntranceZ = 0.0f;
    float EntranceOrientation = 0.0f;
    bool EntranceRequested = false;
};

class DungeonCorpseRunValue : public ManualSetValue<DungeonCorpseRunState>
{
public:
    DungeonCorpseRunValue(PlayerbotAI* botAI) : ManualSetValue<DungeonCorpseRunState>(botAI, {}, "dungeon corpse run")
    {
    }
};

#endif
