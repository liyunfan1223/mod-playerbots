/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RANDOMITEMMGR_H
#define _PLAYERBOT_RANDOMITEMMGR_H

#include "AiFactory.h"
#include "Common.h"
#include "ItemTemplate.h"

#include <map>
#include <set>
#include <vector>

class ChatHandler;

struct ItemTemplate;

enum EquipmentSlots : uint32;

enum RandomItemType
{
    RANDOM_ITEM_GUILD_TASK,
    RANDOM_ITEM_GUILD_TASK_REWARD_EQUIP_BLUE,
    RANDOM_ITEM_GUILD_TASK_REWARD_EQUIP_GREEN,
    RANDOM_ITEM_GUILD_TASK_REWARD_TRADE,
    RANDOM_ITEM_GUILD_TASK_REWARD_TRADE_RARE
};

#define MAX_STAT_SCALES 32

enum ItemSource
{
    ITEM_SOURCE_NONE,
    ITEM_SOURCE_DROP,
    ITEM_SOURCE_VENDOR,
    ITEM_SOURCE_QUEST,
    ITEM_SOURCE_CRAFT,
    ITEM_SOURCE_PVP
};

struct WeightScaleInfo
{
    uint32 id;
    std::string name;
};

struct WeightScaleStat
{
    std::string stat;
    uint32 weight;
};

struct StatWeight
{
    uint32 id;
    uint32 weight;
};

struct ItemInfoEntry
{
    ItemInfoEntry() : minLevel(0), source(0), sourceId(0), team(0), repRank(0), repFaction(0), quality(0), slot(0), itemId(0)
    {
        for (uint8 i = 1; i <= MAX_STAT_SCALES; ++i)
        {
            weights[i] = 0;
        }
    }

    std::map<uint32, uint32> weights;
    uint32 minLevel;
    uint32 source;
    uint32 sourceId;
    uint32 team;
    uint32 repRank;
    uint32 repFaction;
    uint32 quality;
    uint32 slot;
    uint32 itemId;
};

typedef std::vector<WeightScaleStat> WeightScaleStats;
//typedef std::map<WeightScaleInfo, WeightScaleStats> WeightScaleList;

struct WeightScale
{
    WeightScaleInfo info;
    WeightScaleStats stats;
};

//typedef map<uint32, WeightScale> WeightScales;

class RandomItemPredicate
{
    public:
        virtual ~RandomItemPredicate() { };

        virtual bool Apply(ItemTemplate const* proto) = 0;
};

typedef std::vector<uint32> RandomItemList;
typedef std::map<RandomItemType, RandomItemList> RandomItemCache;

class BotEquipKey
{
    public:
        BotEquipKey() : level(0), clazz(0), slot(0), quality(0), key(GetKey()) { }
        BotEquipKey(uint32 level, uint8 clazz, uint8 slot, uint32 quality) : level(level), clazz(clazz), slot(slot), quality(quality), key(GetKey()) { }
        BotEquipKey(BotEquipKey const& other)  : level(other.level), clazz(other.clazz), slot(other.slot), quality(other.quality), key(GetKey()) { }

        bool operator<(BotEquipKey const& other) const
        {
            return other.key < this->key;
        }

        uint32 level;
        uint8 clazz;
        uint8 slot;
        uint32 quality;
        uint64 key;

    private:
        uint64 GetKey();
};

typedef std::map<BotEquipKey, RandomItemList> BotEquipCache;

class RandomItemMgr
{
    public:
        RandomItemMgr();
        virtual ~RandomItemMgr();
        static RandomItemMgr* instance()
        {
            static RandomItemMgr instance;
            return &instance;
        }

	public:
        void Init();
        void InitAfterAhBot();
        static bool HandleConsoleCommand(ChatHandler* handler, char const* args);
        RandomItemList Query(uint32 level, RandomItemType type, RandomItemPredicate* predicate);
        RandomItemList Query(uint32 level, uint8 clazz, uint8 slot, uint32 quality);
        uint32 GetUpgrade(Player* player, std::string spec, uint8 slot, uint32 quality, uint32 itemId);
        std::vector<uint32> GetUpgradeList(Player* player, std::string spec, uint8 slot, uint32 quality, uint32 itemId, uint32 amount = 1);
        bool HasStatWeight(uint32 itemId);
        uint32 GetMinLevelFromCache(uint32 itemId);
        uint32 GetStatWeight(Player* player, uint32 itemId);
        uint32 GetLiveStatWeight(Player* player, uint32 itemId);
        uint32 GetRandomItem(uint32 level, RandomItemType type, RandomItemPredicate* predicate = nullptr);
        uint32 GetAmmo(uint32 level, uint32 subClass);
        uint32 GetRandomPotion(uint32 level, uint32 effect);
        uint32 GetRandomFood(uint32 level, uint32 category);
        uint32 GetFood(uint32 level, uint32 category);
        uint32 GetRandomTrade(uint32 level);
        uint32 CalculateStatWeight(uint8 playerclass, uint8 spec, ItemTemplate const* proto);
        uint32 CalculateSingleStatWeight(uint8 playerclass, uint8 spec, std::string stat, uint32 value);
        bool CanEquipArmor(uint8 clazz, uint32 level, ItemTemplate const* proto);
        bool ShouldEquipArmorForSpec(uint8 playerclass, uint8 spec, ItemTemplate const* proto);
        bool CanEquipWeapon(uint8 clazz, ItemTemplate const* proto);
        bool ShouldEquipWeaponForSpec(uint8 playerclass, uint8 spec, ItemTemplate const* proto);
        float GetItemRarity(uint32 itemId);
        uint32 GetQuestIdForItem(uint32 itemId);
        std::vector<uint32> GetQuestIdsForItem(uint32 itemId);
        static bool IsUsedBySkill(ItemTemplate const* proto, uint32 skillId);
        bool IsTestItem(uint32 itemId) { return itemForTest.find(itemId) != itemForTest.end(); }
        std::vector<uint32> GetCachedEquipments(uint32 requiredLevel, uint32 inventoryType);

    private:
        void BuildRandomItemCache();
        void BuildEquipCache();
        void BuildEquipCacheNew();
        void BuildItemInfoCache();
        void BuildAmmoCache();
        void BuildFoodCache();
        void BuildPotionCache();
        void BuildTradeCache();
        void BuildRarityCache();
        bool CanEquipItem(BotEquipKey key, ItemTemplate const* proto);
        bool CanEquipItemNew(ItemTemplate const* proto);
        void AddItemStats(uint32 mod, uint8& sp, uint8& ap, uint8& tank);
        bool CheckItemStats(uint8 clazz, uint8 sp, uint8 ap, uint8 tank);
    private:
        std::map<uint32, RandomItemCache> randomItemCache;
        std::map<RandomItemType, RandomItemPredicate*> predicates;
        BotEquipCache equipCache;
        std::map<EquipmentSlots, std::set<InventoryType>> viableSlots;
        std::map<uint32, std::map<uint32, uint32> > ammoCache;
        std::map<uint32, std::map<uint32, std::vector<uint32> > > potionCache;
        std::map<uint32, std::map<uint32, std::vector<uint32> > > foodCache;
        std::map<uint32, std::vector<uint32> > tradeCache;
        std::map<uint32, float> rarityCache;
        std::map<uint8, WeightScale> m_weightScales[MAX_CLASSES];
        std::map<std::string, uint32 > weightStatLink;
        std::map<std::string, uint32 > weightRatingLink;
        std::map<uint32, ItemInfoEntry> itemInfoCache;
        std::set<uint32> itemForTest;
        static std::set<uint32> itemCache;
        // equipCacheNew[RequiredLevel][InventoryType]
        std::map<uint32, std::map<uint32, std::vector<uint32>>> equipCacheNew;
};

#define sRandomItemMgr RandomItemMgr::instance()

#endif
