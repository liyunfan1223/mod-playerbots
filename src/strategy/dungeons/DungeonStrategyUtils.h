#ifndef _PLAYERBOT_DUNGEONUTILS_H
#define _PLAYERBOT_DUNGEONUTILS_H


template<class T> inline
const T& DUNGEON_MODE(Player* bot, const T& normal5, const T& heroic10)
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
