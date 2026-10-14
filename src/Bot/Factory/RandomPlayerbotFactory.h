/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RANDOMPLAYERBOTFACTORY_H
#define PLAYERBOTS_RANDOMPLAYERBOTFACTORY_H

#include "ArenaTeam.h"
#include "Battleground.h"
#include "Common.h"
#include "DBCEnums.h"
#include "SharedDefines.h"
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

class Player;
class WorldSession;

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

    static constexpr NameRaceAndGender CombineRaceAndGender(uint8 race, uint8 gender);

    RandomPlayerbotFactory() {};
    virtual ~RandomPlayerbotFactory() {}

    Player* CreateRandomBot(WorldSession* session, uint8 cls, std::unordered_map<NameRaceAndGender, std::vector<std::string>>& names);
    static void CreateRandomBots();
    static std::string const CreateRandomGuildName();
    static uint32 CalculateTotalAccountCount();
    static uint32 CalculateAvailableCharsPerAccount();
    static std::string GetLocalizedNameSelector(std::string const& column);

    // Arena team management
    static void AssignBotToArenaTeam(Player* bot);
    static void DeleteBotArenaTeams();
    static uint32 GetBotArenaTeamCount(ArenaType type);
    static void LoadArenaTeamData();

private:
    friend class ArenaTeamAssignOperation;

    static bool IsValidRaceClassCombination(uint8 race, uint8 class_, uint32 expansion);
    std::string const CreateRandomBotName(NameRaceAndGender raceAndGender);

    static bool IsEligibleForBotArenaTeam(Player* bot);
    static void AssignBotToArenaTeamInternal(Player* bot);
    static void CollectJoinableBotArenaTeams(ArenaType type, TeamId faction, std::vector<ArenaTeam*>& out);
    static void CreateBotArenaTeam(Player* bot, ArenaType type);
    static bool IsBotArenaTeam(ArenaTeam const* team);
    static std::string CreateRandomArenaTeamName();

    static inline std::vector<std::string> _availableArenaTeamNames;
    static inline std::map<ArenaType, std::vector<uint32>> _botArenaTeamRegistry;
    static inline std::map<ArenaType, uint32> _configTargets;
};

#endif
