/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CraftOrder.h"

#include "Bag.h"
#include "ChatHelper.h"
#include "CraftValue.h"
#include "GameTime.h"
#include "MailAction.h"
#include "Mail.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include "PlayerbotsDatabase.h"

#include <algorithm>
#include <mutex>
#include <sstream>

// Orders saved before a crash, read at startup and handed to each bot as it logs in.
static std::map<uint32, CraftData> savedOrders;
static std::mutex savedOrdersLock;

static bool IsGeneralBag(Bag const* bag) { return bag && bag->GetTemplate()->SubClass == ITEM_SUBCLASS_CONTAINER; }

static std::vector<Item*> FindBagItems(PlayerbotAI* botAI, uint32 itemId)
{
    std::vector<Item*> items = botAI->GetInventoryItems();
    std::erase_if(items, [itemId](Item* item) { return item->GetEntry() != itemId; });
    return items;
}

// Takes up to count items out of the bags, splitting the last stack when it holds more than is left to take.
static void TakeItems(PlayerbotAI* botAI, uint32 itemId, uint32 count, CharacterDatabaseTransaction trans,
                      std::vector<Item*>& taken)
{
    Player* bot = botAI->GetBot();
    for (Item* item : FindBagItems(botAI, itemId))
    {
        if (!count)
            return;

        uint32 const stack = item->GetCount();
        if (stack <= count)
        {
            bot->MoveItemFromInventory(item->GetBagSlot(), item->GetSlot(), true);
            item->DeleteFromInventoryDB(trans);
            taken.push_back(item);
            count -= stack;
            continue;
        }

        Item* part = item->CloneItem(count, bot);
        if (!part)
            return;

        item->SetCount(stack - count);
        item->SetState(ITEM_CHANGED, bot);
        taken.push_back(part);
        return;
    }
}

// The first empty slot in the backpack or a general-purpose bag, as a bag << 8 | slot position; 0 when there is none.
static uint16 FindEmptySlot(Player* bot)
{
    for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; ++slot)
    {
        if (!bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
            return (INVENTORY_SLOT_BAG_0 << 8) | slot;
    }

    for (uint8 bagSlot = INVENTORY_SLOT_BAG_START; bagSlot < INVENTORY_SLOT_BAG_END; ++bagSlot)
    {
        Bag* bag = bot->GetBagByPos(bagSlot);
        if (!IsGeneralBag(bag))
            continue;

        for (uint8 slot = 0; slot < bag->GetBagSize(); ++slot)
        {
            if (!bag->GetItemByPos(slot))
                return (bagSlot << 8) | slot;
        }
    }

    return 0;
}

// Moves count items of a stack into an empty general-purpose slot and returns the new stack.
static Item* SplitStack(Player* bot, Item* item, uint32 count)
{
    uint16 const pos = FindEmptySlot(bot);
    if (!pos)
        return nullptr;

    bot->SplitItem(item->GetPos(), pos, count);
    Item* part = bot->GetItemByPos(pos);
    return part && part->GetEntry() == item->GetEntry() ? part : nullptr;
}

static uint32 Now() { return uint32(GameTime::GetGameTime().count()); }

static void DeleteSaved(uint32 botGuid)
{
    PlayerbotsDatabasePreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_DEL_CRAFT_ORDER);
    stmt->SetData(0, botGuid);
    PlayerbotsDatabase.Execute(stmt);
}

static std::string RefundSubject()
{
    return PlayerbotTextMgr::instance().GetBotTextOrDefault("craft_refund_mail_subject", "Your crafting order", {});
}

// The fee is split over the mails by the items each carries, so returning one mail does not avoid it on the others.
static void MailItems(Player* bot, ObjectGuid owner, MailReceiver const& receiver, std::vector<Item*> const& items,
                      uint32 fee, CharacterDatabaseTransaction trans)
{
    std::vector<MailDraft> drafts;
    std::vector<uint32> mailItemCounts;
    std::string const subject = RefundSubject();
    for (size_t i = 0; i < items.size(); ++i)
    {
        if (i % MAX_MAIL_ITEMS == 0)
        {
            drafts.emplace_back(subject, "");
            mailItemCounts.push_back(0);
        }

        items[i]->SetOwnerGUID(owner);
        items[i]->SaveToDB(trans);
        drafts.back().AddItem(items[i]);
        mailItemCounts.back() += items[i]->GetCount();
    }

    std::vector<uint32> const cods = CraftRules::SplitFee(fee, mailItemCounts);
    for (size_t mail = 0; mail < drafts.size(); ++mail)
    {
        if (cods[mail])
            drafts[mail].AddCOD(cods[mail]);

        drafts[mail].SendMailTo(trans, receiver, MailSender(bot));
    }
}

static void TellOwner(PlayerbotAI* botAI, CraftData const& data, std::string const& text)
{
    if (Player* owner = ObjectAccessor::FindConnectedPlayer(data.owner))
        CraftOrder::Tell(botAI, owner, text);
}

void CraftOrder::Touch(CraftData& data) { data.lastProgress = getMSTime(); }

void CraftOrder::Tell(PlayerbotAI* botAI, Player* player, std::string const& text)
{
    if (player == botAI->GetMaster())
        botAI->TellMaster(text);
    else
        botAI->GetBot()->Whisper(text, LANG_UNIVERSAL, player);
}

uint32 CraftOrder::FreeBagSlots(Player* bot)
{
    uint32 free = 0;
    for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; ++slot)
    {
        if (!bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
            ++free;
    }

    for (uint8 bagSlot = INVENTORY_SLOT_BAG_START; bagSlot < INVENTORY_SLOT_BAG_END; ++bagSlot)
    {
        Bag* bag = bot->GetBagByPos(bagSlot);
        if (IsGeneralBag(bag))
            free += bag->GetFreeSlots();
    }

    return free;
}

// Matches Spell::DoCreateItem: the effect amount, at least one and at most a stack.
uint32 CraftOrder::GetMaxCreated(SpellInfo const* spellInfo, ItemTemplate const* proto)
{
    int32 created = 1;
    for (SpellEffectInfo const& effect : spellInfo->GetEffects())
    {
        if (effect.Effect == SPELL_EFFECT_CREATE_ITEM && effect.ItemType == proto->ItemId)
            created = std::max(created, effect.BasePoints + std::max(effect.DieSides, 1));
    }

    return std::min(uint32(created), std::max<uint32>(proto->GetMaxStackSize(), 1));
}

uint32 CraftOrder::GetOwedProducts(Player* bot, CraftData const& data)
{
    return data.OwedProducts(bot->GetItemCount(data.itemId));
}

void CraftOrder::SyncWithBags(Player* bot, CraftData& data)
{
    for (auto const& [item, reagent] : data.reagents)
        data.LimitObtained(item, bot->GetItemCount(item));
}

// Only random-account bots price their trades; a player's own bots craft for free.
uint32 CraftOrder::GetFee(Player* bot, CraftData const& data, uint32 crafts)
{
    if (!sPlayerbotAIConfig.IsInRandomAccountList(bot->GetSession()->GetAccountId()))
        return 0;

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(data.itemId);
    if (!proto)
        return 0;

    return CraftRules::CraftFee(proto->ItemLevel, proto->RequiredLevel, crafts);
}

void CraftOrder::TellStatus(PlayerbotAI* botAI)
{
    CraftData const& data = botAI->GetCraftOrder();
    if (data.IsEmpty())
    {
        botAI->TellMaster(
            PlayerbotTextMgr::instance().GetBotTextOrDefault("craft_reset", "I will not craft anything", {}));
        return;
    }

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(data.itemId);
    if (!proto)
        return;

    std::ostringstream reagentsOut;
    bool first = true;
    for (auto const& [item, reagent] : data.reagents)
    {
        ItemTemplate const* reagentProto = sObjectMgr->GetItemTemplate(item);
        if (!reagentProto)
            continue;

        if (!first)
            reagentsOut << ", ";
        first = false;

        reagentsOut << ChatHelper::FormatItem(reagentProto, reagent.perCraft * data.count);
        if (reagent.obtained)
            reagentsOut << "|cffffff00(x" << reagent.obtained << " given)|r";
    }

    botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "craft_summary", "I will craft %item using reagents: %reagents (craft fee: %money)",
        {{"%item", ChatHelper::FormatItem(proto, data.count)},
         {"%reagents", reagentsOut.str()},
         {"%money", ChatHelper::formatMoney(GetFee(botAI->GetBot(), data, data.count))}}));

    if (data.crafted)
    {
        botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "craft_progress", "Crafted %crafted of %count",
            {{"%crafted", std::to_string(data.crafted)}, {"%count", std::to_string(data.count)}}));
    }
}

// Only casts the order action started count: a recipe cast by hand would use the bot's own reagents.
void CraftOrder::OnItemCreated(PlayerbotAI* botAI, Item* item, uint32 itemCount)
{
    CraftData& data = botAI->GetCraftOrder();
    if (!data.castPending || data.IsEmpty() || data.IsCrafted() || item->GetEntry() != data.itemId)
        return;

    data.castPending = false;
    data.OnCrafted(itemCount);
    Touch(data);
    Save(botAI);

    if (data.IsCrafted())
    {
        TellOwner(botAI, data,
                  PlayerbotTextMgr::instance().GetBotTextOrDefault(
                      "craft_done", "I've crafted %item. Open a trade with me to collect it",
                      {{"%item", ChatHelper::FormatItem(item->GetTemplate(), data.made)}}));
    }
}

bool CraftOrder::CheckTrade(PlayerbotAI* botAI, Player* trader)
{
    CraftData const& data = botAI->GetCraftOrder();
    if (data.IsEmpty())
        return true;

    auto const refuse =
        [&](std::string const& key, std::string const& text, std::map<std::string, std::string> const& placeholders)
    {
        botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(key, text, placeholders));
        botAI->PlaySound(TEXT_EMOTE_NO);
        return false;
    };

    Player* bot = botAI->GetBot();
    bool const isCustomer = data.IsCustomer(trader->GetGUID());
    uint32 productsOffered = 0;
    for (uint32 slot = 0; slot < TRADE_SLOT_TRADED_COUNT; ++slot)
    {
        Item* item = bot->GetTradeData()->GetItem((TradeSlots)slot);
        if (item && data.IsReserved(item->GetEntry()))
        {
            if (item->GetEntry() != data.itemId || !isCustomer || !data.IsCrafted())
            {
                return refuse("craft_item_reserved", "%item is reserved for a crafting order",
                              {{"%item", ChatHelper::FormatItem(item->GetTemplate())}});
            }

            productsOffered += item->GetCount();
        }

        item = trader->GetTradeData()->GetItem((TradeSlots)slot);
        if (item && (item->GetEntry() == data.itemId || (data.IsRequired(item->GetEntry()) && !isCustomer)))
        {
            return refuse("trade_item_not_needed", "%item - I don't need this",
                          {{"%item", ChatHelper::FormatItem(item->GetTemplate())}});
        }
    }

    uint32 const owed = GetOwedProducts(bot, data);
    if (productsOffered > owed)
    {
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(data.itemId);
        return refuse("craft_collect_limit", "I can only hand over %count of %item",
                      {{"%count", std::to_string(owed)}, {"%item", proto ? ChatHelper::FormatItem(proto) : ""}});
    }

    return true;
}

bool CraftOrder::OfferCraftedItems(PlayerbotAI* botAI, Player* trader)
{
    Player* bot = botAI->GetBot();
    CraftData const& data = botAI->GetCraftOrder();
    if (!data.IsCrafted() || !data.IsCustomer(trader->GetGUID()))
        return false;

    uint32 const owed = GetOwedProducts(bot, data);
    uint32 offered = 0;
    uint8 tradeSlot = 0;
    for (Item* item : FindBagItems(botAI, data.itemId))
    {
        if (offered >= owed || tradeSlot >= TRADE_SLOT_TRADED_COUNT)
            break;

        // Never hand over more than is owed: the rest of the stack is the bot's own.
        if (item->GetCount() > owed - offered)
        {
            item = SplitStack(bot, item, owed - offered);
            if (!item)
                continue;
        }

        WorldPacket packet(CMSG_SET_TRADE_ITEM, 3);
        packet << tradeSlot++;
        packet << uint8(item->GetBagSlot());
        packet << uint8(item->GetSlot());
        bot->GetSession()->HandleSetTradeItemOpcode(packet);
        offered += item->GetCount();
    }

    return offered > 0;
}

void CraftOrder::OnTradeCompleted(PlayerbotAI* botAI, Player* trader, std::map<uint32, uint32> const& given,
                                  std::map<uint32, uint32> const& taken)
{
    CraftData& data = botAI->GetCraftOrder();
    if (!data.IsCustomer(trader->GetGUID()))
        return;

    bool reagentsReceived = false;
    for (auto const& [item, itemCount] : given)
        reagentsReceived |= data.AddObtained(item, itemCount);

    auto const products = taken.find(data.itemId);
    if (products != taken.end())
        data.OnDelivered(products->second);

    Touch(data);

    // Anything the customer gave beyond what the crafts used goes back by mail.
    if (products != taken.end() && data.IsCrafted() && !GetOwedProducts(botAI->GetBot(), data))
    {
        Refund(botAI);
        return;
    }

    Save(botAI);
    if (reagentsReceived)
        TellStatus(botAI);
}

void CraftOrder::Refund(PlayerbotAI* botAI)
{
    CraftData& data = botAI->GetCraftOrder();
    if (data.IsEmpty())
        return;

    Player* bot = botAI->GetBot();
    SyncWithBags(bot, data);
    uint32 const owedProducts = GetOwedProducts(bot, data);

    CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();

    std::vector<Item*> reagents;
    for (auto const& [item, reagent] : data.reagents)
        TakeItems(botAI, item, reagent.obtained, trans, reagents);

    std::vector<Item*> products;
    TakeItems(botAI, data.itemId, owedProducts, trans, products);

    if (!reagents.empty() || !products.empty())
    {
        Player* receiver = ObjectAccessor::FindConnectedPlayer(data.owner);
        MailReceiver const mailReceiver(receiver, data.owner.GetCounter());
        uint32 const fee = data.FeePaid() ? 0 : GetFee(bot, data, data.crafted);

        MailItems(bot, data.owner, mailReceiver, reagents, 0, trans);
        MailItems(bot, data.owner, mailReceiver, products, fee, trans);
        data.WatchMail(Now());

        if (ItemTemplate const* proto = sObjectMgr->GetItemTemplate(data.itemId))
        {
            TellOwner(botAI, data,
                      PlayerbotTextMgr::instance().GetBotTextOrDefault(
                          "craft_refund_mailed", "I've mailed you back what's left of your order for %item",
                          {{"%item", ChatHelper::FormatItem(proto)}}));
        }
    }

    bot->SaveInventoryAndGoldToDB(trans);
    CharacterDatabase.CommitTransaction(trans);

    data.Reset();
    Save(botAI);
}

void CraftOrder::Save(PlayerbotAI* botAI)
{
    CraftData const& data = botAI->GetCraftOrder();
    uint32 const botGuid = botAI->GetBot()->GetGUID().GetCounter();

    if (data.IsIdle(Now()))
    {
        DeleteSaved(botGuid);
        return;
    }

    PlayerbotsDatabasePreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_REP_CRAFT_ORDER);
    stmt->SetData(0, botGuid);
    stmt->SetData(1, data.owner.GetCounter());
    stmt->SetData(2, data.itemId);
    stmt->SetData(3, data.spellId);
    stmt->SetData(4, data.count);
    stmt->SetData(5, data.crafted);
    stmt->SetData(6, data.made);
    stmt->SetData(7, data.delivered);
    stmt->SetData(8, data.ownProducts);
    stmt->SetData(9, CraftRules::FormatReagents(data.reagents));
    stmt->SetData(10, data.mailWatchUntil);
    PlayerbotsDatabase.Execute(stmt);
}

void CraftOrder::LoadAll()
{
    PlayerbotsDatabasePreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_CRAFT_ORDERS);
    PreparedQueryResult result = PlayerbotsDatabase.Query(stmt);
    if (!result)
        return;

    uint32 const now = Now();
    std::lock_guard<std::mutex> lock(savedOrdersLock);
    do
    {
        Field* fields = result->Fetch();
        uint32 const botGuid = fields[0].Get<uint32>();
        CraftData data;
        data.owner = ObjectGuid::Create<HighGuid::Player>(fields[1].Get<uint32>());
        data.itemId = fields[2].Get<uint32>();
        data.spellId = fields[3].Get<uint32>();
        data.count = fields[4].Get<uint32>();
        data.crafted = fields[5].Get<uint32>();
        data.made = fields[6].Get<uint32>();
        data.delivered = fields[7].Get<uint32>();
        data.ownProducts = fields[8].Get<uint32>();
        if (!CraftRules::ParseReagents(fields[9].Get<std::string>(), data.reagents))
            LOG_ERROR("playerbots", "Crafting order of bot {} has malformed reagents", botGuid);
        data.mailWatchUntil = fields[10].Get<uint32>();

        // A refund's mail window that closed while the server was down.
        if (data.IsIdle(now))
        {
            DeleteSaved(botGuid);
            continue;
        }

        savedOrders[botGuid] = std::move(data);
    } while (result->NextRow());
}

void CraftOrder::Load(PlayerbotAI* botAI)
{
    std::lock_guard<std::mutex> lock(savedOrdersLock);
    auto const itr = savedOrders.find(botAI->GetBot()->GetGUID().GetCounter());
    if (itr == savedOrders.end())
        return;

    CraftData& data = botAI->GetCraftOrder();
    data = std::move(itr->second);
    savedOrders.erase(itr);
    Touch(data);
}

// A payment for a refund, which keeps the subject of the mail it paid for, or a refund the customer sent back, that has
// been in the mailbox for at least waited seconds.
static bool IsCollectable(Mail const* mail, uint32 now, std::string const& subject, uint32 waited = 0)
{
    if (mail->state == MAIL_STATE_DELETED || !CraftRules::HasWaited(uint32(mail->deliver_time), now, waited))
        return false;

    bool const payment = mail->IsCODPayment() && mail->money;
    return (payment || mail->IsReturnedMail()) && mail->subject == subject;
}

bool CraftOrder::HasCollectableMail(PlayerbotAI* botAI, uint32 waited)
{
    Player* bot = botAI->GetBot();
    uint32 const now = Now();
    if (!botAI->GetCraftOrder().IsWatchingMail(now) || !bot->GetMailSize())
        return false;

    std::string const subject = RefundSubject();
    uint32 const freeSlots = FreeBagSlots(bot);
    for (Mail* mail : bot->GetMails())
    {
        if (IsCollectable(mail, now, subject, waited) && mail->items.size() <= freeSlots)
            return true;
    }

    return false;
}

bool CraftMailOverdueValue::Calculate()
{
    uint32 const delay = sPlayerbotAIConfig.CraftMailTripDelay;
    return delay && CraftOrder::HasCollectableMail(botAI, delay);
}

void CraftOrder::CollectMail(PlayerbotAI* botAI, ObjectGuid mailbox)
{
    Player* bot = botAI->GetBot();
    uint32 const now = Now();
    std::string const subject = RefundSubject();
    std::vector<Mail*> mails;
    for (Mail* mail : bot->GetMails())
    {
        if (IsCollectable(mail, now, subject))
            mails.push_back(mail);
    }

    uint32 freeSlots = FreeBagSlots(bot);
    for (Mail* mail : mails)
    {
        if (mail->items.size() > freeSlots)
            continue;

        freeSlots -= mail->items.size();
        if (mail->money)
        {
            WorldPacket packet;
            packet << mailbox << mail->messageID;
            bot->GetSession()->HandleMailTakeMoney(packet);
        }

        std::vector<uint32> itemGuids;
        for (MailItemInfo const& info : mail->items)
            itemGuids.push_back(info.item_guid);

        for (uint32 itemGuid : itemGuids)
        {
            WorldPacket packet;
            packet << mailbox << mail->messageID << itemGuid;
            bot->GetSession()->HandleMailTakeItem(packet);
        }

        // Deleting a mail destroys what is still in it, so only an emptied one goes.
        if (!mail->money && mail->items.empty())
            MailProcessor::RemoveMail(bot, mail->messageID, mailbox);
    }
}
