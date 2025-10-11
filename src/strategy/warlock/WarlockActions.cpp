/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "WarlockActions.h"
#include "RitualActions.h"
#include "MovementActions.h"
#include "Battleground.h"
#include "Log.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"

#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include "Event.h"
#include "Item.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "Unit.h"
#include "Timer.h"
#include <unordered_map>
#include <mutex>

// Global constants for spells and items
namespace WarlockConstants
{
    // Ritual of Souls spells
    const uint32 RITUAL_OF_SOULS_RANK_1 = 29893;
    const uint32 RITUAL_OF_SOULS_RANK_2 = 58887;
    
    // Soul Portal game objects
    const uint32 SOUL_PORTAL_RANK_1 = 181622;
    const uint32 SOUL_PORTAL_RANK_2 = 193168;
    
    // Soul Well game objects
    const uint32 SOUL_WELL_RANK_1 = 181621;
    const uint32 SOUL_WELL_RANK_2 = 193169;
    const uint32 SOUL_WELL_RANK_2_VARIANT_1 = 193170;
    const uint32 SOUL_WELL_RANK_2_VARIANT_2 = 193171;
    
    // Soul Shard item
    const uint32 SOUL_SHARD_ITEM = 6265;
    
    // Healthstone items
    const uint32 MINOR_HEALTHSTONE = 5512;
    const uint32 LESSER_HEALTHSTONE = 5511;
    const uint32 MAJOR_HEALTHSTONE = 9421;
    const uint32 MINOR_HEALTHSTONE_ALT = 19004;
    const uint32 LESSER_HEALTHSTONE_ALT = 19005;
    const uint32 FEL_HEALTHSTONE = 36892;
    const uint32 DEMONIC_HEALTHSTONE = 22103;
    
    // Soulstone items
    const uint32 MINOR_SOULSTONE = 5232;
    const uint32 LESSER_SOULSTONE = 16892;
    const uint32 SOULSTONE = 16893;
    const uint32 GREATER_SOULSTONE = 16895;
    const uint32 MAJOR_SOULSTONE = 16896;
    const uint32 MASTER_SOULSTONE = 22116;
    const uint32 DEMONIC_SOULSTONE = 36895;
    
    // Soulstone spells
    const uint32 SOULSTONE_SPELL_MINOR = 20707;
    const uint32 SOULSTONE_SPELL_LESSER = 20762;
    const uint32 SOULSTONE_SPELL_NORMAL = 20763;
    const uint32 SOULSTONE_SPELL_GREATER = 20764;
    const uint32 SOULSTONE_SPELL_MAJOR = 20765;
    const uint32 SOULSTONE_SPELL_MASTER = 27239;
    const uint32 SOULSTONE_SPELL_DEMONIC = 47883;
    
    // Firestone spells
    const uint32 CREATE_FIRESTONE_RANK_1 = 6366;
    const uint32 CREATE_FIRESTONE_RANK_2 = 17951;
    const uint32 CREATE_FIRESTONE_RANK_3 = 17952;
    const uint32 CREATE_FIRESTONE_RANK_4 = 17953;
    const uint32 CREATE_FIRESTONE_RANK_5 = 27250;
    const uint32 CREATE_FIRESTONE_RANK_6 = 60219;
    const uint32 CREATE_FIRESTONE_RANK_7 = 60220;

    // Seed of Corruption spells
    const uint32 SEED_OF_CORRUPTION_RANK_1 = 27243;
    const uint32 SEED_OF_CORRUPTION_RANK_2 = 47835;
    const uint32 SEED_OF_CORRUPTION_RANK_3 = 47836;
}

// Function declared in RitualActions.cpp
static std::unordered_map<ObjectGuid, uint32> lastRitualUsage;
static std::unordered_map<ObjectGuid, bool> hasMovedForRitual;
std::unordered_map<ObjectGuid, uint32> lastRitualCreation;

// Track if bot has completed ritual interaction to avoid loops
extern std::unordered_map<uint64, bool> hasCompletedRitualInteraction;

// Reservation map for soulstone targets (GUID -> reservation expiry in ms)
static std::unordered_map<ObjectGuid, uint32> soulstoneReservations;
static std::mutex soulstoneReservationsMutex;

// Checks if the bot has less than 26 soul shards, and if so, allows casting Drain Soul
bool CastDrainSoulAction::isUseful() 
{ 
    return AI_VALUE2(uint32, "item count", "soul shard") < 26; 
}

// Checks if the bot's health is above a certain threshold, and if so, allows casting Life Tap
bool CastLifeTapAction::isUseful() 
{ 
    return AI_VALUE2(uint8, "health", "self target") > sPlayerbotAIConfig->lowHealth; 
}

// Checks if the target marked with the moon icon can be banished
bool CastBanishOnCcAction::isPossible()
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    // Only possible on elementals or demons
    uint32 creatureType = target->GetCreatureType();
    if (creatureType != CREATURE_TYPE_DEMON && creatureType != CREATURE_TYPE_ELEMENTAL)
        return false;

    // Use base class to check spell available, range, etc
    return CastCrowdControlSpellAction::isPossible();
}

// Checks if the target marked with the moon icon can be feared
bool CastFearOnCcAction::isPossible()
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    // Fear cannot be cast on mechanical or undead creatures
    uint32 creatureType = target->GetCreatureType();
    if (creatureType == CREATURE_TYPE_MECHANICAL || creatureType == CREATURE_TYPE_UNDEAD)
        return false;

    // Use base class to check spell available, range, etc
    return CastCrowdControlSpellAction::isPossible();
}

// Checks if the enemies are close enough to use Shadowflame
bool CastShadowflameAction::isUseful()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;
    
    bool facingTarget = AI_VALUE2(bool, "facing", "current target");
    bool targetClose = bot->IsWithinCombatRange(target, 7.0f);  // 7 yard cone
    return facingTarget && targetClose;
}

// Checks if the bot knows Seed of Corruption, and prevents the use of Rain of Fire if it does
bool CastRainOfFireAction::isUseful()
{
    Unit* target = GetTarget();
    if (!target)
        return false;
    
    // Seed of Corruption spell IDs (constants declared above)
    if (bot->HasSpell(WarlockConstants::SEED_OF_CORRUPTION_RANK_1) ||
        bot->HasSpell(WarlockConstants::SEED_OF_CORRUPTION_RANK_2) ||
        bot->HasSpell(WarlockConstants::SEED_OF_CORRUPTION_RANK_3))
        return false;
    
    return true;
}

// Checks if the enemies are close enough to use Hellfire
bool CastHellfireAction::isUseful()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    return bot->IsWithinCombatRange(target, 5.0f);  // 5 yard AoE radius
}

// Checks if the "meta melee aoe" strategy is active, OR if the bot is in melee range of the target
bool CastImmolationAuraAction::isUseful()
{
    if (botAI->HasStrategy("meta melee", BOT_STATE_COMBAT))
        return true;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    if (!bot->HasAura(47241))  // 47241 is Metamorphosis spell ID (WotLK)
        return false;

    return bot->IsWithinCombatRange(target, 5.0f);  // 5 yard AoE radius
}

// Checks if the "warlock tank" strategy is active, and if so, prevents the use of Soulshatter
bool CastSoulshatterAction::isUseful()
{
    if (botAI->HasStrategy("tank", BOT_STATE_COMBAT))
        return false;
    return true;
}

// Checks if the bot has enough bag space to create a soul shard, then does so
bool CreateSoulShardAction::Execute(Event event)
{
    if (!bot)
        return false;

    ItemPosCountVec dest;
    uint32 count = 1;
    if (bot->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, WarlockConstants::SOUL_SHARD_ITEM, count) == EQUIP_ERR_OK)
    {
        bot->StoreNewItem(dest, WarlockConstants::SOUL_SHARD_ITEM, true, Item::GenerateItemRandomPropertyId(WarlockConstants::SOUL_SHARD_ITEM));
        SQLTransaction<CharacterDatabaseConnection> trans = CharacterDatabase.BeginTransaction();
        bot->SaveInventoryAndGoldToDB(trans);
        CharacterDatabase.CommitTransaction(trans);
        return true;
    }
    return false;
}

// Checks if the bot has less than 6 soul shards, allowing the creation of a new one
bool CreateSoulShardAction::isUseful()
{
    if (!bot)
        return false;

    uint32 currentShards = bot->GetItemCount(WarlockConstants::SOUL_SHARD_ITEM, false);  // false = only bags
    const uint32 SHARD_CAP = 6;                                    // adjust as needed

    // Only allow if under cap AND there is space for a new shard
    ItemPosCountVec dest;
    uint32 count = 1;
    bool hasSpace = (bot->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, WarlockConstants::SOUL_SHARD_ITEM, count) == EQUIP_ERR_OK);

    return (currentShards < SHARD_CAP) && hasSpace;
}

bool CastCreateSoulstoneAction::isUseful()
{
    if (!bot)
        return false;

    // List of all Soulstone item IDs
    static const std::vector<uint32> soulstoneIds = {
        WarlockConstants::MINOR_SOULSTONE,
        WarlockConstants::LESSER_SOULSTONE,
        WarlockConstants::SOULSTONE,
        WarlockConstants::GREATER_SOULSTONE,
        WarlockConstants::MAJOR_SOULSTONE,
        WarlockConstants::MASTER_SOULSTONE,
        WarlockConstants::DEMONIC_SOULSTONE
    };

    // Check if the bot already has any soulstone
    for (uint32 id : soulstoneIds)
    {
        if (bot->GetItemCount(id, false) > 0)
            return false;                      // Already has a soulstone
    }

    // Only need to check one soulstone type for bag space (usually the highest-tier)
    ItemPosCountVec dest;
    uint32 count = 1;
    // Use the last in the list (highest tier)
    uint32 soulstoneToCreate = soulstoneIds.back();

    bool hasSpace = (bot->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, soulstoneToCreate, count) == EQUIP_ERR_OK);
    return hasSpace;
}

bool DestroySoulShardAction::Execute(Event event)
{
    // Look for the first soul shard in any bag and destroy it
    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        if (Bag* pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
            {
                if (Item* pItem = pBag->GetItemByPos(j))
                {
                    if (pItem->GetTemplate()->ItemId == WarlockConstants::SOUL_SHARD_ITEM)
                    {
                        bot->DestroyItem(pItem->GetBagSlot(), pItem->GetSlot(), true);
                        return true;  // Only destroy one!
                    }
                }
            }
        }
    }
    
    // Also check main inventory slots (not in bags)
    for (int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            if (pItem->GetTemplate()->ItemId == WarlockConstants::SOUL_SHARD_ITEM)
            {
                bot->DestroyItem(pItem->GetBagSlot(), pItem->GetSlot(), true);
                return true;
            }
        }
    }
    return false;
}

// Checks if the target has a soulstone aura
static bool HasSoulstoneAura(Unit* unit)
{
    static const std::vector<uint32> soulstoneAuraIds = {
        WarlockConstants::SOULSTONE_SPELL_MINOR,
        WarlockConstants::SOULSTONE_SPELL_LESSER,
        WarlockConstants::SOULSTONE_SPELL_NORMAL,
        WarlockConstants::SOULSTONE_SPELL_GREATER,
        WarlockConstants::SOULSTONE_SPELL_MAJOR,
        WarlockConstants::SOULSTONE_SPELL_MASTER,
        WarlockConstants::SOULSTONE_SPELL_DEMONIC
    };
    for (uint32 spellId : soulstoneAuraIds)
    {
        if (unit->HasAura(spellId))
            return true;
    }
    return false;
}

// Use the soulstone item on the bot itself with nc strategy "ss self"
bool UseSoulstoneSelfAction::Execute(Event event)
{
    std::vector<Item*> items = AI_VALUE2(std::vector<Item*>, "inventory items", "soulstone");
    if (items.empty())
        return false;

    if (HasSoulstoneAura(bot))
        return false;

    bot->SetSelection(bot->GetGUID());
    return UseItem(items[0], ObjectGuid::Empty, nullptr, bot);
}

// Helper to clean up expired reservations
void CleanupSoulstoneReservations()
{
    uint32 now = getMSTime();
    std::lock_guard<std::mutex> lock(soulstoneReservationsMutex);
    for (auto it = soulstoneReservations.begin(); it != soulstoneReservations.end();)
    {
        if (it->second <= now)
        {
            it = soulstoneReservations.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

// Use the soulstone item on the bot's master with nc strategy "ss master"
bool UseSoulstoneMasterAction::Execute(Event event)
{
    CleanupSoulstoneReservations();

    std::vector<Item*> items = AI_VALUE2(std::vector<Item*>, "inventory items", "soulstone");
    if (items.empty())
        return false;

    Player* master = botAI->GetMaster();
    if (!master || HasSoulstoneAura(master))
        return false;

    uint32 now = getMSTime();

    // local scope so that std::lock_guard scope will be destroyed as soon as the closing brace is hit
    {
        std::lock_guard<std::mutex> lock(soulstoneReservationsMutex);
        if (soulstoneReservations.count(master->GetGUID()) && soulstoneReservations[master->GetGUID()] > now)
            return false;  // Already being soulstoned

        soulstoneReservations[master->GetGUID()] = now + 2500;  // Reserve for 2.5 seconds
    }

    float distance = sServerFacade->GetDistance2d(bot, master);
    if (distance >= 30.0f)
        return false;

    if (!bot->IsWithinLOSInMap(master))
        return false;

    bot->SetSelection(master->GetGUID());
    return UseItem(items[0], ObjectGuid::Empty, nullptr, master);
}

// Use the soulstone item on a tank in the group with nc strategy "ss tank"
bool UseSoulstoneTankAction::Execute(Event event)
{
    CleanupSoulstoneReservations();

    std::vector<Item*> items = AI_VALUE2(std::vector<Item*>, "inventory items", "soulstone");
    if (items.empty())
        return false;

    Player* chosenTank = nullptr;
    Group* group = bot->GetGroup();
    uint32 now = getMSTime();

    // First: Try to soulstone the main tank
    if (group)
    {
        for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* member = gref->GetSource();
            if (member && member->IsAlive() && 
                botAI->IsTank(member) && botAI->IsMainTank(member) && !HasSoulstoneAura(member))
            {
                std::lock_guard<std::mutex> lock(soulstoneReservationsMutex);
                if (soulstoneReservations.count(member->GetGUID()) && soulstoneReservations[member->GetGUID()] > now)
                    continue;  // Already being soulstoned

                float distance = sServerFacade->GetDistance2d(bot, member);
                if (distance < 30.0f && bot->IsWithinLOSInMap(member))
                {
                    chosenTank = member;
                    soulstoneReservations[chosenTank->GetGUID()] = now + 2500;  // Reserve for 2.5 seconds
                    break;
                }
            }
        }

        // If no main tank found, soulstone another tank
        if (!chosenTank)
        {
            for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
            {
                Player* member = gref->GetSource();
                if (member && member->IsAlive() && botAI->IsTank(member) && !HasSoulstoneAura(member))
                {
                    std::lock_guard<std::mutex> lock(soulstoneReservationsMutex);
                    if (soulstoneReservations.count(member->GetGUID()) &&
                        soulstoneReservations[member->GetGUID()] > now)
                        continue;  // Already being soulstoned

                    float distance = sServerFacade->GetDistance2d(bot, member);
                    if (distance < 30.0f && bot->IsWithinLOSInMap(member))
                    {
                        chosenTank = member;
                        soulstoneReservations[chosenTank->GetGUID()] = now + 2500;  // Reserve for 2.5 seconds
                        break;
                    }
                }
            }
        }
    }

    if (!chosenTank)
        return false;

    bot->SetSelection(chosenTank->GetGUID());
    return UseItem(items[0], ObjectGuid::Empty, nullptr, chosenTank);
}

// Use the soulstone item on a healer in the group with nc strategy "ss healer"
bool UseSoulstoneHealerAction::Execute(Event event)
{
    CleanupSoulstoneReservations();

    std::vector<Item*> items = AI_VALUE2(std::vector<Item*>, "inventory items", "soulstone");
    if (items.empty())
        return false;

    Player* healer = nullptr;
    Group* group = bot->GetGroup();
    uint32 now = getMSTime();
    if (group)
    {
        for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* member = gref->GetSource();
            if (member && member->IsAlive() && botAI->IsHeal(member) && !HasSoulstoneAura(member))
            {
                {
                    std::lock_guard<std::mutex> lock(soulstoneReservationsMutex);
                    if (soulstoneReservations.count(member->GetGUID()) &&
                        soulstoneReservations[member->GetGUID()] > now)
                        continue;  // Already being soulstoned

                    float distance = sServerFacade->GetDistance2d(bot, member);
                    if (distance < 30.0f && bot->IsWithinLOSInMap(member))
                    {
                        healer = member;
                        soulstoneReservations[healer->GetGUID()] = now + 2500;  // Reserve for 2.5 seconds
                        break;
                    }
                }
            }
        }
    }

    if (!healer)
        return false;

    bot->SetSelection(healer->GetGUID());
    return UseItem(items[0], ObjectGuid::Empty, nullptr, healer);
}

const std::vector<uint32> CastCreateFirestoneAction::firestoneSpellIds = {
    WarlockConstants::CREATE_FIRESTONE_RANK_7,
    WarlockConstants::CREATE_FIRESTONE_RANK_6,
    WarlockConstants::CREATE_FIRESTONE_RANK_5,
    WarlockConstants::CREATE_FIRESTONE_RANK_4,
    WarlockConstants::CREATE_FIRESTONE_RANK_3,
    WarlockConstants::CREATE_FIRESTONE_RANK_2,
    WarlockConstants::CREATE_FIRESTONE_RANK_1
};

CastCreateFirestoneAction::CastCreateFirestoneAction(PlayerbotAI* botAI)
    : CastBuffSpellAction(botAI, "create firestone")
{
}

bool CastCreateFirestoneAction::Execute(Event event)
{
    for (uint32 spellId : firestoneSpellIds)
    {
        if (bot->HasSpell(spellId))
            return botAI->CastSpell(spellId, bot);
    }
    return false;
}

bool CastCreateFirestoneAction::isUseful()
{
    for (uint32 spellId : firestoneSpellIds)
    {
        if (bot->HasSpell(spellId))
            return true;
    }
    return false;
}

// Ritual Actions Implementation

bool CastRitualOfSoulsAction::isUseful()
{
    // Fast-fail preconditions in a single check for readability
    const bool hasAnyRank = bot->HasSpell(WarlockConstants::RITUAL_OF_SOULS_RANK_1) ||
                            bot->HasSpell(WarlockConstants::RITUAL_OF_SOULS_RANK_2);
    const bool bothRanksOnCd = bot->GetSpellCooldownDelay(WarlockConstants::RITUAL_OF_SOULS_RANK_1) > 0 &&
                               bot->GetSpellCooldownDelay(WarlockConstants::RITUAL_OF_SOULS_RANK_2) > 0;
    uint32 currentTime = time(nullptr);
    auto it = lastRitualUsage.find(bot->GetGUID());
    const bool underCustomCooldown = (it != lastRitualUsage.end()) && ((currentTime - it->second) < 900); // 15 min

    if (!CanUseRituals(bot) || !hasAnyRank || bothRanksOnCd || underCustomCooldown)
    {
        return false;
    }
    
    // NEW FUNCTIONALITY: Coordination between multiple warlocks in party
    // Only allows one warlock to perform the ritual if there are other warlocks in the group
    if (Group* group = bot->GetGroup())
    {
        Player* selectedWarlock = nullptr;
        uint32 lowestGuid = UINT32_MAX;
        
        // Find the warlock with the lowest GUID (priority by group entry order)
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            if (Player* member = itr->GetSource())
            {
                if (member->getClass() == CLASS_WARLOCK && member->IsAlive())
                {
                    // In Battlegrounds, only consider same faction
                    bool shouldConsider = true;
                    if (bot->GetMap()->IsBattleground())
                    {
                        shouldConsider = (member->GetTeamId() == bot->GetTeamId());
                    }

                    if (shouldConsider && member->GetGUID().GetCounter() < lowestGuid)
                    {
                        lowestGuid = member->GetGUID().GetCounter();
                        selectedWarlock = member;
                    }
                }
            }
        }
        
        // Only the selected warlock can perform the ritual
        if (selectedWarlock && selectedWarlock != bot)
        {
            return false; // Not the selected warlock
        }
    }
    
    if (!HasRitualComponent(bot, WarlockConstants::RITUAL_OF_SOULS_RANK_1))
    {
        // Give Soul Shard to bot
        bot->AddItem(WarlockConstants::SOUL_SHARD_ITEM, 1);
    }
    
    GameObject* existingSoulPortal = bot->FindNearestGameObject(WarlockConstants::SOUL_PORTAL_RANK_1, 30.0f);
    if (!existingSoulPortal)
        existingSoulPortal = bot->FindNearestGameObject(WarlockConstants::SOUL_PORTAL_RANK_2, 30.0f);
    
    return !existingSoulPortal;
}

bool CastRitualOfSoulsAction::Execute(Event event)
{
    // Check if bot is currently channeling the ritual
    if (bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
    {
        // Don't interrupt the channeling - wait for it to complete naturally
        botAI->SetNextCheckDelay(2000); // Check again in 2 seconds
        return false;
    }
    
    // Check if there are portals very close before casting to avoid overlap
    GameObject* nearbyPortal = bot->FindNearestGameObject(WarlockConstants::SOUL_PORTAL_RANK_1, 10.0f);
    if (!nearbyPortal)
        nearbyPortal = bot->FindNearestGameObject(WarlockConstants::SOUL_PORTAL_RANK_2, 10.0f);

    // In Battlegrounds, also check if there are Mages of the SAME FACTION nearby
    bool hasMageNearby = false;
    if (bot->GetMap()->IsBattleground()) 
    {
        std::list<Player*> nearbyPlayers;
        Acore::AnyPlayerInObjectRangeCheck check(bot, 50.0f);
        Acore::PlayerListSearcher<Acore::AnyPlayerInObjectRangeCheck> searcher(bot, nearbyPlayers, check);
        Cell::VisitObjects(bot, searcher, 50.0f);
        
        for (Player* nearbyPlayer : nearbyPlayers) 
        {
            if (nearbyPlayer->getClass() == CLASS_MAGE && 
                nearbyPlayer->GetTeamId() == bot->GetTeamId()) 
            { 
                // Only same faction in BG
                hasMageNearby = true;
                break;
            }
        }
    }

    if ((nearbyPortal && bot->GetDistance(nearbyPortal) < 8.0f) || hasMageNearby) {
        // There is a portal very close or a Mage of the same faction (in BG), change orientation to avoid overlap
        float currentOrientation = bot->GetOrientation();
        float newOrientation = currentOrientation + (M_PI / 2); // 90 degrees
        bot->SetOrientation(newOrientation);
        bot->SetFacingTo(newOrientation);
    }
    
    // Determine rank via switch-style mapping for readability
    uint32 ritualRankSpellId = 0;
    if (bot->HasSpell(WarlockConstants::RITUAL_OF_SOULS_RANK_2))
    {
        ritualRankSpellId = WarlockConstants::RITUAL_OF_SOULS_RANK_2;
    }
    else if (bot->HasSpell(WarlockConstants::RITUAL_OF_SOULS_RANK_1))
    {
        ritualRankSpellId = WarlockConstants::RITUAL_OF_SOULS_RANK_1;
    }

    if (ritualRankSpellId == 0)
        return false;
    
    if (bot->GetSpellCooldownDelay(WarlockConstants::RITUAL_OF_SOULS_RANK_1) > 0 && bot->GetSpellCooldownDelay(WarlockConstants::RITUAL_OF_SOULS_RANK_2) > 0)
    {
        return false;
    }
    
    bool result = CastSpellAction::Execute(event);
    
    // If successful, add a long delay to prevent immediate re-casting
    if (result)
    {
        // Mark that this bot has completed ritual interaction
        uint64 botGuid = bot->GetGUID().GetRawValue();
        hasCompletedRitualInteraction[botGuid] = true;
        
        // Set custom cooldown (15 minutes)
        uint32 currentTime = time(nullptr);
        lastRitualUsage[bot->GetGUID()] = currentTime;
        // Reset movement state for next ritual
        hasMovedForRitual[bot->GetGUID()] = false;
        
        // Record ritual creation time to prevent immediate re-triggering
        lastRitualCreation[bot->GetGUID()] = currentTime;
        
        // Don't clear movement state immediately - let the ritual complete naturally
        // The follow behavior will be restored by other actions if needed
    }
    
    return result;
}

// InteractWithSoulPortalAction implementation moved to RitualActions.cpp

bool LootSoulwellAction::isUseful()
{
    // Only useful in dungeons/raids
    if (!bot->GetMap()->IsDungeon() && !bot->GetMap()->IsRaid() && !bot->GetMap()->IsBattleground())
        return false;
    
    // Check if bot already has any healthstone (data-driven for maintainability)
    constexpr std::array<uint32, 7> kHealthstoneIds = {
        WarlockConstants::MINOR_HEALTHSTONE,
        WarlockConstants::LESSER_HEALTHSTONE,
        WarlockConstants::MAJOR_HEALTHSTONE,
        WarlockConstants::MINOR_HEALTHSTONE_ALT,
        WarlockConstants::LESSER_HEALTHSTONE_ALT,
        WarlockConstants::FEL_HEALTHSTONE,
        WarlockConstants::DEMONIC_HEALTHSTONE
    };
    if (std::any_of(kHealthstoneIds.begin(), kHealthstoneIds.end(), [&](uint32 id){ return bot->GetItemCount(id, false) > 0; }))
    {
        return false; // Already has a healthstone
    }
    
    // Check if there's a soulwell nearby
    GameObject* soulwell = bot->FindNearestGameObject(WarlockConstants::SOUL_WELL_RANK_1, 30.0f);
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(WarlockConstants::SOUL_WELL_RANK_2, 30.0f);
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(WarlockConstants::SOUL_WELL_RANK_2_VARIANT_1, 30.0f);
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(WarlockConstants::SOUL_WELL_RANK_2_VARIANT_2, 30.0f);
    
    if (!soulwell)
        return false;
    
    return true;
}

bool LootSoulwellAction::Execute(Event event)
{
    // Find the soulwell
    GameObject* soulwell = bot->FindNearestGameObject(WarlockConstants::SOUL_WELL_RANK_1, 30.0f);
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(WarlockConstants::SOUL_WELL_RANK_2, 30.0f);
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(WarlockConstants::SOUL_WELL_RANK_2_VARIANT_1, 30.0f);
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(WarlockConstants::SOUL_WELL_RANK_2_VARIANT_2, 30.0f);
    
    if (!soulwell)
        return false;
    
    // Check if we're close enough to interact
    if (bot->GetDistance(soulwell) > 5.0f)
    {
        // Move closer to the soulwell
        bot->GetMotionMaster()->MovePoint(0, soulwell->GetPositionX(), soulwell->GetPositionY(), soulwell->GetPositionZ());
        return false;
    }
    
    // Interact with the soulwell to get healthstone
    soulwell->Use(bot);
    
    // Clear movement state and add a delay to prevent spam
    FinalizeRitualInteraction(bot, botAI, 5000);
    
    return true;
}

// EnableSoulstoneDungeonAction implementation
bool EnableSoulstoneDungeonAction::isUseful()
{
    if (!bot)
        return false;
    
    // Only useful for warlocks in dungeons or raids
    if (bot->getClass() != CLASS_WARLOCK)
        return false;
    
    // Check if we're in a dungeon or raid
    if (!bot->GetMap()->IsDungeon() && !bot->GetMap()->IsRaid())
        return false;
    
    // Check if we have soulstone strategy enabled
    if (botAI->HasStrategy("ss healer", BOT_STATE_NON_COMBAT))
        return false; // Already enabled
    
    return true;
}

bool EnableSoulstoneDungeonAction::Execute(Event event)
{
    if (!bot)
        return false;
    
    // Enable the soulstone healer strategy
    botAI->ChangeStrategy("+ss healer", BOT_STATE_NON_COMBAT);
       
    return true;
}
