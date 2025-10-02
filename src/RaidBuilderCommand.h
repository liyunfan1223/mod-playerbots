#ifndef _PLAYERBOT_RAIDBUILDERCOMMAND_H
#define _PLAYERBOT_RAIDBUILDERCOMMAND_H

#include "PlayerbotMgr.h"
#include "PlayerbotAIConfig.h"
#include "RandomPlayerbotMgr.h"
#include "RandomPlayerbotFactory.h"
#include "Chat.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "SharedDefines.h"
#include "WorldSessionMgr.h"
#include <sstream>
#include <vector>
#include <map>
#include <cmath>

class RaidComposition
{
public:
    struct RoleComposition
    {
        int tanks;
        int healers;
        int dps;
        std::vector<std::pair<std::string, std::string>> classSpecs;
    };

    static constexpr double TANK_RATIO = 0.08;
    static constexpr double HEALER_RATIO = 0.20;
    static constexpr int MIN_TANKS = 1;
    static constexpr int MIN_HEALERS = 1;
    static constexpr int MAX_TANKS = 4;
    static constexpr int MAX_HEALERS = 8;
    static constexpr int MIN_RAID_SIZE = 5;
    static constexpr int MAX_RAID_SIZE = 40;
    static constexpr int GROUP_TO_RAID_THRESHOLD = 5;
    static constexpr uint8 RACE_HUMAN = 1;
    static constexpr uint8 RACE_ORC = 2;
    static constexpr uint8 RACE_DWARF = 3;
    static constexpr uint8 RACE_NIGHT_ELF = 4;
    static constexpr uint8 RACE_UNDEAD = 5;
    static constexpr uint8 RACE_TAUREN = 6;
    static constexpr uint8 RACE_GNOME = 7;
    static constexpr uint8 RACE_TROLL = 8;
    static constexpr uint8 RACE_BLOOD_ELF = 10;
    static constexpr uint8 RACE_DRAENEI = 11;
    static constexpr uint8 SPEC_ARMS = 0;
    static constexpr uint8 SPEC_FURY = 1;
    static constexpr uint8 SPEC_PROTECTION = 2;
    static constexpr uint8 SPEC_HOLY = 1;
    static constexpr uint8 SPEC_RETRIBUTION = 2;
    static constexpr uint8 SPEC_SHADOW = 0;
    static constexpr uint8 SPEC_BLOOD = 0;
    static constexpr uint8 SPEC_FROST = 1;
    static constexpr uint8 SPEC_UNHOLY = 2;
    static constexpr uint8 SPEC_RESTORATION = 2;
    static constexpr uint8 SPEC_BALANCE = 0;
    static constexpr uint8 SPEC_FERAL = 1;
    static constexpr uint8 RAID_ICON_STAR = 0;
    static constexpr uint8 RAID_ICON_CIRCLE = 1;
    static constexpr uint8 RAID_ICON_DIAMOND = 2;
    static constexpr uint8 RAID_ICON_TRIANGLE = 3;
    static constexpr uint8 RAID_ICON_MOON = 4;
    static constexpr uint8 RAID_ICON_SQUARE = 5;
    static constexpr uint8 RAID_ICON_CROSS = 6;

    static RoleComposition GetOptimalComposition(int raidSize);
};

class RaidBuilderCommand
{
public:
    static std::vector<std::string> HandleRaidBuilderCommand(ChatHandler* handler, char const* args);

private:
    static Player* CreateRaidBot(Player* master, const std::string& className, const std::string& botName, const std::string& gender, const std::string& role);
    static uint8 GetClassId(const std::string& className);
    static std::string CreateBotWithAddClass(Player* master, const std::string& botName, const std::string& className, const std::string& gender);
    static void ConfigureBotRole(Player* master, const std::string& botName, const std::string& role);
    static void AddBotToRaid(Player* master, const std::string& botName, const std::string& role);
    static Player* FindBotByName(const std::string& botName);
    static uint8 GetClassIdFromName(const std::string& className);
    static std::string GetClassNameFromId(uint8 classId);
    static std::string GetPlayerRole(Player* player);
    static bool AddBotToGroup(Player* master, Player* bot);
    static void AssignTankMarkers(Group* group, const std::vector<Player*>& tanks);
};

#endif