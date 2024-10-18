/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PLAYERbotAICONFIG_H
#define _PLAYERBOT_PLAYERbotAICONFIG_H

#include <mutex>

#include "Common.h"
#include "DBCEnums.h"
#include "SharedDefines.h"
#include "Talentspec.h"

enum class BotCheatMask : uint32
{
    none = 0,
    taxi = 1,
    gold = 2,
    health = 4,
    mana = 8,
    power = 16,
    maxMask = 32
};

enum class HealingManaEfficiency : uint8
{
    VERY_LOW = 1,
    LOW = 2,
    MEDIUM = 4,
    HIGH = 8,
    VERY_HIGH = 16,
    SUPERIOR = 32
};

#define MAX_SPECNO 20

class PlayerbotAIConfig
{
public:
    PlayerbotAIConfig(){};
    static PlayerbotAIConfig* instance()
    {
        static PlayerbotAIConfig instance;
        return &instance;
    }

    bool Initialize();
    bool IsInRandomAccountList(uint32 id);
    bool IsInRandomQuestItemList(uint32 id);
    bool IsPvpProhibited(uint32 zoneId, uint32 areaId);
    bool IsInPvpProhibitedZone(uint32 id);
    bool IsInPvpProhibitedArea(uint32 id);

    bool enabled;
    bool allowGuildBots, allowPlayerBots;
    bool randomBotGuildNearby, randomBotInvitePlayer, inviteChat;
    uint32 globalCoolDown, reactDelay, maxWaitForMove, disableMoveSplinePath, maxMovementSearchTime, expireActionTime,
        dispelAuraDuration, passiveDelay, repeatDelay, errorDelay, rpgDelay, sitDelay, returnDelay, lootDelay;
    bool dynamicReactDelay;
    float sightDistance, spellDistance, reactDistance, grindDistance, lootDistance, shootDistance, fleeDistance,
        tooCloseDistance, meleeDistance, followDistance, whisperDistance, contactDistance, aoeRadius, rpgDistance,
        targetPosRecalcDistance, farDistance, healDistance, aggroDistance;
    uint32 criticalHealth, lowHealth, mediumHealth, almostFullHealth;
    uint32 lowMana, mediumMana, highMana;
    bool autoSaveMana;
    uint32 saveManaThreshold;
    bool autoAvoidAoe;
    float maxAoeAvoidRadius;
    std::set<uint32> aoeAvoidSpellWhitelist;
    bool tellWhenAvoidAoe;

    uint32 openGoSpell;
    bool randomBotAutologin;
    bool botAutologin;
    std::string randomBotMapsAsString;
    float probTeleToBankers;
    std::vector<uint32> randomBotMaps;
    std::vector<uint32> randomBotQuestItems;
    std::vector<uint32> randomBotAccounts;
    std::vector<uint32> randomBotSpellIds;
    std::vector<uint32> randomBotQuestIds;
    uint32 randomBotTeleportDistance;
    float randomGearLoweringChance;
    int32 randomGearQualityLimit;
    int32 randomGearScoreLimit;
    float randomBotMaxLevelChance;
    float randomBotRpgChance;
    uint32 minRandomBots, maxRandomBots;
    uint32 randomBotUpdateInterval, randomBotCountChangeMinInterval, randomBotCountChangeMaxInterval;
    uint32 minRandomBotInWorldTime, maxRandomBotInWorldTime;
    uint32 minRandomBotRandomizeTime, maxRandomBotRandomizeTime;
    uint32 minRandomBotChangeStrategyTime, maxRandomBotChangeStrategyTime;
    uint32 minRandomBotReviveTime, maxRandomBotReviveTime;
    uint32 minRandomBotTeleportInterval, maxRandomBotTeleportInterval;
    uint32 randomBotInWorldWithRotationDisabled;
    uint32 minRandomBotPvpTime, maxRandomBotPvpTime;
    uint32 randomBotsPerInterval;
    uint32 minRandomBotsPriceChangeInterval, maxRandomBotsPriceChangeInterval;
    bool randomBotJoinLfg;

    // chat
    bool randomBotTalk;
    bool randomBotEmote;
    bool randomBotSuggestDungeons;
    bool enableBroadcasts;
    bool enableGreet;
    bool randomBotSayWithoutMaster;

    uint32 broadcastChanceMaxValue;

    uint32 broadcastToGuildGlobalChance;
    uint32 broadcastToWorldGlobalChance;
    uint32 broadcastToGeneralGlobalChance;
    uint32 broadcastToTradeGlobalChance;
    uint32 broadcastToLFGGlobalChance;
    uint32 broadcastToLocalDefenseGlobalChance;
    uint32 broadcastToWorldDefenseGlobalChance;
    uint32 broadcastToGuildRecruitmentGlobalChance;

    uint32 broadcastChanceLootingItemPoor;
    uint32 broadcastChanceLootingItemNormal;
    uint32 broadcastChanceLootingItemUncommon;
    uint32 broadcastChanceLootingItemRare;
    uint32 broadcastChanceLootingItemEpic;
    uint32 broadcastChanceLootingItemLegendary;
    uint32 broadcastChanceLootingItemArtifact;

    uint32 broadcastChanceQuestAccepted;
    uint32 broadcastChanceQuestUpdateObjectiveCompleted;
    uint32 broadcastChanceQuestUpdateObjectiveProgress;
    uint32 broadcastChanceQuestUpdateFailedTimer;
    uint32 broadcastChanceQuestUpdateComplete;
    uint32 broadcastChanceQuestTurnedIn;

    uint32 broadcastChanceKillNormal;
    uint32 broadcastChanceKillElite;
    uint32 broadcastChanceKillRareelite;
    uint32 broadcastChanceKillWorldboss;
    uint32 broadcastChanceKillRare;
    uint32 broadcastChanceKillUnknown;
    uint32 broadcastChanceKillPet;
    uint32 broadcastChanceKillPlayer;

    uint32 broadcastChanceLevelupGeneric;
    uint32 broadcastChanceLevelupTenX;
    uint32 broadcastChanceLevelupMaxLevel;

    uint32 broadcastChanceSuggestInstance;
    uint32 broadcastChanceSuggestQuest;
    uint32 broadcastChanceSuggestGrindMaterials;
    uint32 broadcastChanceSuggestGrindReputation;
    uint32 broadcastChanceSuggestSell;
    uint32 broadcastChanceSuggestSomething;

    uint32 broadcastChanceSuggestSomethingToxic;

    uint32 broadcastChanceSuggestToxicLinks;
    std::string toxicLinksPrefix;
    uint32 toxicLinksRepliesChance;

    uint32 broadcastChanceSuggestThunderfury;
    uint32 thunderfuryRepliesChance;

    uint32 broadcastChanceGuildManagement;

    uint32 guildRepliesRate;

    bool suggestDungeonsInLowerCaseRandomly;

    // --

    bool randomBotJoinBG;
    bool randomBotAutoJoinBG;
    uint32 randomBotAutoJoinWarsongBracket;
    uint32 randomBotAutoJoinArenaBracket;
    uint32 randomBotAutoJoinBGWarsongCount;
    uint32 randomBotAutoJoinBGRatedArena2v2Count;
    uint32 randomBotAutoJoinBGRatedArena3v3Count;
    uint32 randomBotAutoJoinBGRatedArena5v5Count;
    bool randomBotLoginAtStartup;
    uint32 randomBotTeleLowerLevel, randomBotTeleHigherLevel;
    bool logInGroupOnly, logValuesPerTick;
    bool fleeingEnabled;
    bool summonAtInnkeepersEnabled;
    std::string combatStrategies, nonCombatStrategies;
    std::string randomBotCombatStrategies, randomBotNonCombatStrategies;
    bool applyInstanceStrategies;
    uint32 randomBotMinLevel, randomBotMaxLevel;
    float randomChangeMultiplier;

    // std::string premadeLevelSpec[MAX_CLASSES][10][91]; //lvl 10 - 100
    // ClassSpecs classSpecs[MAX_CLASSES];

    std::string premadeSpecName[MAX_CLASSES][MAX_SPECNO];
    std::string premadeSpecGlyph[MAX_CLASSES][MAX_SPECNO];
    std::vector<uint32> parsedSpecGlyph[MAX_CLASSES][MAX_SPECNO];
    std::string premadeSpecLink[MAX_CLASSES][MAX_SPECNO][MAX_LEVEL];
    std::string premadeHunterPetLink[3][21];
    std::vector<std::vector<uint32>> parsedSpecLinkOrder[MAX_CLASSES][MAX_SPECNO][MAX_LEVEL];
    std::vector<std::vector<uint32>> parsedHunterPetLinkOrder[3][21];
    uint32 randomClassSpecProb[MAX_CLASSES][MAX_SPECNO];
    uint32 randomClassSpecIndex[MAX_CLASSES][MAX_SPECNO];

    std::string commandPrefix, commandSeparator;
    std::string randomBotAccountPrefix;
    uint32 randomBotAccountCount;
    bool randomBotRandomPassword;
    bool deleteRandomBotAccounts;
    uint32 randomBotGuildCount;
    bool deleteRandomBotGuilds;
    std::vector<uint32> randomBotGuilds;
    std::vector<uint32> pvpProhibitedZoneIds;
    std::vector<uint32> pvpProhibitedAreaIds;
    bool fastReactInBG;

    bool randombotsWalkingRPG;
    bool randombotsWalkingRPGInDoors;
    uint32 minEnchantingBotLevel;
    uint32 limitEnchantExpansion;
    uint32 limitGearExpansion;
    uint32 randombotStartingLevel;
    bool enableRotation;
    uint32 rotationPoolSize;
    bool gearscorecheck;
    bool randomBotPreQuests;

    bool guildTaskEnabled;
    uint32 minGuildTaskChangeTime, maxGuildTaskChangeTime;
    uint32 minGuildTaskAdvertisementTime, maxGuildTaskAdvertisementTime;
    uint32 minGuildTaskRewardTime, maxGuildTaskRewardTime;
    uint32 guildTaskAdvertCleanupTime;
    uint32 guildTaskKillTaskDistance;

    uint32 iterationsPerTick;

    std::mutex m_logMtx;
    std::vector<std::string> allowedLogFiles;
    std::unordered_map<std::string, std::pair<FILE*, bool>> logFiles;

    std::vector<std::string> botCheats;
    uint32 botCheatMask = 0;

    struct worldBuff
    {
        uint32 spellId;
        uint32 factionId = 0;
        uint32 classId = 0;
        uint32 minLevel = 0;
        uint32 maxLevel = 0;
    };

    std::vector<worldBuff> worldBuffs;

    uint32 commandServerPort;
    bool perfMonEnabled;
    bool summonWhenGroup;
    bool randomBotShowHelmet;
    bool randomBotShowCloak;
    bool randomBotFixedLevel;
    bool disableRandomLevels;
    uint32 playerbotsXPrate;
    bool disableDeathKnightLogin;
    uint32 botActiveAlone;
    bool botActiveAloneSmartScale;
    uint32 botActiveAloneSmartScaleWhenMinLevel;
    uint32 botActiveAloneSmartScaleWhenMaxLevel;
    uint32 botActiveAloneSmartScaleDiffWithPlayer;
    uint32 botActiveAloneSmartScaleDiffEmpty;

    bool freeMethodLoot;
    int32 lootRollLevel;
    std::string autoPickReward;
    bool autoEquipUpgradeLoot;
    float equipUpgradeThreshold;
    bool twoRoundsGearInit;
    bool syncQuestWithPlayer;
    bool syncQuestForPlayer;
    std::string autoTrainSpells;
    bool autoPickTalents;
    bool autoUpgradeEquip;
    bool autoLearnTrainerSpells;
    bool autoDoQuests;
    bool syncLevelWithPlayers;
    bool freeFood;
    bool autoLearnQuestSpells;
    bool autoTeleportForLevel;
    bool randomBotGroupNearby;
    uint32 tweakValue;  // Debugging config

    uint32 randomBotArenaTeamCount;
    uint32 randomBotArenaTeamMaxRating;
    uint32 randomBotArenaTeamMinRating;
    uint32 randomBotArenaTeam2v2Count;
    uint32 randomBotArenaTeam3v3Count;
    uint32 randomBotArenaTeam5v5Count;
    bool deleteRandomBotArenaTeams;
    std::vector<uint32> randomBotArenaTeams;

    uint32 selfBotLevel;
    bool downgradeMaxLevelBot;
    bool equipmentPersistence;
    int32 equipmentPersistenceLevel;
    int32 groupInvitationPermission;
    bool allowSummonInCombat;
    bool allowSummonWhenMasterIsDead;
    bool allowSummonWhenBotIsDead;
    int reviveBotWhenSummoned;
    bool botRepairWhenSummon;
    bool autoInitOnly;
    float autoInitEquipLevelLimitRatio;
    int32 maxAddedBots, maxAddedBotsPerClass;
    int32 addClassCommand;
    int32 addClassAccountPoolSize;
    int32 maintenanceCommand;
    int32 autoGearCommand, autoGearQualityLimit, autoGearScoreLimit;

    std::string const GetTimestampStr();
    bool hasLog(std::string const fileName)
    {
        return std::find(allowedLogFiles.begin(), allowedLogFiles.end(), fileName) != allowedLogFiles.end();
    };
    bool openLog(std::string const fileName, char const* mode = "a");
    bool isLogOpen(std::string const fileName)
    {
        auto it = logFiles.find(fileName);
        return it != logFiles.end() && it->second.second;
    }
    void log(std::string const fileName, const char* str, ...);

    void loadWorldBuf(uint32 factionId, uint32 classId, uint32 minLevel, uint32 maxLevel);
    static std::vector<std::vector<uint32>> ParseTempTalentsOrder(uint32 cls, std::string temp_talents_order);
    static std::vector<std::vector<uint32>> ParseTempPetTalentsOrder(uint32 spec, std::string temp_talents_order);
};

#define sPlayerbotAIConfig PlayerbotAIConfig::instance()

#endif
