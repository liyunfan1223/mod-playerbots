/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "LivingEconomyActions.h"

#include <cmath>
#include <map>
#include <mutex>
#include <unordered_map>

#include "AuctionHouseMgr.h"
#include "Bag.h"
#include "GameTime.h"
#include "ItemUsageValue.h"
#include "LivingEconomyPolicy.h"
#include "LivingFieldActions.h"
#include "LivingProfessionActions.h"
#include "LivingWorldMgr.h"
#include "Mail.h"
#include "PlayerbotWorldThreadProcessor.h"
#include "Playerbots.h"
#include "TravelMgr.h"

namespace LivingProfessions
{
std::vector<Item*> BagItems(Player* player)
{
    std::vector<Item*> items;
    for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; ++slot)
        if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
            items.push_back(item);
    for (uint8 slot = INVENTORY_SLOT_BAG_START; slot < INVENTORY_SLOT_BAG_END; ++slot)
        if (Bag* bag = player->GetBagByPos(slot))
            for (uint32 index = 0; index < bag->GetBagSize(); ++index)
                if (Item* item = bag->GetItemByPos(index))
                    items.push_back(item);
    return items;
}

Stock ProfessionStock(Player* player)
{
    Stock stock;
    for (auto const& [id, learned] : player->GetSpellMap())
    {
        if (!learned || learned->State == PLAYERSPELL_REMOVED || !learned->Active)
            continue;
        auto const* spell = sSpellMgr->GetSpellInfo(id);
        if (!spell || !LivingProfessions::IsProfessionRecipe(player, spell))
            continue;
        uint64 cost = 0;
        for (uint8 index = 0; index < MAX_SPELL_REAGENTS; ++index)
        {
            if (spell->Reagent[index] <= 0 || !spell->ReagentCount[index])
                continue;
            uint32 const entry = spell->Reagent[index];
            auto const* item = sObjectMgr->GetItemTemplate(entry);
            if (!item)
                continue;
            uint32 const count = spell->ReagentCount[index];
            stock.reserve[entry] = std::max(stock.reserve[entry], std::max(count, item->GetMaxStackSize()));
            uint64 const unit = std::max<uint64>(item->SellPrice, item->BuyPrice / std::max<uint32>(1, item->BuyCount));
            cost = std::min<uint64>(LivingWorld::MoneyLimit, cost + unit * count);
        }
        for (uint32 tool : spell->Totem)
            if (tool)
                stock.reserve[tool] = std::max<uint32>(1, stock.reserve[tool]);
        for (auto const& effect : spell->GetEffects())
        {
            if ((!effect.IsEffect(SPELL_EFFECT_CREATE_ITEM) && !effect.IsEffect(SPELL_EFFECT_CREATE_ITEM_2)) ||
                !effect.ItemType)
                continue;
            uint32 const unitCost = static_cast<uint32>(cost / std::max<int32>(1, effect.CalcValue()));
            auto [it, inserted] = stock.costs.emplace(effect.ItemType, unitCost);
            if (!inserted)
                it->second = std::min(it->second, unitCost);
        }
    }
    return stock;
}
}  // namespace LivingProfessions

namespace
{
using LivingProfessions::BagItems;
using LivingProfessions::ProfessionStock;
using LivingProfessions::Stock;
using Counts = std::unordered_map<uint32, uint32>;

bool IsTrader(Player* player)
{
    return LivingWorldMgr::instance().GetProfile(player).motivation == LivingWorld::Motivation::Trader;
}

// What traders paid for flipped goods (so the resale covers it) and when they last looked for a
// bargain. Bot threads read it while planning; world-thread operations update it.
struct FlipLedger
{
    std::mutex lock;
    std::unordered_map<uint32, std::unordered_map<uint32, uint32>> paid;  // bot -> item -> unit price paid
    std::unordered_map<uint32, time_t> lastScan;                            // bot -> last bargain scan
};

FlipLedger& Flips()
{
    static FlipLedger ledger;
    return ledger;
}

uint32 FlipPaid(Player* player, uint32 entry)
{
    FlipLedger& ledger = Flips();
    std::lock_guard<std::mutex> guard(ledger.lock);
    auto const bot = ledger.paid.find(player->GetGUID().GetCounter());
    if (bot == ledger.paid.end())
        return 0;
    auto const item = bot->second.find(entry);
    return item == bot->second.end() ? 0 : item->second;
}

bool FlipScanDue(Player* player)
{
    FlipLedger& ledger = Flips();
    std::lock_guard<std::mutex> guard(ledger.lock);
    auto const it = ledger.lastScan.find(player->GetGUID().GetCounter());
    return it == ledger.lastScan.end() ||
           GameTime::GetGameTime().count() - it->second >= LivingWorld::FlipCooldownSeconds;
}

bool IsLivingMail(Mail const* mail)
{
    return mail && (mail->messageType == MAIL_AUCTION ||
                    (mail->messageType == MAIL_CREATURE && mail->sender == LivingWorld::PostmasterEntry));
}

uint32 SaleCount(PlayerbotAI* ai, Item* item, Stock const& stock)
{
    if (!item || LivingProfessions::FieldReservedItem(ai, item->GetGUID()) || item->IsEquipped() ||
        !item->CanBeTraded() || item->IsNotEmptyBag() || item->IsWrapped() || item->GetUInt32Value(ITEM_FIELD_DURATION))
        return 0;
    bool const backpack = item->GetBagSlot() == INVENTORY_SLOT_BAG_0 && item->GetSlot() >= INVENTORY_SLOT_ITEM_START &&
                          item->GetSlot() < INVENTORY_SLOT_ITEM_END;
    bool const bag = item->GetBagSlot() >= INVENTORY_SLOT_BAG_START && item->GetBagSlot() < INVENTORY_SLOT_BAG_END;
    if (!backpack && !bag)
        return 0;
    auto const* proto = item->GetTemplate();
    if (proto->HasFlag(ITEM_FLAG_CONJURED) || proto->Class == ITEM_CLASS_QUEST || proto->StartQuest ||
        proto->TotemCategory ||
        (proto->Class == ITEM_CLASS_WEAPON &&
         (proto->SubClass == ITEM_SUBCLASS_WEAPON_MISC || proto->SubClass == ITEM_SUBCLASS_WEAPON_FISHING_POLE)))
        return 0;
    // Goods bought to flip are for resale whatever the bot itself would use them for.
    bool const flipped = FlipPaid(ai->GetBot(), item->GetEntry()) != 0;
    auto* context = ai->GetAiObjectContext();
    // The qualifier includes the random property, so an upgrade isn't mistaken for a spare.
    std::string const qualifier =
        std::to_string(item->GetEntry()) + "," + std::to_string(item->GetItemRandomPropertyId());
    if (!flipped && context->GetValue<ItemUsage>("item usage", qualifier)->Get() != ITEM_USAGE_AH)
        return 0;
    auto const it = stock.reserve.find(item->GetEntry());
    uint32 const reserve = flipped || it == stock.reserve.end() ? 0 : it->second;
    return LivingWorld::Surplus(ai->GetBot()->GetItemCount(item->GetEntry(), false), reserve, item->GetCount());
}

struct VendorOffer
{
    ObjectGuid vendor;
    uint32 slot;
    uint32 bundle;
    uint32 price;
    uint32 available;
};
using Offers = std::unordered_map<uint32, VendorOffer>;

Offers NearbyOffers(PlayerbotAI* ai)
{
    Offers offers;
    Player* player = ai->GetBot();
    for (ObjectGuid guid : ai->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get())
    {
        Unit* unit = ai->GetUnit(guid);
        Creature* npc = unit ? unit->ToCreature() : nullptr;
        if (!npc || !npc->IsAlive() || npc->IsHostileTo(player) || !npc->IsVendor() ||
            !player->IsWithinDistInMap(npc, 40.0f))
            continue;
        auto const* inventory = npc->GetVendorItems();
        if (!inventory)
            continue;
        for (uint32 slot = 0; slot < inventory->GetItemCount(); ++slot)
        {
            auto const* offer = inventory->GetItem(slot);
            auto const* item = sObjectMgr->GetItemTemplate(offer->item);
            if (!item || !item->BuyCount || offer->ExtendedCost)
                continue;
            uint32 const price =
                static_cast<uint32>(std::ceil(item->BuyPrice * player->GetReputationPriceDiscount(npc)));
            uint32 const available = offer->maxcount ? npc->GetVendorItemCurrentCount(offer) : UINT32_MAX;
            if (available < item->BuyCount)
                continue;
            auto const current = offers.find(item->ItemId);
            if (current == offers.end() || price < current->second.price)
                offers[item->ItemId] = {guid, slot, item->BuyCount, price, available};
        }
    }
    return offers;
}

bool HasFishingPole(Player* player)
{
    auto isPole = [](Item* item)
    {
        return item && item->GetTemplate()->Class == ITEM_CLASS_WEAPON &&
               item->GetTemplate()->SubClass == ITEM_SUBCLASS_WEAPON_FISHING_POLE;
    };
    if (isPole(player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND)))
        return true;
    for (Item* item : BagItems(player))
        if (isPole(item))
            return true;
    return false;
}

LivingEconomyPlan ToolPlan(Player* player, Offers const& offers)
{
    Counts tools;
    Counts categories;
    for (auto const& [id, learned] : player->GetSpellMap())
    {
        auto const* spell = sSpellMgr->GetSpellInfo(id);
        if (!learned || learned->State == PLAYERSPELL_REMOVED || !learned->Active || !spell ||
            !LivingProfessions::IsProfessionRecipe(player, spell))
            continue;
        for (uint32 tool : spell->Totem)
            if (tool && !player->HasItemCount(tool, 1, false))
                tools[tool] = id;
        for (uint32 category : spell->TotemCategory)
            if (category && !player->HasItemTotemCategory(category))
                categories[category] = id;
    }
    bool const needsPole = player->HasSkill(SKILL_FISHING) && !HasFishingPole(player);
    LivingEconomyPlan best;
    uint32 cheapest = UINT32_MAX;
    for (auto const& [entry, offer] : offers)
    {
        auto const* item = sObjectMgr->GetItemTemplate(entry);
        if (!item || offer.price >= cheapest || player->CanUseItem(item) != EQUIP_ERR_OK ||
            !LivingWorld::CanAffordTraining(player->GetMoney(), offer.price, player->GetLevel()))
            continue;
        LivingEconomyPlan candidate;
        if (auto const it = tools.find(entry); it != tools.end())
            candidate = {offer.vendor, {}, entry, offer.slot, 1, it->second, LivingSupplyKind::Tool};
        else if (needsPole && item->Class == ITEM_CLASS_WEAPON && item->SubClass == ITEM_SUBCLASS_WEAPON_FISHING_POLE)
            candidate = {offer.vendor, {}, entry, offer.slot, 1, 0, LivingSupplyKind::FishingPole};
        else
            for (auto const& [category, spell] : categories)
                if (player->IsTotemCategoryCompatiableWith(item, category))
                {
                    candidate = {offer.vendor, {}, entry, offer.slot, 1, spell, LivingSupplyKind::Tool, category};
                    break;
                }
        if (!candidate.target.IsEmpty())
        {
            best = candidate;
            cheapest = offer.price;
        }
    }
    return best;
}

bool StillNeedsSupply(Player* player, LivingEconomyPlan const& plan, ItemTemplate const* item)
{
    if (plan.supplyKind == LivingSupplyKind::FishingPole)
        return player->HasSkill(SKILL_FISHING) && !HasFishingPole(player);
    auto const* recipe = sSpellMgr->GetSpellInfo(plan.recipe);
    if (!recipe || !player->HasSpell(plan.recipe) || !LivingProfessions::IsProfessionRecipe(player, recipe))
        return false;
    if (plan.supplyKind == LivingSupplyKind::Reagent)
        return LivingProfessions::NeedsProduct(player, recipe) && !player->HasItemCount(plan.entry, plan.needed, false);
    if (plan.category)
        return !player->HasItemTotemCategory(plan.category) &&
               player->IsTotemCategoryCompatiableWith(item, plan.category);
    return !player->HasItemCount(plan.entry, 1, false);
}

LivingEconomyPlan SupplyPlan(PlayerbotAI* ai)
{
    Player* player = ai->GetBot();
    Offers const offers = NearbyOffers(ai);
    if (offers.empty())
        return {};
    auto const tool = ToolPlan(player, offers);
    if (!tool.target.IsEmpty())
        return tool;
    LivingEconomyPlan best;
    uint32 bestScore = 0;
    for (auto const& [id, learned] : player->GetSpellMap())
    {
        if (!learned || learned->State == PLAYERSPELL_REMOVED || !learned->Active)
            continue;
        auto const* spell = sSpellMgr->GetSpellInfo(id);
        if (!spell || !LivingProfessions::IsProfessionRecipe(player, spell) ||
            !LivingProfessions::NeedsProduct(player, spell))
            continue;
        LivingEconomyPlan first;
        uint64 total = 0;
        bool available = true;
        for (uint8 index = 0; index < MAX_SPELL_REAGENTS; ++index)
        {
            if (spell->Reagent[index] <= 0 || !spell->ReagentCount[index])
                continue;
            uint32 const entry = spell->Reagent[index];
            uint32 const owned = player->GetItemCount(entry, false);
            uint32 const needed = spell->ReagentCount[index];
            if (owned >= needed)
                continue;
            auto const it = offers.find(entry);
            if (it == offers.end())
            {
                available = false;
                break;
            }
            auto const& offer = it->second;
            uint32 const bundles = LivingWorld::PurchaseBundles(owned, needed, offer.bundle);
            if (uint64(bundles) * offer.bundle < needed - owned || uint64(bundles) * offer.bundle > offer.available)
            {
                available = false;
                break;
            }
            total += uint64(offer.price) * bundles;
            if (first.target.IsEmpty())
                first = {offer.vendor, {}, entry, offer.slot, needed, id};
        }
        if (!available || first.target.IsEmpty() || total > LivingWorld::MoneyLimit ||
            !LivingWorld::CanAffordTraining(player->GetMoney(), static_cast<uint32>(total), player->GetLevel()))
            continue;
        uint32 const score = (ItemUsageValue::SpellGivesSkillUp(id, player) ? 100000U : 0U) +
                             LivingWorld::Mix(player->GetGUID().GetCounter() ^ id) % 10000U + 1;
        if (score > bestScore)
        {
            best = first;
            bestScore = score;
        }
    }
    return best;
}

// This snapshot is read and updated only by world-thread operations.
struct Market
{
    time_t refreshed = 0;
    std::unordered_map<uint32, std::vector<std::pair<ObjectGuid, uint32>>> prices;
    std::map<ObjectGuid, uint32> owners;
    std::map<std::pair<ObjectGuid, uint32>, uint32> listings;
};

Market& ReadMarket(AuctionHouseObject* house)
{
    static std::map<AuctionHouseObject*, Market> markets;
    Market& market = markets[house];
    time_t const now = GameTime::GetGameTime().count();
    if (!market.refreshed || now - market.refreshed >= 60)
    {
        market = {};
        market.refreshed = now;
        for (auto const& [id, auction] : house->GetAuctions())
        {
            if (!auction || auction->expire_time <= now)
                continue;
            ++market.owners[auction->owner];
            ++market.listings[{auction->owner, auction->item_template}];
            if (auction->buyout && auction->itemCount)
                market.prices[auction->item_template].emplace_back(auction->owner,
                                                                   auction->buyout / auction->itemCount);
        }
    }
    return market;
}

uint32 PeerPrice(Market const& market, uint32 entry, ObjectGuid owner)
{
    auto const it = market.prices.find(entry);
    if (it == market.prices.end())
        return 0;
    std::vector<uint32> prices;
    for (auto const& [seller, price] : it->second)
        if (seller != owner && price)
            prices.push_back(price);
    if (prices.empty())
        return 0;
    auto middle = prices.begin() + prices.size() / 2;
    std::nth_element(prices.begin(), middle, prices.end());
    return *middle;
}

// Going rate for judging a flip: the median of every other listing of the item, including the same
// seller's. Comparing only with other sellers found no bargains, because nearly every listing belongs
// to the single AH bot (2026-10-21). The flipper's own listings and the judged listing are left out.
uint32 FlipMedian(Market const& market, uint32 entry, ObjectGuid flipper, uint32 unit, uint32& count)
{
    count = 0;
    auto const it = market.prices.find(entry);
    if (it == market.prices.end())
        return 0;
    std::vector<uint32> prices;
    bool judgedSkipped = false;
    for (auto const& [seller, price] : it->second)
    {
        if (seller == flipper || !price)
            continue;
        if (!judgedSkipped && price == unit)
        {
            judgedSkipped = true;
            continue;
        }
        prices.push_back(price);
    }
    count = static_cast<uint32>(prices.size());
    if (prices.empty())
        return 0;
    auto middle = prices.begin() + prices.size() / 2;
    std::nth_element(prices.begin(), middle, prices.end());
    return *middle;
}

class LivingAuctionOperation : public PlayerbotOperation
{
public:
    LivingAuctionOperation(ObjectGuid bot, LivingEconomyPlan plan) : _bot(bot), _plan(plan) {}
    ObjectGuid GetBotGuid() const override { return _bot; }
    std::string GetName() const override { return "Living auction sale"; }
    bool Execute() override
    {
        Player* player = ObjectAccessor::FindPlayer(_bot);
        PlayerbotAI* ai = player ? GET_PLAYERBOT_AI(player) : nullptr;
        if (!ai || !LivingProfessions::CanWork(ai) || !IsTrader(player))
            return false;
        Creature* npc = player->GetNPCIfCanInteractWith(_plan.target, UNIT_NPC_FLAG_AUCTIONEER);
        Item* item = player->GetItemByGuid(_plan.item);
        if (!npc || !item)
            return false;
        Stock const stock = ProfessionStock(player);
        uint32 const count = SaleCount(ai, item, stock);
        if (!count)
            return false;
        auto const* entry = AuctionHouseMgr::GetAuctionHouseEntryFromFactionTemplate(npc->GetFaction());
        auto* house = sAuctionMgr->GetAuctionsMap(npc->GetFaction());
        if (!entry || !house)
            return false;
        Market& market = ReadMarket(house);
        uint32 const deposit = sAuctionMgr->GetAuctionDeposit(entry, MIN_AUCTION_TIME, item, count);
        uint32 const product = item->GetEntry();
        if (!LivingWorld::CanListAuction(market.owners[_bot], market.listings[{_bot, product}], player->GetMoney(),
                                         deposit, player->GetLevel()))
            return false;
        auto const cost = stock.costs.find(product);
        uint32 const paid = FlipPaid(player, product);
        uint32 const price =
            paid ? LivingWorld::FlipResalePrice(paid, PeerPrice(market, product, _bot))
                 : LivingWorld::AuctionUnitPrice(item->GetTemplate()->SellPrice,
                                                 cost == stock.costs.end() ? 0 : cost->second,
                                                 PeerPrice(market, product, _bot), count);
        if (price && uint64(price) * count > LivingWorld::MoneyLimit)
            return false;
        if (!price)
            return false;
        uint32 const previous = player->GetItemCount(product, false);
        WorldPacket packet(CMSG_AUCTION_SELL_ITEM);
        packet << _plan.target << uint32(1) << _plan.item << count << uint32(price * count) << uint32(price * count)
               << uint32(MIN_AUCTION_TIME / MINUTE);
        player->GetSession()->HandleAuctionSellItem(packet);
        if (player->GetItemCount(product, false) >= previous)
            return false;
        ++market.owners[_bot];
        ++market.listings[{_bot, product}];
        market.prices[product].emplace_back(_bot, price);
        if (paid && !player->GetItemCount(product, true))
        {
            FlipLedger& ledger = Flips();
            std::lock_guard<std::mutex> guard(ledger.lock);
            ledger.paid[_bot.GetCounter()].erase(product);
        }
        LOG_DEBUG("playerbots", "Living trader {} listed {} x {} at {} copper{}", _bot.ToString(), count, product, price,
                  paid ? " (flip)" : "");
        return true;
    }

private:
    ObjectGuid _bot;
    LivingEconomyPlan _plan;
};

// Buys one clearly underpriced listing to resell. Runs on the world thread like the sale.
class LivingFlipOperation : public PlayerbotOperation
{
public:
    LivingFlipOperation(ObjectGuid bot, ObjectGuid auctioneer) : _bot(bot), _auctioneer(auctioneer) {}
    ObjectGuid GetBotGuid() const override { return _bot; }
    std::string GetName() const override { return "Living auction flip"; }
    bool Execute() override
    {
        Player* player = ObjectAccessor::FindPlayer(_bot);
        PlayerbotAI* ai = player ? GET_PLAYERBOT_AI(player) : nullptr;
        if (!ai || !LivingProfessions::CanWork(ai) || !IsTrader(player))
            return false;
        Creature* npc = player->GetNPCIfCanInteractWith(_auctioneer, UNIT_NPC_FLAG_AUCTIONEER);
        auto* house = npc ? sAuctionMgr->GetAuctionsMap(npc->GetFaction()) : nullptr;
        if (!house)
            return false;
        time_t const now = GameTime::GetGameTime().count();
        {
            FlipLedger& ledger = Flips();
            std::lock_guard<std::mutex> guard(ledger.lock);
            ledger.lastScan[_bot.GetCounter()] = now;
        }

        Market& market = ReadMarket(house);
        uint32 bestId = 0, bestBuyout = 0, bestEntry = 0, bestUnit = 0;
        uint64 bestRatio = 0;
        for (auto const& [id, auction] : house->GetAuctions())
        {
            if (!auction || auction->owner == _bot || auction->bidder == _bot || !auction->buyout ||
                !auction->itemCount || auction->expire_time <= now)
                continue;
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(auction->item_template);
            if (!proto)
                continue;
            uint32 const unit = auction->buyout / auction->itemCount;
            uint32 others = 0;
            uint32 const median = FlipMedian(market, auction->item_template, _bot, unit, others);
            if (!LivingWorld::IsFlipBargain(unit, median, proto->SellPrice, others) ||
                !LivingWorld::CanAffordTraining(player->GetMoney(), auction->buyout, player->GetLevel()))
                continue;
            uint64 const ratio = uint64(unit) * 1000 / median;
            if (!bestId || ratio < bestRatio)
            {
                bestId = id;
                bestBuyout = auction->buyout;
                bestEntry = auction->item_template;
                bestUnit = unit;
                bestRatio = ratio;
            }
        }
        if (!bestId)
        {
            LOG_INFO("playerbots", "Living trader {} found no flip bargain among {} listings", player->GetName(),
                     house->GetAuctions().size());
            return false;
        }

        uint32 const before = player->GetMoney();
        WorldPacket packet(CMSG_AUCTION_PLACE_BID);
        packet << _auctioneer << bestId << bestBuyout;
        player->GetSession()->HandleAuctionPlaceBid(packet);
        if (player->GetMoney() >= before)
        {
            LOG_INFO("playerbots", "Living trader {} could not buy out auction {} ({} copper)", player->GetName(),
                     bestId, bestBuyout);
            return false;
        }
        {
            FlipLedger& ledger = Flips();
            std::lock_guard<std::mutex> guard(ledger.lock);
            ledger.paid[_bot.GetCounter()][bestEntry] = bestUnit;
        }
        LOG_INFO("playerbots", "Living trader {} bought auction {} (item {} at {} copper each, {}% of the going rate) to flip",
                 player->GetName(), bestId, bestEntry, bestUnit, bestRatio / 10);
        return true;
    }

private:
    ObjectGuid _bot;
    ObjectGuid _auctioneer;
};

class LivingMailOperation : public PlayerbotOperation
{
public:
    LivingMailOperation(ObjectGuid bot, ObjectGuid mailbox) : _bot(bot), _mailbox(mailbox) {}
    ObjectGuid GetBotGuid() const override { return _bot; }
    std::string GetName() const override { return "Living auction mail"; }
    bool Execute() override
    {
        Player* player = ObjectAccessor::FindPlayer(_bot);
        PlayerbotAI* ai = player ? GET_PLAYERBOT_AI(player) : nullptr;
        if (!ai || !LivingProfessions::CanWork(ai) || !player->GetSession()->CanOpenMailBox(_mailbox))
            return false;
        time_t const now = GameTime::GetGameTime().count();
        uint32 processed = 0;
        for (Mail* mail : player->GetMails())
        {
            if (!IsLivingMail(mail) || mail->state == MAIL_STATE_DELETED || mail->deliver_time > now ||
                mail->expire_time <= now || mail->COD)
                continue;
            uint32 const id = mail->messageID;
            if (mail->money)
            {
                WorldPacket packet(CMSG_MAIL_TAKE_MONEY);
                packet << _mailbox << id;
                player->GetSession()->HandleMailTakeMoney(packet);
            }
            // Core handlers update the attachment list; copy identifiers before taking anything.
            std::vector<uint32> items;
            for (auto const& item : mail->items)
                items.push_back(item.item_guid);
            for (uint32 item : items)
            {
                WorldPacket packet(CMSG_MAIL_TAKE_ITEM);
                packet << _mailbox << id << item;
                player->GetSession()->HandleMailTakeItem(packet);
            }
            if (LivingWorld::CanDeleteLivingMail(IsLivingMail(mail), mail->money, mail->items.size(), mail->COD))
            {
                WorldPacket packet(CMSG_MAIL_DELETE);
                packet << _mailbox << id << uint32(mail->mailTemplateId);
                player->GetSession()->HandleMailDelete(packet);
            }
            if (++processed >= 5)
                break;
        }
        // Failed takes retain their money/items and can be retried on the next visit.
        return processed > 0;
    }

private:
    ObjectGuid _bot;
    ObjectGuid _mailbox;
};
}  // namespace

LivingEconomyPlan LivingEconomyValue::Calculate()
{
    if (!LivingProfessions::CanWork(botAI))
        return {};
    if (_job == LivingEconomyJob::Supplies)
        return SupplyPlan(botAI);
    if (_job == LivingEconomyJob::Auction && !IsTrader(bot))
        return {};
    if (_job == LivingEconomyJob::Mail && !bot->GetMailSize())
        return {};
    LivingEconomyPlan result;
    for (ObjectGuid guid : AI_VALUE(GuidVector, "nearest npcs"))
    {
        Unit* unit = botAI->GetUnit(guid);
        Creature* npc = unit ? unit->ToCreature() : nullptr;
        NPCFlags const flag = _job == LivingEconomyJob::Auction ? UNIT_NPC_FLAG_AUCTIONEER : UNIT_NPC_FLAG_MAILBOX;
        if (npc && npc->IsAlive() && !npc->IsHostileTo(bot) && npc->HasNpcFlag(flag) &&
            bot->IsWithinDistInMap(npc, 40.0f))
        {
            result.target = guid;
            break;
        }
    }
    if (_job == LivingEconomyJob::Mail && result.target.IsEmpty())
        for (ObjectGuid guid : AI_VALUE(GuidVector, "nearest game objects no los"))
            if (GameObject* object = botAI->GetGameObject(guid))
                if (object->GetGoType() == GAMEOBJECT_TYPE_MAILBOX && bot->IsWithinDistInMap(object, 40.0f))
                {
                    result.target = guid;
                    break;
                }
    if (result.target.IsEmpty() || _job == LivingEconomyJob::Mail)
        return result;
    Stock const stock = ProfessionStock(bot);
    uint32 best = 0;
    uint32 const interval = static_cast<uint32>(GameTime::GetGameTime().count() / 60);
    for (Item* item : BagItems(bot))
        if (SaleCount(botAI, item, stock))
        {
            uint32 const score = LivingWorld::Mix(item->GetGUID().GetCounter() ^ interval);
            if (result.item.IsEmpty() || score > best)
            {
                result.item = item->GetGUID();
                best = score;
            }
        }
    if (result.item.IsEmpty())
    {
        // Nothing of its own to sell: a trader at the auction house looks for a bargain instead.
        if (!FlipScanDue(bot))
            return {};
        result.flip = true;
    }
    return result;
}

LivingMarketTrip LivingMarketTripValue::Calculate()
{
    LivingMarketTrip trip;
    if (!LivingProfessions::CanWork(botAI) || !IsTrader(bot))
        return trip;
    Stock const stock = ProfessionStock(bot);
    bool hasGoods = false;
    for (Item* item : BagItems(bot))
        if (SaleCount(botAI, item, stock))
        {
            hasGoods = true;
            break;
        }
    // With nothing to sell, a trader with spare gold still goes when a bargain scan is due: flips only
    // happen at the auction house, so without this trip none happened in 6 hours (2026-10-21).
    bool const bargainHunt = !hasGoods && FlipScanDue(bot) &&
                             LivingWorld::CanAffordTraining(bot->GetMoney(), LivingWorld::FlipTripBudget, bot->GetLevel());
    TravelMgr::NpcLocation auctioneer;
    if ((!hasGoods && !bargainHunt) || !sTravelMgr.SelectAuctioneerByMap(bot, auctioneer))
        return trip;
    trip.go = LivingWorld::WorthMarketTrip(bot->GetExactDist(auctioneer.loc));
    trip.destination = auctioneer.loc;
    return trip;
}

bool LivingEconomyAction::isUseful() { return LivingProfessions::CanWork(botAI); }
bool LivingEconomyAction::isPossible() { return !AI_VALUE(LivingEconomyPlan, getName()).target.IsEmpty(); }

bool LivingEconomyAction::Execute(Event /*event*/)
{
    if (!isUseful())
        return false;
    auto* value = context->GetValue<LivingEconomyPlan>(getName());
    auto const plan = value->Get();
    WorldObject* target = plan.target.IsGameObject() ? static_cast<WorldObject*>(botAI->GetGameObject(plan.target))
                                                     : static_cast<WorldObject*>(botAI->GetUnit(plan.target));
    if (!target || !bot->IsWithinDistInMap(target, 40.0f))
    {
        value->Set({});
        return false;
    }
    if (!bot->IsWithinDistInMap(target, INTERACTION_DISTANCE - 1.0f))
    {
        if (MoveNear(target, INTERACTION_DISTANCE - 1.0f))
            return true;
        value->Set({});
        return false;
    }
    value->Set({});
    if (_job == LivingEconomyJob::Auction && plan.flip)
        return PlayerbotWorldThreadProcessor::instance().QueueOperation(
            std::make_unique<LivingFlipOperation>(bot->GetGUID(), plan.target));
    if (_job == LivingEconomyJob::Auction)
        return PlayerbotWorldThreadProcessor::instance().QueueOperation(
            std::make_unique<LivingAuctionOperation>(bot->GetGUID(), plan));
    if (_job == LivingEconomyJob::Mail)
        return PlayerbotWorldThreadProcessor::instance().QueueOperation(
            std::make_unique<LivingMailOperation>(bot->GetGUID(), plan.target));
    Creature* npc = bot->GetNPCIfCanInteractWith(plan.target, UNIT_NPC_FLAG_VENDOR);
    auto const* inventory = npc ? npc->GetVendorItems() : nullptr;
    auto const* offer = inventory ? inventory->GetItem(plan.slot) : nullptr;
    auto const* item = sObjectMgr->GetItemTemplate(plan.entry);
    if (!offer || offer->item != plan.entry || offer->ExtendedCost || !item || !StillNeedsSupply(bot, plan, item))
        return false;
    uint32 const owned = bot->GetItemCount(plan.entry, false);
    uint32 const bundles = LivingWorld::PurchaseBundles(owned, plan.needed, item->BuyCount);
    uint64 const basePrice = uint64(item->BuyPrice) * bundles;
    if (!bundles || basePrice > LivingWorld::MoneyLimit)
        return false;
    uint32 const price = static_cast<uint32>(basePrice * bot->GetReputationPriceDiscount(npc));
    if (!LivingWorld::CanAffordTraining(bot->GetMoney(), price, bot->GetLevel()))
        return false;
    // This plan uses the NPC's ordinary inventory, not a previous custom gossip vendor.
    bot->GetSession()->SetCurrentVendor(0);
    bot->BuyItemFromVendorSlot(plan.target, plan.slot, plan.entry, static_cast<uint8>(bundles), NULL_BAG, NULL_SLOT);
    return bot->GetItemCount(plan.entry, false) > owned;
}
