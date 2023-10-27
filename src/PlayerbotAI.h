/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PLAYERbotAI_H
#define _PLAYERBOT_PLAYERbotAI_H

#include "Chat.h"
#include "ChatHelper.h"
#include "ChatFilter.h"
#include "Common.h"
#include "Event.h"
#include "PlayerbotAIBase.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotSecurity.h"
#include "SpellAuras.h"
#include "WorldPacket.h"
#include "PlayerbotTextMgr.h"

#include <stack>
#include <queue>

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

enum HealingItemDisplayId
{
    HEALTHSTONE_DISPLAYID            = 8026,
    MAJOR_HEALING_POTION             = 24152,
    WHIPPER_ROOT_TUBER               = 21974,
    NIGHT_DRAGON_BREATH              = 21975,
    LIMITED_INVULNERABILITY_POTION   = 24213,
    GREATER_DREAMLESS_SLEEP_POTION   = 17403,
    SUPERIOR_HEALING_POTION          = 15714,
    CRYSTAL_RESTORE                  = 2516,
    DREAMLESS_SLEEP_POTION           = 17403,
    GREATER_HEALING_POTION           = 15713,
    HEALING_POTION                   = 15712,
    LESSER_HEALING_POTION            = 15711,
    DISCOLORED_HEALING_POTION        = 15736,
    MINOR_HEALING_POTION             = 15710,
    VOLATILE_HEALING_POTION          = 24212,
    SUPER_HEALING_POTION             = 37807,
    CRYSTAL_HEALING_POTION           = 47132,
    FEL_REGENERATION_POTION          = 37864,
    MAJOR_DREAMLESS_SLEEP_POTION     = 37845
};

enum BotState
{
    BOT_STATE_COMBAT        = 0,
    BOT_STATE_NON_COMBAT    = 1,
    BOT_STATE_DEAD          = 2,

    BOT_STATE_MAX
};

bool IsAlliance(uint8 race);

class PlayerbotChatHandler: protected ChatHandler
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
        MinValueCalculator(float def = 0.0f) : param(nullptr), minValue(def) { }

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

enum RoguePoisonDisplayId
{
    DEADLY_POISON_DISPLAYID      = 13707,
    INSTANT_POISON_DISPLAYID     = 13710,
    WOUND_POISON_DISPLAYID       = 37278
};

enum SharpeningStoneDisplayId
{
    ROUGH_SHARPENING_DISPLAYID       = 24673,
    COARSE_SHARPENING_DISPLAYID      = 24674,
    HEAVY_SHARPENING_DISPLAYID       = 24675,
    SOLID_SHARPENING_DISPLAYID       = 24676,
    DENSE_SHARPENING_DISPLAYID       = 24677,
    CONSECRATED_SHARPENING_DISPLAYID = 24674,    // will not be used because bot can not know if it will face undead targets
    ELEMENTAL_SHARPENING_DISPLAYID   = 21072,
    FEL_SHARPENING_DISPLAYID         = 39192,
    ADAMANTITE_SHARPENING_DISPLAYID  = 39193
};

enum WeightStoneDisplayId
{
    ROUGH_WEIGHTSTONE_DISPLAYID      = 24683,
    COARSE_WEIGHTSTONE_DISPLAYID     = 24684,
    HEAVY_WEIGHTSTONE_DISPLAYID      = 24685,
    SOLID_WEIGHTSTONE_DISPLAYID      = 24686,
    DENSE_WEIGHTSTONE_DISPLAYID      = 24687,
    FEL_WEIGHTSTONE_DISPLAYID        = 39548,
    ADAMANTITE_WEIGHTSTONE_DISPLAYID = 39549
};

enum WizardOilDisplayId
{
    MINOR_WIZARD_OIL         = 9731,
    LESSER_WIZARD_OIL        = 47903,
    BRILLIANT_WIZARD_OIL     = 47901,
    WIZARD_OIL               = 47905,
    SUPERIOR_WIZARD_OIL      = 47904,
  /// Blessed Wizard Oil    = 26865 //scourge inv
};

enum ManaOilDisplayId
{
    MINOR_MANA_OIL           = 34492,
    LESSER_MANA_OIL          = 47902,
    BRILLIANT_MANA_OIL       = 41488,
    SUPERIOR_MANA_OIL        = 36862
};

enum ShieldWardDisplayId
{
    LESSER_WARD_OFSHIELDING  = 38759,
    GREATER_WARD_OFSHIELDING = 38760
};

enum class BotTypeNumber : uint8
{
    ACTIVITY_TYPE_NUMBER = 1,
    GROUPER_TYPE_NUMBER  = 2,
    GUILDER_TYPE_NUMBER  = 3,
};

enum class GrouperType : uint8
{
    SOLO        = 0,
    MEMBER      = 1,
    LEADER_2    = 2,
    LEADER_3    = 3,
    LEADER_4    = 4,
    LEADER_5    = 5
};

enum class GuilderType : uint8
{
    SOLO   = 0,
    TINY   = 30,
    SMALL  = 50,
    MEDIUM = 70,
    LARGE  = 120,
    HUGE   = 250
};

enum ActivityType
{
    GRIND_ACTIVITY          = 1,
    RPG_ACTIVITY            = 2,
    TRAVEL_ACTIVITY         = 3,
    OUT_OF_PARTY_ACTIVITY   = 4,
    PACKET_ACTIVITY         = 5,
    DETAILED_MOVE_ACTIVITY  = 6,
    PARTY_ACTIVITY          = 7,
    ALL_ACTIVITY            = 8,

    MAX_ACTIVITY_TYPE
};

enum BotRoles : uint8
{
    BOT_ROLE_NONE   = 0x00,
    BOT_ROLE_TANK   = 0x01,
    BOT_ROLE_HEALER = 0x02,
    BOT_ROLE_DPS    = 0x04
};

enum HUNTER_TABS {
    HUNTER_TAB_BEASTMASTER,
    HUNTER_TAB_MARKSMANSHIP,
    HUNTER_TAB_SURVIVAL,
};

enum ROGUE_TABS {
    ROGUE_TAB_ASSASSINATION,
    ROGUE_TAB_COMBAT,
    ROGUE_TAB_SUBTLETY
};

enum PRIEST_TABS {
    PRIEST_TAB_DISIPLINE,
    PRIEST_TAB_HOLY,
    PRIEST_TAB_SHADOW,
};

enum DEATHKNIGT_TABS {
    DEATHKNIGT_TAB_BLOOD,
    DEATHKNIGT_TAB_FROST,
    DEATHKNIGT_TAB_UNHOLY,
};

enum DRUID_TABS {
    DRUID_TAB_BALANCE,
    DRUID_TAB_FERAL,
    DRUID_TAB_RESTORATION,
};

enum MAGE_TABS {
    MAGE_TAB_ARCANE,
    MAGE_TAB_FIRE,
    MAGE_TAB_FROST,
};

enum SHAMAN_TABS {
    SHAMAN_TAB_ELEMENTAL,
    SHAMAN_TAB_ENHANCEMENT,
    SHAMAN_TAB_RESTORATION,
};

enum PALADIN_TABS {
    PALADIN_TAB_HOLY,
    PALADIN_TAB_PROTECTION,
    PALADIN_TAB_RETRIBUTION,
};

enum WARLOCK_TABS {
    WARLOCK_TAB_AFFLICATION,
    WARLOCK_TAB_DEMONOLOGY,
    WARLOCK_TAB_DESTRUCTION,
};

enum WARRIOR_TABS {
    WARRIOR_TAB_ARMS,
    WARRIOR_TAB_FURY,
    WARRIOR_TAB_PROTECTION,
};

class PacketHandlingHelper
{
    public:
        void AddHandler(uint16 opcode, std::string const handler);
        void Handle(ExternalEventHelper &helper);
        void AddPacket(WorldPacket const& packet);

    private:
        std::map<uint16, std::string> handlers;
        std::stack<WorldPacket> queue;
};

class ChatCommandHolder
{
    public:
        ChatCommandHolder(std::string const command, Player* owner = nullptr, uint32 type = CHAT_MSG_WHISPER, time_t time = 0) : command(command), owner(owner), type(type), time(time) { }
        ChatCommandHolder(ChatCommandHolder const& other) : command(other.command), owner(other.owner), type(other.type), time(other.time) { }

        std::string const GetCommand() { return command; }
        Player* GetOwner() { return owner; }
        uint32 GetType() { return type; }
        time_t GetTime() { return time; }

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
        void QueueChatResponse(uint8 msgtype, ObjectGuid guid1, ObjectGuid guid2, std::string message, std::string chanName, std::string name);
	    void HandleBotOutgoingPacket(WorldPacket const& packet);
        void HandleMasterIncomingPacket(WorldPacket const& packet);
        void HandleMasterOutgoingPacket(WorldPacket const& packet);
	    void HandleTeleportAck();
        void ChangeEngine(BotState type);
        void DoNextAction(bool minimal = false);
        virtual bool DoSpecificAction(std::string const name, Event event = Event(), bool silent = false, std::string const qualifier = "");
        void ChangeStrategy(std::string const name, BotState type);
        void ClearStrategies(BotState type);
        std::vector<std::string> GetStrategies(BotState type);
        bool ContainsStrategy(StrategyType type);
        bool HasStrategy(std::string const name, BotState type);
        BotState GetState() { return currentState; };
        void ResetStrategies(bool load = false);
        void ReInitCurrentEngine();
        void Reset(bool full = false);
        bool IsTank(Player* player);
        bool IsHeal(Player* player);
        bool IsDps(Player* player);
        bool IsRanged(Player* player);
        bool IsRangedDps(Player* player);
        bool IsMainTank(Player* player);
        bool IsAssistTank(Player* player);
        bool IsAssistTankOfIndex(Player* player, int index);
        bool IsHealAssistantOfIndex(Player* player, int index);
        bool IsRangedDpsAssistantOfIndex(Player* player, int index);
        bool HasAggro(Unit* unit);
        int32 GetGroupSlotIndex(Player* player);
        int32 GetRangedIndex(Player* player);
        int32 GetClassIndex(Player* player, uint8_t cls);
        int32 GetRangedDpsIndex(Player* player);
        int32 GetMeleeIndex(Player* player);

        Creature* GetCreature(ObjectGuid guid);
        Unit* GetUnit(ObjectGuid guid);
        Player* GetPlayer(ObjectGuid guid);
        static Unit* GetUnit(CreatureData const* creatureData);
        GameObject* GetGameObject(ObjectGuid guid);
        static GameObject* GetGameObject(GameObjectData const* gameObjectData);
        WorldObject* GetWorldObject(ObjectGuid guid);
        bool TellMaster(std::ostringstream& stream, PlayerbotSecurityLevel securityLevel = PLAYERBOT_SECURITY_ALLOW_ALL);
        bool TellMaster(std::string const text, PlayerbotSecurityLevel securityLevel = PLAYERBOT_SECURITY_ALLOW_ALL);
        bool TellMasterNoFacing(std::ostringstream& stream, PlayerbotSecurityLevel securityLevel = PLAYERBOT_SECURITY_ALLOW_ALL);
        bool TellMasterNoFacing(std::string const text, PlayerbotSecurityLevel securityLevel = PLAYERBOT_SECURITY_ALLOW_ALL);
        bool TellError(std::string const text, PlayerbotSecurityLevel securityLevel = PLAYERBOT_SECURITY_ALLOW_ALL);
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
        Item* FindBandage() const;
        Item* FindConsumable(uint32 displayId) const;
        Item* FindStoneFor(Item* weapon) const;
        Item* FindOilFor(Item* weapon) const;
        void ImbueItem(Item* item, uint32 targetFlag, ObjectGuid targetGUID);
        void ImbueItem(Item* item, uint8 targetInventorySlot);
        void ImbueItem(Item* item, Unit* target);
        void ImbueItem(Item* item);
        void EnchantItemT(uint32 spellid, uint8 slot);
        uint32 GetBuffedCount(Player* player, std::string const spellname);

        virtual bool CanCastSpell(std::string const name, Unit* target, Item* itemTarget = nullptr);
        virtual bool CastSpell(std::string const name, Unit* target, Item* itemTarget = nullptr);
        virtual bool HasAura(std::string const spellName, Unit* player, bool maxStack = false, bool checkIsOwner = false, int maxAmount = -1, bool checkDuration = false);
        virtual bool HasAnyAuraOf(Unit* player, ...);

        virtual bool IsInterruptableSpellCasting(Unit* player, std::string const spell);
        virtual bool HasAuraToDispel(Unit* player, uint32 dispelType);
        bool CanCastSpell(uint32 spellid, Unit* target, bool checkHasSpell = true, Item* itemTarget = nullptr);
        bool CanCastSpell(uint32 spellid, GameObject* goTarget, uint8 effectMask, bool checkHasSpell = true);
        bool CanCastSpell(uint32 spellid, float x, float y, float z, uint8 effectMask, bool checkHasSpell = true, Item* itemTarget = nullptr);

        bool HasAura(uint32 spellId, Unit const* player);
        Aura* GetAura(std::string const spellName, Unit* unit, bool checkIsOwner = false, bool checkDuration = false, int checkStack = -1);
        bool CastSpell(uint32 spellId, Unit* target, Item* itemTarget = nullptr);
        bool CastSpell(uint32 spellId, float x, float y, float z, Item* itemTarget = nullptr);
        bool canDispel(SpellInfo const* spellInfo, uint32 dispelType);

        bool CanCastVehicleSpell(uint32 spellid, Unit* target);
        bool CastVehicleSpell(uint32 spellId, Unit* target);
        bool CastVehicleSpell(uint32 spellId, float x, float y, float z);
        bool IsInVehicle(bool canControl = false, bool canCast = false, bool canAttack = false, bool canTurn = false, bool fixed = false);

        uint32 GetEquipGearScore(Player* player, bool withBags, bool withBank);
        static uint32 GetMixedGearScore(Player* player, bool withBags, bool withBank, uint32 topN = 0);
        bool HasSkill(SkillType skill);
        bool IsAllowedCommand(std::string const text);
        float GetRange(std::string const type);

        Player* GetBot() { return bot; }
        Player* GetMaster() { return master; }

        //Checks if the bot is really a player. Players always have themselves as master.
        bool IsRealPlayer() { return master ? (master == bot) : false; }
        //Bot has a master that is a player.
        bool HasRealPlayerMaster();
        //Bot has a master that is activly playing.
        bool HasActivePlayerMaster();
        //Get the group leader or the master of the bot.
        //Checks if the bot is summoned as alt of a player
        bool IsAlt();
        Player* GetGroupMaster();
        //Returns a semi-random (cycling) number that is fixed for each bot.
        uint32 GetFixedBotNumer(BotTypeNumber typeNumber, uint32 maxNum = 100, float cyclePerMin = 1);
        GrouperType GetGrouperType();
        GuilderType GetGuilderType();
        bool HasPlayerNearby(WorldPosition* pos, float range = sPlayerbotAIConfig->reactDistance);
        bool HasPlayerNearby(float range = sPlayerbotAIConfig->reactDistance);
        bool HasManyPlayersNearby(uint32 trigerrValue = 20, float range = sPlayerbotAIConfig->sightDistance);
        bool AllowActive(ActivityType activityType);
        bool AllowActivity(ActivityType activityType = ALL_ACTIVITY, bool checkNow = false);

        bool HasCheat(BotCheatMask mask) { return ((uint32)mask & (uint32)cheatMask) != 0 || ((uint32)mask & (uint32)sPlayerbotAIConfig->botCheatMask) != 0; }
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
        bool IsInRealGuild();
        static std::vector<std::string> dispel_whitelist;
        bool EqualLowercaseName(std::string s1, std::string s2);
    private:
        static void _fillGearScoreData(Player* player, Item* item, std::vector<uint32>* gearScore, uint32& twoHandScore, bool mixed = false);
        bool IsTellAllowed(PlayerbotSecurityLevel securityLevel = PLAYERBOT_SECURITY_ALLOW_ALL);
    protected:
	    Player* bot;
	    Player* master;
	    uint32 accountId;
        AiObjectContext* aiObjectContext;
        Engine* currentEngine;
        Engine* engines[BOT_STATE_MAX];
        BotState currentState;
        ChatHelper chatHelper;
        std::queue<ChatCommandHolder> chatCommands;
        std::queue<ChatQueuedReply> chatReplies;
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
};

#endif
