/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PvpGearListMgr.h"
#include "AiFactory.h"
#include "AreaDefines.h"
#include "BisListMgr.h"
#include "DBCStores.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotFactory.h"
#include "PvpLoadoutLog.h"
#include "PvpLoadoutStats.h"
#include "RandomItemMgr.h"
#include "StatsWeightCalculator.h"
#include <algorithm>
#include <array>
#include <iterator>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <unordered_set>

namespace
{
uint32 VendorRequiredRating(uint32 extendedCost)
{
    if (!extendedCost)
        return 0;

    ItemExtendedCostEntry const* cost = sItemExtendedCostStore.LookupEntry(extendedCost);
    return cost ? cost->reqpersonalarenarating : 0;
}

// Honor or arena points: PvP gear without resilience, like the level-60 sets and the Gladiator relics, still costs one.
bool CostsPvpCurrency(uint32 extendedCost)
{
    ItemExtendedCostEntry const* cost = extendedCost ? sItemExtendedCostStore.LookupEntry(extendedCost) : nullptr;
    return cost && (cost->reqhonorpoints || cost->reqarenapoints);
}

bool IsGear(ItemTemplate const& proto)
{
    return (proto.Class == ITEM_CLASS_ARMOR || proto.Class == ITEM_CLASS_WEAPON) &&
           proto.InventoryType != INVTYPE_NON_EQUIP;
}

// GM Island (zone 876) sits in Kalimdor's far corner, beyond every other spawn on the map.
constexpr float GM_ISLAND_MIN_COORD = 16000.0f;

// Its vendors sell almost every item, some without the currency or rating the live vendors charge.
bool OnGmIsland(CreatureData const& spawn)
{
    return spawn.mapid == MAP_KALIMDOR && spawn.posX > GM_ISLAND_MIN_COORD && spawn.posY > GM_ISLAND_MIN_COORD;
}

// The slots a PvP set covers; a loadout keeps four of them from one set.
constexpr std::array<uint8, 5> SET_SLOTS = {EQUIPMENT_SLOT_HEAD, EQUIPMENT_SLOT_SHOULDERS, EQUIPMENT_SLOT_CHEST,
                                            EQUIPMENT_SLOT_LEGS, EQUIPMENT_SLOT_HANDS};
constexpr uint32 SET_PIECES_KEPT = 4;
// The ranking already orders items for the spec, so the EP choice only needs its best few eligible ones.
constexpr size_t PVP_CANDIDATES_PER_SLOT = 8;
constexpr size_t WEAPON_PAIR_CANDIDATES = 3;

// Faction-only items stay in the ranking; CanWear drops them per bot when its gear is picked.
uint32 RankingKey(Player* bot, int32 specNo)
{
    return uint32(bot->getClass()) | uint32(uint8(specNo)) << 8 | uint32(bot->GetLevel()) << 16;
}

// The talent tree, not the PvP premade, because trees without a premade still differ in their PvE items. Faction,
// because CanWear drops the other faction's items.
uint64 LoadoutKey(Player* bot, uint8 tab, uint32 pveItemLevel, uint32 unlockedRequirement)
{
    return uint64(bot->getClass()) | uint64(tab) << 8 | uint64(bot->GetLevel()) << 16 | uint64(bot->GetTeamId()) << 24 |
           uint64(pveItemLevel) << 32 | uint64(unlockedRequirement) << 48;
}

struct BuildContext
{
    Player* bot;
    uint32 rating;
    int32 specNo;
    PvpLoadout::RatingRules rules;
    PvpLoadout::GearLimits limits;
    PvpLoadout::Role role;
    std::map<uint8, uint32> pveItems;
};

bool IsUnique(ItemTemplate const* proto) { return proto->MaxCount == 1 || proto->HasFlag(ITEM_FLAG_UNIQUE_EQUIPPABLE); }

std::unordered_set<uint32> SlotTypes(uint8 slot)
{
    std::vector<InventoryType> const types = PlayerbotFactory::GetPossibleInventoryTypeListBySlot(EquipmentSlots(slot));
    return {types.begin(), types.end()};
}

// The heaviest armor the bot is proficient in. RandomItemMgr::CanEquipArmor has no case for death knights and would
// hold them to cloth.
uint32 BodyArmorSubclass(Player* bot)
{
    if (bot->HasSkill(SKILL_PLATE_MAIL))
        return ITEM_SUBCLASS_ARMOR_PLATE;

    if (bot->HasSkill(SKILL_MAIL))
        return ITEM_SUBCLASS_ARMOR_MAIL;

    return bot->HasSkill(SKILL_LEATHER) ? ITEM_SUBCLASS_ARMOR_LEATHER : ITEM_SUBCLASS_ARMOR_CLOTH;
}

// Whether the bot's class could wear the item in the slot at this level. Body armor must be the heaviest type the class
// wears; jewelry, cloaks, shields and relics are left to CanWear.
bool FitsClass(ItemTemplate const* proto, uint8 slot, std::unordered_set<uint32> const& slotTypes, Player* bot)
{
    if (!slotTypes.count(proto->InventoryType) || proto->RequiredLevel > bot->GetLevel() ||
        !(proto->AllowableClass & bot->getClassMask()))
        return false;

    if (proto->Class == ITEM_CLASS_ARMOR && PlayerbotFactory::IsBodyArmorSlot(slot) &&
        proto->SubClass != BodyArmorSubclass(bot))
        return false;

    return proto->Class != ITEM_CLASS_WEAPON || sRandomItemMgr.CanEquipWeapon(proto, bot->getClass());
}

// CanUseItem plus the weapon, armor and shield proficiency it leaves to equipping; without it a hunter is offered
// shields it then cannot put on.
bool CanWear(Player* bot, ItemTemplate const* proto)
{
    uint32 const skill = proto->GetSkill();
    return bot->CanUseItem(proto) == EQUIP_ERR_OK && (!skill || bot->GetSkillValue(skill));
}

PvpGearListMgr::LoadoutItem ToLoadoutItem(BuildContext const& ctx, uint8 slot, ItemTemplate const* proto,
                                          char const* source, uint32 requiredRating)
{
    return {slot,           proto->ItemId,   source,
            requiredRating, IsUnique(proto), PvpLoadoutStats::ItemStats(ctx.bot, ctx.role, proto)};
}

// The eligible PvP items for a slot that pass filter, best ranked first.
template <typename Filter>
std::vector<PvpGearListMgr::LoadoutItem> EligiblePvp(BuildContext const& ctx, uint8 slot, size_t count,
                                                     Filter const& filter)
{
    std::vector<PvpGearListMgr::LoadoutItem> eligible;
    if (ctx.specNo == PvpLoadout::NO_SPEC)
        return eligible;

    for (PvpLoadout::ItemCandidate const& candidate : PvpGearListMgr::instance().GetRanked(ctx.bot, ctx.specNo, slot))
    {
        if (eligible.size() >= count)
            break;

        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(candidate.itemId);
        if (filter(proto) && PvpLoadout::WithinLimits(candidate, ctx.limits) &&
            PvpLoadout::IsPvpEligible(candidate, ctx.rating, ctx.rules) && CanWear(ctx.bot, proto))
            eligible.push_back(ToLoadoutItem(ctx, slot, proto, "pvp", candidate.requiredRating));
    }
    return eligible;
}

std::vector<PvpGearListMgr::LoadoutItem> EligiblePvp(BuildContext const& ctx, uint8 slot)
{
    return EligiblePvp(ctx, slot, PVP_CANDIDATES_PER_SLOT, [](ItemTemplate const*) { return true; });
}

// The highest BiS tier at or below the curve's item level whose items the bot can mostly wear. Below the level the
// curve is written for, its tiers need a higher level, so this is the best tier of the bot's bracket.
std::map<uint8, uint32> PveItems(Player* bot, uint32 curveItemLevel)
{
    uint8 const cls = bot->getClass();
    uint8 const tab = AiFactory::GetPlayerSpecTab(bot);
    uint8 const faction = bot->GetTeamId() == TEAM_ALLIANCE ? 1 : 2;
    std::vector<uint16> const tiers = sBisListMgr->GetTiers(cls, tab);
    for (auto tier = tiers.rbegin(); tier != tiers.rend(); ++tier)
    {
        if (*tier > curveItemLevel)
            continue;

        std::map<uint8, uint32> items = sBisListMgr->GetBisFor(*tier, cls, tab, faction);
        // Most, not all: the table has rows above their tier, which PveItem drops per slot.
        size_t const wearable = std::count_if(items.begin(), items.end(),
                                              [bot](auto const& item)
                                              {
                                                  ItemTemplate const* proto = sObjectMgr->GetItemTemplate(item.second);
                                                  return proto && proto->RequiredLevel <= bot->GetLevel();
                                              });
        if (wearable * 2 > items.size())
            return items;
    }

    return {};
}

// The curve's PvE item for a slot, if usable. The BiS table has rows above their tier, in the wrong slot and of the
// wrong armor type, so it gets the same class checks as the PvP pool.
std::optional<PvpGearListMgr::LoadoutItem> PveItem(BuildContext const& ctx, uint8 slot)
{
    auto const it = ctx.pveItems.find(slot);
    if (it == ctx.pveItems.end())
        return std::nullopt;

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(it->second);
    if (!proto || proto->ItemLevel > ctx.rules.freeItemLevelCap || !FitsClass(proto, slot, SlotTypes(slot), ctx.bot) ||
        !PvpLoadout::WithinLimits(PvpGearListMgr::ToCandidate(proto, 0), ctx.limits) || !CanWear(ctx.bot, proto))
        return std::nullopt;

    return ToLoadoutItem(ctx, slot, proto, "pve", 0);
}

// The picks one decision chooses from.
struct GroupBuilder
{
    std::vector<uint8> slots;
    std::vector<PvpGearListMgr::Pick> picks;

    void Add(std::vector<PvpGearListMgr::LoadoutItem> items)
    {
        PvpLoadout::StatVector stats{};
        for (PvpGearListMgr::LoadoutItem const& item : items)
            stats = PvpLoadout::Add(stats, item.stats);
        picks.push_back({std::move(items), slots, stats, false});
    }
};

// pvp: the slot's eligible PvP items, already walked for the set.
GroupBuilder SlotGroup(BuildContext const& ctx, uint8 slot, std::vector<PvpGearListMgr::LoadoutItem> const& pvp)
{
    GroupBuilder builder{{slot}, {}};
    for (PvpGearListMgr::LoadoutItem const& item : pvp)
        builder.Add({item});

    if (std::optional<PvpGearListMgr::LoadoutItem> pve = PveItem(ctx, slot))
        builder.Add({*pve});

    return builder;
}

// Without Titan's Grip, main hand and off-hand are one decision: a two-hander, or a main hand plus an off-hand.
GroupBuilder WeaponGroup(BuildContext const& ctx)
{
    Player* bot = ctx.bot;
    auto const isTwoHander = [](ItemTemplate const* proto) { return proto->InventoryType == INVTYPE_2HWEAPON; };
    auto const isMainHand = [](ItemTemplate const* proto)
    { return proto->InventoryType == INVTYPE_WEAPON || proto->InventoryType == INVTYPE_WEAPONMAINHAND; };
    auto const isOffHand = [bot](ItemTemplate const* proto)
    {
        return proto->InventoryType == INVTYPE_SHIELD || proto->InventoryType == INVTYPE_HOLDABLE ||
               ((proto->InventoryType == INVTYPE_WEAPONOFFHAND || proto->InventoryType == INVTYPE_WEAPON) &&
                bot->CanDualWield());
    };

    std::vector<PvpGearListMgr::LoadoutItem> twoHanders =
        EligiblePvp(ctx, EQUIPMENT_SLOT_MAINHAND, PVP_CANDIDATES_PER_SLOT, isTwoHander);
    std::vector<PvpGearListMgr::LoadoutItem> mainHands =
        EligiblePvp(ctx, EQUIPMENT_SLOT_MAINHAND, WEAPON_PAIR_CANDIDATES, isMainHand);
    std::vector<PvpGearListMgr::LoadoutItem> offHands =
        EligiblePvp(ctx, EQUIPMENT_SLOT_OFFHAND, WEAPON_PAIR_CANDIDATES, isOffHand);

    if (std::optional<PvpGearListMgr::LoadoutItem> pve = PveItem(ctx, EQUIPMENT_SLOT_MAINHAND))
        (isTwoHander(sObjectMgr->GetItemTemplate(pve->itemId)) ? twoHanders : mainHands).push_back(*pve);
    if (std::optional<PvpGearListMgr::LoadoutItem> pve = PveItem(ctx, EQUIPMENT_SLOT_OFFHAND))
        if (isOffHand(sObjectMgr->GetItemTemplate(pve->itemId)))
            offHands.push_back(*pve);

    GroupBuilder builder{{EQUIPMENT_SLOT_MAINHAND, EQUIPMENT_SLOT_OFFHAND}, {}};
    for (PvpGearListMgr::LoadoutItem const& twoHander : twoHanders)
        builder.Add({twoHander});

    for (PvpGearListMgr::LoadoutItem const& mainHand : mainHands)
    {
        if (offHands.empty())
            builder.Add({mainHand});

        for (PvpGearListMgr::LoadoutItem const& offHand : offHands)
            if (mainHand.itemId != offHand.itemId || !mainHand.unique)
                builder.Add({mainHand, offHand});
    }

    return builder;
}

void LogLoadout(Player* bot, uint32 pveItemLevel, uint32 unlockedRequirement, PvpGearListMgr::Loadout const& loadout)
{
    std::ostringstream items;
    PvpLoadout::StatVector totals{};
    for (PvpGearListMgr::Pick const& pick : loadout.picks)
    {
        totals = PvpLoadout::Add(totals, pick.stats);
        for (PvpGearListMgr::LoadoutItem const& item : pick.items)
            items << " " << uint32(item.slot) << ":" << item.itemId << "("
                  << sObjectMgr->GetItemTemplate(item.itemId)->ItemLevel << "," << item.source << ","
                  << item.requiredRating << ")";
    }

    // slot:item(item level, source, required rating), then the item totals against the role's targets
    PvpLoadout::LogDebug(
        "PvP loadout for class {} tree {} level {} team {}, PvE item level {}, rating requirement {}:{}; {}",
        uint32(bot->getClass()), uint32(AiFactory::GetPlayerSpecTab(bot)), bot->GetLevel(), uint32(bot->GetTeamId()),
        pveItemLevel, unlockedRequirement, items.str(),
        PvpLoadout::FormatTargets(totals, sPlayerbotAIConfig.PvpProfiles[static_cast<size_t>(loadout.role)]));
}
}  // namespace

void PvpGearListMgr::LoadAll()
{
    if (_loaded)
        return;

    _loaded = true;

    // Only vendors a player can reach: spawned somewhere, and not only on GM Island.
    std::unordered_set<uint32> liveVendors;
    for (auto const& [spawnId, spawn] : sObjectMgr->GetAllCreatureData())
        if (!OnGmIsland(spawn))
            for (uint32 entry : {spawn.id, spawn.id2, spawn.id3})
                if (entry)
                    liveVendors.insert(entry);

    // PvP gear has resilience or costs honor or arena points at some vendor; every vendor row of it then counts for
    // its rating requirement.
    std::unordered_map<uint32, ItemTemplate const*> pvpItems;
    std::vector<PvpLoadout::VendorCost> gearCosts;
    for (auto const& [entry, creature] : *sObjectMgr->GetCreatureTemplates())
    {
        if (!(creature.npcflag & UNIT_NPC_FLAG_VENDOR) || !liveVendors.count(entry))
            continue;

        VendorItemData const* vendorItems = sObjectMgr->GetNpcVendorItemList(entry);
        if (!vendorItems)
            continue;

        for (VendorItem const* vendorItem : vendorItems->m_items)
        {
            ItemTemplate const* proto = vendorItem ? sObjectMgr->GetItemTemplate(vendorItem->item) : nullptr;
            if (!proto || !IsGear(*proto))
                continue;

            if (proto->HasStat(ITEM_MOD_RESILIENCE_RATING) || CostsPvpCurrency(vendorItem->ExtendedCost))
                pvpItems.emplace(proto->ItemId, proto);

            gearCosts.push_back(
                {vendorItem->item, VendorRequiredRating(vendorItem->ExtendedCost), vendorItem->ExtendedCost == 0});
        }
    }

    std::vector<PvpLoadout::VendorCost> vendorCosts;
    std::copy_if(gearCosts.begin(), gearCosts.end(), std::back_inserter(vendorCosts),
                 [&pvpItems](PvpLoadout::VendorCost const& cost) { return pvpItems.count(cost.itemId) != 0; });

    std::set<uint32> requirements;
    for (auto const& [itemId, requiredRating] : PvpLoadout::ResolveRequiredRatings(vendorCosts))
    {
        _pool.push_back({pvpItems[itemId], requiredRating});
        if (requiredRating)
            requirements.insert(requiredRating);
    }
    _requirements.assign(requirements.begin(), requirements.end());

    LOG_INFO("server.loading", "Loaded {} PvP gear items sold by vendors", static_cast<uint32>(_pool.size()));
}

std::vector<PvpLoadout::ItemCandidate> const& PvpGearListMgr::GetRanked(Player* bot, int32 specNo, uint8 slot)
{
    uint32 const key = RankingKey(bot, specNo);
    {
        std::lock_guard<std::mutex> lock(_cacheMutex);
        auto const it = _ranked.find(key);
        if (it != _ranked.end())
            return it->second[slot];
    }

    // Ranked outside the lock; two bots racing on a new key compute the same ranking and the first insert wins.
    RankedSlots ranked = Rank(bot);

    std::lock_guard<std::mutex> lock(_cacheMutex);
    return _ranked.emplace(key, std::move(ranked)).first->second[slot];
}

PvpGearListMgr::RankedSlots PvpGearListMgr::Rank(Player* bot) const
{
    RankedSlots ranked(EQUIPMENT_SLOT_END);

    // Role by spec, and no overflow or set bonuses: strategies and worn gear vary per bot, the cache key does not.
    StatsWeightCalculator calculator(bot, true);
    calculator.SetPvpSpec(true);
    calculator.SetOverflowPenalty(false);
    calculator.SetItemSetBonus(false);

    for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        if (slot == EQUIPMENT_SLOT_BODY || slot == EQUIPMENT_SLOT_TABARD)
            continue;

        std::unordered_set<uint32> const slotTypes = SlotTypes(slot);
        std::vector<std::pair<float, PvpLoadout::ItemCandidate>> scored;
        for (PoolItem const& item : _pool)
        {
            ItemTemplate const* proto = item.proto;
            if (!FitsClass(proto, slot, slotTypes, bot))
                continue;

            float const score = calculator.CalculateItem(proto->ItemId, 0, slot);
            if (score <= 0.0f)
                continue;

            PvpLoadout::ItemCandidate const candidate = ToCandidate(proto, item.requiredRating);
            scored.emplace_back(score, candidate);
        }

        std::sort(scored.begin(), scored.end(),
                  [](auto const& lhs, auto const& rhs)
                  {
                      if (lhs.first != rhs.first)
                          return lhs.first > rhs.first;
                      return lhs.second.itemLevel > rhs.second.itemLevel;
                  });

        for (auto const& [score, candidate] : scored)
            ranked[slot].push_back(candidate);
    }

    return ranked;
}

PvpLoadout::ItemCandidate PvpGearListMgr::ToCandidate(ItemTemplate const* proto, uint32 requiredRating)
{
    return {proto->ItemId,  proto->ItemLevel,
            proto->Quality, proto->RequiredLevel,
            requiredRating, PlayerbotFactory::CalcMixedGearScore(proto->ItemLevel, proto->Quality)};
}

PvpGearListMgr::Loadout const& PvpGearListMgr::GetLoadout(Player* bot, int32 specNo, uint32 rating)
{
    PvpLoadout::RatingRules const rules = {
        sPlayerbotAIConfig.PvpLoadoutMinRating, sPlayerbotAIConfig.PvpLoadoutRatingMargin,
        PvpLoadout::CurveItemLevel(sPlayerbotAIConfig.PvpLoadoutPveIlvlCurve, rating)};
    uint32 const unlocked = PvpLoadout::UnlockedRequirement(_requirements, rating, rules);
    uint64 const key = LoadoutKey(bot, AiFactory::GetPlayerSpecTab(bot), rules.freeItemLevelCap, unlocked);
    {
        std::lock_guard<std::mutex> lock(_cacheMutex);
        auto const it = _loadouts.find(key);
        if (it != _loadouts.end())
            return it->second;
    }

    // Built outside the lock, like the ranking. Every rating in the tier sees the same eligible items, so building
    // with this bot's rating gives the tier's loadout.
    Loadout loadout = BuildLoadout(bot, specNo, rating, rules);
    LogLoadout(bot, rules.freeItemLevelCap, unlocked, loadout);

    std::lock_guard<std::mutex> lock(_cacheMutex);
    return _loadouts.emplace(key, std::move(loadout)).first->second;
}

PvpGearListMgr::Loadout PvpGearListMgr::BuildLoadout(Player* bot, int32 specNo, uint32 rating,
                                                     PvpLoadout::RatingRules const& rules)
{
    uint32 const qualityLimit = sPlayerbotAIConfig.PvpLoadoutQualityLimit >= 0
                                    ? sPlayerbotAIConfig.PvpLoadoutQualityLimit
                                    : sPlayerbotAIConfig.RandomGearQualityLimit;
    uint32 const scoreLimit = sPlayerbotAIConfig.PvpLoadoutScoreLimit >= 0 ? sPlayerbotAIConfig.PvpLoadoutScoreLimit
                                                                           : sPlayerbotAIConfig.RandomGearScoreLimit;
    BuildContext const ctx{
        bot,
        rating,
        specNo,
        rules,
        {qualityLimit, scoreLimit ? PlayerbotFactory::CalcMixedGearScore(scoreLimit, qualityLimit) : 0,
         bot->GetLevel()},
        PvpLoadoutStats::RoleOf(bot),
        PveItems(bot, rules.freeItemLevelCap)};

    Loadout loadout{ctx.role, {}};

    // A four-piece of the best season the rating allows. The walks are kept for the set slots it leaves.
    std::map<uint8, std::vector<LoadoutItem>> setSlotItems;
    std::vector<PvpLoadout::SetPiece> setPieces;
    for (uint8 slot : SET_SLOTS)
    {
        setSlotItems[slot] = EligiblePvp(ctx, slot);
        for (LoadoutItem const& item : setSlotItems[slot])
        {
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(item.itemId);
            if (proto->ItemSet)
                setPieces.push_back({slot, proto->ItemSet, item.itemId, proto->ItemLevel});
        }
    }

    // The totals so far are the set's.
    PvpLoadout::StatVector setTotals{};
    std::map<uint8, uint32> const setChoice = PvpLoadout::PickSetPieces(setPieces, SET_PIECES_KEPT);
    for (auto const& [slot, itemId] : setChoice)
    {
        std::vector<LoadoutItem> const& items = setSlotItems[slot];
        LoadoutItem item = *std::find_if(items.begin(), items.end(),
                                         [itemId = itemId](LoadoutItem const& item) { return item.itemId == itemId; });
        item.source = "set";
        setTotals = PvpLoadout::Add(setTotals, item.stats);
        loadout.picks.push_back({{item}, {slot}, item.stats, true});
    }

    // The remaining slots, biggest marginal EP first.
    std::vector<GroupBuilder> builders;
    for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        bool const weapon = slot == EQUIPMENT_SLOT_MAINHAND || slot == EQUIPMENT_SLOT_OFFHAND;
        if (slot == EQUIPMENT_SLOT_BODY || slot == EQUIPMENT_SLOT_TABARD || setChoice.count(slot) ||
            (weapon && !bot->CanTitanGrip()))
            continue;

        auto const walked = setSlotItems.find(slot);
        builders.push_back(
            SlotGroup(ctx, slot, walked != setSlotItems.end() ? walked->second : EligiblePvp(ctx, slot)));
    }
    if (!bot->CanTitanGrip())
        builders.push_back(WeaponGroup(ctx));

    std::vector<PvpLoadout::Group> groups;
    for (GroupBuilder const& builder : builders)
    {
        PvpLoadout::Group& group = groups.emplace_back();
        for (Pick const& pick : builder.picks)
            group.options.push_back(ToOption(pick));
    }

    PvpLoadout::Profile const& profile = sPlayerbotAIConfig.PvpProfiles[static_cast<size_t>(ctx.role)];
    std::vector<int32> const chosen = PvpLoadout::SelectBiggestGainFirst(setTotals, groups, profile);
    for (size_t g = 0; g < builders.size(); ++g)
        if (chosen[g] >= 0)
            loadout.picks.push_back(std::move(builders[g].picks[chosen[g]]));

    return loadout;
}

PvpLoadout::Option PvpGearListMgr::ToOption(Pick const& pick)
{
    PvpLoadout::Option option{};
    for (LoadoutItem const& item : pick.items)
    {
        option.itemIds.push_back(item.itemId);
        option.unique = option.unique || item.unique;
    }
    option.stats = pick.stats;
    return option;
}
