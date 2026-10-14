/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RANDOMITEMMGR_H
#define PLAYERBOTS_RANDOMITEMMGR_H

#include "AiFactory.h"
#include "ItemTemplate.h"
#include "Player.h"
#include <array>
#include <atomic>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class ChatHandler;

struct ItemTemplate;

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
    ItemInfoEntry()
        : minLevel(0), source(0), sourceId(0), team(0), repRank(0), repFaction(0), quality(0), slot(0), itemId(0)
    {
        for (uint8 i = 1; i <= MAX_STAT_SCALES; ++i)
            weights[i] = 0;
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
// typedef std::map<WeightScaleInfo, WeightScaleStats> WeightScaleList;

struct WeightScale
{
    WeightScaleInfo info;
    WeightScaleStats stats;
};

// typedef map<uint32, WeightScale> WeightScales;

class RandomItemPredicate
{
public:
    virtual ~RandomItemPredicate() {};

    virtual bool Apply(ItemTemplate const* proto) = 0;
};

typedef std::vector<uint32> RandomItemList;
typedef std::unordered_map<RandomItemType, RandomItemList> RandomItemCache;

class BotEquipKey
{
public:
    BotEquipKey() : level(0), clazz(0), slot(0), quality(0), m_key(GetKey()) {}
    BotEquipKey(uint32 level, uint8 clazz, uint8 slot, uint32 quality)
        : level(level), clazz(clazz), slot(slot), quality(quality), m_key(GetKey())
    {
    }

    BotEquipKey(BotEquipKey const&) = default;
    BotEquipKey& operator=(BotEquipKey const&) = default;

    bool operator<(BotEquipKey const& other) const { return m_key < other.m_key; }
    bool operator==(BotEquipKey const& other) const { return m_key == other.m_key; }

    uint32 level;
    uint8 clazz;
    uint8 slot;
    uint32 quality;

private:
    uint64 m_key;

    uint64 GetKey() const
    {
        // bit layout - 8 bits per field, masked to prevent uint32 overflow bleed
        return (static_cast<uint64>(level   & 0xFFu) << 24) |
               (static_cast<uint64>(clazz   & 0xFFu) << 16) |
               (static_cast<uint64>(slot    & 0xFFu) <<  8) |
               (static_cast<uint64>(quality & 0xFFu));
    }
};

namespace std
{
    template<>
    struct hash<BotEquipKey>
    {
        public:
            std::size_t operator()(BotEquipKey const& key) const
            {
                // identical bit layout to BotEquipKey::GetKey()
                uint64 const packed =
                    (static_cast<uint64>(key.level   & 0xFFu) << 24) |
                    (static_cast<uint64>(key.clazz   & 0xFFu) << 16) |
                    (static_cast<uint64>(key.slot    & 0xFFu) <<  8) |
                    (static_cast<uint64>(key.quality & 0xFFu));

                return std::hash<uint64>()(packed);
            }
    };
}

typedef std::unordered_map<BotEquipKey, RandomItemList> BotEquipCache;
typedef std::unordered_map<InventoryType, RandomItemList> EquipByInventoryType;
typedef std::unordered_map<uint32, EquipByInventoryType> BotEquipCacheNew;

class RandomItemMgr
{
public:
    static RandomItemMgr& instance()
    {
        static RandomItemMgr instance;

        return instance;
    }

public:
    void Init();
    void InitAfterAhBot();

    [[nodiscard]] static bool HandleConsoleCommand(ChatHandler* handler, char const* args);

    [[nodiscard]] RandomItemList Query(uint32 level, RandomItemType type, RandomItemPredicate* predicate) const;

    [[nodiscard]] uint32 GetUpgrade(Player* player, std::string spec, uint8 slot, uint32 quality, uint32 itemId);
    [[nodiscard]] std::vector<uint32> GetUpgradeList(Player* player, std::string spec, uint8 slot, uint32 quality,
                                                     uint32 itemId, uint32 amount = 1);

    [[nodiscard]] bool HasStatWeight(uint32 itemId);
    [[nodiscard]] uint32 CalculateStatWeight(ItemTemplate const* proto, uint8 playerclass, uint8 spec);
    [[nodiscard]] uint32 CalculateSingleStatWeight(uint8 playerclass, uint8 spec, std::string stat, uint32 value);
    [[nodiscard]] uint32 GetStatWeight(Player* player, uint32 itemId);
    [[nodiscard]] uint32 GetLiveStatWeight(Player* player, uint32 itemId);

    [[nodiscard]] uint32 GetMinLevelFromCache(uint32 itemId) const;

    [[nodiscard]] RandomItemList const& GetEquipment(uint32 level, uint8 clazz, uint8 slot, uint32 quality) const;
    [[nodiscard]] RandomItemList const& GetEquipmentNew(uint32 level, InventoryType invType) const;
    [[nodiscard]] uint32 GetRandomItem(uint32 level, RandomItemType type, RandomItemPredicate* predicate = nullptr) const;
    [[nodiscard]] uint32 GetAmmo(uint32 level, uint32 subClass) const;
    [[nodiscard]] uint32 GetRandomPotion(uint32 level, uint32 effect) const;
    [[nodiscard]] uint32 GetRandomFood(uint32 level, uint32 category) const;
    [[nodiscard]] uint32 GetRandomTrade(uint32 level) const;
    [[nodiscard]] float GetItemRarity(uint32 itemId) const;
    [[nodiscard]] std::vector<uint32> const& GetEnchantmentPool(uint32 entry) const;

    [[nodiscard]] bool CanEquipArmor(ItemTemplate const* proto, uint8 clazz, uint32 level) const;
    [[nodiscard]] bool CanEquipWeapon(ItemTemplate const* proto, uint8 clazz) const;
    [[nodiscard]] bool ShouldEquipArmorForSpec(ItemTemplate const* proto, uint8 clazz, uint8 spec) const;
    [[nodiscard]] bool ShouldEquipWeaponForSpec(ItemTemplate const* proto, uint8 clazz, uint8 spec) const;

    [[nodiscard]] uint32 GetQuestIdForItem(uint32 itemId) const;
    [[nodiscard]] std::vector<uint32> GetQuestIdsForItem(uint32 itemId) const;

    [[nodiscard]] static bool IsInternalItem(ItemTemplate const* proto);
    [[nodiscard]] static bool IsValidItem(ItemTemplate const* proto);
    [[nodiscard]] static bool IsUsedBySkill(ItemTemplate const* proto, uint32 skillId);

private:
    void InitWeaponProficiency();
    void InitPredicates();
    void InitViableSlots();
    void InitWeightLinks();

    [[nodiscard]] bool LoadCacheEquip();
    [[nodiscard]] bool LoadCacheRandomItem();
    [[nodiscard]] bool LoadCacheRarity();
    void LoadEnchantmentPool();

    void BuildCacheRandomItem();
    void BuildCacheEquip();
    void BuildCacheEquipNew();
    void BuildCacheItemInfo();
    void BuildCacheAmmo();
    void BuildCacheFood();
    void BuildCachePotion();
    void BuildCacheTrade();
    void BuildCacheRarity();

    void DebugCacheRandomItem();

    static void AddItemStats(uint32 mod, uint8& sp, uint8& ap, uint8& tank);
    [[nodiscard]] static bool CanUseItemStats(uint8 clazz, uint8 sp, uint8 ap, uint8 tank);
    [[nodiscard]] bool CanEquipItem(ItemTemplate const* proto, uint32 level) const;
    [[nodiscard]] std::vector<EquipmentSlots> const* GetViableSlots(InventoryType invType) const;
    [[nodiscard]] uint32 NormalizeLevel(uint32 level) const;

private:
    RandomItemMgr();
    ~RandomItemMgr();

    RandomItemMgr(RandomItemMgr const&) = delete;
    RandomItemMgr& operator=(RandomItemMgr const&) = delete;

    RandomItemMgr(RandomItemMgr&&) = delete;
    RandomItemMgr& operator=(RandomItemMgr&&) = delete;

    std::atomic<bool> m_initialized{false};
    std::array<uint32, MAX_CLASSES> m_weaponProficiency = {};

    static std::unordered_set<uint32> itemCache;

    BotEquipCache equipCache;
    BotEquipCacheNew equipCacheNew;

    std::unordered_map<uint32, RandomItemCache> randomItemCache;
    std::unordered_map<RandomItemType, RandomItemPredicate*> predicates;
    std::unordered_map<InventoryType, std::vector<EquipmentSlots>> viableSlots;
    std::unordered_map<uint32, std::unordered_map<uint32, RandomItemList>> ammoCache;
    std::unordered_map<uint32, std::unordered_map<uint32, RandomItemList>> potionCache;
    std::unordered_map<uint32, std::unordered_map<uint32, RandomItemList>> foodCache;
    std::unordered_map<uint32, RandomItemList> tradeCache;
    std::unordered_map<uint32, float> rarityCache;
    std::unordered_map<uint8, WeightScale> m_weightScales[MAX_CLASSES];
    std::unordered_map<std::string, uint32> weightStatLink;
    std::unordered_map<std::string, uint32> weightRatingLink;
    std::unordered_map<uint32, ItemInfoEntry> itemInfoCache;

    // enchPoolCache[item_enchantment_template.entry] -> list of enchantment ids
    std::unordered_map<uint32, std::vector<uint32>> enchPoolCache;
};

#define sRandomItemMgr RandomItemMgr::instance()

#endif
