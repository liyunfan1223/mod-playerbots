/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PLAYERbotAI_H
#define _PLAYERBOT_PLAYERbotAI_H

#include <queue>
#include <stack>

#include "Chat.h"
#include "ChatFilter.h"
#include "ChatHelper.h"
#include "Common.h"
#include "CreatureData.h"
#include "Event.h"
#include "Item.h"
#include "NewRpgInfo.h"
#include "NewRpgStrategy.h"
#include "PlayerbotAIBase.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotSecurity.h"
#include "PlayerbotTextMgr.h"
#include "SpellAuras.h"
#include "Util.h"
#include "WorldPacket.h"

class AiObjectContext;
class Creature;
class Engine;
class ExternalEventHelper;
class Gameobject;
class Item;
class ObjectGuid;
class Player;
class PlayerbotMgr;
class Spell;
class SpellInfo;
class Unit;
class WorldObject;
class WorldPosition;

struct CreatureData;
struct GameObjectData;

enum StrategyType : uint32;

enum HealingItemId
{
    HEALTHSTONE = 5512,
    MAJOR_HEALING_POTION = 13446,
    WHIPPER_ROOT_TUBER = 11951,
    NIGHT_DRAGON_BREATH = 11952,
    LIMITED_INVULNERABILITY_POTION = 3387,
    GREATER_DREAMLESS_SLEEP_POTION = 22886,
    SUPERIOR_HEALING_POTION = 3928,
    CRYSTAL_RESTORE = 11564,
    DREAMLESS_SLEEP_POTION = 12190,
    GREATER_HEALING_POTION = 1710,
    HEALING_POTION = 929,
    LESSER_HEALING_POTION = 858,
    DISCOLORED_HEALING_POTION = 3391,
    MINOR_HEALING_POTION = 118,
    VOLATILE_HEALING_POTION = 28100,
    SUPER_HEALING_POTION = 22829,
    CRYSTAL_HEALING_POTION = 13462,
    FEL_REGENERATION_POTION = 28101,
    MAJOR_DREAMLESS_SLEEP_POTION = 20002
};

enum BotState
{
    BOT_STATE_COMBAT = 0,
    BOT_STATE_NON_COMBAT = 1,
    BOT_STATE_DEAD = 2,

    BOT_STATE_MAX
};

bool IsAlliance(uint8 race);

class PlayerbotChatHandler : protected ChatHandler
{
public:
    explicit PlayerbotChatHandler(Player* pMasterPlayer);
    void sysmessage(std::string const str) { SendSysMessage(str.c_str()); }
    uint32 extractQuestId(std::string const str);
    uint32 extractSpellId(std::string const str)
    {
        char* source = (char*)str.c_str();
        return extractSpellIdFromLink(source);
    }
};

class MinValueCalculator
{
public:
    MinValueCalculator(float def = 0.0f) : param(nullptr), minValue(def) {}

    void probe(float value, void* p)
    {
        if (!param || minValue >= value)
        {
            minValue = value;
            param = p;
        }
    }

    void* param;
    float minValue;
};
enum ChatChannelSource
{
    SRC_GUILD,
    SRC_WORLD,
    SRC_GENERAL,
    SRC_TRADE,
    SRC_LOOKING_FOR_GROUP,
    SRC_LOCAL_DEFENSE,
    SRC_WORLD_DEFENSE,
    SRC_GUILD_RECRUITMENT,

    SRC_SAY,
    SRC_WHISPER,
    SRC_EMOTE,
    SRC_TEXT_EMOTE,
    SRC_YELL,

    SRC_PARTY,
    SRC_RAID,

    SRC_UNDEFINED
};
static std::map<ChatChannelSource, std::string> ChatChannelSourceStr = {
    {SRC_GUILD, "SRC_GUILD"},
    {SRC_WORLD, "SRC_WORLD"},
    {SRC_GENERAL, "SRC_GENERAL"},
    {SRC_TRADE, "SRC_TRADE"},
    {SRC_LOOKING_FOR_GROUP, "SRC_LOOKING_FOR_GROUP"},
    {SRC_LOCAL_DEFENSE, "SRC_LOCAL_DEFENSE"},
    {SRC_WORLD_DEFENSE, "SRC_WORLD_DEFENSE"},
    {SRC_GUILD_RECRUITMENT, "SRC_GUILD_RECRUITMENT"},

    {SRC_SAY, "SRC_SAY"},
    {SRC_WHISPER, "SRC_WHISPER"},
    {SRC_EMOTE, "SRC_EMOTE"},
    {SRC_TEXT_EMOTE, "SRC_TEXT_EMOTE"},
    {SRC_YELL, "SRC_YELL"},

    {SRC_PARTY, "SRC_PARTY"},
    {SRC_RAID, "SRC_RAID"},

    {SRC_UNDEFINED, "SRC_UNDEFINED"}};

enum ChatChannelId
{
    GENERAL = 1,
    TRADE = 2,
    LOCAL_DEFENSE = 22,
    WORLD_DEFENSE = 23,
    LOOKING_FOR_GROUP = 26,
    GUILD_RECRUITMENT = 25,
};

enum RoguePoisonId
{
    INSTANT_POISON      = 6947,
    INSTANT_POISON_II   = 6949,
    INSTANT_POISON_III  = 6950,
    INSTANT_POISON_IV   = 8926,
    INSTANT_POISON_V    = 8927,
    INSTANT_POISON_VI   = 8928,
    INSTANT_POISON_VII  = 21927,
    INSTANT_POISON_VIII = 43230,
    INSTANT_POISON_IX   = 43231,
    DEADLY_POISON       = 2892,
    DEADLY_POISON_II    = 2893,
    DEADLY_POISON_III   = 8984,
    DEADLY_POISON_IV    = 8985,
    DEADLY_POISON_V     = 20844,
    DEADLY_POISON_VI    = 22053,
    DEADLY_POISON_VII   = 22054,
    DEADLY_POISON_VIII  = 43232,
    DEADLY_POISON_IX    = 43233
};

enum SharpeningStoneId
{
    ROUGH_SHARPENING_STONE      = 2862,
    COARSE_SHARPENING_STONE     = 2863,
    HEAVY_SHARPENING_STONE      = 2871,
    SOLID_SHARPENING_STONE      = 7964,
    DENSE_SHARPENING_STONE      = 12404,
    ELEMENTAL_SHARPENING_STONE  = 18262,
    FEL_SHARPENING_STONE        = 23528,
    ADAMANTITE_SHARPENING_STONE = 23529
};

enum WeightstoneId
{
    ROUGH_WEIGHTSTONE      = 3239,
    COARSE_WEIGHTSTONE     = 3240,
    HEAVY_WEIGHTSTONE      = 3241,
    SOLID_WEIGHTSTONE      = 7965,
    DENSE_WEIGHTSTONE      = 12643,
    FEL_WEIGHTSTONE        = 28420,
    ADAMANTITE_WEIGHTSTONE = 28421
};

enum WizardOilId
{
    MINOR_WIZARD_OIL      = 20744,
    LESSER_WIZARD_OIL     = 20746,
    WIZARD_OIL            = 20750,
    BRILLIANT_WIZARD_OIL  = 20749,
    SUPERIOR_WIZARD_OIL   = 22522
};

enum ManaOilId
{
    MINOR_MANA_OIL        = 20745,
    LESSER_MANA_OIL       = 20747,
    BRILLIANT_MANA_OIL    = 20748,
    SUPERIOR_MANA_OIL     = 22521
};

enum class BotTypeNumber : uint8
{
    ACTIVITY_TYPE_NUMBER = 1,
    GROUPER_TYPE_NUMBER = 2,
    GUILDER_TYPE_NUMBER = 3,
};

enum class GrouperType : uint8
{
    SOLO = 0,
    MEMBER = 1,
    LEADER_2 = 2,
    LEADER_3 = 3,
    LEADER_4 = 4,
    LEADER_5 = 5
};

enum class GuilderType : uint8
{
    SOLO = 0,
    TINY = 30,
    SMALL = 50,
    MEDIUM = 70,
    LARGE = 120,
    VERY_LARGE = 250
};

enum ActivityType
{
    GRIND_ACTIVITY = 1,
    RPG_ACTIVITY = 2,
    TRAVEL_ACTIVITY = 3,
    OUT_OF_PARTY_ACTIVITY = 4,
    PACKET_ACTIVITY = 5,
    DETAILED_MOVE_ACTIVITY = 6,
    PARTY_ACTIVITY = 7,
    ALL_ACTIVITY = 8,

    MAX_ACTIVITY_TYPE
};

enum BotRoles : uint8
{
    BOT_ROLE_NONE = 0x00,
    BOT_ROLE_TANK = 0x01,
    BOT_ROLE_HEALER = 0x02,
    BOT_ROLE_DPS = 0x04
};

enum HUNTER_TABS
{
    HUNTER_TAB_BEASTMASTERY,
    HUNTER_TAB_MARKSMANSHIP,
    HUNTER_TAB_SURVIVAL,
};

enum ROGUE_TABS
{
    ROGUE_TAB_ASSASSINATION,
    ROGUE_TAB_COMBAT,
    ROGUE_TAB_SUBTLETY,
};

enum PRIEST_TABS
{
    PRIEST_TAB_DISCIPLINE,
    PRIEST_TAB_HOLY,
    PRIEST_TAB_SHADOW,
};

enum DEATHKNIGHT_TABS
{
    DEATHKNIGHT_TAB_BLOOD,
    DEATHKNIGHT_TAB_FROST,
    DEATHKNIGHT_TAB_UNHOLY,
};

enum DRUID_TABS
{
    DRUID_TAB_BALANCE,
    DRUID_TAB_FERAL,
    DRUID_TAB_RESTORATION,
};

enum MAGE_TABS
{
    MAGE_TAB_ARCANE,
    MAGE_TAB_FIRE,
    MAGE_TAB_FROST,
};

enum SHAMAN_TABS
{
    SHAMAN_TAB_ELEMENTAL,
    SHAMAN_TAB_ENHANCEMENT,
    SHAMAN_TAB_RESTORATION,
};

enum PALADIN_TABS
{
    PALADIN_TAB_HOLY,
    PALADIN_TAB_PROTECTION,
    PALADIN_TAB_RETRIBUTION,
};

enum WARLOCK_TABS
{
    WARLOCK_TAB_AFFLICTION,
    WARLOCK_TAB_DEMONOLOGY,
    WARLOCK_TAB_DESTRUCTION,
};

enum WARRIOR_TABS
{
    WARRIOR_TAB_ARMS,
    WARRIOR_TAB_FURY,
    WARRIOR_TAB_PROTECTION,
};

class PacketHandlingHelper
{
public:
    void AddHandler(uint16 opcode, std::string const handler);
    void Handle(ExternalEventHelper& helper);
    void AddPacket(WorldPacket const& packet);

private:
    std::map<uint16, std::string> handlers;
    std::stack<WorldPacket> queue;
};

class ChatCommandHolder
{
public:
    ChatCommandHolder(std::string const command, Player* owner = nullptr, uint32 type = CHAT_MSG_WHISPER,
                      time_t time = 0)
        : command(command), owner(owner), type(type), time(time)
    {
    }
    ChatCommandHolder(ChatCommandHolder const& other)
        : command(other.command), owner(other.owner), type(other.type), time(other.time)
    {
    }

    const std::string& GetCommand() { return command; }
    Player* GetOwner() { return owner; }
    uint32& GetType() { return type; }
    time_t& GetTime() { return time; }

private:
    std::string const command;
    Player* owner;
    uint32 type;
    time_t time;
};

class PlayerbotAI : public PlayerbotAIBase
{
public:
    PlayerbotAI();
    PlayerbotAI(Player* bot);
    virtual ~PlayerbotAI();

    void UpdateAI(uint32 elapsed, bool minimal = false) override;
    void UpdateAIInternal(uint32 elapsed, bool minimal = false) override;

    std::string const HandleRemoteCommand(std::string const command);
    void HandleCommand(uint32 type, std::string const text, Player* fromPlayer);
    void QueueChatResponse(const ChatQueuedReply reply);
    void HandleBotOutgoingPacket(WorldPacket const& packet);
    void HandleMasterIncomingPacket(WorldPacket const& packet);
    void HandleMasterOutgoingPacket(WorldPacket const& packet);
    void HandleTeleportAck();
    void ChangeEngine(BotState type);
    void ChangeEngineOnCombat();
    void ChangeEngineOnNonCombat();
    void DoNextAction(bool minimal = false);
    virtual bool DoSpecificAction(std::string const name, Event event = Event(), bool silent = false,
                                  std::string const qualifier = "");
    void ChangeStrategy(std::string const name, BotState type);
    void ClearStrategies(BotState type);
    std::vector<std::string> GetStrategies(BotState type);
    void ApplyInstanceStrategies(uint32 mapId, bool tellMaster = false);
    void EvaluateHealerDpsStrategy();
    bool ContainsStrategy(StrategyType type);
    bool HasStrategy(std::string const name, BotState type);
    BotState GetState() { return currentState; };
    void ResetStrategies(bool load = false);
    void ReInitCurrentEngine();
    void Reset(bool full = false);
    static bool IsTank(Player* player, bool bySpec = false);
    static bool IsHeal(Player* player, bool bySpec = false);
    static bool IsDps(Player* player, bool bySpec = false);
    static bool IsRanged(Player* player, bool bySpec = false);
    static bool IsMelee(Player* player, bool bySpec = false);
    static bool IsCaster(Player* player, bool bySpec = false);
    static bool IsRangedDps(Player* player, bool bySpec = false);
    static bool IsCombo(Player* player);
    static bool IsBotMainTank(Player* player);
    static bool IsMainTank(Player* player);
    static uint32 GetGroupTankNum(Player* player);
    static bool IsAssistTank(Player* player);
    static bool IsAssistTankOfIndex(Player* player, int index);
    static bool IsHealAssistantOfIndex(Player* player, int index);
    static bool IsRangedDpsAssistantOfIndex(Player* player, int index);
    bool HasAggro(Unit* unit);
    static int32 GetAssistTankIndex(Player* player);
    int32 GetGroupSlotIndex(Player* player);
    int32 GetRangedIndex(Player* player);
    int32 GetClassIndex(Player* player, uint8 cls);
    int32 GetRangedDpsIndex(Player* player);
    int32 GetMeleeIndex(Player* player);

    Creature* GetCreature(ObjectGuid guid);
    Unit* GetUnit(ObjectGuid guid);
    Player* GetPlayer(ObjectGuid guid);
    static Unit* GetUnit(CreatureData const* creatureData);
    GameObject* GetGameObject(ObjectGuid guid);
    // static GameObject* GetGameObject(GameObjectData const* gameObjectData);
    WorldObject* GetWorldObject(ObjectGuid guid);
    std::vector<Player*> GetPlayersInGroup();
    const AreaTableEntry* GetCurrentArea();
    const AreaTableEntry* GetCurrentZone();
    static std::string GetLocalizedAreaName(const AreaTableEntry* entry);
    static std::string GetLocalizedCreatureName(uint32 entry);
    static std::string GetLocalizedGameObjectName(uint32 entry);
    bool TellMaster(std::ostringstream& stream, PlayerbotSecurityLevel securityLevel = PLAYERBOT_SECURITY_ALLOW_ALL);
    bool TellMaster(std::string const text, PlayerbotSecurityLevel securityLevel = PLAYERBOT_SECURITY_ALLOW_ALL);
    bool TellMasterNoFacing(std::ostringstream& stream,
                            PlayerbotSecurityLevel securityLevel = PLAYERBOT_SECURITY_ALLOW_ALL);
    bool TellMasterNoFacing(std::string const text,
                            PlayerbotSecurityLevel securityLevel = PLAYERBOT_SECURITY_ALLOW_ALL);
    bool TellError(std::string const text, PlayerbotSecurityLevel securityLevel = PLAYERBOT_SECURITY_ALLOW_ALL);
    bool SayToGuild(const std::string& msg);
    bool SayToWorld(const std::string& msg);
    bool SayToChannel(const std::string& msg, const ChatChannelId& chanId);
    bool SayToParty(const std::string& msg);
    bool SayToRaid(const std::string& msg);
    bool Yell(const std::string& msg);
    bool Say(const std::string& msg);
    bool Whisper(const std::string& msg, const std::string& receiverName);

    void SpellInterrupted(uint32 spellid);
    int32 CalculateGlobalCooldown(uint32 spellid);
    void InterruptSpell();
    void RemoveAura(std::string const name);
    void RemoveShapeshift();
    void WaitForSpellCast(Spell* spell);
    bool PlaySound(uint32 emote);
    bool PlayEmote(uint32 emote);
    void Ping(float x, float y);
    Item* FindPoison() const;
    Item* FindAmmo() const;
    Item* FindBandage() const;
    Item* FindOpenableItem() const;
    Item* FindLockedItem() const;
    Item* FindConsumable(uint32 itemId) const;
    Item* FindStoneFor(Item* weapon) const;
    Item* FindOilFor(Item* weapon) const;
    void ImbueItem(Item* item, uint32 targetFlag, ObjectGuid targetGUID);
    void ImbueItem(Item* item, uint8 targetInventorySlot);
    void ImbueItem(Item* item, Unit* target);
    void ImbueItem(Item* item);
    void EnchantItemT(uint32 spellid, uint8 slot);
    uint32 GetBuffedCount(Player* player, std::string const spellname);
    int32 GetNearGroupMemberCount(float dis = sPlayerbotAIConfig->sightDistance);

    virtual bool CanCastSpell(std::string const name, Unit* target, Item* itemTarget = nullptr);
    virtual bool CastSpell(std::string const name, Unit* target, Item* itemTarget = nullptr);
    virtual bool HasAura(std::string const spellName, Unit* player, bool maxStack = false, bool checkIsOwner = false,
                         int maxAmount = -1, bool checkDuration = false);
    virtual bool HasAnyAuraOf(Unit* player, ...);

    virtual bool IsInterruptableSpellCasting(Unit* player, std::string const spell);
    virtual bool HasAuraToDispel(Unit* player, uint32 dispelType);
    bool CanCastSpell(uint32 spellid, Unit* target, bool checkHasSpell = true, Item* itemTarget = nullptr,
                      Item* castItem = nullptr);
    bool CanCastSpell(uint32 spellid, GameObject* goTarget, bool checkHasSpell = true);
    bool CanCastSpell(uint32 spellid, float x, float y, float z, bool checkHasSpell = true,
                      Item* itemTarget = nullptr);

    bool HasAura(uint32 spellId, Unit const* player);
    Aura* GetAura(std::string const spellName, Unit* unit, bool checkIsOwner = false, bool checkDuration = false,
                  int checkStack = -1);
    bool CastSpell(uint32 spellId, Unit* target, Item* itemTarget = nullptr);
    bool CastSpell(uint32 spellId, float x, float y, float z, Item* itemTarget = nullptr);
    bool canDispel(SpellInfo const* spellInfo, uint32 dispelType);

    bool CanCastVehicleSpell(uint32 spellid, Unit* target);
    bool CastVehicleSpell(uint32 spellId, Unit* target);
    bool CastVehicleSpell(uint32 spellId, float x, float y, float z);
    bool IsInVehicle(bool canControl = false, bool canCast = false, bool canAttack = false, bool canTurn = false,
                     bool fixed = false);

    uint32 GetEquipGearScore(Player* player);
    //uint32 GetEquipGearScore(Player* player, bool withBags, bool withBank);
    static uint32 GetMixedGearScore(Player* player, bool withBags, bool withBank, uint32 topN = 0);
    bool HasSkill(SkillType skill);
    bool IsAllowedCommand(std::string const text);
    float GetRange(std::string const type);

    Player* GetBot() { return bot; }
    Player* GetMaster() { return master; }

    // Checks if the bot is really a player. Players always have themselves as master.
    bool IsRealPlayer() { return master ? (master == bot) : false; }
    // Bot has a master that is a player.
    bool HasRealPlayerMaster();
    // Bot has a master that is activly playing.
    bool HasActivePlayerMaster();
    // Get the group leader or the master of the bot.
    // Checks if the bot is summoned as alt of a player
    bool IsAlt();
    Player* GetGroupMaster();
    // Returns a semi-random (cycling) number that is fixed for each bot.
    uint32 GetFixedBotNumer(uint32 maxNum = 100, float cyclePerMin = 1);
    GrouperType GetGrouperType();
    GuilderType GetGuilderType();
    bool HasPlayerNearby(WorldPosition* pos, float range = sPlayerbotAIConfig->reactDistance);
    bool HasPlayerNearby(float range = sPlayerbotAIConfig->reactDistance);
    bool HasManyPlayersNearby(uint32 trigerrValue = 20, float range = sPlayerbotAIConfig->sightDistance);
    bool AllowActive(ActivityType activityType);
    bool AllowActivity(ActivityType activityType = ALL_ACTIVITY, bool checkNow = false);
    uint32 AutoScaleActivity(uint32 mod);

    // Check if player is safe to use.
    bool IsSafe(Player* player);
    bool IsSafe(WorldObject* obj);
    ChatChannelSource GetChatChannelSource(Player* bot, uint32 type, std::string channelName);

    bool CheckLocationDistanceByLevel(Player* player, const WorldLocation &loc, bool fromStartUp = false);

    bool HasCheat(BotCheatMask mask)
    {
        return ((uint32)mask & (uint32)cheatMask) != 0 ||
               ((uint32)mask & (uint32)sPlayerbotAIConfig->botCheatMask) != 0;
    }
    BotCheatMask GetCheat() { return cheatMask; }
    void SetCheat(BotCheatMask mask) { cheatMask = mask; }

    void SetMaster(Player* newMaster) { master = newMaster; }
    AiObjectContext* GetAiObjectContext() { return aiObjectContext; }
    ChatHelper* GetChatHelper() { return &chatHelper; }
    bool IsOpposing(Player* player);
    static bool IsOpposing(uint8 race1, uint8 race2);
    PlayerbotSecurity* GetSecurity() { return &security; }

    Position GetJumpDestination() { return jumpDestination; }
    void SetJumpDestination(Position pos) { jumpDestination = pos; }
    void ResetJumpDestination() { jumpDestination = Position(); }

    bool CanMove();
    static bool IsRealGuild(uint32 guildId);
    bool IsInRealGuild();
    static std::vector<std::string> dispel_whitelist;
    bool EqualLowercaseName(std::string s1, std::string s2);
    InventoryResult CanEquipItem(uint8 slot, uint16& dest, Item* pItem, bool swap, bool not_loading = true) const;
    uint8 FindEquipSlot(ItemTemplate const* proto, uint32 slot, bool swap) const;
    std::vector<Item*> GetInventoryAndEquippedItems();
    std::vector<Item*> GetInventoryItems();
    uint32 GetInventoryItemsCountWithId(uint32 itemId);
    bool HasItemInInventory(uint32 itemId);
    std::vector<std::pair<const Quest*, uint32>> GetCurrentQuestsRequiringItemId(uint32 itemId);
    uint32 GetReactDelay();

    std::vector<const Quest*> GetAllCurrentQuests();
    std::vector<const Quest*> GetCurrentIncompleteQuests();
    std::set<uint32> GetAllCurrentQuestIds();
    std::set<uint32> GetCurrentIncompleteQuestIds();
    void PetFollow();
    static float GetItemScoreMultiplier(ItemQualities quality);
    static bool IsHealingSpell(uint32 spellFamilyName, flag96 spelFalimyFlags);
    static SpellFamilyNames Class2SpellFamilyName(uint8 cls);
    NewRpgInfo rpgInfo;
    NewRpgStatistic rpgStatistic;
    std::unordered_set<uint32> lowPriorityQuest;

    // Schedules a callback to run once after <delayMs> milliseconds.
    void AddTimedEvent(std::function<void()> callback, uint32 delayMs);

private:
    static void _fillGearScoreData(Player* player, Item* item, std::vector<uint32>* gearScore, uint32& twoHandScore,
                                   bool mixed = false);
    bool IsTellAllowed(PlayerbotSecurityLevel securityLevel = PLAYERBOT_SECURITY_ALLOW_ALL);
    void UpdateAIGroupMembership();
    Item* FindItemInInventory(std::function<bool(ItemTemplate const*)> checkItem) const;
    void HandleCommands();
    void HandleCommand(uint32 type, const std::string& text, Player& fromPlayer, const uint32 lang = LANG_UNIVERSAL);
    bool _isBotInitializing = false;

protected:
    Player* bot;
    Player* master;
    uint32 accountId;
    AiObjectContext* aiObjectContext;
    Engine* currentEngine;
    Engine* engines[BOT_STATE_MAX];
    BotState currentState;
    ChatHelper chatHelper;
    std::list<ChatCommandHolder> chatCommands;
    std::list<ChatQueuedReply> chatReplies;
    PacketHandlingHelper botOutgoingPacketHandlers;
    PacketHandlingHelper masterIncomingPacketHandlers;
    PacketHandlingHelper masterOutgoingPacketHandlers;
    CompositeChatFilter chatFilter;
    PlayerbotSecurity security;
    std::map<std::string, time_t> whispers;
    std::pair<ChatMsg, time_t> currentChat;
    static std::set<std::string> unsecuredCommands;
    bool allowActive[MAX_ACTIVITY_TYPE];
    time_t allowActiveCheckTimer[MAX_ACTIVITY_TYPE];
    bool inCombat = false;
    BotCheatMask cheatMask = BotCheatMask::none;
    Position jumpDestination = Position();
    uint32 nextTransportCheck = 0;
};

#endif
