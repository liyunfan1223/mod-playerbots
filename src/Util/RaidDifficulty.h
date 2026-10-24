/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAIDDIFFICULTY_H
#define PLAYERBOTS_RAIDDIFFICULTY_H

#include "Map.h"
#include "Player.h"

// Ask the map, not the player: a group left on 25 Heroic in a raid with no heroic mode (Ulduar,
// Naxxramas, Eye of Eternity) plays 25 Normal but still reports 25 Heroic, so a 25 Normal check reads it as 10-man.
inline Difficulty GetRaidMapDifficulty(Player* bot)
{
    Map* map = bot ? bot->GetMap() : nullptr;
    return map ? map->GetDifficulty() : RAID_DIFFICULTY_10MAN_NORMAL;
}

// True on 25 Normal and 25 Heroic alike.
inline bool IsRaid25Man(Player* bot)
{
    Map* map = bot ? bot->GetMap() : nullptr;
    return map && map->Is25ManRaid();
}

#endif
