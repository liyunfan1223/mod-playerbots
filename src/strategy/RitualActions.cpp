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

// Helper function to get appropriate search range based on map type
static float GetRitualSearchRange(Player* bot)
{
    return bot->GetMap()->IsBattleground() ? 100.0f : 30.0f;
}
// Track if bot has completed ritual interaction to avoid loops
std::unordered_map<uint64, bool> hasCompletedRitualInteraction;

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

bool MoveAwayFromSpawnAction::isUseful()
{
    Player* bot = botAI->GetBot();
    
    if (!CanUseRituals(bot))
        return false;
    
    return false;
}

bool MoveAwayFromSpawnAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    
    float x = bot->GetPositionX() + (rand() % 20 - 10);
    float y = bot->GetPositionY() + (rand() % 20 - 10);
    float z = bot->GetPositionZ();
    bot->GetMotionMaster()->MovePoint(0, x, y, z);
    
    return true;
}

// InteractWithSoulPortalAction - Interact with Soul Portal to create Soul Well and get Healthstone
bool InteractWithSoulPortalAction::isUseful()
{
    Player* bot = botAI->GetBot();
    
    // Check if rituals can be used in current map
    if (!CanUseRituals(bot))
        return false;
    
    // Check if bot already has Healthstone (limit to 1)
    if (bot->GetItemCount(5512) >= 1 || // Minor Healthstone
        bot->GetItemCount(5511) >= 1 || // Lesser Healthstone
        bot->GetItemCount(9421) >= 1 || // Major Healthstone
        bot->GetItemCount(19004) >= 1 || // Minor Healthstone
        bot->GetItemCount(19005) >= 1)   // Lesser Healthstone
    {
        return false;
    }
    
    // Search for soul portal nearby (within 30 yards)
    // Try correct Soul Portal IDs from user
    GameObject* soulPortal = bot->FindNearestGameObject(181622, GetRitualSearchRange(bot)); // Soul Portal Rank 1
    if (!soulPortal)
        soulPortal = bot->FindNearestGameObject(193168, GetRitualSearchRange(bot)); // Soul Portal Rank 2
    
    if (!soulPortal)
    {
        return false;
    }
    
    return true;
}

bool InteractWithSoulPortalAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();

    // Search for soul portal nearby (within 30 yards)
    // Try correct Soul Portal IDs from user
    GameObject* soulPortal = bot->FindNearestGameObject(181622, GetRitualSearchRange(bot)); // Soul Portal Rank 1
    if (!soulPortal)
        soulPortal = bot->FindNearestGameObject(193168, GetRitualSearchRange(bot)); // Soul Portal Rank 2
    
    if (!soulPortal)
    {
        return false;
    }
    
    // Check if we're close enough to interact
    if (bot->GetDistance(soulPortal) > 5.0f)
    {
        // Move closer to the soul portal using unique movement ID
        uint32 movementId = bot->GetGUID().GetCounter() + 4000; // Unique ID for soul portal movement
        bot->GetMotionMaster()->MovePoint(movementId, soulPortal->GetPositionX(), soulPortal->GetPositionY(), soulPortal->GetPositionZ());
        
        return false;
    }
    
    // Interact with the soul portal to create soul well
    soulPortal->Use(bot);
    
    // Clear movement state to allow normal AI behavior
    bot->GetMotionMaster()->Clear();
    bot->GetMotionMaster()->MoveIdle();

    // Add a small delay to allow re-evaluation of triggers and for mage to cast ritual
    botAI->SetNextCheckDelay(3000); // 3 seconds delay
    
    return true;
}

// InteractWithRefreshmentPortalAction - Interact with Refreshment Portal to create Refreshment Table
bool InteractWithRefreshmentPortalAction::isUseful()
{
    Player* bot = botAI->GetBot();
    
    // Check if rituals can be used in current map
    if (!CanUseRituals(bot))
        return false;
    
    // Find refreshment portal nearby (both ranks)
    GameObject* refreshmentPortal = bot->FindNearestGameObject(186811, GetRitualSearchRange(bot)); // Refreshment Portal Rank 1
    if (!refreshmentPortal)
        refreshmentPortal = bot->FindNearestGameObject(193062, GetRitualSearchRange(bot)); // Refreshment Portal Rank 2
    
    if (!refreshmentPortal)
    {
        return false;
    }
    
    return true;
}

bool InteractWithRefreshmentPortalAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    
    
    // Find refreshment portal nearby (both ranks)
    GameObject* refreshmentPortal = bot->FindNearestGameObject(186811, GetRitualSearchRange(bot)); // Refreshment Portal Rank 1
    if (!refreshmentPortal)
        refreshmentPortal = bot->FindNearestGameObject(193062, GetRitualSearchRange(bot)); // Refreshment Portal Rank 2
    
    if (!refreshmentPortal)
    {
        return false;
    }
    
    // Check if we're close enough to interact
    if (bot->GetDistance(refreshmentPortal) > 5.0f)
    {
        // Move closer to the refreshment portal using unique movement ID
        uint32 movementId = bot->GetGUID().GetCounter() + 5000; // Unique ID for refreshment portal movement
        bot->GetMotionMaster()->MovePoint(movementId, refreshmentPortal->GetPositionX(), refreshmentPortal->GetPositionY(), refreshmentPortal->GetPositionZ());
        
        return false;
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
    Player* bot = botAI->GetBot();
    
    // Check if rituals can be used in current map
    if (!CanUseRituals(bot))
        return false;
    
    // Check if bot already has food/drink (limit to 1 stack each)
    // Check for various types of conjured food and drink
    if (bot->GetItemCount(43523) >= 20 || // Conjured Mana Biscuit
        bot->GetItemCount(43518) >= 20 || // Conjured Mana Strudel
        bot->GetItemCount(43517) >= 20 || // Conjured Mana Cookie
        bot->GetItemCount(43516) >= 20 || // Conjured Mana Cake
        bot->GetItemCount(43515) >= 20 || // Conjured Mana Pie
        bot->GetItemCount(43514) >= 20 || // Conjured Mana Bread
        bot->GetItemCount(43513) >= 20 || // Conjured Mana Muffin
        bot->GetItemCount(43512) >= 20 || // Conjured Mana Donut
        bot->GetItemCount(43511) >= 20 || // Conjured Mana Bagel
        bot->GetItemCount(43510) >= 20 || // Conjured Mana Pretzel
        bot->GetItemCount(43519) >= 20 || // Conjured Mana Water
        bot->GetItemCount(43520) >= 20 || // Conjured Mana Juice
        bot->GetItemCount(43521) >= 20 || // Conjured Mana Tea
        bot->GetItemCount(43522) >= 20)   // Conjured Mana Coffee
    {
        return false; // Already has enough food/drink
    }
    
    // Find refreshment table nearby
    GameObject* refreshmentTable = bot->FindNearestGameObject(186812, GetRitualSearchRange(bot)); // Refreshment Table ID
    if (!refreshmentTable)
        refreshmentTable = bot->FindNearestGameObject(193061, GetRitualSearchRange(bot)); // Refreshment Table Rank 2
    
    if (!refreshmentTable)
    {
        return false;
    }
    
    return true;
}

bool InteractWithRefreshmentTableAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    
    // Find refreshment table nearby (both ranks)
    GameObject* refreshmentTable = bot->FindNearestGameObject(186812, GetRitualSearchRange(bot)); // Refreshment Table Rank 1
    if (!refreshmentTable)
        refreshmentTable = bot->FindNearestGameObject(193061, GetRitualSearchRange(bot)); // Refreshment Table Rank 2
    
    if (!refreshmentTable)
    {
        return false;
    }
    
    // Check if we're close enough to interact
    if (bot->GetDistance(refreshmentTable) > 5.0f)
    {
        // Move closer to the refreshment table using unique movement ID
        uint32 movementId = bot->GetGUID().GetCounter() + 6000; // Unique ID for refreshment table movement
        bot->GetMotionMaster()->MovePoint(movementId, refreshmentTable->GetPositionX(), refreshmentTable->GetPositionY(), refreshmentTable->GetPositionZ());
        
        return false;
    }
    
    // Interact with the refreshment table
    refreshmentTable->Use(bot);
    
    // Mark that this bot has completed ritual interaction
    uint64 botGuid = bot->GetGUID().GetRawValue();
    hasCompletedRitualInteraction[botGuid] = true;
    
    // Clear movement state to allow normal AI behavior
    bot->GetMotionMaster()->Clear();
    bot->GetMotionMaster()->MoveIdle();
    
    // Don't restore follow behavior immediately - let the ritual complete naturally
    
    // Add a delay to prevent spam
    botAI->SetNextCheckDelay(3000); // 3 seconds delay (reduced from 5)
    
    return true;
}


// SoulPortalAvailableTrigger - Detect if Soul Portal is available (no delay)
bool SoulPortalAvailableTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    
    
    if (!bot->GetMap()->IsDungeon() && !bot->GetMap()->IsRaid() && !bot->GetMap()->IsBattleground())
    {
        return false;
    }
    
    // CRITICAL FIX: Don't activate for Mages when they should be casting their own ritual
    // This prevents the Mage from interrupting its own ritual to interact with the Soul Portal
    if (bot->getClass() == CLASS_MAGE)
    {
        // Check if Mage has ritual of refreshment and should cast it
        if (bot->HasSpell(43987) || bot->HasSpell(58659)) // Rank 1 or Rank 2
        {
            // Check if Mage is currently channeling its ritual
            if (bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
            {
                return false;
            }
            
            // Check if Mage should cast its ritual (no refreshment table nearby)
            GameObject* refreshmentTable = bot->FindNearestGameObject(186812, GetRitualSearchRange(bot)); // Refreshment Table Rank 1
            if (!refreshmentTable)
                refreshmentTable = bot->FindNearestGameObject(193061, GetRitualSearchRange(bot)); // Refreshment Table Rank 2
            
            if (!refreshmentTable)
            {
                return false;
            }
        }
    }
    
    // Find soul portal nearby
    // Check for Soul Portal (created by ritual)
    GameObject* soulPortal = bot->FindNearestGameObject(181622, GetRitualSearchRange(bot)); // Soul Portal Rank 1
    if (!soulPortal)
        soulPortal = bot->FindNearestGameObject(193168, GetRitualSearchRange(bot)); // Soul Portal Rank 2
    
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
    Player* bot = botAI->GetBot();
    
    // Check if rituals can be used in current map
    if (!CanUseRituals(bot))
    {
        return false;
    }
    
    // Check if bot needs healthstone
    bool needsHealthstone = (bot->GetItemCount(5512) == 0 && // Minor Healthstone
                            bot->GetItemCount(5511) == 0 && // Lesser Healthstone
                            bot->GetItemCount(9421) == 0 && // Major Healthstone
                            bot->GetItemCount(19004) == 0 && // Minor Healthstone
                            bot->GetItemCount(19005) == 0);  // Lesser Healthstone
    
    if (!needsHealthstone)
    {
        return false;
    }
    
    // Find soulwell nearby
    GameObject* soulwell = bot->FindNearestGameObject(181621, GetRitualSearchRange(bot)); // Soul Well Rank 1
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(193169, GetRitualSearchRange(bot)); // Soul Well Rank 2
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(193170, GetRitualSearchRange(bot)); // Soul Well Rank 2 variant
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(193171, GetRitualSearchRange(bot)); // Soul Well Rank 2 variant
    
    if (!soulwell)
    {
        return false;
    }
    
    return true;
}

bool InteractWithSoulwellAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    
    
    // Find soulwell nearby (both ranks)
    GameObject* soulwell = bot->FindNearestGameObject(181621, GetRitualSearchRange(bot)); // Soul Well Rank 1
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(193169, GetRitualSearchRange(bot)); // Soul Well Rank 2
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(193170, GetRitualSearchRange(bot)); // Soul Well Rank 2 variant
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(193171, GetRitualSearchRange(bot)); // Soul Well Rank 2 variant
    
    if (!soulwell)
    {
        return false;
    }
    
    // Check if we're close enough to interact
    if (bot->GetDistance(soulwell) > 5.0f)
    {
        // Move closer to the soulwell using unique movement ID
        uint32 movementId = bot->GetGUID().GetCounter() + 7000; // Unique ID for soulwell movement
        bot->GetMotionMaster()->MovePoint(movementId, soulwell->GetPositionX(), soulwell->GetPositionY(), soulwell->GetPositionZ());
        return false;
    }
    
    // Interact with the soulwell to get healthstone
    soulwell->Use(bot);
    
    // Record interaction time to prevent spam
    uint32 currentTime = time(nullptr);
    uint64 botGuid = bot->GetGUID().GetRawValue();
    lastSoulwellInteraction[botGuid] = currentTime;
    
    // Mark that this bot has completed ritual interaction
    hasCompletedRitualInteraction[botGuid] = true;
    
 
    // Clear movement state to allow normal AI behavior
    bot->GetMotionMaster()->Clear();
    bot->GetMotionMaster()->MoveIdle();
    
    // Add a delay to prevent spam
    botAI->SetNextCheckDelay(3000); // 3 seconds delay (reduced from 5)
    
    return true;
}

// SoulwellAvailableTrigger - Detect if Soulwell is available to get healthstones
bool SoulwellAvailableTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    
    
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
    bool needsHealthstone = (bot->GetItemCount(5512) == 0 && // Minor Healthstone
                            bot->GetItemCount(5511) == 0 && // Lesser Healthstone
                            bot->GetItemCount(9421) == 0 && // Major Healthstone
                            bot->GetItemCount(19004) == 0 && // Minor Healthstone
                            bot->GetItemCount(19005) == 0 && // Lesser Healthstone
                            bot->GetItemCount(36892) == 0);  // Fel Healthstone

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
        if (timeSinceLastInteraction < 10) // 10 seconds cooldown
        {
            return false;
        }
    }
    
    // Check for soulwell nearby (both ranks)
    GameObject* soulwell = bot->FindNearestGameObject(181621, GetRitualSearchRange(bot)); // Soul Well Rank 1
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(193169, GetRitualSearchRange(bot)); // Soul Well Rank 2
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(193170, GetRitualSearchRange(bot)); // Soul Well Rank 2 variant
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(193171, GetRitualSearchRange(bot)); // Soul Well Rank 2 variant
    
    if (soulwell)
    {
        return true;
    }
    
    return false;
}

// MageRitualWithDelayTrigger - Trigger for mage ritual with delay after detecting soul portal
bool MageRitualWithDelayTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    
    
    // Only for mages
    if (bot->getClass() != CLASS_MAGE)
    {
        {
        }
        return false;
    }
    
    if (!bot->GetMap()->IsDungeon() && !bot->GetMap()->IsRaid() && !bot->GetMap()->IsBattleground())
    {
        {
        }
        return false;
    }
    
    // Check if mage has the ritual spell (either rank)
    bool hasSpell1 = bot->HasSpell(43987); // Ritual of Refreshment Rank 1
    bool hasSpell2 = bot->HasSpell(58659); // Ritual of Refreshment Rank 2
    
    if (!hasSpell1 && !hasSpell2)
    {
        return false;
    }
    
    // Check if there's already a refreshment table
    GameObject* existingTable = bot->FindNearestGameObject(186812, GetRitualSearchRange(bot)); // Refreshment Table Rank 1
    if (!existingTable)
        existingTable = bot->FindNearestGameObject(193061, GetRitualSearchRange(bot)); // Refreshment Table Rank 2
    
    if (existingTable)
    {
        return false; // Already have a table
    }
    
    // Check if there are Warlocks in the group AND if they are nearby
    bool hasWarlockInGroup = false;
    bool hasWarlockNearby = false;

    if (Group* group = bot->GetGroup()) {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next()) {
            if (Player* member = itr->GetSource()) {
                if (member->getClass() == CLASS_WARLOCK) {
                    // In Battlegrounds, only consider same faction. In dungeons, consider all
                    bool isSameFaction = (member->GetTeamId() == bot->GetTeamId());
                    bool shouldConsider = bot->GetMap()->IsBattleground() ? isSameFaction : true;
                    
                    if (shouldConsider) {
                        hasWarlockInGroup = true;
                        // Check if the Warlock is nearby (same area)
                        if (bot->GetDistance(member) < 100.0f) {
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
        Acore::AnyPlayerInObjectRangeCheck check(bot, 150.0f);
        Acore::PlayerListSearcher<Acore::AnyPlayerInObjectRangeCheck> searcher(bot, nearbyPlayers, check);
        Cell::VisitObjects(bot, searcher, 150.0f);
        
        for (Player* nearbyPlayer : nearbyPlayers) {
            if (nearbyPlayer->getClass() == CLASS_WARLOCK && 
                nearbyPlayer->GetTeamId() == bot->GetTeamId()) { // Only same faction in BG
                hasWarlockNearby = true;
                break;
            }
        }
    }

    // Check if there's a soulwell nearby (created after bots interact with soul portal)
    GameObject* soulwell = bot->FindNearestGameObject(181621, GetRitualSearchRange(bot)); // Soul Well Rank 1
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(193169, GetRitualSearchRange(bot)); // Soul Well Rank 2
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(193170, GetRitualSearchRange(bot)); // Soul Well Rank 2 variant
    if (!soulwell)
        soulwell = bot->FindNearestGameObject(193171, GetRitualSearchRange(bot)); // Soul Well Rank 2 variant
    
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
        
        if (timePassed >= 10) // Approximately 1 second with 10 calls per second
        {
            return true; // Delay completed, allow ritual casting
        }
        
        return false; // Still waiting for delay
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
    Player* bot = botAI->GetBot();
    
    if (!bot->GetMap()->IsDungeon() && !bot->GetMap()->IsRaid() && !bot->GetMap()->IsBattleground())
        return false;
    
    // Check for refreshment portal nearby (both ranks)
    GameObject* refreshmentPortal = bot->FindNearestGameObject(186811, GetRitualSearchRange(bot)); // Refreshment Portal Rank 1
    if (!refreshmentPortal)
        refreshmentPortal = bot->FindNearestGameObject(193062, GetRitualSearchRange(bot)); // Refreshment Portal Rank 2
    
    if (refreshmentPortal)
    {
        return true;
    }
    
    
    return false;
}

bool RefreshmentTableAvailableTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    
    // Warlock and Mage should NOT interact with refreshment table - they create it
    if (bot->getClass() == CLASS_WARLOCK || bot->getClass() == CLASS_MAGE)
    {
        return false;
    }
    
    if (!bot->GetMap()->IsDungeon() && !bot->GetMap()->IsRaid() && !bot->GetMap()->IsBattleground())
        return false;
    
    // Check for refreshment table nearby (both ranks)
    GameObject* refreshmentTable = bot->FindNearestGameObject(186812, GetRitualSearchRange(bot)); // Refreshment Table Rank 1
    if (!refreshmentTable)
        refreshmentTable = bot->FindNearestGameObject(193061, GetRitualSearchRange(bot)); // Refreshment Table Rank 2
    
    if (refreshmentTable)
    {
        return true;
    }
    
    return false;
}

bool InDungeonOrRaidTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    return CanUseRituals(bot);
}

bool NeedsConjuredItemsTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    
    // Warlock and Mage should NOT check for conjured items - they create them
    if (bot->getClass() == CLASS_WARLOCK || bot->getClass() == CLASS_MAGE)
    {
        return false;
    }
    
    // Check if rituals can be used in current map
    if (!CanUseRituals(bot))
        return false;
    
    // Check if bot needs healthstone
    bool needsHealthstone = (bot->GetItemCount(5512) == 0 && // Minor Healthstone
                            bot->GetItemCount(5511) == 0 && // Lesser Healthstone
                            bot->GetItemCount(9421) == 0 && // Major Healthstone
                            bot->GetItemCount(19004) == 0 && // Minor Healthstone
                            bot->GetItemCount(19005) == 0);  // Lesser Healthstone
    
    // Check if bot needs food/drink
    bool needsFoodDrink = (bot->GetItemCount(43523) < 20 && // Conjured Mana Biscuit
                          bot->GetItemCount(43518) < 20 && // Conjured Mana Strudel
                          bot->GetItemCount(43517) < 20 && // Conjured Mana Cookie
                          bot->GetItemCount(43516) < 20 && // Conjured Mana Cake
                          bot->GetItemCount(43515) < 20 && // Conjured Mana Pie
                          bot->GetItemCount(43514) < 20 && // Conjured Mana Bread
                          bot->GetItemCount(43513) < 20 && // Conjured Mana Muffin
                          bot->GetItemCount(43512) < 20 && // Conjured Mana Donut
                          bot->GetItemCount(43511) < 20 && // Conjured Mana Bagel
                          bot->GetItemCount(43510) < 20 && // Conjured Mana Pretzel
                          bot->GetItemCount(43519) < 20 && // Conjured Mana Water
                          bot->GetItemCount(43520) < 20 && // Conjured Mana Juice
                          bot->GetItemCount(43521) < 20 && // Conjured Mana Tea
                          bot->GetItemCount(43522) < 20);  // Conjured Mana Coffee
    
    // Check if there are conjured objects nearby that we can interact with
    bool hasSoulwell = (bot->FindNearestGameObject(181621, GetRitualSearchRange(bot)) || // Soul Well Rank 1
                       bot->FindNearestGameObject(193169, GetRitualSearchRange(bot)) || // Soul Well Rank 2
                       bot->FindNearestGameObject(193170, GetRitualSearchRange(bot)) || // Soul Well Rank 2 variant
                       bot->FindNearestGameObject(193171, GetRitualSearchRange(bot)));  // Soul Well Rank 2 variant
    
    bool hasRefreshmentTable = (bot->FindNearestGameObject(186812, GetRitualSearchRange(bot)) || // Refreshment Table Rank 1
                               bot->FindNearestGameObject(193061, GetRitualSearchRange(bot)));   // Refreshment Table Rank 2
    
    // Return true if bot needs items and there are objects to interact with
    return (needsHealthstone && hasSoulwell) || (needsFoodDrink && hasRefreshmentTable);
}

// CheckConjuredItemsAction - Check if bot needs conjured items and re-interact if necessary
bool CheckConjuredItemsAction::isUseful()
{
    Player* bot = botAI->GetBot();
    
    // Check if rituals can be used in current map
    if (!CanUseRituals(bot))
        return false;
    
    // Check if bot needs healthstone
    bool needsHealthstone = (bot->GetItemCount(5512) == 0 && // Minor Healthstone
                            bot->GetItemCount(5511) == 0 && // Lesser Healthstone
                            bot->GetItemCount(9421) == 0 && // Major Healthstone
                            bot->GetItemCount(19004) == 0 && // Minor Healthstone
                            bot->GetItemCount(19005) == 0);  // Lesser Healthstone
    
    // Check if bot needs food/drink
    bool needsFoodDrink = (bot->GetItemCount(43523) < 20 && // Conjured Mana Biscuit
                          bot->GetItemCount(43518) < 20 && // Conjured Mana Strudel
                          bot->GetItemCount(43517) < 20 && // Conjured Mana Cookie
                          bot->GetItemCount(43516) < 20 && // Conjured Mana Cake
                          bot->GetItemCount(43515) < 20 && // Conjured Mana Pie
                          bot->GetItemCount(43514) < 20 && // Conjured Mana Bread
                          bot->GetItemCount(43513) < 20 && // Conjured Mana Muffin
                          bot->GetItemCount(43512) < 20 && // Conjured Mana Donut
                          bot->GetItemCount(43511) < 20 && // Conjured Mana Bagel
                          bot->GetItemCount(43510) < 20 && // Conjured Mana Pretzel
                          bot->GetItemCount(43519) < 20 && // Conjured Mana Water
                          bot->GetItemCount(43520) < 20 && // Conjured Mana Juice
                          bot->GetItemCount(43521) < 20 && // Conjured Mana Tea
                          bot->GetItemCount(43522) < 20);  // Conjured Mana Coffee
    
    // Check if there are conjured objects nearby that we can interact with
    bool hasSoulwell = (bot->FindNearestGameObject(181621, GetRitualSearchRange(bot)) || // Soul Well Rank 1
                       bot->FindNearestGameObject(193169, GetRitualSearchRange(bot)) || // Soul Well Rank 2
                       bot->FindNearestGameObject(193170, GetRitualSearchRange(bot)) || // Soul Well Rank 2 variant
                       bot->FindNearestGameObject(193171, GetRitualSearchRange(bot)));  // Soul Well Rank 2 variant
    
    bool hasRefreshmentTable = (bot->FindNearestGameObject(186812, GetRitualSearchRange(bot)) || // Refreshment Table Rank 1
                               bot->FindNearestGameObject(193061, GetRitualSearchRange(bot)));   // Refreshment Table Rank 2
    
    // Return true if bot needs items and there are objects to interact with
    return (needsHealthstone && hasSoulwell) || (needsFoodDrink && hasRefreshmentTable);
}

bool CheckConjuredItemsAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    
    // Check if bot needs healthstone and there's a soulwell nearby
    bool needsHealthstone = (bot->GetItemCount(5512) == 0 && // Minor Healthstone
                            bot->GetItemCount(5511) == 0 && // Lesser Healthstone
                            bot->GetItemCount(9421) == 0 && // Major Healthstone
                            bot->GetItemCount(19004) == 0 && // Minor Healthstone
                            bot->GetItemCount(19005) == 0 && // Lesser Healthstone
                            bot->GetItemCount(36892) == 0);  // Fel Healthstone
    
    if (needsHealthstone)
    {
        // Find soulwell nearby
        GameObject* soulwell = bot->FindNearestGameObject(181621, GetRitualSearchRange(bot)); // Soul Well Rank 1
        if (!soulwell)
            soulwell = bot->FindNearestGameObject(193169, GetRitualSearchRange(bot)); // Soul Well Rank 2
        if (!soulwell)
            soulwell = bot->FindNearestGameObject(193170, GetRitualSearchRange(bot)); // Soul Well Rank 2 variant
        if (!soulwell)
            soulwell = bot->FindNearestGameObject(193171, GetRitualSearchRange(bot)); // Soul Well Rank 2 variant
        
        if (soulwell)
        {
            // Check if we're close enough to interact
            if (bot->GetDistance(soulwell) > 5.0f)
            {
                // Move closer to the soulwell
                bot->GetMotionMaster()->MovePoint(0, soulwell->GetPositionX(), soulwell->GetPositionY(), soulwell->GetPositionZ());
                return false;
            }
            
            // Interact with the soulwell to get healthstone
            soulwell->Use(bot);
            botAI->SetNextCheckDelay(3000); // 3 seconds delay
            return true;
        }
    }
    
    // Check if bot needs food/drink and there's a refreshment table nearby
    bool needsFoodDrink = (bot->GetItemCount(43523) < 20 && // Conjured Mana Biscuit
                          bot->GetItemCount(43518) < 20 && // Conjured Mana Strudel
                          bot->GetItemCount(43517) < 20 && // Conjured Mana Cookie
                          bot->GetItemCount(43516) < 20 && // Conjured Mana Cake
                          bot->GetItemCount(43515) < 20 && // Conjured Mana Pie
                          bot->GetItemCount(43514) < 20 && // Conjured Mana Bread
                          bot->GetItemCount(43513) < 20 && // Conjured Mana Muffin
                          bot->GetItemCount(43512) < 20 && // Conjured Mana Donut
                          bot->GetItemCount(43511) < 20 && // Conjured Mana Bagel
                          bot->GetItemCount(43510) < 20 && // Conjured Mana Pretzel
                          bot->GetItemCount(43519) < 20 && // Conjured Mana Water
                          bot->GetItemCount(43520) < 20 && // Conjured Mana Juice
                          bot->GetItemCount(43521) < 20 && // Conjured Mana Tea
                          bot->GetItemCount(43522) < 20);  // Conjured Mana Coffee
    
    if (needsFoodDrink)
    {
    // Find refreshment table nearby
    GameObject* refreshmentTable = bot->FindNearestGameObject(186812, GetRitualSearchRange(bot)); // Refreshment Table Rank 1
    if (!refreshmentTable)
        refreshmentTable = bot->FindNearestGameObject(193061, GetRitualSearchRange(bot)); // Refreshment Table Rank 2
        
        if (refreshmentTable)
        {
            // Check if we're close enough to interact
            if (bot->GetDistance(refreshmentTable) > 5.0f)
            {
                // Move closer to the refreshment table
                bot->GetMotionMaster()->MovePoint(0, refreshmentTable->GetPositionX(), refreshmentTable->GetPositionY(), refreshmentTable->GetPositionZ());
                return false;
            }
            
            // Interact with the refreshment table
            refreshmentTable->Use(bot);
            botAI->SetNextCheckDelay(5000); // 5 seconds delay
            return true;
        }
    }
    
    return false;
}

bool HasRitualComponent(Player* bot, uint32 spellId)
{
    if (spellId == 29893 || spellId == 58887)
    {
        if (bot->GetItemCount(6265) == 0)
        {
            bot->AddItem(6265, 1);
            return true;
        }
        return true;
    }
    
    if (spellId == 43987)
    {
        if (bot->GetItemCount(17020) < 2)
        {
            bot->AddItem(17020, 2);
        }
        return true;
    }
    
    return false;
}

// NeedsFollowRestorationTrigger - Detects when a bot needs to restore follow behavior
bool NeedsFollowRestorationTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    
    // Only active in dungeons/raids/bgs
    if (!bot->GetMap()->IsDungeon() && !bot->GetMap()->IsRaid() && !bot->GetMap()->IsBattleground())
        return false;
    
    // Only active if bot has follow strategy
    if (!botAI->HasStrategy("follow", BOT_STATE_NON_COMBAT))
        return false;
    
    // Check follow restoration cooldown (30 seconds)
    uint32 currentTime = time(nullptr);
    auto it = lastFollowRestoration.find(bot->GetGUID().GetCounter());
    if (it != lastFollowRestoration.end())
    {
        uint32 timeSinceLastRestoration = currentTime - it->second;
        if (timeSinceLastRestoration < 30) // 30 segundos cooldown
        {
            return false;
        }
    }
    
    // Check if bot is stationary and not channeling
    if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() != IDLE_MOTION_TYPE || bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
        return false;
    
    // Check if bot has a master to follow
    Unit* master = botAI->GetGroupMaster();
    if (!master || master == bot)
        return false;
    
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
    bool hasHealthstone = bot->HasItemCount(5512, 1) || bot->HasItemCount(5511, 1) || bot->HasItemCount(9421, 1) || 
                         bot->HasItemCount(19004, 1) || bot->HasItemCount(19005, 1) || bot->HasItemCount(36892, 1);
    
    bool hasRefreshment = bot->HasItemCount(43523, 1); // Conjured Mana Strudel
    
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
    Player* bot = botAI->GetBot();
    
    // Only useful in dungeons/raids/bgs
    if (!bot->GetMap()->IsDungeon() && !bot->GetMap()->IsRaid() && !bot->GetMap()->IsBattleground())
        return false;
    
    // Only useful if bot has follow strategy
    if (!botAI->HasStrategy("follow", BOT_STATE_NON_COMBAT))
        return false;
    
    // Check if bot has a master to follow
    Unit* master = botAI->GetGroupMaster();
    if (!master || master == bot)
        return false;
    
    // Check if bot is not currently following
    if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE)
        return false;
    
    return true;
}

bool RestoreFollowBehaviorAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    
    
    // Record restoration time to avoid loops
    uint32 currentTime = time(nullptr);
    uint64 botGuid = bot->GetGUID().GetCounter();
    lastFollowRestoration[botGuid] = currentTime;
    
    // Clear completed interaction flag to avoid reactivations
    hasCompletedRitualInteraction[botGuid] = false;
    
    // Try to find master to follow
    Unit* master = botAI->GetGroupMaster();
    if (master && master != bot)
    {
        
        // Use direct follow approach
        bot->GetMotionMaster()->MoveFollow(master, sPlayerbotAIConfig->followDistance, 0.0f);
        
        return true;
    }
    else
    {
        
        // Fallback to DoSpecificAction
        bool followResult = botAI->DoSpecificAction("follow", Event(), true);
        
        // Check if follow action exists
        Action* followAction = botAI->GetAiObjectContext()->GetAction("follow");
        
        return followResult;
    }
}