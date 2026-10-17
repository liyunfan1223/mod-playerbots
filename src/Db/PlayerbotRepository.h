/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PLAYERBOTREPOSITORY_H
#define PLAYERBOTS_PLAYERBOTREPOSITORY_H

#include "PlayerbotAI.h"
#include "PlayerbotsDatabase.h"
#include <cstdint>
#include <string>
#include <vector>

class PlayerbotRepository
{
public:
    static PlayerbotRepository& instance()
    {
        static PlayerbotRepository instance;

        return instance;
    }

    void Save(PlayerbotAI* botAI);
    void Load(PlayerbotAI* botAI);
    void Reset(PlayerbotAI* botAI);

private:
    PlayerbotRepository() = default;
    ~PlayerbotRepository() = default;

    PlayerbotRepository(PlayerbotRepository const&) = delete;
    PlayerbotRepository& operator=(PlayerbotRepository const&) = delete;

    PlayerbotRepository(PlayerbotRepository&&) = delete;
    PlayerbotRepository& operator=(PlayerbotRepository&&) = delete;

    void SaveValue(PlayerbotsDatabaseTransaction trans, uint32_t guid, std::string const key,
                   std::string const value);
    std::string const FormatStrategies(std::string const type, std::vector<std::string> strategies);
};

#endif
