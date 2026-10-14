/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "LootAction.h"

#include "BroadcastHelper.h"
#include "ChatHelper.h"
#include "Event.h"
#include "GuildMgr.h"
#include "GuildTaskMgr.h"
#include "ItemUsageValue.h"
#include "LootObjectStack.h"
#include "LootStrategyValue.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotSpellRepository.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "UseItemAction.h"

bool LootAction::Execute(Event /*event*/)
{
    if (AI_VALUE(LootObjectStack*, "available loot")->IsLootPending() || bot->GetLootGUID() ||
        bot->IsNonMeleeSpellCast(false))
        return false;

    if (!AI_VALUE(bool, "has available loot"))
        return false;

    LootObject prevLoot = AI_VALUE(LootObject, "loot target");
    LootObject const& lootObject =
        AI_VALUE(LootObjectStack*, "available loot")->GetLoot(sPlayerbotAIConfig.LootDistance);

    if (!prevLoot.IsEmpty() && prevLoot.guid != lootObject.guid)
    {
        WorldPacket* packet = new WorldPacket(CMSG_LOOT_RELEASE, 8);
        *packet << prevLoot.guid;
        bot->GetSession()->QueuePacket(packet);
        // bot->GetSession()->HandleLootReleaseOpcode(packet);
    }

    if (lootObject.guid.IsGameObject() && sPlayerbotAIConfig.DisallowedGameObjects.contains(lootObject.guid.GetEntry()))
    {
        return false;  // Game object ID is disallowed, so do not proceed
    }
    else
    {
        context->GetValue<LootObject>("loot target")->Set(lootObject);
        return true;
    }
}

bool LootAction::isUseful()
{
    return sPlayerbotAIConfig.FreeMethodLoot || !bot->GetGroup() || bot->GetGroup()->GetLootMethod() != FREE_FOR_ALL ||
           IsSelfBot(bot);
}

enum ProfessionSpells
{
    ALCHEMY = 2259,
    BLACKSMITHING = 2018,
    COOKING = 2550,
    ENCHANTING = 7411,
    ENGINEERING = 49383,
    FIRST_AID = 3273,
    FISHING = 7620,
    HERB_GATHERING = 2366,
    INSCRIPTION = 45357,
    JEWELCRAFTING = 25229,
    MINING = 2575,
    SKINNING = 8613,
    TAILORING = 3908
};

bool OpenLootAction::Execute(Event /*event*/)
{
    LootObjectStack* availableLoot = AI_VALUE(LootObjectStack*, "available loot");
    if (availableLoot->IsLootPending() || bot->GetLootGUID() || bot->IsNonMeleeSpellCast(false))
        return false;

    LootObject lootObject = AI_VALUE(LootObject, "loot target");
    if (!availableLoot->CanAttemptLoot(lootObject.guid))
        return false;

    bool result = DoLoot(lootObject);
    if (result)
        availableLoot->BeginLoot(lootObject.guid);
    else if (!lootObject.IsEmpty() && !bot->isMoving() && !bot->IsMounted())
    {
        availableLoot->DeferLoot(lootObject.guid);
        if (AI_VALUE(LootObject, "loot target").guid == lootObject.guid)
            context->GetValue<LootObject>("loot target")->Set(LootObject());
    }

    return result;
}

bool OpenLootAction::DoLoot(LootObject& lootObject)
{
    if (lootObject.IsEmpty() || !lootObject.IsLootPossible(bot))
        return false;

    Creature* creature = botAI->GetCreature(lootObject.guid);
    if (creature && bot->GetDistance(creature) > INTERACTION_DISTANCE - 2.0f)
        return false;

    if (bot->isMoving())
    {
        bot->StopMoving();
        botAI->SetNextCheckDelay(sPlayerbotAIConfig.LootDelay);
        return false;
    }

    if (bot->IsMounted())
    {
        bot->RemoveAurasByType(SPELL_AURA_MOUNTED);
        botAI->SetNextCheckDelay(sPlayerbotAIConfig.ReactDelay);
        return false;
    }

    if (creature && creature->HasFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE))
    {
        WorldPacket* packet = new WorldPacket(CMSG_LOOT, 8);
        *packet << lootObject.guid;
        bot->GetSession()->QueuePacket(packet);
        // bot->GetSession()->HandleLootOpcode(packet);
        botAI->SetNextCheckDelay(sPlayerbotAIConfig.LootDelay);
        return true;
    }

    if (creature)
    {
        SkillType skill = creature->GetCreatureTemplate()->GetRequiredLootSkill();
        if (!CanOpenLock(skill, lootObject.reqSkillValue))
            return false;

        switch (skill)
        {
            case SKILL_ENGINEERING:
                return botAI->HasSkill(SKILL_ENGINEERING) ? botAI->CastSpell(ENGINEERING, creature) : false;
            case SKILL_HERBALISM:
                return botAI->HasSkill(SKILL_HERBALISM) ? botAI->CastSpell(32605, creature) : false;
            case SKILL_MINING:
                return botAI->HasSkill(SKILL_MINING) ? botAI->CastSpell(32606, creature) : false;
            default:
                return botAI->HasSkill(SKILL_SKINNING) ? botAI->CastSpell(SKINNING, creature) : false;
        }
    }

    GameObject* go = botAI->GetGameObject(lootObject.guid);
    if (go && bot->GetDistance(go) > INTERACTION_DISTANCE - 2.0f)
        return false;

    if (go && (go->GetGoState() != GO_STATE_READY))
        return false;

    // This prevents dungeon chests like Tribunal Chest (Halls of Stone) from being ninja'd by the bots.
    // Quest objects carry the same flag but are gated on quest state, which ActivateToQuest answers.
    if (go && go->HasFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND) && !go->ActivateToQuest(bot))
        return false;

    // This prevents raid chests like Gunship Armory (ICC) from being ninja'd by the bots
    if (go && go->HasFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE))
        return false;

    if (lootObject.reqItem)
    {
        Item* key = bot->GetItemByEntry(lootObject.reqItem);
        if (!key)
            return false;

        UseItemAction useItem(botAI);
        return useItem.UseItemOnGameObject(key, lootObject.guid);
    }

    if (lootObject.skillId == SKILL_MINING)
        return botAI->HasSkill(SKILL_MINING) ? botAI->CastSpell(MINING, bot) : false;

    if (lootObject.skillId == SKILL_HERBALISM)
        return botAI->HasSkill(SKILL_HERBALISM) ? botAI->CastSpell(HERB_GATHERING, bot) : false;

    uint32 spellId = GetOpeningSpell(lootObject);
    if (!spellId)
        return false;

    return botAI->CastSpell(spellId, bot);
}

uint32 OpenLootAction::GetOpeningSpell(LootObject& lootObject)
{
    if (GameObject* go = botAI->GetGameObject(lootObject.guid))
        if (go->isSpawned())
            return GetOpeningSpell(lootObject, go);

    return 0;
}

uint32 OpenLootAction::GetOpeningSpell(LootObject& lootObject, GameObject* go)
{
    for (PlayerSpellMap::iterator itr = bot->GetSpellMap().begin(); itr != bot->GetSpellMap().end(); ++itr)
    {
        uint32 spellId = itr->first;

        if (itr->second->State == PLAYERSPELL_REMOVED || !itr->second->Active)
            continue;

        if (spellId == MINING || spellId == HERB_GATHERING)
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
            continue;

        if (CanOpenLock(lootObject, spellInfo, go))
            return spellId;
    }

    for (uint32 spellId : PlayerbotSpellRepository::Instance().GetOpeningSpells(lootObject.GetLockType()))
    {
        if (spellId == MINING || spellId == HERB_GATHERING)
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (spellInfo && CanOpenLock(lootObject, spellInfo, go))
            return spellId;
    }

    return sPlayerbotAIConfig.OpenGoSpell;
}

bool OpenLootAction::CanOpenLock(LootObject& lootObject, SpellInfo const* spellInfo, GameObject* /*go*/)
{
    if (!lootObject.GetLockType())
        return false;

    for (uint8 effIndex = 0; effIndex <= EFFECT_2; effIndex++)
    {
        if (spellInfo->Effects[effIndex].Effect != SPELL_EFFECT_OPEN_LOCK &&
            spellInfo->Effects[effIndex].Effect != SPELL_EFFECT_SKINNING)
            continue;

        if (uint32(spellInfo->Effects[effIndex].MiscValue) == lootObject.GetLockType())
            return CanOpenLock(lootObject.skillId, lootObject.reqSkillValue);
    }

    return false;
}

bool OpenLootAction::CanOpenLock(uint32 skillId, uint32 reqSkillValue)
{
    if (skillId == SKILL_NONE)
        return true;

    uint32 skillValue = bot->GetSkillValue(skillId);
    return skillValue >= reqSkillValue || !reqSkillValue;
}

/*
uint32 StoreLootAction::RoundPrice(double price)
{
    if (price < 100)
    {
        return (uint32)price;
    }

    if (price < 10000)
    {
        return (uint32)(price / 100.0) * 100;
    }

    if (price < 100000)
    {
        return (uint32)(price / 1000.0) * 1000;
    }

    return (uint32)(price / 10000.0) * 10000;
}

bool StoreLootAction::AuctionItem(uint32 itemId)
{
    ItemTemplate const* proto = sItemStorage.LookupEntry<ItemTemplate>(itemId);
    if (!proto)
        return false;

    if (!proto || proto->Bonding == BIND_WHEN_PICKED_UP || proto->Bonding == BIND_QUEST_ITEM)
        return false;

    Item* oldItem = bot->GetItemByEntry(itemId);
    if (!oldItem)
        return false;

    AuctionHouseEntry const* ahEntry = AuctionHouseMgr::GetAuctionHouseEntry(unit->getFaction());
    if (!ahEntry)
        return false;

    AuctionHouseObject* auctionHouse = sAuctionMgr->GetAuctionsMap(ahEntry);

    uint32 price = oldItem->GetCount() * proto->BuyPrice * sRandomPlayerbotMgr.GetBuyMultiplier(bot);

uint32 stackCount = urand(1, proto->GetMaxStackSize());
    if (!price || !stackCount)
        return false;

    if (!stackCount)
        stackCount = 1;

    if (urand(0, 100) <= sAhBotConfig.underPriceProbability * 100)
        price = price * 100 / urand(100, 200);

    uint32 bidPrice = RoundPrice(stackCount * price);
    uint32 buyoutPrice = RoundPrice(stackCount * urand(price, 4 * price / 3));

    Item* item = Item::CreateItem(proto->ItemId, stackCount);
    if (!item)
        return false;

    uint32 auction_time = uint32(urand(8, 24) * HOUR * sWorld->getConfig(CONFIG_FLOAT_RATE_AUCTION_TIME));

    AuctionEntry* auctionEntry = new AuctionEntry;
    auctionEntry->Id = sObjectMgr->GenerateAuctionID();
    auctionEntry->itemGuidLow = item->GetGUID().GetCounter();
    auctionEntry->itemTemplate = item->GetEntry();
    auctionEntry->itemCount = item->GetCount();
    auctionEntry->itemRandomPropertyId = item->GetItemRandomPropertyId();
    auctionEntry->owner = bot->GetGUID().GetCounter();
    auctionEntry->startbid = bidPrice;
    auctionEntry->bidder = 0;
    auctionEntry->bid = 0;
    auctionEntry->buyout = buyoutPrice;
    auctionEntry->expireTime = time(nullptr) + auction_time;
    //auctionEntry->moneyDeliveryTime = 0;
    auctionEntry->deposit = 0;
    auctionEntry->auctionHouseEntry = ahEntry;

    auctionHouse->AddAuction(auctionEntry);

    sAuctionMgr.AddAItem(item);

    item->SaveToDB();
    auctionEntry->SaveToDB();

    LOG_ERROR("playerbots", "AhBot {} added {} of {} to auction {} for {}..{}", bot->GetName().c_str(), stackCount,
proto->Name1.c_str(), 1, bidPrice, buyoutPrice);

    if (oldItem->GetCount() > stackCount)
        oldItem->SetCount(oldItem->GetCount() - stackCount);
    else
        bot->RemoveItem(item->GetBagSlot(), item->GetSlot(), true);

    return true;
}
*/

bool StoreLootAction::Execute(Event event)
{
    WorldPacket p(event.getPacket());  // (8+1+4+1+1+4+4+4+4+4+1)
    ObjectGuid guid;
    uint8 loot_type;
    uint32 gold = 0;
    uint8 items = 0;

    p.rpos(0);
    if (p.size() < 9)
        return false;

    p >> guid;       // 8 corpse/gameobject guid
    p >> loot_type;  // 1 loot type

    LootObjectStack* availableLoot = AI_VALUE(LootObjectStack*, "available loot");
    ObjectGuid currentLoot = bot->GetLootGUID();
    if (loot_type == LOOT_NONE)
    {
        availableLoot->RetryLoot(guid);
        if (AI_VALUE(LootObject, "loot target").guid == guid)
            context->GetValue<LootObject>("loot target")->Set(LootObject());
        return false;
    }

    if (currentLoot && currentLoot != guid)
    {
        availableLoot->CancelLoot(guid);
        return false;
    }

    if (p.size() < 14)
        return false;

    p >> gold;
    p >> items;
    if (p.size() - p.rpos() < size_t(items) * 22)
        return false;

    availableLoot->LootOpened(guid);

    if (!currentLoot)
        bot->SetLootGUID(guid);

    if (gold > 0)
    {
        WorldPacket* packet = new WorldPacket(CMSG_LOOT_MONEY, 0);
        bot->GetSession()->QueuePacket(packet);
        // bot->GetSession()->HandleLootMoneyOpcode(packet);
    }

    for (uint8 i = 0; i < items; ++i)
    {
        uint32 itemid;
        uint32 itemcount;
        uint8 lootslot_type;
        uint8 itemindex;

        p >> itemindex;
        p >> itemid;
        p >> itemcount;
        p.read_skip<uint32>();  // display id
        p.read_skip<uint32>();  // randomSuffix
        p.read_skip<uint32>();  // randomPropertyId
        p >> lootslot_type;     // 0 = can get, 1 = look only, 2 = master get

        if (lootslot_type != LOOT_SLOT_TYPE_ALLOW_LOOT && lootslot_type != LOOT_SLOT_TYPE_OWNER)
            continue;

        if (loot_type != LOOT_SKINNING && !IsLootAllowed(itemid, botAI))
            continue;

        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemid);
        if (!proto)
            continue;

        if (!IsRealPlayer(botAI->GetMaster()) && AI_VALUE(uint8, "bag space") > 80)
        {
            uint32 maxStack = proto->GetMaxStackSize();
            if (maxStack == 1)
                continue;

            std::vector<Item*> found = parseItems(chat->FormatItem(proto));

            bool hasFreeStack = false;

            for (auto stack : found)
            {
                if (stack->GetCount() + itemcount < maxStack)
                {
                    hasFreeStack = true;
                    break;
                }
            }

            if (!hasFreeStack)
                continue;
        }

        Player* master = botAI->GetMaster();
        if (sRandomPlayerbotMgr.IsRandomBot(bot) && master)
        {
            uint32 price = itemcount * proto->BuyPrice * sRandomPlayerbotMgr.GetBuyMultiplier(bot) + gold;
            if (price)
                sRandomPlayerbotMgr.AddTradeDiscount(bot, master, price);

            if (Group* group = bot->GetGroup())
                for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
                    if (ref->GetSource() != bot)
                        GuildTaskMgr::instance().CheckItemTask(itemid, itemcount, ref->GetSource(), bot);
        }

        WorldPacket* packet = new WorldPacket(CMSG_AUTOSTORE_LOOT_ITEM, 1);
        *packet << itemindex;
        bot->GetSession()->QueuePacket(packet);
        // bot->GetSession()->HandleAutostoreLootItemOpcode(packet);
        botAI->SetNextCheckDelay(sPlayerbotAIConfig.LootDelay);

        if (proto->Quality > ITEM_QUALITY_NORMAL && !urand(0, 50) &&
            botAI->HasStrategy("emote", BOT_STATE_NON_COMBAT) && sPlayerbotAIConfig.RandomBotEmote)
            botAI->PlayEmote(TEXT_EMOTE_CHEER);

        if (proto->Quality >= ITEM_QUALITY_RARE && !urand(0, 1) && botAI->HasStrategy("emote", BOT_STATE_NON_COMBAT) &&
            sPlayerbotAIConfig.RandomBotEmote)
            botAI->PlayEmote(TEXT_EMOTE_CHEER);

        BroadcastHelper::BroadcastLootingItem(botAI, bot, proto);
    }

    availableLoot->Remove(guid);
    if (AI_VALUE(LootObject, "loot target").guid == guid)
        context->GetValue<LootObject>("loot target")->Set(LootObject());

    // release loot
    WorldPacket* packet = new WorldPacket(CMSG_LOOT_RELEASE, 8);
    *packet << guid;
    bot->GetSession()->QueuePacket(packet);
    // bot->GetSession()->HandleLootReleaseOpcode(packet);
    return true;
}

bool StoreLootAction::IsLootAllowed(uint32 itemid, PlayerbotAI* botAI)
{
    AiObjectContext* context = botAI->GetAiObjectContext();
    LootStrategy* lootStrategy = AI_VALUE(LootStrategy*, "loot strategy");

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemid);
    if (!proto)
        return false;

    std::set<uint32>& lootItems = AI_VALUE(std::set<uint32>&, "always loot list");
    if (lootItems.find(itemid) != lootItems.end())
        return true;

    uint32 max = proto->MaxCount;
    if (max > 0 && botAI->GetBot()->HasItemCount(itemid, max, true))
        return false;

    if (proto->StartQuest)
    {
        return true;
    }

    for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 entry = botAI->GetBot()->GetQuestSlotQuestId(slot);
        Quest const* quest = sObjectMgr->GetQuestTemplate(entry);
        if (!quest)
            continue;

        for (uint8 i = 0; i < 4; i++)
        {
            if (quest->RequiredItemId[i] == itemid)
            {
                // if (AI_VALUE2(uint32, "item count", proto->Name1) < quest->RequiredItemCount[i])
                // {
                //     if (botAI->GetMaster() && sPlayerbotAIConfig.SyncQuestWithPlayer)
                //         return false; //Quest is autocomplete for the bot so no item needed.
                // }

                return true;
            }
        }
    }

    // if (proto->Bonding == BIND_QUEST_ITEM ||  //Still testing if it works ok without these lines.
    //     proto->Bonding == BIND_QUEST_ITEM1 || //Eventually this has to be removed.
    //     proto->Class == ITEM_CLASS_QUEST)
    //{

    bool canLoot = lootStrategy->CanLoot(proto, context);
    // if (canLoot && proto->Bonding == BIND_WHEN_PICKED_UP && IsRealPlayer(botAI->GetMaster()))
    // canLoot = sPlayerbotAIConfig.IsInRandomAccountList(botAI->GetBot()->GetSession()->GetAccountId());

    return canLoot;
}

bool ReleaseLootAction::Execute(Event /*event*/)
{
    ObjectGuid guid = bot->GetLootGUID();
    if (!guid)
        return false;

    WorldPacket* packet = new WorldPacket(CMSG_LOOT_RELEASE, 8);
    *packet << guid;
    bot->GetSession()->QueuePacket(packet);
    return true;
}
