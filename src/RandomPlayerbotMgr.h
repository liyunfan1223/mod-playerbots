/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RANDOMPLAYERBOTMGR_H
#define _PLAYERBOT_RANDOMPLAYERBOTMGR_H

#include "PlayerbotMgr.h"

class ChatHandler;
class PerformanceMonitorOperation;
class WorldLocation;

class CachedEvent
{
    public:
        CachedEvent() : value(0), lastChangeTime(0), validIn(0), data("") { }
        CachedEvent(const CachedEvent& other) : value(other.value), lastChangeTime(other.lastChangeTime), validIn(other.validIn), data(other.data) { }
        CachedEvent(uint32 value, uint32 lastChangeTime, uint32 validIn, std::string const data = "") : value(value), lastChangeTime(lastChangeTime), validIn(validIn), data(data) { }

        bool IsEmpty() { return !lastChangeTime; }

    public:
        uint32 value;
        uint32 lastChangeTime;
        uint32 validIn;
        std::string data;
};

class RandomPlayerbotMgr : public PlayerbotHolder
{
    public:
        RandomPlayerbotMgr();
        virtual ~RandomPlayerbotMgr();
        static RandomPlayerbotMgr* instance()
        {
            static RandomPlayerbotMgr instance;
            return &instance;
        }

        void LogPlayerLocation();
        void UpdateAIInternal(uint32 elapsed, bool minimal = false) override;

	public:
        uint32 activeBots = 0;
        static bool HandlePlayerbotConsoleCommand(ChatHandler* handler, char const* args);
        bool IsRandomBot(Player* bot);
        bool IsRandomBot(ObjectGuid::LowType bot);
        void Randomize(Player* bot);
        void Clear(Player* bot);
        void RandomizeFirst(Player* bot);
        void RandomizeMin(Player* bot);
        void IncreaseLevel(Player* bot);
        void ScheduleTeleport(uint32 bot, uint32 time = 0);
        void ScheduleChangeStrategy(uint32 bot, uint32 time = 0);
        void HandleCommand(uint32 type, std::string const text, Player* fromPlayer, std::string channelName = "");
        std::string const HandleRemoteCommand(std::string const request);
        void OnPlayerLogout(Player* player);
        void OnPlayerLogin(Player* player);
        void OnPlayerLoginError(uint32 bot);
        Player* GetRandomPlayer();
        std::vector<Player*> GetPlayers() { return players; };
        PlayerBotMap GetAllBots() { return playerBots; };
        void PrintStats();
        double GetBuyMultiplier(Player* bot);
        double GetSellMultiplier(Player* bot);
        void AddTradeDiscount(Player* bot, Player* master, int32 value);
        void SetTradeDiscount(Player* bot, Player* master, uint32 value);
        uint32 GetTradeDiscount(Player* bot, Player* master);
        void Refresh(Player* bot);
        void RandomTeleportForLevel(Player* bot);
        void RandomTeleportGrindForLevel(Player* bot);
        void RandomTeleportForRpg(Player* bot);
        uint32 GetMaxAllowedBotCount();
        bool ProcessBot(Player* player);
        void Revive(Player* player);
        void ChangeStrategy(Player* player);
        void ChangeStrategyOnce(Player* player);
    uint32 GetValue(Player* bot, std::string const type);
        uint32 GetValue(uint32 bot, std::string const type);
        std::string const GetData(uint32 bot, std::string const type);
        void SetValue(uint32 bot, std::string const type, uint32 value, std::string const data = "");
        void SetValue(Player* bot, std::string const type, uint32 value, std::string const data = "");
        void Remove(Player* bot);
        ObjectGuid const GetBattleMasterGUID(Player* bot, BattlegroundTypeId bgTypeId);
        CreatureData const* GetCreatureDataByEntry(uint32 entry);
        void LoadBattleMastersCache();
        std::map<uint32, std::map<uint32, std::map<TeamId, bool>>> NeedBots;
        std::map<uint32, std::map<uint32, std::map<TeamId, uint32>>> BgBots;
        std::map<uint32, std::map<uint32, std::map<TeamId, uint32>>> VisualBots;
        std::map<uint32, std::map<uint32, std::map<TeamId, uint32>>> BgPlayers;
        std::map<uint32, std::map<uint32, std::map<TeamId, std::map<TeamId, uint32>>>> ArenaBots;
        std::map<uint32, std::map<uint32, std::map<uint32, uint32>>> Rating;
        std::map<uint32, std::map<uint32, std::map<uint32, uint32>>> Supporters;
        std::map<TeamId, std::vector<uint32>> LfgDungeons;
        void CheckBgQueue();
        void CheckLfgQueue();
        void CheckPlayers();

        std::map<TeamId, std::map<BattlegroundTypeId, std::vector<uint32>>> getBattleMastersCache() { return BattleMastersCache; }

	protected:
	    void OnBotLoginInternal(Player* const bot) override;

    private:
        uint32 GetEventValue(uint32 bot, std::string const event);
        std::string const GetEventData(uint32 bot, std::string const event);
        uint32 SetEventValue(uint32 bot, std::string const event, uint32 value, uint32 validIn, std::string const data = "");
        void GetBots();
        std::vector<uint32> GetBgBots(uint32 bracket);
        time_t BgCheckTimer;
        time_t LfgCheckTimer;
        time_t PlayersCheckTimer;
        uint32 AddRandomBots();
        bool ProcessBot(uint32 bot);
        void ScheduleRandomize(uint32 bot, uint32 time);
        void RandomTeleport(Player* bot);
        void RandomTeleport(Player* bot, std::vector<WorldLocation>& locs, bool hearth = false);
        uint32 GetZoneLevel(uint16 mapId, float teleX, float teleY, float teleZ);
        void PrepareTeleportCache();
        typedef void(RandomPlayerbotMgr::*ConsoleCommandHandler)(Player*);

        std::vector<Player*> players;
        uint32 processTicks;
        std::map<uint8, std::vector<WorldLocation>> locsPerLevelCache;
        std::map<uint8, std::vector<WorldLocation>> bankerLocsPerLevelCache;

        // std::map<uint32, std::vector<WorldLocation>> rpgLocsCache;
        std::map<uint32, std::map<uint32, std::vector<WorldLocation>>> rpgLocsCacheLevel;
        std::map<TeamId, std::map<BattlegroundTypeId, std::vector<uint32>>> BattleMastersCache;
        std::map<uint32, std::map<std::string, CachedEvent>> eventCache;
        std::list<uint32> currentBots;
        uint32 bgBotsCount;
        uint32 playersLevel;
        PerformanceMonitorOperation* totalPmo;
};

#define sRandomPlayerbotMgr RandomPlayerbotMgr::instance()

#endif
