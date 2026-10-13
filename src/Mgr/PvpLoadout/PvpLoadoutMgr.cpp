/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PvpLoadoutMgr.h"
#include "AiFactory.h"
#include "Bag.h"
#include "Battleground.h"
#include "Log.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotFactory.h"
#include "PlayerbotPvpLoadoutRepository.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include "PvpGearListMgr.h"
#include "PvpLoadoutLog.h"
#include "PvpLoadoutStats.h"
#include "Talentspec.h"
#include <algorithm>
#include <limits>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace
{
// This close to the gates opening, a skirmish scales against whichever opponents are present.
constexpr uint32 SKIRMISH_RATING_FALLBACK_MS = 10 * IN_MILLISECONDS;

bool HasTalentPoints(std::string const& link)
{
    return std::any_of(link.begin(), link.end(), [](char c) { return c >= '1' && c <= '9'; });
}

uint16 EquipmentPos(uint8 slot) { return (INVENTORY_SLOT_BAG_0 << 8) | slot; }

uint32 EquippedEntry(Player* bot, uint8 slot)
{
    Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
    return item ? item->GetEntry() : 0;
}

int32 PvpSpecNo(Player* bot)
{
    uint8 const tab = AiFactory::GetPlayerSpecTab(bot);
    return tab < PvpLoadout::TALENT_TAB_COUNT ? sPlayerbotAIConfig.PvpSpecNoByTab[bot->getClass()][tab]
                                              : PvpLoadout::NO_SPEC;
}

// Where the bot already wears this item, other than in slot; NULL_SLOT when it does not.
uint8 EquippedElsewhere(Player* bot, uint32 itemId, uint8 slot)
{
    for (uint8 other = EQUIPMENT_SLOT_START; other < EQUIPMENT_SLOT_END; ++other)
        if (other != slot && EquippedEntry(bot, other) == itemId)
            return other;

    return NULL_SLOT;
}

// A unique pick the bot already wears in the other ring or trinket slot goes to that slot, and the pick there takes
// its place: the pair's slots are interchangeable, and this way nothing clashes or needs creating.
void AlignPairedSlots(Player* bot, std::vector<PvpGearListMgr::Pick>& picks)
{
    auto const retarget = [](PvpGearListMgr::Pick& pick, uint8 slot)
    {
        pick.items.front().slot = slot;
        pick.slots = {slot};
    };

    for (auto const& [first, second] : {std::pair<uint8, uint8>{EQUIPMENT_SLOT_FINGER1, EQUIPMENT_SLOT_FINGER2},
                                        std::pair<uint8, uint8>{EQUIPMENT_SLOT_TRINKET1, EQUIPMENT_SLOT_TRINKET2}})
    {
        for (PvpGearListMgr::Pick& pick : picks)
        {
            uint8 const slot = pick.items.size() == 1 ? pick.items.front().slot : uint8(NULL_SLOT);
            uint8 const paired = slot == first ? second : slot == second ? first : uint8(NULL_SLOT);
            if (paired == NULL_SLOT || !pick.items.front().unique ||
                EquippedEntry(bot, paired) != pick.items.front().itemId)
                continue;

            for (PvpGearListMgr::Pick& other : picks)
                if (other.items.size() == 1 && other.items.front().slot == paired)
                    retarget(other, slot);
            retarget(pick, paired);
            break;
        }
    }
}

// The bot's own items in these slots, template stats only, and their entries.
PvpLoadout::Option OwnOption(Player* bot, PvpLoadout::Role role, std::vector<uint8> const& slots)
{
    PvpLoadout::Option own{};
    own.own = true;
    for (uint8 slot : slots)
    {
        if (Item const* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        {
            own.itemIds.push_back(item->GetEntry());
            own.stats = PvpLoadout::Add(own.stats, PvpLoadoutStats::ItemStats(bot, role, item->GetTemplate()));
        }
    }
    return own;
}

// Gems and enchants by marginal EP against the bot's measured totals, kept current as each one goes on, so
// they close the gaps to the targets instead of stacking past them.
class TargetScorer : public EnchantScorer
{
public:
    TargetScorer(Player* bot, PvpLoadout::Role role, PvpLoadout::Profile const& profile)
        : _bot(bot), _role(role), _profile(profile), _totals(PvpLoadoutStats::MeasureCappedTotals(bot, role))
    {
    }

    float Score(uint32 enchantId) override { return PvpLoadout::MarginalEp(_totals, Stats(enchantId), _profile); }

    void Applied(uint32 removedEnchantId, uint32 addedEnchantId) override
    {
        _totals = PvpLoadout::Subtract(_totals, Stats(removedEnchantId));
        if (PvpLoadout::DebugLogging())
            _chosen << " " << addedEnchantId << "(" << uint32(Score(addedEnchantId)) << ")";
        _totals = PvpLoadout::Add(_totals, Stats(addedEnchantId));
    }

    // enchant(marginal EP when chosen), in the order they went on
    std::string Chosen() const { return _chosen.str(); }

private:
    // The same few hundred gems are scored for every socket.
    PvpLoadout::StatVector const& Stats(uint32 enchantId)
    {
        auto [it, inserted] = _stats.try_emplace(enchantId);
        if (inserted)
            it->second = PvpLoadoutStats::EnchantStats(_bot, _role, enchantId);
        return it->second;
    }

    Player* _bot;
    PvpLoadout::Role _role;
    PvpLoadout::Profile const& _profile;
    PvpLoadout::StatVector _totals;
    std::unordered_map<uint32, PvpLoadout::StatVector> _stats;
    std::ostringstream _chosen;
};

// The item as the core persists it, to recreate it identically after the match.
PvpLoadout::ItemCopy CopyOf(Item const* item, uint8 slot)
{
    PvpLoadout::ItemCopy copy{};
    copy.slot = slot;
    copy.itemGuid = item->GetGUID().GetCounter();
    copy.entry = item->GetEntry();
    copy.creatorGuid = item->GetGuidValue(ITEM_FIELD_CREATOR).GetCounter();
    copy.giftCreatorGuid = item->GetGuidValue(ITEM_FIELD_GIFTCREATOR).GetCounter();
    copy.flags = item->GetUInt32Value(ITEM_FIELD_FLAGS);
    copy.duration = item->GetUInt32Value(ITEM_FIELD_DURATION);
    for (uint8 i = 0; i < PvpLoadout::ITEM_SPELL_CHARGE_COUNT; ++i)
        copy.charges[i] = item->GetSpellCharges(i);

    for (uint8 i = 0; i < PvpLoadout::ITEM_ENCHANTMENT_SLOT_COUNT; ++i)
    {
        EnchantmentSlot const enchantSlot = EnchantmentSlot(i);
        copy.enchantments[i] = {item->GetEnchantmentId(enchantSlot), item->GetEnchantmentDuration(enchantSlot),
                                item->GetEnchantmentCharges(enchantSlot)};
    }

    copy.randomPropertyId = item->GetItemRandomPropertyId();
    copy.durability = item->GetUInt32Value(ITEM_FIELD_DURABILITY);
    copy.playedTime = item->GetUInt32Value(ITEM_FIELD_CREATE_PLAYED_TIME);
    copy.text = item->GetText();
    return copy;
}

// A new item from a copy, owned by the bot but not yet in its inventory. As Item::CloneItem, without the refund and
// trade windows, which belong to the old item guid.
Item* Recreate(Player* bot, PvpLoadout::ItemCopy const& copy)
{
    // As a clone, so an item without a random property does not roll one.
    Item* item = Item::CreateItem(copy.entry, 1, bot, true, uint32(copy.randomPropertyId));
    if (!item)
        return nullptr;

    item->SetGuidValue(ITEM_FIELD_CREATOR, ObjectGuid::Create<HighGuid::Player>(copy.creatorGuid));
    item->SetGuidValue(ITEM_FIELD_GIFTCREATOR, ObjectGuid::Create<HighGuid::Player>(copy.giftCreatorGuid));
    item->SetUInt32Value(ITEM_FIELD_FLAGS, copy.flags & ~(ITEM_FIELD_FLAG_REFUNDABLE | ITEM_FIELD_FLAG_BOP_TRADEABLE));
    item->SetUInt32Value(ITEM_FIELD_DURATION, copy.duration);
    for (uint8 i = 0; i < PvpLoadout::ITEM_SPELL_CHARGE_COUNT; ++i)
        item->SetSpellCharges(i, copy.charges[i]);

    for (uint8 i = 0; i < PvpLoadout::ITEM_ENCHANTMENT_SLOT_COUNT; ++i)
        item->SetEnchantment(EnchantmentSlot(i), copy.enchantments[i].id, copy.enchantments[i].duration,
                             copy.enchantments[i].charges);

    item->SetUInt32Value(ITEM_FIELD_DURABILITY, copy.durability);
    item->SetUInt32Value(ITEM_FIELD_CREATE_PLAYED_TIME, copy.playedTime);
    item->SetText(copy.text);
    return item;
}

// Puts a recreated item in its slot, or else the bags, or else the bot's mailbox: an item is never lost.
void Return(Player* bot, Item* item, uint8 slot)
{
    uint16 equipDest = 0;
    InventoryResult const equipResult = bot->CanEquipItem(slot, equipDest, item, false);
    if (equipResult == EQUIP_ERR_OK)
    {
        bot->EquipItem(equipDest, item, true);
        return;
    }

    ItemPosCountVec storeDest;
    if (bot->CanStoreItem(NULL_BAG, NULL_SLOT, storeDest, item, false) == EQUIP_ERR_OK)
    {
        bot->StoreItem(storeDest, item, true);
        PvpLoadout::LogDebug("Bot {} cannot wear its PvE item {} in slot {} (equip error {}), put it in its bags",
                             bot->GetName(), item->GetEntry(), uint32(slot), uint32(equipResult));
        return;
    }

    LOG_INFO("playerbots", "Bot {} has no room for its PvE item {}, mailed it to itself", bot->GetName(),
             item->GetEntry());
    item->RemoveFromUpdateQueueOf(bot);
    bot->SendItemRetrievalMail(item);
}
}  // namespace

uint32 PvpLoadoutMgr::GetArenaInstanceId(Player* bot)
{
    if (!bot->InArena())
        return 0;

    Battleground* arena = bot->GetBattleground();
    return arena ? arena->GetInstanceID() : 0;
}

PvpLoadout::Transition PvpLoadoutMgr::GetTransition(Player* bot, std::optional<PvpLoadout::Snapshot> const& loadout)
{
    uint32 const arenaInstanceId = GetArenaInstanceId(bot);
    bool const matchOver = arenaInstanceId && bot->GetBattleground()->GetStatus() == STATUS_WAIT_LEAVE;
    return PvpLoadout::Decide(sPlayerbotAIConfig.PvpLoadoutSwap, arenaInstanceId, matchOver, loadout.has_value(),
                              loadout ? loadout->arenaInstanceId : 0, loadout && !loadout->plannedItems.empty());
}

void PvpLoadoutMgr::CaptureTalents(Player* bot, PvpLoadout::Snapshot& snapshot)
{
    TalentSpec current(bot);
    snapshot.talentLink = current.GetTalentLink();

    for (uint8 slot = 0; slot < PvpLoadout::GLYPH_SLOT_COUNT; ++slot)
        snapshot.glyphs[slot] = bot->GetGlyph(slot);
}

void PvpLoadoutMgr::ApplyPvpTalents(Player* bot)
{
    int32 const specNo = PvpSpecNo(bot);
    if (specNo == PvpLoadout::NO_SPEC)
        return;

    PlayerbotFactory::InitTalentsBySpecNo(bot, specNo, true);

    // InitGlyphs recognises the PvP premade from its talents and applies that premade's glyphs.
    PlayerbotFactory factory(bot, bot->GetLevel());
    factory.InitGlyphs(false);
}

void PvpLoadoutMgr::RestoreTalents(Player* bot, PvpLoadout::Snapshot const& snapshot)
{
    std::vector<std::vector<uint32>> const parsed =
        PlayerbotAIConfig::ParseTempTalentsOrder(bot->getClass(), snapshot.talentLink);

    PlayerbotFactory factory(bot, bot->GetLevel());
    if (parsed.empty() && HasTalentPoints(snapshot.talentLink))
    {
        LOG_ERROR("playerbots", "Bot {} PvP loadout talent link '{}' does not parse, re-rolling its talent tree",
                  bot->GetName(), snapshot.talentLink);
        factory.InitTalentsTree(true, true, true);
    }
    else
    {
        PlayerbotFactory::InitTalentsByParsedSpecLink(bot, parsed, true);
        if (bot->GetFreeTalentPoints())
            factory.InitTalentsTree(true);
    }

    PlayerbotFactory::RemoveGlyphs(bot);
    for (uint8 slot = 0; slot < PvpLoadout::GLYPH_SLOT_COUNT; ++slot)
        if (snapshot.glyphs[slot])
            PlayerbotFactory::ApplyGlyph(bot, slot, snapshot.glyphs[slot]);

    bot->SendTalentsInfoData(false);
}

PvpLoadout::MatchRating PvpLoadoutMgr::ResolveMatchRating(Player* bot)
{
    Battleground* arena = bot->GetBattleground();
    if (!arena)
        return {true, 0};

    TeamId const opposing = Battleground::GetOtherTeamId(bot->GetBgTeamId());
    std::vector<PvpLoadout::Opponent> opponents;
    // Teammates only matter to the debug line, which a waiting bot would otherwise build every tick.
    bool const debug = PvpLoadout::DebugLogging();
    std::ostringstream present;
    for (auto const& [guid, player] : arena->GetPlayers())
    {
        bool const opponent = player && player->GetBgTeamId() == opposing;
        if (!player || (!opponent && !debug))
            continue;

        // A self-bot is a person's own character, so its rating counts like a real player's.
        bool const realPlayer = IsRealPlayer(player) || IsSelfBot(player);
        uint32 const personalRating = player->GetMaxPersonalArenaRatingRequirement(0);
        if (debug)
            present << " " << player->GetName() << "(team " << uint32(player->GetBgTeamId()) << ", " << personalRating
                    << (realPlayer ? ", player" : "") << ")";
        if (opponent)
            opponents.push_back({personalRating, realPlayer});
    }

    // The core only starts the preparation timer once the arena is set up, after the first player ports in; until then
    // the timer reads 0, which must not count as the gates being about to open.
    uint32 prepRemainingMs = std::numeric_limits<uint32>::max();
    if (arena->GetStatus() >= STATUS_IN_PROGRESS)
        prepRemainingMs = 0;
    else if (arena->GetStartDelayTime() > 0)
        prepRemainingMs = static_cast<uint32>(arena->GetStartDelayTime());
    PvpLoadout::MatchRating const rating =
        PvpLoadout::ResolveMatchRating(arena->isRated(), arena->GetArenaMatchmakerRating(opposing), opponents,
                                       arena->GetArenaType(), prepRemainingMs, SKIRMISH_RATING_FALLBACK_MS);
    PvpLoadout::LogDebug("Bot {} (team {}) arena rating: rated {}, team size {}, prep left {} ms, players{} -> {} {}",
                         bot->GetName(), uint32(bot->GetBgTeamId()), arena->isRated(), arena->GetArenaType(),
                         prepRemainingMs, present.str(), rating.ready ? "ready" : "waiting", rating.rating);
    return rating;
}

void PvpLoadoutMgr::PlanMatchGear(Player* bot, uint32 rating, PvpLoadout::Snapshot& snapshot)
{
    PvpGearListMgr::Loadout const& loadout = PvpGearListMgr::instance().GetLoadout(bot, PvpSpecNo(bot), rating);
    PvpLoadout::Profile const& profile = sPlayerbotAIConfig.PvpProfiles[static_cast<size_t>(loadout.role)];
    std::vector<PvpGearListMgr::Pick> picks = loadout.picks;
    AlignPairedSlots(bot, picks);

    // Own item or cached pick against the bot's measured caps. The decided items come out of the measurement but their
    // gems and enchants stay in, standing in for the ones the match items get afterwards.
    PvpLoadout::StatVector base = PvpLoadoutStats::MeasureCappedTotals(bot, loadout.role);
    std::vector<PvpGearListMgr::LoadoutItem const*> items;
    std::vector<PvpLoadout::Group> groups;
    std::vector<PvpGearListMgr::Pick const*> groupPicks;
    for (PvpGearListMgr::Pick const& pick : picks)
    {
        PvpLoadout::Option own = OwnOption(bot, loadout.role, pick.slots);
        // Only the capped stats are measured; the others are linear, so their totals do not change an item's worth.
        base = PvpLoadout::Subtract(base, own.stats);
        if (pick.setPiece)
        {
            base = PvpLoadout::Add(base, pick.stats);
            for (PvpGearListMgr::LoadoutItem const& item : pick.items)
                items.push_back(&item);
            continue;
        }

        PvpLoadout::Group group;
        if (!own.itemIds.empty())
            group.options.push_back(std::move(own));

        group.options.push_back(PvpGearListMgr::ToOption(pick));

        groups.push_back(std::move(group));
        groupPicks.push_back(&pick);
    }

    std::vector<int32> const chosen = PvpLoadout::SelectBiggestGainFirst(base, groups, profile);
    for (size_t g = 0; g < groups.size(); ++g)
        if (chosen[g] >= 0 && !groups[g].options[chosen[g]].own)
            for (PvpGearListMgr::LoadoutItem const& item : groupPicks[g]->items)
                items.push_back(&item);

    // Any other unique item the bot already wears elsewhere (a one-handed weapon in the other hand) stays there, and
    // the slot the loadout wanted it in keeps the bot's own item: no copy can be equipped next to it.
    std::unordered_set<uint8> keepSlots;
    for (PvpGearListMgr::LoadoutItem const* item : items)
    {
        if (!item->unique)
            continue;

        uint8 const other = EquippedElsewhere(bot, item->itemId, item->slot);
        if (other != NULL_SLOT)
        {
            keepSlots.insert(item->slot);
            keepSlots.insert(other);
        }
    }

    std::ostringstream summary;
    snapshot.plannedItems.clear();
    snapshot.pveCopies.clear();
    for (PvpGearListMgr::LoadoutItem const* item : items)
    {
        if (keepSlots.count(item->slot) || EquippedEntry(bot, item->slot) == item->itemId)
            continue;

        snapshot.plannedItems.emplace_back(item->slot, item->itemId);
        if (PvpLoadout::DebugLogging())
            summary << " " << uint32(item->slot) << ":" << item->itemId << "("
                    << sObjectMgr->GetItemTemplate(item->itemId)->ItemLevel << "," << item->source << ","
                    << item->requiredRating << ")";
    }

    // Without Titan's Grip a two-hander leaves no room for an off-hand, which then comes off like a replaced item.
    auto const slotOf = &std::pair<uint8, uint32>::first;
    auto const mainHand = std::ranges::find(snapshot.plannedItems, EQUIPMENT_SLOT_MAINHAND, slotOf);
    if (mainHand != snapshot.plannedItems.end() &&
        std::ranges::find(snapshot.plannedItems, EQUIPMENT_SLOT_OFFHAND, slotOf) == snapshot.plannedItems.end() &&
        !bot->CanTitanGrip() && sObjectMgr->GetItemTemplate(mainHand->second)->InventoryType == INVTYPE_2HWEAPON &&
        bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND))
        snapshot.plannedItems.emplace_back(EQUIPMENT_SLOT_OFFHAND, 0);

    // Emptied slots first, so the off-hand is off before a two-hander goes on.
    std::stable_partition(snapshot.plannedItems.begin(), snapshot.plannedItems.end(),
                          [](auto const& planned) { return !planned.second; });

    for (auto const& [slot, itemId] : snapshot.plannedItems)
        if (Item const* pveItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
            snapshot.pveCopies.push_back(CopyOf(pveItem, slot));

    snapshot.plannedRating = rating;
    // slot:item(item level, source, required rating)
    PvpLoadout::LogDebug("Bot {} plans match gear for rating {}:{}", bot->GetName(), rating, summary.str());
}

void PvpLoadoutMgr::ApplyPlannedGear(Player* bot, PvpLoadout::Snapshot& snapshot)
{
    // Nothing can be equipped while casting, and bots buff during the arena's preparation.
    bot->InterruptNonMeleeSpells(true);

    std::ostringstream summary;
    std::unordered_set<uint8> replaced;
    for (auto const& [slot, itemId] : snapshot.plannedItems)
    {
        Item* pveItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (pveItem)
        {
            auto const copy = std::ranges::find(snapshot.pveCopies, slot, &PvpLoadout::ItemCopy::slot);
            if (copy == snapshot.pveCopies.end() || copy->itemGuid != pveItem->GetGUID().GetCounter())
            {
                summary << " " << uint32(slot) << ":kept, changed since planned";
                continue;
            }
        }

        uint16 dest = 0;
        if (itemId && bot->CanEquipNewItem(slot, dest, itemId, true) != EQUIP_ERR_OK)
        {
            summary << " " << uint32(slot) << ":" << itemId << "(not equipped)";
            continue;
        }

        replaced.insert(slot);
        if (pveItem)
            bot->DestroyItem(INVENTORY_SLOT_BAG_0, slot, true);

        if (!itemId)
            continue;

        if (Item* item = bot->EquipNewItem(dest, itemId, true))
        {
            snapshot.matchItems.push_back(item->GetGUID().GetCounter());
            summary << " " << uint32(slot) << ":" << itemId;
        }
    }
    snapshot.plannedItems.clear();
    // A slot left alone keeps its PvE item, so its copy must not be recreated at restore.
    std::erase_if(snapshot.pveCopies,
                  [&replaced](PvpLoadout::ItemCopy const& copy) { return !replaced.count(copy.slot); });

    PvpLoadout::Role const role = PvpLoadoutStats::RoleOf(bot);
    PvpLoadout::Profile const& profile = sPlayerbotAIConfig.PvpProfiles[static_cast<size_t>(role)];
    // slot:item equipped, then the bot's totals against the role's targets
    if (PvpLoadout::DebugLogging())
        PvpLoadout::LogDebug("Bot {} match gear for rating {}:{}; {}", bot->GetName(), snapshot.plannedRating,
                             summary.str(),
                             PvpLoadout::FormatTargets(PvpLoadoutStats::MeasureCappedTotals(bot, role), profile));

    if (snapshot.matchItems.empty() || bot->GetLevel() < sPlayerbotAIConfig.MinEnchantingBotLevel)
        return;

    std::unordered_set<uint32> const matchItems(snapshot.matchItems.begin(), snapshot.matchItems.end());
    TargetScorer scorer(bot, role, profile);
    PlayerbotFactory factory(bot, bot->GetLevel());
    factory.ApplyEnchantAndGemsNew(true, &matchItems, &scorer);
    // enchant(marginal EP when chosen), then the bot's final totals against the role's targets
    if (PvpLoadout::DebugLogging())
        PvpLoadout::LogDebug("Bot {} match gems and enchants:{}; {}", bot->GetName(), scorer.Chosen(),
                             PvpLoadout::FormatTargets(PvpLoadoutStats::MeasureCappedTotals(bot, role), profile));
}

void PvpLoadoutMgr::Announce(PlayerbotAI* botAI, uint32 rating)
{
    Player* bot = botAI->GetBot();
    Battleground* arena = bot->GetBattleground();
    if (!sPlayerbotAIConfig.PvpLoadoutAnnounce || !arena || arena->isRated())
        return;

    int32 const specNo = PvpSpecNo(bot);
    std::string const spec = specNo != PvpLoadout::NO_SPEC ? sPlayerbotAIConfig.PremadeSpecName[bot->getClass()][specNo]
                                                           : AiFactory::GetPlayerSpecName(bot);
    std::map<std::string, std::string> const placeholders = {
        {"%spec", spec}, {"%ilvl", std::to_string(botAI->GetEquipGearScore(bot))}, {"%rating", std::to_string(rating)}};
    botAI->SayToParty(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "pvp_loadout_announce", "Going in as %spec, average item level %ilvl, geared for rating %rating",
        placeholders));
}

void PvpLoadoutMgr::RestoreGear(Player* bot, PvpLoadout::Snapshot& snapshot)
{
    // Nothing can be equipped while casting, and a bot may still be mid-cast as the match ends.
    bot->InterruptNonMeleeSpells(true);
    for (uint32 guid : snapshot.matchItems)
        if (Item* item = bot->GetItemByGuid(ObjectGuid::Create<HighGuid::Item>(guid)))
            bot->DestroyItem(item->GetBagSlot(), item->GetSlot(), true);

    snapshot.matchItems.clear();

    for (PvpLoadout::ItemCopy const& copy : snapshot.pveCopies)
    {
        // An original the bot still has (a crash before the character save that destroyed it) goes back instead.
        if (Item* original = bot->GetItemByGuid(ObjectGuid::Create<HighGuid::Item>(copy.itemGuid)))
        {
            if (original->GetPos() != EquipmentPos(copy.slot))
                bot->SwapItem(original->GetPos(), EquipmentPos(copy.slot));
        }
        else if (Item* item = Recreate(bot, copy))
            Return(bot, item, copy.slot);
        else
            LOG_ERROR("playerbots", "Bot {} could not recreate its PvE item {} for slot {}", bot->GetName(), copy.entry,
                      uint32(copy.slot));
    }

    snapshot.pveCopies.clear();
    snapshot.plannedItems.clear();
}

bool PvpLoadoutMgr::Restore(PlayerbotAI* botAI)
{
    std::optional<PvpLoadout::Snapshot>& loadout =
        botAI->GetAiObjectContext()->GetValue<std::optional<PvpLoadout::Snapshot>>("pvp loadout")->RefGet();
    if (!loadout)
        return false;

    // Talents first: they decide dual wield and Titan's Grip, which the PvE weapons may need.
    Player* bot = botAI->GetBot();
    RestoreTalents(bot, *loadout);
    RestoreGear(bot, *loadout);

    // The recreated PvE items, talents and glyphs exist only in memory until saved; the snapshot is deleted once
    // they are. Mid far-teleport the core defers the full save and adds nothing, so the items are saved anyway.
    CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
    bot->SaveToDB(trans, false, false);
    if (bot->IsBeingTeleportedFar())
        bot->SaveInventoryAndGoldToDB(trans);
    PlayerbotPvpLoadoutRepository::instance().Forget(bot->GetGUID().GetCounter(),
                                                     CharacterDatabase.AsyncCommitTransaction(trans));
    loadout.reset();
    PvpLoadout::LogDebug("Bot {} restored its PvE loadout", bot->GetName());
    return true;
}
