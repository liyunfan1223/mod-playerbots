/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RANDOMPLAYERBOTFACTORY_H
#define _PLAYERBOT_RANDOMPLAYERBOTFACTORY_H

#include <map>
#include <unordered_map>
#include <vector>

#include "Common.h"
#include "DBCEnums.h"

class Player;
class WorldSession;

enum ArenaType : uint8;

class RandomPlayerbotFactory
{
public:
    RandomPlayerbotFactory(uint32 accountId);
    virtual ~RandomPlayerbotFactory() {}

    Player* CreateRandomBot(WorldSession* session, uint8 cls);
    static void CreateRandomBots();
    static void CreateRandomGuilds();
    static void CreateRandomArenaTeams(ArenaType slot, uint32 count);
    static std::string const CreateRandomGuildName();

private:
    std::string const CreateRandomBotName(uint8 gender);
    static std::string const CreateRandomArenaTeamName();

    uint32 accountId;
    static std::map<uint8, std::vector<uint8>> availableRaces;
};

#endif
