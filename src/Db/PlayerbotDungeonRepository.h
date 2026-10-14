/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PLAYERBOTDUNGEONREPOSITORY_H
#define PLAYERBOTS_PLAYERBOTDUNGEONREPOSITORY_H

#include "DBCEnums.h"
#include <string>
#include <vector>

struct DungeonSuggestion
{
    std::string name;
    Difficulty difficulty;
    uint8 min_level;
    uint8 max_level;
    std::string abbrevation;
    std::string strategy;
};

// @TODO: Completely unused at this moment.
class PlayerbotDungeonRepository
{
public:
    static PlayerbotDungeonRepository& instance()
    {
        static PlayerbotDungeonRepository instance;

        return instance;
    }

    void LoadDungeonSuggestions();
    std::vector<DungeonSuggestion> const GetDungeonSuggestions();

private:
    PlayerbotDungeonRepository() = default;
    ~PlayerbotDungeonRepository() = default;

    PlayerbotDungeonRepository(PlayerbotDungeonRepository const&) = delete;
    PlayerbotDungeonRepository& operator=(PlayerbotDungeonRepository const&) = delete;

    PlayerbotDungeonRepository(PlayerbotDungeonRepository&&) = delete;
    PlayerbotDungeonRepository& operator=(PlayerbotDungeonRepository&&) = delete;

    std::vector<DungeonSuggestion> m_dungeonSuggestions;
};

#endif
