#include "RitualActions.h"
#include "PlayerbotAI.h"
#include "PlayerbotMgr.h"
#include "Player.h"
#include "GameObject.h"
#include "MotionMaster.h"
#include "Battleground.h"
#include "Log.h"
#include "MovementActions.h"
#include "PlayerbotAIConfig.h"
#include "Cell.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include <unordered_map>
#include <fstream>
#include <iomanip>

// Static maps to track bot states
static std::unordered_map<uint64, uint32> soulPortalDetectionTime;
static uint32 detectionCounter = 0;
// Cooldown to avoid repeated interaction with soulwell
static std::unordered_map<uint64, uint32> lastSoulwellInteraction;
// Cooldown to avoid repeated follow restoration
static std::unordered_map<uint64, uint32> lastFollowRestoration;

// Track if bot has completed ritual interaction to avoid loops
std::unordered_map<uint64, bool> hasCompletedRitualInteraction;

// Helper function to get appropriate search range based on map type
float GetRitualSearchRange(Player* bot)
{
    return bot->GetMap()->IsBattleground() ? RITUAL_BATTLEGROUND_SEARCH_RANGE : RITUAL_DUNGEON_SEARCH_RANGE;
}

// Helper to find nearest gameobject by trying two rank IDs within appropriate range
GameObject* FindNearestByRanks(Player* bot, uint32 rank1Id, uint32 rank2Id)
{
    GameObject* go = bot->FindNearestGameObject(rank1Id, GetRitualSearchRange(bot));
    if (!go)
        go = bot->FindNearestGameObject(rank2Id, GetRitualSearchRange(bot));
    return go;
}

// Helper to finalize interaction: stop movement and set a short delay
void FinalizeRitualInteraction(Player* bot, PlayerbotAI* botAI, uint32 delayMs)
{
    bot->GetMotionMaster()->Clear();
    bot->GetMotionMaster()->MoveIdle();
    botAI->SetNextCheckDelay(delayMs);
}

// Helper function to check if rituals can be used in current map
bool CanUseRituals(Player* bot)
{
    // Only useful in dungeons/raids/battlegrounds
    if (!bot->GetMap()->IsDungeon() && !bot->GetMap()->IsRaid() && !bot->GetMap()->IsBattleground())
        return false;
    
    // In battlegrounds, only during preparation phase
    if (bot->GetMap()->IsBattleground())
    {
        Battleground* bg = bot->GetBattleground();
        if (!bg || bg->GetStatus() != STATUS_WAIT_JOIN)
        {
            return false; // Not in preparation phase
        }
    }
    
    return true;
}

bool HasAnyItemAtLeast(Player* bot, const std::vector<uint32>& itemIds, uint32 limit)
{
    for (uint32 id : itemIds)
    {
        if (bot->GetItemCount(id) >= limit)
            return true;
    }
    return false;
}

bool HasAllItemsBelow(Player* bot, const std::vector<uint32>& itemIds, uint32 limit)
{
    for (uint32 id : itemIds)
    {
        if (bot->GetItemCount(id) >= limit)
            return false;
    }
    return true;
}

bool MoveAwayFromSpawnAction::isUseful()
{
    if (!CanUseRituals(bot))
        return false;
    
    return false;
}

bool MoveAwayFromSpawnAction::Execute(Event event)
{
    float x = bot->GetPositionX() + (rand() % (RITUAL_MOVEMENT_OFFSET_MAX - RITUAL_MOVEMENT_OFFSET_MIN) + RITUAL_MOVEMENT_OFFSET_MIN);
    float y = bot->GetPositionY() + (rand() % (RITUAL_MOVEMENT_OFFSET_MAX - RITUAL_MOVEMENT_OFFSET_MIN) + RITUAL_MOVEMENT_OFFSET_MIN);
    float z = bot->GetPositionZ();
    
    return MoveTo(bot->GetMapId(), x, y, z, false, false);
}

// InteractWithSoulPortalAction - Interact with Soul Portal to create Soul Well and get Healthstone
bool InteractWithSoulPortalAction::isUseful()
{
    // Check if rituals can be used in current map
    if (!CanUseRituals(bot))
        return false;
    
    // Check if bot already has Healthstone (limit to 1)
    {
        const std::vector<uint32> healthstoneIdsAtLeast = {
            RITUAL_MINOR_HEALTHSTONE, RITUAL_LESSER_HEALTHSTONE, RITUAL_MAJOR_HEALTHSTONE,
            RITUAL_MINOR_HEALTHSTONE_ALT, RITUAL_LESSER_HEALTHSTONE_ALT, RITUAL_FEL_HEALTHSTONE,
            RITUAL_DEMONIC_HEALTHSTONE
        };
        if (HasAnyItemAtLeast(bot, healthstoneIdsAtLeast, RITUAL_HEALTHSTONE_LIMIT))
        {
            return false;
        }
    }
    
    // Search for soul portal nearby (within range)
    GameObject* soulPortal = FindNearestByRanks(bot, RITUAL_SOUL_PORTAL_RANK_1, RITUAL_SOUL_PORTAL_RANK_2);
    
    if (!soulPortal)
    {
        return false;
    }
    
    return true;
}

bool InteractWithSoulPortalAction::Execute(Event event)
{
    // Search for soul portal nearby (within range)
    GameObject* soulPortal = FindNearestByRanks(bot, RITUAL_SOUL_PORTAL_RANK_1, RITUAL_SOUL_PORTAL_RANK_2);
    
    if (!soulPortal)
    {
        return false;
    }
    
    // Check if we're close enough to interact
    if (bot->GetDistance(soulPortal) > RITUAL_INTERACTION_DISTANCE)
    {
        // Move closer to the soul portal
        return MoveTo(bot->GetMapId(), soulPortal->GetPositionX(), soulPortal->GetPositionY(), soulPortal->GetPositionZ(), false, false);
    }
    
    // Interact with the soul portal to create soul well
    soulPortal->Use(bot);
    
    // Clear movement and add small delay for re-evaluation
    FinalizeRitualInteraction(bot, botAI, RITUAL_DELAY_MS);
    
    return true;
}

// InteractWithRefreshmentPortalAction - Interact with Refreshment Portal to create Refreshment Table
bool InteractWithRefreshmentPortalAction::isUseful()
{
    // Check if rituals can be used in current map
    if (!CanUseRituals(bot))
        return false;
    
    // Find refreshment portal nearby (both ranks)
    GameObject* refreshmentPortal = FindNearestByRanks(bot, RITUAL_REFRESHMENT_PORTAL_RANK_1, RITUAL_REFRESHMENT_PORTAL_RANK_2);
    
    if (!refreshmentPortal)
    {
        return false;
    }
    
    return true;
}

bool InteractWithRefreshmentPortalAction::Execute(Event event)
{
    // Find refreshment portal nearby (both ranks)
    GameObject* refreshmentPortal = FindNearestByRanks(bot, RITUAL_REFRESHMENT_PORTAL_RANK_1, RITUAL_REFRESHMENT_PORTAL_RANK_2);
    
    if (!refreshmentPortal)
    {
        return false;
    }
    
    // Check if we're close enough to interact
    if (bot->GetDistance(refreshmentPortal) > RITUAL_INTERACTION_DISTANCE)
    {
        // Move closer to the refreshment portal
        return MoveTo(bot->GetMapId(), refreshmentPortal->GetPositionX(), refreshmentPortal->GetPositionY(), refreshmentPortal->GetPositionZ(), false, false);
    }
    
    // Interact with the refreshment portal to create refreshment table
    refreshmentPortal->Use(bot);
    
    // Clear movement state to allow normal AI behavior
    bot->GetMotionMaster()->Clear();
    bot->GetMotionMaster()->MoveIdle();
    
    // Don't restore follow behavior immediately - let the ritual complete naturally
    return true;
}

// InteractWithRefreshmentTableAction - Interact with Refreshment Table to get food/drink
bool InteractWithRefreshmentTableAction::isUseful()
{
    // Check if rituals can be used in current map
    if (!CanUseRituals(bot))
        return false;
    
    // Check if bot already has food/drink (limit to 1 stack each)
    const std::vector<uint32> refreshmentIds = {
        RITUAL_CONJURED_MANA_BISCUIT, RITUAL_CONJURED_MANA_STRUDEL, RITUAL_CONJURED_MANA_COOKIE,
        RITUAL_CONJURED_MANA_CAKE, RITUAL_CONJURED_MANA_PIE, RITUAL_CONJURED_MANA_BREAD,
        RITUAL_CONJURED_MANA_MUFFIN, RITUAL_CONJURED_MANA_DONUT, RITUAL_CONJURED_MANA_BAGEL,
        RITUAL_CONJURED_MANA_PRETZEL, RITUAL_CONJURED_MANA_WATER, RITUAL_CONJURED_MANA_JUICE,
        RITUAL_CONJURED_MANA_TEA, RITUAL_CONJURED_MANA_COFFEE
    };
    if (HasAnyItemAtLeast(bot, refreshmentIds, RITUAL_FOOD_DRINK_LIMIT))
    {
        return false; // Already has enough food/drink
    }
    
    // Find refreshment table nearby
    GameObject* refreshmentTable = bot->FindNearestGameObject(RITUAL_REFRESHMENT_TABLE_RANK_1, GetRitualSearchRange(bot)); // Refreshment Table ID
    if (!refreshmentTable)
    {
        refreshmentTable = bot->FindNearestGameObject(RITUAL_REFRESHMENT_TABLE_RANK_2, GetRitualSearchRange(bot)); // Refreshment Table Rank 2
    }
    
    if (!refreshmentTable)
    {
        return false;
    }
    
    return true;
}

bool InteractWithRefreshmentTableAction::Execute(Event event)
{
    // Find refreshment table nearby (both ranks)
    GameObject* refreshmentTable = bot->FindNearestGameObject(RITUAL_REFRESHMENT_TABLE_RANK_1, GetRitualSearchRange(bot)); // Refreshment Table Rank 1
    if (!refreshmentTable)
    {
        refreshmentTable = bot->FindNearestGameObject(RITUAL_REFRESHMENT_TABLE_RANK_2, GetRitualSearchRange(bot)); // Refreshment Table Rank 2
    }
    
    if (!refreshmentTable)
    {
        return false;
    }
    
    // Check if we're close enough to interact
    if (bot->GetDistance(refreshmentTable) > RITUAL_INTERACTION_DISTANCE)
    {
        // Move closer to the refreshment table
        return MoveTo(bot->GetMapId(), refreshmentTable->GetPositionX(), refreshmentTable->GetPositionY(), refreshmentTable->GetPositionZ(), false, false);
    }
    
    // Interact with the refreshment table
    refreshmentTable->Use(bot);
    
    // Mark that this bot has completed ritual interaction
    uint64 botGuid = bot->GetGUID().GetRawValue();
    hasCompletedRitualInteraction[botGuid] = true;
    
    // Don't restore follow behavior immediately; add a delay to prevent spam
    FinalizeRitualInteraction(bot, botAI, RITUAL_DELAY_MS);
    
    return true;
}


// SoulPortalAvailableTrigger - Detect if Soul Portal is available (no delay)
bool SoulPortalAvailableTrigger::IsActive()
{
    if (!IsInRitualMap(bot))
    {
        return false;
    }
    
    // This prevents the Mage from interrupting its own ritual to interact with the Soul Portal
    if (bot->getClass() == CLASS_MAGE)
    {
        // Check if Mage has ritual of refreshment and should cast it
        if (bot->HasSpell(RITUAL_OF_REFRESHMENT_RANK_1) || bot->HasSpell(RITUAL_OF_REFRESHMENT_RANK_2)) // Rank 1 or Rank 2
        {
            // Check if Mage is currently channeling its ritual
            if (bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
            {
                return false;
            }
            
            // Check if Mage should cast its ritual (no refreshment table nearby)
            GameObject* refreshmentTable = bot->FindNearestGameObject(RITUAL_REFRESHMENT_TABLE_RANK_1, GetRitualSearchRange(bot)); // Refreshment Table Rank 1
            if (!refreshmentTable)
                refreshmentTable = bot->FindNearestGameObject(RITUAL_REFRESHMENT_TABLE_RANK_2, GetRitualSearchRange(bot)); // Refreshment Table Rank 2
            
            if (!refreshmentTable)
            {
                return false;
            }
        }
    }
    
    // Find soul portal nearby
    // Check for Soul Portal (created by ritual)
    GameObject* soulPortal = bot->FindNearestGameObject(RITUAL_SOUL_PORTAL_RANK_1, GetRitualSearchRange(bot)); // Soul Portal Rank 1
    if (!soulPortal)
        soulPortal = bot->FindNearestGameObject(RITUAL_SOUL_PORTAL_RANK_2, GetRitualSearchRange(bot)); // Soul Portal Rank 2
    
    if (soulPortal)
    {
        // Increment counter for timing (used by mage delay trigger)
        detectionCounter++;
        return true; // Allow immediate interaction
    }
    
    // Increment counter even when no soul portal found
    detectionCounter++;
    return false;
}

// InteractWithSoulwellAction - Interact with Soulwell to get healthstones
bool InteractWithSoulwellAction::isUseful()
{
    // Check if rituals can be used in current map
    if (!CanUseRituals(bot))
    {
        return false;
    }
    
    // Check if bot needs healthstone
    const std::vector<uint32> healthstoneIdsBelow = {
        RITUAL_MINOR_HEALTHSTONE, RITUAL_LESSER_HEALTHSTONE, RITUAL_MAJOR_HEALTHSTONE,
        RITUAL_MINOR_HEALTHSTONE_ALT, RITUAL_LESSER_HEALTHSTONE_ALT, RITUAL_FEL_HEALTHSTONE,
        RITUAL_DEMONIC_HEALTHSTONE
    };
    bool needsHealthstone = HasAllItemsBelow(bot, healthstoneIdsBelow, 1);
    
    if (!needsHealthstone)
    {
        return false;
    }
    
    // Find soulwell nearby
    GameObject* soulwell = bot->FindNearestGameObject(RITUAL_SOUL_WELL_RANK_1, GetRitualSearchRange(bot)); // Soul Well Rank 1
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(RITUAL_SOUL_WELL_RANK_2, GetRitualSearchRange(bot)); // Soul Well Rank 2
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(RITUAL_SOUL_WELL_RANK_2_VARIANT_1, GetRitualSearchRange(bot)); // Soul Well Rank 2 variant
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(RITUAL_SOUL_WELL_RANK_2_VARIANT_2, GetRitualSearchRange(bot)); // Soul Well Rank 2 variant
    
    if (!soulwell)
    {
        return false;
    }
    
    return true;
}

bool InteractWithSoulwellAction::Execute(Event event)
{
    // Find soulwell nearby (both ranks)
    GameObject* soulwell = bot->FindNearestGameObject(RITUAL_SOUL_WELL_RANK_1, GetRitualSearchRange(bot)); // Soul Well Rank 1
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(RITUAL_SOUL_WELL_RANK_2, GetRitualSearchRange(bot)); // Soul Well Rank 2
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(RITUAL_SOUL_WELL_RANK_2_VARIANT_1, GetRitualSearchRange(bot)); // Soul Well Rank 2 variant
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(RITUAL_SOUL_WELL_RANK_2_VARIANT_2, GetRitualSearchRange(bot)); // Soul Well Rank 2 variant
    
    if (!soulwell)
    {
        return false;
    }
    
    // Check if we're close enough to interact
    if (bot->GetDistance(soulwell) > RITUAL_INTERACTION_DISTANCE)
    {
        // Move closer to the soulwell
        return MoveTo(bot->GetMapId(), soulwell->GetPositionX(), soulwell->GetPositionY(), soulwell->GetPositionZ(), false, false);
    }
    
    // Interact with the soulwell to get healthstone
    soulwell->Use(bot);
    
    // Record interaction time to prevent spam
    uint32 currentTime = time(nullptr);
    uint64 botGuid = bot->GetGUID().GetRawValue();
    lastSoulwellInteraction[botGuid] = currentTime;
    
    // Mark that this bot has completed ritual interaction
    hasCompletedRitualInteraction[botGuid] = true;
    
 
    // Add a delay to prevent spam
    FinalizeRitualInteraction(bot, botAI, RITUAL_DELAY_MS);
    
    return true;
}

// SoulwellAvailableTrigger - Detect if Soulwell is available to get healthstones
bool SoulwellAvailableTrigger::IsActive()
{
    // Warlock and Mage should NOT interact with soulwell - they create it
    if (bot->getClass() == CLASS_WARLOCK || bot->getClass() == CLASS_MAGE)
    {
        return false;
    }
    
    if (!bot->GetMap()->IsDungeon() && !bot->GetMap()->IsRaid() && !bot->GetMap()->IsBattleground())
    {
        return false;
    }
    
    // Check if bot needs healthstone
    const std::vector<uint32> healthstoneIdsZeroOrMore = {
        RITUAL_MINOR_HEALTHSTONE, RITUAL_LESSER_HEALTHSTONE, RITUAL_MAJOR_HEALTHSTONE,
        RITUAL_MINOR_HEALTHSTONE_ALT, RITUAL_LESSER_HEALTHSTONE_ALT, RITUAL_FEL_HEALTHSTONE, RITUAL_DEMONIC_HEALTHSTONE
    };
    bool needsHealthstone = HasAllItemsBelow(bot, healthstoneIdsZeroOrMore, 1);

    if (!needsHealthstone)
    {
        return false;
    }
    
    // Check cooldown to prevent spam interaction
    uint32 currentTime = time(nullptr);
    uint64 botGuid = bot->GetGUID().GetRawValue();
    auto it = lastSoulwellInteraction.find(botGuid);
    if (it != lastSoulwellInteraction.end())
    {
        uint32 timeSinceLastInteraction = currentTime - it->second;
        if (timeSinceLastInteraction < RITUAL_MAGE_DELAY_CALLS) // RITUAL_MAGE_DELAY_CALLS seconds cooldown
        {
            return false;
        }
    }
    
    // Check for soulwell nearby (both ranks)
    GameObject* soulwell = bot->FindNearestGameObject(RITUAL_SOUL_WELL_RANK_1, GetRitualSearchRange(bot)); // Soul Well Rank 1
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(RITUAL_SOUL_WELL_RANK_2, GetRitualSearchRange(bot)); // Soul Well Rank 2
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(RITUAL_SOUL_WELL_RANK_2_VARIANT_1, GetRitualSearchRange(bot)); // Soul Well Rank 2 variant
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(RITUAL_SOUL_WELL_RANK_2_VARIANT_2, GetRitualSearchRange(bot)); // Soul Well Rank 2 variant
    
    if (soulwell)
    {
        return true;
    }
    
    return false;
}

// MageRitualWithDelayTrigger - Trigger for mage ritual with delay after detecting soul portal
bool MageRitualWithDelayTrigger::IsActive()
{
    // Only for mages
    if (bot->getClass() != CLASS_MAGE)
    {
        return false;
    }
    
    if (!bot->GetMap()->IsDungeon() && !bot->GetMap()->IsRaid() && !bot->GetMap()->IsBattleground())
    {
        return false;
    }
    
    // Check if mage has the ritual spell (either rank)
    bool hasSpell1 = bot->HasSpell(RITUAL_OF_REFRESHMENT_RANK_1); // Ritual of Refreshment Rank 1
    bool hasSpell2 = bot->HasSpell(RITUAL_OF_REFRESHMENT_RANK_2); // Ritual of Refreshment Rank 2
    if (!hasSpell1 && !hasSpell2)
    {
        return false;
    }
    
    // Check if there's already a refreshment table
    GameObject* existingTable = bot->FindNearestGameObject(RITUAL_REFRESHMENT_TABLE_RANK_1, GetRitualSearchRange(bot)); // Refreshment Table Rank 1
    if (!existingTable)
        existingTable = bot->FindNearestGameObject(RITUAL_REFRESHMENT_TABLE_RANK_2, GetRitualSearchRange(bot)); // Refreshment Table Rank 2
    
    if (existingTable)
    {
        return false; // Already have a table
    }
    
    // Check if there are Warlocks in the group AND if they are nearby
    bool hasWarlockInGroup = false;
    bool hasWarlockNearby = false;

    if (Group* group = bot->GetGroup()) 
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next()) 
        {
            if (Player* member = itr->GetSource()) 
            {
                if (member->getClass() == CLASS_WARLOCK)
                {
                    // In Battlegrounds, only consider same faction. In dungeons, consider all
                    bool isSameFaction = (member->GetTeamId() == bot->GetTeamId());
                    bool shouldConsider = bot->GetMap()->IsBattleground() ? isSameFaction : true;
                    
                    if (shouldConsider) 
                    {
                        hasWarlockInGroup = true;
                        // Check if the Warlock is nearby (same area)
                        if (bot->GetDistance(member) < RITUAL_WARLOCK_NEARBY_DISTANCE) 
                        {
                            hasWarlockNearby = true;
                        }
                    }
                }
            }
        }
    }

    // In Battlegrounds, also search for Warlocks of the SAME FACTION that are not in group
    if (bot->GetMap()->IsBattleground() && !hasWarlockNearby) {
        std::list<Player*> nearbyPlayers;
        Acore::AnyPlayerInObjectRangeCheck check(bot, RITUAL_NEARBY_PLAYER_SEARCH_RANGE);
        Acore::PlayerListSearcher<Acore::AnyPlayerInObjectRangeCheck> searcher(bot, nearbyPlayers, check);
        Cell::VisitObjects(bot, searcher, RITUAL_NEARBY_PLAYER_SEARCH_RANGE);
        
        for (Player* nearbyPlayer : nearbyPlayers) 
        {
            if (nearbyPlayer->getClass() == CLASS_WARLOCK && 
                nearbyPlayer->GetTeamId() == bot->GetTeamId()) 
            { 
                // Only same faction in BG
                hasWarlockNearby = true;
                break;
            }
        }
    }

    // Check if there's a soulwell nearby (created after bots interact with soul portal)
    GameObject* soulwell = bot->FindNearestGameObject(RITUAL_SOUL_WELL_RANK_1, GetRitualSearchRange(bot)); // Soul Well Rank 1
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(RITUAL_SOUL_WELL_RANK_2, GetRitualSearchRange(bot)); // Soul Well Rank 2
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(RITUAL_SOUL_WELL_RANK_2_VARIANT_1, GetRitualSearchRange(bot)); // Soul Well Rank 2 variant
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(RITUAL_SOUL_WELL_RANK_2_VARIANT_2, GetRitualSearchRange(bot)); // Soul Well Rank 2 variant
    
    // Only wait for soulwell if there is Warlock in group AND nearby
    if (hasWarlockInGroup && hasWarlockNearby && soulwell)
    {
        uint64 botGuid = bot->GetGUID().GetRawValue();
        
        // Check if this is the first time we detect the soulwell
        auto it = soulPortalDetectionTime.find(botGuid);
        if (it == soulPortalDetectionTime.end())
        {
            // First time detecting, record the counter
            soulPortalDetectionTime[botGuid] = detectionCounter;
            return false; // Don't activate yet, wait for delay
        }
        
        // Check if enough time has passed (approximately 1 second delay)
        uint32 timePassed = detectionCounter - it->second;
        return timePassed >= RITUAL_MAGE_DELAY_CALLS; // Delay completed, allow ritual casting
    }
    else
    {
        // No warlock in group/nearby or no soulwell - allow immediate ritual casting
        uint64 botGuid = bot->GetGUID().GetRawValue();
        soulPortalDetectionTime.erase(botGuid); // Clear any previous detection
        
        // Increment counter for timing
        detectionCounter++;
        return true; // Allow immediate ritual casting
    }
    
    // Increment counter for timing
    detectionCounter++;
    
    return false;
}

// RefreshmentPortalAvailableTrigger - Detect if Refreshment Portal is available
bool RefreshmentPortalAvailableTrigger::IsActive()
{
    if (!bot->GetMap()->IsDungeon() && !bot->GetMap()->IsRaid() && !bot->GetMap()->IsBattleground())
        return false;
    
    // Check for refreshment portal nearby (both ranks)
    GameObject* refreshmentPortal = bot->FindNearestGameObject(RITUAL_REFRESHMENT_PORTAL_RANK_1, GetRitualSearchRange(bot)); // Refreshment Portal Rank 1
    if (!refreshmentPortal)
        refreshmentPortal = bot->FindNearestGameObject(RITUAL_REFRESHMENT_PORTAL_RANK_2, GetRitualSearchRange(bot)); // Refreshment Portal Rank 2
    
    if (refreshmentPortal)
    {
        return true;
    }
    
    return false;
}

bool RefreshmentTableAvailableTrigger::IsActive()
{
    // Warlock and Mage should NOT interact with refreshment table - they create it
    if (bot->getClass() == CLASS_WARLOCK || bot->getClass() == CLASS_MAGE)
    {
        return false;
    }
    
    if (!CanUseRituals(bot))
        return false;
    
    // Check for refreshment table nearby (both ranks)
    GameObject* refreshmentTable = bot->FindNearestGameObject(RITUAL_REFRESHMENT_TABLE_RANK_1, GetRitualSearchRange(bot)); // Refreshment Table Rank 1
    if (!refreshmentTable)
        refreshmentTable = bot->FindNearestGameObject(RITUAL_REFRESHMENT_TABLE_RANK_2, GetRitualSearchRange(bot)); // Refreshment Table Rank 2
    
    if (refreshmentTable)
    {
        return true;
    }
    
    return false;
}

bool InDungeonOrRaidTrigger::IsActive()
{
    return CanUseRituals(bot);
}

bool NeedsConjuredItemsTrigger::IsActive()
{
    // Warlock and Mage should NOT check for conjured items - they create them
    if (bot->getClass() == CLASS_WARLOCK || bot->getClass() == CLASS_MAGE)
    {
        return false;
    }
    
    // Check if rituals can be used in current map
    if (!CanUseRituals(bot))
        return false;
    
    // Check if bot needs healthstone
    const std::vector<uint32> healthstoneIdsCheck1 = {
        RITUAL_MINOR_HEALTHSTONE, RITUAL_LESSER_HEALTHSTONE, RITUAL_MAJOR_HEALTHSTONE,
        RITUAL_MINOR_HEALTHSTONE_ALT, RITUAL_LESSER_HEALTHSTONE_ALT, RITUAL_FEL_HEALTHSTONE,
        RITUAL_DEMONIC_HEALTHSTONE
    };
    bool needsHealthstone = HasAllItemsBelow(bot, healthstoneIdsCheck1, 1);
    
    // Check if bot needs food/drink
    const std::vector<uint32> refreshmentIds = {
        RITUAL_CONJURED_MANA_BISCUIT, RITUAL_CONJURED_MANA_STRUDEL, RITUAL_CONJURED_MANA_COOKIE,
        RITUAL_CONJURED_MANA_CAKE, RITUAL_CONJURED_MANA_PIE, RITUAL_CONJURED_MANA_BREAD,
        RITUAL_CONJURED_MANA_MUFFIN, RITUAL_CONJURED_MANA_DONUT, RITUAL_CONJURED_MANA_BAGEL,
        RITUAL_CONJURED_MANA_PRETZEL, RITUAL_CONJURED_MANA_WATER, RITUAL_CONJURED_MANA_JUICE,
        RITUAL_CONJURED_MANA_TEA, RITUAL_CONJURED_MANA_COFFEE
    };
    bool needsFoodDrink = HasAllItemsBelow(bot, refreshmentIds, RITUAL_FOOD_DRINK_LIMIT);
    
    // Check if there are conjured objects nearby that we can interact with
    bool hasSoulwell = 
        bot->FindNearestGameObject(RITUAL_SOUL_WELL_RANK_1, GetRitualSearchRange(bot)) || // Soul Well Rank 1
        bot->FindNearestGameObject(RITUAL_SOUL_WELL_RANK_2, GetRitualSearchRange(bot)) || // Soul Well Rank 2
        bot->FindNearestGameObject(RITUAL_SOUL_WELL_RANK_2_VARIANT_1, GetRitualSearchRange(bot)) || // Soul Well Rank 2 variant
        bot->FindNearestGameObject(RITUAL_SOUL_WELL_RANK_2_VARIANT_2, GetRitualSearchRange(bot));  // Soul Well Rank 2 variant
    
    bool hasRefreshmentTable =
        bot->FindNearestGameObject(RITUAL_REFRESHMENT_TABLE_RANK_1, GetRitualSearchRange(bot)) || // Refreshment Table Rank 1
        bot->FindNearestGameObject(RITUAL_REFRESHMENT_TABLE_RANK_2, GetRitualSearchRange(bot));   // Refreshment Table Rank 2
    
    // Return true if bot needs items and there are objects to interact with
    return (needsHealthstone && hasSoulwell) || (needsFoodDrink && hasRefreshmentTable);
}

// CheckConjuredItemsAction - Check if bot needs conjured items and re-interact if necessary
bool CheckConjuredItemsAction::isUseful()
{
    // Check if rituals can be used in current map
    if (!CanUseRituals(bot))
        return false;
    
    // Check if bot needs healthstone
    bool needsHealthstone = 
        bot->GetItemCount(RITUAL_MINOR_HEALTHSTONE) == 0 && // Minor Healthstone
        bot->GetItemCount(RITUAL_LESSER_HEALTHSTONE) == 0 && // Lesser Healthstone
        bot->GetItemCount(RITUAL_MAJOR_HEALTHSTONE) == 0 && // Major Healthstone
        bot->GetItemCount(RITUAL_MINOR_HEALTHSTONE_ALT) == 0 && // Minor Healthstone
        bot->GetItemCount(RITUAL_LESSER_HEALTHSTONE_ALT) == 0;  // Lesser Healthstone
    
    // Check if bot needs food/drink
    const std::vector<uint32> refreshmentIds = {
        RITUAL_CONJURED_MANA_BISCUIT, RITUAL_CONJURED_MANA_STRUDEL, RITUAL_CONJURED_MANA_COOKIE,
        RITUAL_CONJURED_MANA_CAKE, RITUAL_CONJURED_MANA_PIE, RITUAL_CONJURED_MANA_BREAD,
        RITUAL_CONJURED_MANA_MUFFIN, RITUAL_CONJURED_MANA_DONUT, RITUAL_CONJURED_MANA_BAGEL,
        RITUAL_CONJURED_MANA_PRETZEL, RITUAL_CONJURED_MANA_WATER, RITUAL_CONJURED_MANA_JUICE,
        RITUAL_CONJURED_MANA_TEA, RITUAL_CONJURED_MANA_COFFEE
    };
    bool needsFoodDrink = HasAllItemsBelow(bot, refreshmentIds, RITUAL_FOOD_DRINK_LIMIT);
    
    // Check if there are conjured objects nearby that we can interact with
    bool hasSoulwell =
        bot->FindNearestGameObject(RITUAL_SOUL_WELL_RANK_1, GetRitualSearchRange(bot)) || // Soul Well Rank 1
        bot->FindNearestGameObject(RITUAL_SOUL_WELL_RANK_2, GetRitualSearchRange(bot)) || // Soul Well Rank 2
        bot->FindNearestGameObject(RITUAL_SOUL_WELL_RANK_2_VARIANT_1, GetRitualSearchRange(bot)) || // Soul Well Rank 2 variant
        bot->FindNearestGameObject(RITUAL_SOUL_WELL_RANK_2_VARIANT_2, GetRitualSearchRange(bot));  // Soul Well Rank 2 variant
    
    bool hasRefreshmentTable = 
        bot->FindNearestGameObject(RITUAL_REFRESHMENT_TABLE_RANK_1, GetRitualSearchRange(bot)) || // Refreshment Table Rank 1
        bot->FindNearestGameObject(RITUAL_REFRESHMENT_TABLE_RANK_2, GetRitualSearchRange(bot));   // Refreshment Table Rank 2
    
    // Return true if bot needs items and there are objects to interact with
    return (needsHealthstone && hasSoulwell) || (needsFoodDrink && hasRefreshmentTable);
}

bool CheckConjuredItemsAction::Execute(Event event)
{
    // Check if bot needs healthstone and there's a soulwell nearby
    const std::vector<uint32> healthstoneIdsCheck2 = {
        RITUAL_MINOR_HEALTHSTONE, RITUAL_LESSER_HEALTHSTONE, RITUAL_MAJOR_HEALTHSTONE,
        RITUAL_MINOR_HEALTHSTONE_ALT, RITUAL_LESSER_HEALTHSTONE_ALT, RITUAL_FEL_HEALTHSTONE, RITUAL_DEMONIC_HEALTHSTONE
    };
    bool needsHealthstone = HasAllItemsBelow(bot, healthstoneIdsCheck2, 1);
    
    if (needsHealthstone)
    {
        // Find soulwell nearby
        GameObject* soulwell = bot->FindNearestGameObject(RITUAL_SOUL_WELL_RANK_1, GetRitualSearchRange(bot)); // Soul Well Rank 1
        if (!soulwell)
            soulwell = bot->FindNearestGameObject(RITUAL_SOUL_WELL_RANK_2, GetRitualSearchRange(bot)); // Soul Well Rank 2
        if (!soulwell)
            soulwell = bot->FindNearestGameObject(RITUAL_SOUL_WELL_RANK_2_VARIANT_1, GetRitualSearchRange(bot)); // Soul Well Rank 2 variant
        if (!soulwell)
            soulwell = bot->FindNearestGameObject(RITUAL_SOUL_WELL_RANK_2_VARIANT_2, GetRitualSearchRange(bot)); // Soul Well Rank 2 variant
        
        if (soulwell)
        {
            // Check if we're close enough to interact
            if (bot->GetDistance(soulwell) > RITUAL_INTERACTION_DISTANCE)
            {
                // Move closer to the soulwell
                bot->GetMotionMaster()->MovePoint(0, soulwell->GetPositionX(), soulwell->GetPositionY(), soulwell->GetPositionZ());
                return false;
            }
            
            // Interact with the soulwell to get healthstone
            soulwell->Use(bot);
            botAI->SetNextCheckDelay(RITUAL_DELAY_MS); // 3 seconds delay
            return true;
        }
    }
    
    // Check if bot needs food/drink and there's a refreshment table nearby
    const std::vector<uint32> refreshmentIds = {
        RITUAL_CONJURED_MANA_BISCUIT, RITUAL_CONJURED_MANA_STRUDEL, RITUAL_CONJURED_MANA_COOKIE,
        RITUAL_CONJURED_MANA_CAKE, RITUAL_CONJURED_MANA_PIE, RITUAL_CONJURED_MANA_BREAD,
        RITUAL_CONJURED_MANA_MUFFIN, RITUAL_CONJURED_MANA_DONUT, RITUAL_CONJURED_MANA_BAGEL,
        RITUAL_CONJURED_MANA_PRETZEL, RITUAL_CONJURED_MANA_WATER, RITUAL_CONJURED_MANA_JUICE,
        RITUAL_CONJURED_MANA_TEA, RITUAL_CONJURED_MANA_COFFEE
    };
    bool needsFoodDrink = HasAllItemsBelow(bot, refreshmentIds, RITUAL_FOOD_DRINK_LIMIT);
    
    if (needsFoodDrink)
    {
        // Find refreshment table nearby
        GameObject* refreshmentTable = bot->FindNearestGameObject(RITUAL_REFRESHMENT_TABLE_RANK_1, GetRitualSearchRange(bot)); // Refreshment Table Rank 1
        if (!refreshmentTable)
        {
            refreshmentTable = bot->FindNearestGameObject(RITUAL_REFRESHMENT_TABLE_RANK_2, GetRitualSearchRange(bot)); // Refreshment Table Rank 2
        }
        
        if (refreshmentTable)
        {
            // Check if we're close enough to interact
            if (bot->GetDistance(refreshmentTable) > RITUAL_INTERACTION_DISTANCE)
            {
                // Move closer to the refreshment table
                bot->GetMotionMaster()->MovePoint(0, refreshmentTable->GetPositionX(), refreshmentTable->GetPositionY(), refreshmentTable->GetPositionZ());
                return false;
            }
            
            // Interact with the refreshment table
            refreshmentTable->Use(bot);
            botAI->SetNextCheckDelay(RITUAL_REFRESHMENT_PORTAL_MOVEMENT_ID); // 5 seconds delay
            return true;
        }
    }
    
    return false;
}

bool HasRitualComponent(Player* bot, uint32 spellId)
{
    if (spellId == RITUAL_COMPONENT_ITEM_1 || spellId == RITUAL_COMPONENT_ITEM_2)
    {
        if (bot->GetItemCount(RITUAL_COMPONENT_ITEM_3) == 0)
        {
            return bot->AddItem(RITUAL_COMPONENT_ITEM_3, 1);
        }
        return true;
    }
    
    if (spellId == RITUAL_OF_REFRESHMENT_RANK_1)
    {
        if (bot->GetItemCount(RITUAL_COMPONENT_ITEM_4) < 2)
        {
            return bot->AddItem(RITUAL_COMPONENT_ITEM_4, 2);
        }
        return true;
    }
    
    return false;
}

// NeedsFollowRestorationTrigger - Detects when a bot needs to restore follow behavior
bool NeedsFollowRestorationTrigger::IsActive()
{
    // Only active in dungeons/raids/bgs
    if (!bot->GetMap()->IsDungeon() && 
        !bot->GetMap()->IsRaid() && 
        !bot->GetMap()->IsBattleground())
    {
        return false;
    }
    
    // Only active if bot has follow strategy
    if (!botAI->HasStrategy("follow", BOT_STATE_NON_COMBAT))
        return false;
    
    // Check follow restoration cooldown (30 seconds)
    uint32 currentTime = time(nullptr);
    auto it = lastFollowRestoration.find(bot->GetGUID().GetCounter());
    if (it != lastFollowRestoration.end())
    {
        uint32 timeSinceLastRestoration = currentTime - it->second;
        if (timeSinceLastRestoration < 30) // 30 seconds cooldown
        {
            return false;
        }
    }
    
    // Check if bot is stationary and not channeling
    if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() != IDLE_MOTION_TYPE || bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
        return false;
    
    // Check if bot has a real player master
    if (!botAI->HasRealPlayerMaster())
        return false;
    Unit* master = botAI->GetGroupMaster();
    
    // Check if bot is not currently following
    if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE)
        return false;
    
    // Check if bot has completed a ritual interaction recently
    uint64 botGuid = bot->GetGUID().GetCounter();
    auto interactionIt = hasCompletedRitualInteraction.find(botGuid);
    if (interactionIt == hasCompletedRitualInteraction.end() || !interactionIt->second)
    {
        return false; // Has not completed ritual interaction
    }
    
    // Check if bot already has necessary items (indicating it completed the interaction)
    bool hasHealthstone = 
        bot->HasItemCount(RITUAL_MINOR_HEALTHSTONE, 1) || 
        bot->HasItemCount(RITUAL_LESSER_HEALTHSTONE, 1) || 
        bot->HasItemCount(RITUAL_MAJOR_HEALTHSTONE, 1) || 
        bot->HasItemCount(RITUAL_MINOR_HEALTHSTONE_ALT, 1) || 
        bot->HasItemCount(RITUAL_LESSER_HEALTHSTONE_ALT, 1) || 
        bot->HasItemCount(RITUAL_FEL_HEALTHSTONE, 1);
    
    bool hasRefreshment = bot->HasItemCount(RITUAL_CONJURED_MANA_BISCUIT, 1); // Conjured Mana Strudel
    
    // Only activate if bot has completed ritual interaction AND has necessary items
    if (hasHealthstone || hasRefreshment)
    {
        return true;
    }
    
    return false;
}

// RestoreFollowBehaviorAction - Restore follow behavior
bool RestoreFollowBehaviorAction::isUseful()
{
    // Only useful in dungeons/raids/bgs
    if (!bot->GetMap()->IsDungeon() && !bot->GetMap()->IsRaid() && !bot->GetMap()->IsBattleground())
        return false;
    
    // Only useful if bot has follow strategy
    if (!botAI->HasStrategy("follow", BOT_STATE_NON_COMBAT))
        return false;
    
    // Check if bot has a real player master
    if (!botAI->HasRealPlayerMaster())
        return false;
    Unit* master = botAI->GetGroupMaster();
    
    // Check if bot is not currently following
    if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE)
        return false;
    
    return true;
}

bool RestoreFollowBehaviorAction::Execute(Event event)
{
    // Record restoration time to avoid loops
    uint32 currentTime = time(nullptr);
    uint64 botGuid = bot->GetGUID().GetCounter();
    lastFollowRestoration[botGuid] = currentTime;
    
    // Clear completed interaction flag to avoid reactivations
    hasCompletedRitualInteraction[botGuid] = false;
    
    // Try to find master to follow (real player master only)
    if (botAI->HasRealPlayerMaster())
    {        
        Unit* master = botAI->GetGroupMaster();
        if (!master || master == bot)
            return false;
        // Use direct follow approach
        bot->GetMotionMaster()->MoveFollow(master, sPlayerbotAIConfig->followDistance, 0.0f);    
        return true;
    }
    else
    {        
        // Fallback to DoSpecificAction
        return botAI->DoSpecificAction("follow", Event(), true);
    }
}
