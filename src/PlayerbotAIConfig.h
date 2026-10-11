/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PLAYERBOTAICONFIG_H
#define PLAYERBOTS_PLAYERBOTAICONFIG_H

#include "DBCEnums.h"
#include "SharedDefines.h"
#include <algorithm>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

enum class BotCheatMask : uint32
{
    none = 0,
    taxi = 1,
    gold = 2,
    health = 4,
    mana = 8,
    power = 16,
    raid = 32,
    food = 64,
    maxMask = 128
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

enum class ShowHideCosmetic : uint8
{
    ALWAYS_HIDE = 0,
    ALWAYS_SHOW = 1,
    RANDOMIZE = 2
};

enum class AutoPartyBuffMode : uint8
{
    DISABLED = 0,
    RAID_ONLY = 1,
    GROUP_OR_RAID = 2
};

enum NewRpgStatus : int
{
    //Initial Status
    RPG_IDLE = 0,
    RPG_GO_GRIND = 1,
    RPG_GO_CAMP = 2,
    // Exploring nearby
    RPG_WANDER_RANDOM = 3,
    RPG_WANDER_NPC = 4,
    // Do Quest (based on quest status)
    RPG_DO_QUEST = 5,
    // Travel

    RPG_TRAVEL_FLIGHT = 6,
    // Taking a break
    RPG_REST = 7,
    RPG_OUTDOOR_PVP = 8,
    RPG_STATUS_END = 9
};

#define MAX_SPECNO 20

// One level range/bucket used by the random bot level brackets sub-feature (see RandomBotLevelMgr).
struct LevelBracketConfig
{
    uint8 Lower = 1;
    uint8 Upper = 80;
    uint8 Pct = 0;
};

class PlayerbotAIConfig
{
public:
    static PlayerbotAIConfig& Instance()
    {
        static PlayerbotAIConfig instance;

        return instance;
    }

    bool Initialize();
    void LoadRandomBotLevelConfig();
    bool IsInRandomAccountList(uint32 id);
    bool IsInRandomQuestItemList(uint32 id);
    bool IsPvpProhibited(uint32 zoneId, uint32 areaId);
    bool IsInPvpProhibitedZone(uint32 id);
    bool IsInPvpProhibitedArea(uint32 id);

    bool Enabled;
    bool DisabledWithoutRealPlayer;
    bool EnableICCBuffs;
    bool AllowAccountBots, AllowGuildBots, AllowTrustedAccountBots;
    bool RandomBotGuildNearby, RandomBotInvitePlayer, InviteChat;
    uint32 GlobalCoolDown, ReactDelay, MaxWaitForMove, DisableMoveSplinePath, MaxMovementSearchTime, ExpireActionTime,
        DispelAuraDuration, PassiveDelay, RepeatDelay, ErrorDelay, RpgDelay, SitDelay, ReturnDelay, LootDelay;
    bool DynamicReactDelay;
    float SightDistance, SpellDistance, ReactDistance, GrindDistance, LootDistance, ShootDistance, FleeDistance,
        TooCloseDistance, MeleeDistance, FollowDistance, WhisperDistance, ContactDistance, AoeRadius, RpgDistance,
        TargetPosRecalcDistance, FarDistance, HealDistance, AggroDistance;
    uint32 CriticalHealth, LowHealth, MediumHealth, AlmostFullHealth;
    uint32 LowMana, MediumMana, HighMana;
    bool AutoSaveMana;
    uint32 SaveManaThreshold;
    AutoPartyBuffMode AutoGreaterBlessings;
    AutoPartyBuffMode AutoPartyBuffs;
    bool TellWhenMissingBuffReagents;
    uint32 MissingBuffReagentMessageCooldown;
    bool ForceRebuffOnReadyCheck;
    uint32 ForceRebuffMarginSecs;
    bool AutoAvoidAoe;
    float MaxAoeAvoidRadius;
    std::set<uint32> AoeAvoidSpellWhitelist;
    bool TellWhenAvoidAoe;
    std::set<uint32> DisallowedGameObjects;
    std::set<uint32> AttunementQuests;
    std::set<uint32> UnobtainableItems;

    uint32 OpenGoSpell;
    bool RandomBotAutologin;
    bool BotAutologin;
    std::string RandomBotMapsAsString;
    float ProbTeleToBankers;
    bool EnableWeightTeleToCityBankers;
    int WeightTeleToStormwind;
    int WeightTeleToIronforge;
    int WeightTeleToDarnassus;
    int WeightTeleToExodar;
    int WeightTeleToOrgrimmar;
    int WeightTeleToUndercity;
    int WeightTeleToThunderBluff;
    int WeightTeleToSilvermoonCity;
    int WeightTeleToShattrathCity;
    int WeightTeleToDalaran;
    std::vector<uint32> RandomBotMaps;
    std::vector<uint32> RandomBotQuestItems;
    std::vector<uint32> RandomBotAccounts;
    std::vector<uint32> RandomBotSpellIds;
    std::vector<uint32> RandomBotQuestIds;
    uint32 RandomBotTeleportDistance;
    float RandomGearLoweringChance;
    int32 RandomGearQualityLimit;
    int32 RandomGearScoreLimit;
    bool PreferClassArmorType;
    bool PreferredSpecWeapons;
    float RandomBotMinLevelChance, RandomBotMaxLevelChance;
    float RandomBotRpgChance;
    uint32 MinRandomBots, MaxRandomBots;
    uint32 RandomBotUpdateInterval, RandomBotCountChangeMinInterval, RandomBotCountChangeMaxInterval;
    uint32 MinRandomBotInWorldTime, MaxRandomBotInWorldTime;
    uint32 MinRandomBotRandomizeTime, MaxRandomBotRandomizeTime;
    uint32 MinRandomBotChangeStrategyTime, MaxRandomBotChangeStrategyTime;
    uint32 MinRandomBotReviveTime, MaxRandomBotReviveTime;
    uint32 MinRandomBotTeleportInterval, MaxRandomBotTeleportInterval;
    uint32 PermanentlyInWorldTime;
    uint32 MinRandomBotPvpTime, MaxRandomBotPvpTime;
    uint32 RandomBotsPerInterval;
    uint32 RandomBotPrintStatsInterval;
    uint32 MinRandomBotsPriceChangeInterval, MaxRandomBotsPriceChangeInterval;
    uint32 DisabledWithoutRealPlayerLoginDelay, DisabledWithoutRealPlayerLogoutDelay;
    bool RandomBotJoinLfg;

    // Professions
    bool EnableFishingWithMaster;
    uint32 ClassMatchingProfessionChance;
    float FishingDistanceFromMaster, FishingDistance, EndFishingWithMaster;

    // chat
    bool RandomBotTalk;
    bool RandomBotEmote;
    bool RandomBotSuggestDungeons;
    bool EnableBroadcasts;
    bool EnableGreet;
    bool RandomBotSayWithoutMaster;
    bool AnnounceConsumableUse;

    uint32 BroadcastChanceMaxValue;

    uint32 BroadcastToGuildGlobalChance;
    uint32 BroadcastToWorldGlobalChance;
    uint32 BroadcastToGeneralGlobalChance;
    uint32 BroadcastToTradeGlobalChance;
    uint32 BroadcastToLFGGlobalChance;
    uint32 BroadcastToLocalDefenseGlobalChance;
    uint32 BroadcastToWorldDefenseGlobalChance;
    uint32 BroadcastToGuildRecruitmentGlobalChance;

    uint32 BroadcastChanceLootingItemPoor;
    uint32 BroadcastChanceLootingItemNormal;
    uint32 BroadcastChanceLootingItemUncommon;
    uint32 BroadcastChanceLootingItemRare;
    uint32 BroadcastChanceLootingItemEpic;
    uint32 BroadcastChanceLootingItemLegendary;
    uint32 BroadcastChanceLootingItemArtifact;

    uint32 BroadcastChanceQuestAccepted;
    uint32 BroadcastChanceQuestUpdateObjectiveCompleted;
    uint32 BroadcastChanceQuestUpdateObjectiveProgress;
    uint32 BroadcastChanceQuestUpdateFailedTimer;
    uint32 BroadcastChanceQuestUpdateComplete;
    uint32 BroadcastChanceQuestTurnedIn;

    uint32 BroadcastChanceKillNormal;
    uint32 BroadcastChanceKillElite;
    uint32 BroadcastChanceKillRareelite;
    uint32 BroadcastChanceKillWorldboss;
    uint32 BroadcastChanceKillRare;
    uint32 BroadcastChanceKillUnknown;
    uint32 BroadcastChanceKillPet;
    uint32 BroadcastChanceKillPlayer;

    uint32 BroadcastChanceLevelupGeneric;
    uint32 BroadcastChanceLevelupTenX;
    uint32 BroadcastChanceLevelupMaxLevel;

    uint32 BroadcastChanceSuggestInstance;
    uint32 BroadcastChanceSuggestQuest;
    uint32 BroadcastChanceSuggestGrindMaterials;
    uint32 BroadcastChanceSuggestGrindReputation;
    uint32 BroadcastChanceSuggestSell;
    uint32 BroadcastChanceSuggestSomething;

    uint32 BroadcastChanceSuggestSomethingToxic;

    uint32 BroadcastChanceSuggestToxicLinks;
    std::string ToxicLinksPrefix;
    uint32 ToxicLinksRepliesChance;

    uint32 BroadcastChanceSuggestThunderfury;
    uint32 ThunderfuryRepliesChance;

    uint32 BroadcastChanceGuildManagement;

    uint32 GuildRepliesRate;

    bool RandomBotJoinBG;
    bool RandomBotAutoJoinBG;

    std::string RandomBotAutoJoinICBrackets;
    std::string RandomBotAutoJoinEYBrackets;
    std::string RandomBotAutoJoinAVBrackets;
    std::string RandomBotAutoJoinABBrackets;
    std::string RandomBotAutoJoinWSBrackets;

    uint32 RandomBotAutoJoinBGICCount;
    uint32 RandomBotAutoJoinBGEYCount;
    uint32 RandomBotAutoJoinBGAVCount;
    uint32 RandomBotAutoJoinBGABCount;
    uint32 RandomBotAutoJoinBGWSCount;

    uint32 RandomBotAutoJoinArenaBracket;

    uint32 RandomBotAutoJoinBGRatedArena2v2Count;
    uint32 RandomBotAutoJoinBGRatedArena3v3Count;
    uint32 RandomBotAutoJoinBGRatedArena5v5Count;

    uint32 RandomBotTeleLowerLevel, RandomBotTeleHigherLevel;
    std::map<uint32, std::pair<uint32, uint32>> ZoneBrackets;
    bool LogInGroupOnly, LogValuesPerTick;
    bool SummonAtInnkeepersEnabled;
    std::string CombatStrategies, NonCombatStrategies;
    std::string RandomBotCombatStrategies, RandomBotNonCombatStrategies;
    std::string ReactStrategies, RandomBotReactStrategies;
    bool ApplyInstanceStrategies;
    uint32 RandomBotMinLevel, RandomBotMaxLevel;
    float RandomChangeMultiplier;

    // std::string premadeLevelSpec[MAX_CLASSES][10][91]; //lvl 10 - 100
    // ClassSpecs classSpecs[MAX_CLASSES];

    std::string PremadeSpecName[MAX_CLASSES][MAX_SPECNO];
    std::string PremadeSpecGlyph[MAX_CLASSES][MAX_SPECNO];
    std::vector<uint32> ParsedSpecGlyph[MAX_CLASSES][MAX_SPECNO];
    std::string PremadeSpecLink[MAX_CLASSES][MAX_SPECNO][MAX_LEVEL];
    std::string PremadeHunterPetLink[3][21];
    std::vector<std::vector<uint32>> ParsedSpecLinkOrder[MAX_CLASSES][MAX_SPECNO][MAX_LEVEL];
    std::vector<std::vector<uint32>> ParsedHunterPetLinkOrder[3][21];
    uint32 RandomClassSpecProb[MAX_CLASSES][MAX_SPECNO];
    uint32 RandomClassSpecIndex[MAX_CLASSES][MAX_SPECNO];

    std::string CommandPrefix, CommandSeparator;
    std::string RandomBotAccountPrefix;
    uint32 RandomBotAccountCount;
    bool RandomBotRandomPassword;
    bool DeleteRandomBotAccounts;
    uint32 RandomBotGuildCount, RandomBotGuildSizeMax;
    bool DeleteRandomBotGuilds;
    std::vector<uint32> PvpProhibitedZoneIds;
    std::vector<uint32> PvpProhibitedAreaIds;
    bool FastReactInBG;

    bool RandomBotsWalkingRPG;
    bool RandomBotsWalkingRPGInDoors;
    uint32 MinEnchantingBotLevel;
    uint32 LimitEnchantExpansion;
    uint32 LimitGearExpansion;
    uint32 RandomBotStartingLevel;
    bool EnablePeriodicOnlineOffline;
    float PeriodicOnlineOfflineRatio;
    bool GearScoreCheck;
    bool RandomBotPreQuests;
    bool BotSendMailEnabled;

    bool GuildTaskEnabled;
    uint32 MinGuildTaskChangeTime, MaxGuildTaskChangeTime;
    uint32 MinGuildTaskAdvertisementTime, MaxGuildTaskAdvertisementTime;
    uint32 MinGuildTaskRewardTime, MaxGuildTaskRewardTime;
    uint32 GuildTaskAdvertCleanupTime;
    uint32 GuildTaskKillTaskDistance;

    uint32 IterationsPerTick;

    std::mutex LogMutex;
    bool EnableAutoTradeOnItemMention;
    std::vector<std::string> TradeActionExcludedPrefixes;
    std::vector<std::string> AllowedLogFiles;
    std::unordered_map<std::string, std::pair<FILE*, bool>> LogFiles;

    std::vector<std::string> BotCheats;
    uint32 BotCheatMask = 0;

    struct WorldBuff
    {
        uint32 SpellId;
        uint32 FactionId;
        uint32 ClassId;
        uint32 SpecId;
        uint32 MinLevel;
        uint32 MaxLevel;
    };

    std::vector<WorldBuff> WorldBuffs;

    uint32 CommandServerPort;
    bool PerfMonEnabled;
    bool SummonWhenGroup;
    ShowHideCosmetic RandomBotShowHelmet;
    ShowHideCosmetic RandomBotShowCloak;
    bool RandomBotFixedLevel;
    bool DisableRandomLevels;
    float RandomBotXPRate;
    uint32 RandomBotAllianceRatio;
    uint32 RandomBotHordeRatio;
    bool DisableDeathKnightLogin;
    bool LimitTalentsExpansion;
    uint32 BotActiveAlone;
    uint32 BotActiveAloneDurationSeconds;
    uint32 BotActiveAloneForceWhenInRadius;
    bool BotActiveAloneForceWhenInZone;
    bool BotActiveAloneForceWhenInMap;
    bool BotActiveAloneForceWhenIsFriend;
    bool BotActiveAloneForceWhenInGuild;
    bool BotActiveAloneSmartScale;
    uint32 BotActiveAloneSmartScaleDiffLimitFloor;
    uint32 BotActiveAloneSmartScaleDiffLimitCeiling;
    uint32 BotActiveAloneSmartScaleWhenMinLevel;
    uint32 BotActiveAloneSmartScaleWhenMaxLevel;

    bool FreeMethodLoot;
    int32 LootNeedRollLevel;
    bool LootGreedRollLevel;
    bool LootRollRecipe;
    bool LootRollDisenchant;
    std::string AutoPickReward;
    bool AutoEquipUpgradeLoot;
    float EquipUpgradeThreshold;
    bool TwoRoundsGearInit;
    bool SyncQuestWithPlayer;
    bool SyncQuestForPlayer;
    bool DropObsoleteQuests;
    bool AllowLearnTrainerSpells;
    bool AutoPickTalents;
    bool AutoUpgradeEquip;
    int32 HunterWolfPet;
    int32 DefaultPetStance;
    int32 PetChatCommandDebug;
    bool AutoLearnTrainerSpells;
    bool AutoDoQuests;
    bool EnableNewRpgStrategy;
    std::unordered_map<NewRpgStatus, uint32> RpgStatusProbWeight;
    bool SyncLevelWithPlayers;
    bool RandomBotConcentrateInPlayerZone;
    bool AutoLearnQuestSpells;
    bool AutoTeleportForLevel;
    bool RandomBotGroupNearby;
    int32 EnableRandomBotTrading;
    uint32 TweakValue;  // Debugging config

    uint32 RandomBotArenaTeamMaxRating;
    uint32 RandomBotArenaTeamMinRating;
    uint32 RandomBotArenaTeam2v2Count;
    uint32 RandomBotArenaTeam3v3Count;
    uint32 RandomBotArenaTeam5v5Count;
    bool DeleteRandomBotArenaTeams;

    uint32 SelfBotLevel;
    bool DowngradeMaxLevelBot;
    bool EquipAndSpecPersistence;
    int32 EquipAndSpecPersistenceLevel;
    int32 GroupInvitationPermission;
    bool KeepAltsInGroup = false;
    bool AllowSummonInCombat;
    bool AllowSummonWhenMasterIsDead;
    bool AllowSummonWhenBotIsDead;
    int ReviveBotWhenSummoned;
    bool BotRepairWhenSummon;
    bool AutoInitOnly;
    bool ResetInstanceIdForAltBots;
    float AutoInitEquipLevelLimitRatio;
    int32 MaxAddedBots;
    int32 AddClassCommand;
    int32 AddClassAccountPoolSize;
    bool AddClassRandomCharacter;
    int32 MaintenanceCommand;
    bool AltMaintenanceAttunementQs,
            AltMaintenanceBags,
            AltMaintenanceAmmo,
            AltMaintenanceFood,
            AltMaintenanceReagents,
            AltMaintenanceConsumables,
            AltMaintenancePotions,
            AltMaintenanceTalentTree,
            AltMaintenancePet,
            AltMaintenancePetTalents,
            AltMaintenanceClassSpells,
            AltMaintenanceAvailableSpells,
            AltMaintenanceSkills,
            AltMaintenanceReputation,
            AltMaintenanceSpecialSpells,
            AltMaintenanceMounts,
            AltMaintenanceGlyphs,
            AltMaintenanceKeyring,
            AltMaintenanceGemsEnchants;
    int32 AutoGearCommand, AutoGearCommandAltBots, AutoGearQualityLimit, AutoGearScoreLimit;
    int32 AutoGearBisCommand;

    uint32 UseGroundMountAtMinLevel;
    uint32 UseFastGroundMountAtMinLevel;
    uint32 UseFlyMountAtMinLevel;
    uint32 UseFastFlyMountAtMinLevel;

    // stagger flightpath takeoff
    uint32 BotTaxiDelayMin;
    uint32 BotTaxiDelayMax;
    uint32 BotTaxiGapMs;
    uint32 BotTaxiGapJitterMs;

    std::string const GetTimestampStr();
    bool HasLog(std::string const fileName)
    {
        return std::find(AllowedLogFiles.begin(), AllowedLogFiles.end(), fileName) != AllowedLogFiles.end();
    };
    bool OpenLog(std::string const fileName, char const* mode = "a");
    bool IsLogOpen(std::string const fileName)
    {
        auto it = LogFiles.find(fileName);
        return it != LogFiles.end() && it->second.second;
    }
    void Log(std::string const fileName, char const* str, ...);

    void LoadWorldBuff();

    static std::vector<std::vector<uint32>> ParseTempTalentsOrder(uint32 cls, std::string temp_talents_order);
    static std::vector<std::vector<uint32>> ParseTempPetTalentsOrder(uint32 spec, std::string temp_talents_order);

    bool RestrictHealerDPS = false;
    std::vector<uint32> RestrictedHealerDPSMaps;
    bool IsRestrictedHealerDPSMap(uint32 mapId) const;

    std::vector<uint32> ExcludedHunterPetFamilies;

    // Random bot level brackets (periodic redistribution across per-faction level ranges). See
    // RandomBotLevelMgr; percentages here are the as-configured values, not the runtime working copy.
    bool LevelBracketsEnabled;
    uint32 LevelBracketsCheckFrequency;
    uint32 LevelBracketsFlaggedCheckFrequency;
    uint32 LevelBracketsFlaggedProcessLimit;
    bool LevelBracketsIgnoreGuildWithRealPlayers;
    bool LevelBracketsIgnoreArenaTeamBots;
    bool LevelBracketsIgnoreFriendListed;
    std::vector<std::string> LevelBracketsExcludeNames;
    uint8 LevelBracketsNumRanges;
    std::vector<LevelBracketConfig> LevelBracketsAlliance;
    std::vector<LevelBracketConfig> LevelBracketsHorde;
    bool LevelBracketsDynamicDistribution;
    float LevelBracketsRealPlayerWeight;
    bool LevelBracketsSyncFactions;

    // Random bot level reset (reset random bots reaching max level). See RandomBotLevelMgr.
    bool ResetBotLevelEnabled;
    uint8 ResetBotLevelMaxLevel;
    uint8 ResetBotLevelResetTo;
    uint8 ResetBotLevelSkipFrom;
    uint8 ResetBotLevelSkipTo;
    uint8 ResetBotLevelChance;
    bool ResetBotLevelScaledChance;
    bool ResetBotLevelRestrictTimePlayed;
    uint32 ResetBotLevelMinTimePlayed;
    uint32 ResetBotLevelPlayedTimeCheckFrequency;
    bool ResetBotLevelIgnoreGuildWithRealPlayers;
    std::vector<std::string> ResetBotLevelExcludeNames;

private:
    PlayerbotAIConfig() = default;
    ~PlayerbotAIConfig() = default;

    PlayerbotAIConfig(PlayerbotAIConfig const&) = delete;
    PlayerbotAIConfig& operator=(PlayerbotAIConfig const&) = delete;

    PlayerbotAIConfig(PlayerbotAIConfig&&) = delete;
    PlayerbotAIConfig& operator=(PlayerbotAIConfig&&) = delete;
};

#define sPlayerbotAIConfig PlayerbotAIConfig::Instance()

#endif
