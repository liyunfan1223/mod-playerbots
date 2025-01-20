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
    enum class NameRaceAndGender : uint8
    {
        // Generic is the category used for human & undead
        GenericMale = 0,
        GenericFemale,
        GnomeMale,
        GnomeFemale,
        DwarfMale,
        DwarfFemale,
        NightelfMale,
        NightelfFemale,
        DraeneiMale,
        DraeneiFemale,
        OrcMale,
        OrcFemale,
        TrollMale,
        TrollFemale,
        TaurenMale,
        TaurenFemale,
        BloodelfMale,
        BloodelfFemale
    };

    static constexpr NameRaceAndGender CombineRaceAndGender(uint8 gender, uint8 race);

    RandomPlayerbotFactory(uint32 accountId);
    virtual ~RandomPlayerbotFactory() {}

    Player* CreateRandomBot(WorldSession* session, uint8 cls, std::unordered_map<NameRaceAndGender, std::vector<std::string>>& nameCache);
    static void CreateRandomBots();
    static void DeleteRandomBotAccounts();
    static void CreateRandomGuilds();
    static void CreateRandomArenaTeams(ArenaType type, uint32 count);
    static std::string const CreateRandomGuildName();

private:
    std::string const CreateRandomBotName(NameRaceAndGender raceAndGender);
    static std::string const CreateRandomArenaTeamName();
    static void CreateNameCache(std::unordered_map<NameRaceAndGender, std::vector<std::string>>& nameCache);
    static uint32 GetNextBotAccountCounter();
    static void CreateBotAccounts(uint32& botAccountCounter);
    static void CreateAccount(const std::string& accountName);
    static void CreateBotCharacters(std::unordered_map<NameRaceAndGender, std::vector<std::string>>& nameCache);
    static std::vector<uint32> GetBotAccountIds();
    static uint8 SelectRandomRace(uint8 cls, bool alliance);

    uint32 accountId;
    static std::map<uint8, std::vector<uint8>> availableRaces;
};

#endif
