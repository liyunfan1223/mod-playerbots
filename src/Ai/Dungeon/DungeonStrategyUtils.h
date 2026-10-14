/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_DUNGEONSTRATEGYUTILS_H
#define PLAYERBOTS_DUNGEONSTRATEGYUTILS_H

#include "DBCEnums.h"
#include "Player.h"

template<class T> inline
T const& DUNGEON_MODE(Player* bot, T const& normal5, T const& heroic10)
{
    switch (bot->GetMap()->GetDifficulty())
    {
        case DUNGEON_DIFFICULTY_NORMAL:
            return normal5;
        case DUNGEON_DIFFICULTY_HEROIC:
            return heroic10;
        default:
            break;
    }

    return heroic10;
}

#endif
