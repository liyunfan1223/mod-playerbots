/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
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

// Function declared in RitualActions.cpp

static std::unordered_map<ObjectGuid, uint32> lastRitualUsage;
static std::unordered_map<ObjectGuid, bool> hasMovedForRitual;
std::unordered_map<ObjectGuid, uint32> lastRitualCreation;
// Track if bot has completed ritual interaction to avoid loops
extern std::unordered_map<uint64, bool> hasCompletedRitualInteraction;


#include <string>
#include <vector>
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

const int ITEM_SOUL_SHARD = 6265;

// Checks if the bot has less than 26 soul shards, and if so, allows casting Drain Soul
bool CastDrainSoulAction::isUseful() { return AI_VALUE2(uint32, "item count", "soul shard") < 26; }

// Checks if the bot's health is above a certain threshold, and if so, allows casting Life Tap
bool CastLifeTapAction::isUseful() { return AI_VALUE2(uint8, "health", "self target") > sPlayerbotAIConfig->lowHealth; }

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
    if (bot->HasSpell(27243) || bot->HasSpell(47835) || bot->HasSpell(47836)) // Seed of Corruption spell IDs
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
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    ItemPosCountVec dest;
    uint32 count = 1;
    if (bot->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, ITEM_SOUL_SHARD, count) == EQUIP_ERR_OK)
    {
        bot->StoreNewItem(dest, ITEM_SOUL_SHARD, true, Item::GenerateItemRandomPropertyId(ITEM_SOUL_SHARD));
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
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    uint32 currentShards = bot->GetItemCount(ITEM_SOUL_SHARD, false);  // false = only bags
    const uint32 SHARD_CAP = 6;                                    // adjust as needed

    // Only allow if under cap AND there is space for a new shard
    ItemPosCountVec dest;
    uint32 count = 1;
    bool hasSpace = (bot->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, ITEM_SOUL_SHARD, count) == EQUIP_ERR_OK);

    return (currentShards < SHARD_CAP) && hasSpace;
}

bool CastCreateSoulstoneAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // List of all Soulstone item IDs
    static const std::vector<uint32> soulstoneIds = {
        5232,   // Minor Soulstone
        16892,  // Lesser Soulstone
        16893,  // Soulstone
        16895,  // Greater Soulstone
        16896,  // Major Soulstone
        22116,  // Master Soulstone
        36895   // Demonic Soulstone
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
                    if (pItem->GetTemplate()->ItemId == ITEM_SOUL_SHARD)
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
            if (pItem->GetTemplate()->ItemId == ITEM_SOUL_SHARD)
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
    static const std::vector<uint32> soulstoneAuraIds = {20707, 20762, 20763, 20764, 20765, 27239, 47883};
    for (uint32 spellId : soulstoneAuraIds)
        if (unit->HasAura(spellId))
            return true;
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

// Reservation map for soulstone targets (GUID -> reservation expiry in ms)
static std::unordered_map<ObjectGuid, uint32> soulstoneReservations;
static std::mutex soulstoneReservationsMutex;

// Helper to clean up expired reservations
void CleanupSoulstoneReservations()
{
    uint32 now = getMSTime();
    std::lock_guard<std::mutex> lock(soulstoneReservationsMutex);
    for (auto it = soulstoneReservations.begin(); it != soulstoneReservations.end();)
    {
        if (it->second <= now)
            it = soulstoneReservations.erase(it);
        else
            ++it;
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
            if (member && member->IsAlive() && botAI->IsTank(member) && botAI->IsMainTank(member) &&
                !HasSoulstoneAura(member))
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
    60220,  // Create Firestone (Rank 7)
    27250,  // Rank 5
    17953,  // Rank 4
    17952,  // Rank 3
    17951,  // Rank 2
    6366    // Rank 1
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
    Player* bot = botAI->GetBot();
    
    
    if (!CanUseRituals(bot))
    {
        return false;
    }
    
    if (!bot->HasSpell(29893) && !bot->HasSpell(58887)) // Ritual of Souls Rank 1 & 2
    {
        return false;
    }
    
    if (bot->GetSpellCooldownDelay(29893) > 0 && bot->GetSpellCooldownDelay(58887) > 0)
    {
        return false;
    }
    
    uint32 currentTime = time(nullptr);
    auto it = lastRitualUsage.find(bot->GetGUID());
    if (it != lastRitualUsage.end())
    {
        uint32 timeSinceLastUse = currentTime - it->second;
        if (timeSinceLastUse < 900) // 15 minutes = 900 seconds
        {
            return false;
        }
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
                    bool isSameFaction = (member->GetTeamId() == bot->GetTeamId());
                    bool shouldConsider = bot->GetMap()->IsBattleground() ? isSameFaction : true;
                    
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
    
    if (!HasRitualComponent(bot, 29893))
    {
        // Give Soul Shard to bot (item ID 6265)
        bot->AddItem(6265, 1);
    }
    
    GameObject* existingSoulPortal = bot->FindNearestGameObject(181622, 30.0f); // Soul Portal Rank 1
    if (!existingSoulPortal)
        existingSoulPortal = bot->FindNearestGameObject(193168, 30.0f); // Soul Portal Rank 2
    
    if (existingSoulPortal)
    {
        return false;
    }
    
    return true;
}

bool CastRitualOfSoulsAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();

    // Check if bot is currently channeling the ritual
    if (bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
    {
        // Don't interrupt the channeling - wait for it to complete naturally
        botAI->SetNextCheckDelay(2000); // Check again in 2 seconds
        return false;
    }
    
    // Check if there are portals very close before casting to avoid overlap
    GameObject* nearbyPortal = bot->FindNearestGameObject(181622, 10.0f); // Portal Rank 1
    if (!nearbyPortal)
        nearbyPortal = bot->FindNearestGameObject(193168, 10.0f); // Portal Rank 2

    // In Battlegrounds, also check if there are Mages of the SAME FACTION nearby
    bool hasMageNearby = false;
    if (bot->GetMap()->IsBattleground()) {
        std::list<Player*> nearbyPlayers;
        Acore::AnyPlayerInObjectRangeCheck check(bot, 50.0f);
        Acore::PlayerListSearcher<Acore::AnyPlayerInObjectRangeCheck> searcher(bot, nearbyPlayers, check);
        Cell::VisitObjects(bot, searcher, 50.0f);
        
        for (Player* nearbyPlayer : nearbyPlayers) {
            if (nearbyPlayer->getClass() == CLASS_MAGE && 
                nearbyPlayer->GetTeamId() == bot->GetTeamId()) { // Only same faction in BG
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
    
    if (!bot->HasSpell(29893) && !bot->HasSpell(58887))
    {
        return false;
    }
    
    if (bot->GetSpellCooldownDelay(29893) > 0 && bot->GetSpellCooldownDelay(58887) > 0)
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
    Player* bot = botAI->GetBot();
    
    // Only useful in dungeons/raids
    if (!bot->GetMap()->IsDungeon() && !bot->GetMap()->IsRaid() && !bot->GetMap()->IsBattleground())
        return false;
    
    // Check if bot already has a healthstone
    if (bot->GetItemCount(5512, false) > 0 || // Minor Healthstone
        bot->GetItemCount(5511, false) > 0 || // Lesser Healthstone
        bot->GetItemCount(9421, false) > 0 || // Major Healthstone
        bot->GetItemCount(19004, false) > 0 || // Minor Healthstone
        bot->GetItemCount(19005, false) > 0)   // Lesser Healthstone
    {
        return false; // Already has a healthstone
    }
    
    // Check if there's a soulwell nearby
    GameObject* soulwell = bot->FindNearestGameObject(181621, 30.0f); // Soul Well Rank 1
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(193169, 30.0f); // Soul Well Rank 2
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(193170, 30.0f); // Soul Well Rank 2 variant
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(193171, 30.0f); // Soul Well Rank 2 variant
    
    if (!soulwell)
        return false;
    
    return true;
}

bool LootSoulwellAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    
    // Find the soulwell
    GameObject* soulwell = bot->FindNearestGameObject(181621, 30.0f); // Soul Well Rank 1
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(193169, 30.0f); // Soul Well Rank 2
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(193170, 30.0f); // Soul Well Rank 2 variant
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(193171, 30.0f); // Soul Well Rank 2 variant
    
    if (!soulwell)
        return false;
    
    // Check if we're close enough to interact
    if (bot->GetDistance(soulwell) > 5.0f)
    {
        // Move closer to the soulwell using unique movement ID
        uint32 movementId = bot->GetGUID().GetCounter() + 2000; // Unique ID for soulwell movement
        bot->GetMotionMaster()->MovePoint(movementId, soulwell->GetPositionX(), soulwell->GetPositionY(), soulwell->GetPositionZ());
        return false;
    }
    
    // Interact with the soulwell to get healthstone
    soulwell->Use(bot);
    
    // Clear movement state after interaction to allow normal movement
    bot->GetMotionMaster()->Clear();
    bot->GetMotionMaster()->MoveIdle();
    
    // Add a delay to prevent spam
    botAI->SetNextCheckDelay(5000); // 5 seconds delay
    
    return true;
}

// EnableSoulstoneDungeonAction implementation
bool EnableSoulstoneDungeonAction::isUseful()
{
    Player* bot = botAI->GetBot();
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
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
    
    // Enable the soulstone healer strategy
    botAI->ChangeStrategy("+ss healer", BOT_STATE_NON_COMBAT);
    
    // Log the action for debugging
    if (!sPlayerbotAIConfig->logInGroupOnly || (bot->GetGroup() && botAI->HasRealPlayerMaster()))
    {
        botAI->TellMaster("Soulstone healer strategy enabled for dungeon");
    }
    
    return true;
}

// MoveAwayFromSpawnAction implementation moved to RitualActions.cpp
