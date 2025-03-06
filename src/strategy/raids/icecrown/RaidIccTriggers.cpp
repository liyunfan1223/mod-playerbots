#include "RaidIccTriggers.h"
#include "RaidIccActions.h"
#include "strategy/values/NearestNpcsValue.h"
#include "PlayerbotAIConfig.h"
#include "ObjectAccessor.h"
#include "GenericTriggers.h"
#include "DungeonStrategyUtils.h"
#include "EventMap.h"
#include "Playerbots.h"
#include "ScriptedCreature.h"
#include "Trigger.h"

//Lord Marrogwar
bool IccLmTankPositionTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "lord marrowgar");
    if (!boss) { return false; }

    return botAI->IsTank(bot);
}

bool IccSpikeNearTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "lord marrowgar");
    if (!boss) 
        return false;

    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit)
        {
            if (unit->GetEntry() == 36619 || unit->GetEntry() == 38711 || unit->GetEntry() == 38712 ) //spike ID
            {
                if (unit->GetDistance(bot) <= 20.0f)
                {
                    return botAI->IsDps(bot);
                }
    
                return botAI->IsRangedDps(bot);
            }
        }
    }

    return false;
}

//Lady Deathwhisper
bool IccDarkReckoningTrigger::IsActive()
{
    Unit* add = AI_VALUE2(Unit*, "find target", "deathspeaker high priest");
    if (add || bot->HasAura(69483)) 
        return true;

    return false;
}

bool IccRangedPositionLadyDeathwhisperTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "lady deathwhisper");
    if (!boss) 
        return false;

    return (botAI->IsRangedDps(bot) || botAI->IsHeal(bot));
}

bool IccAddsLadyDeathwhisperTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "lady deathwhisper");
    if (!boss) 
        return false;

     return true;
}

bool IccShadeLadyDeathwhisperTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "lady deathwhisper");
    if (!boss) { return false; }

    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit)
        {
            if (unit->GetEntry() == 38222 ) //vengeful sahde ID
            {
                return true;
            }
        }
    }

    return false;
}


//Gunship Battle
bool IccRottingFrostGiantTankPositionTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "rotting frost giant");
    if (!boss) { return false; }

    return true;
}

bool IccInCannonTrigger::IsActive()
{
    Unit* vehicleBase = bot->GetVehicleBase();
    Vehicle* vehicle = bot->GetVehicle();
    if (!vehicleBase || !vehicle)
        return false;

    uint32 entry = vehicleBase->GetEntry();
    return entry == 36838 || entry == 36839;
}

bool IccGunshipCannonNearTrigger::IsActive()
{
    if (bot->GetVehicle())
        return false;
    
    if (!botAI->IsDps(bot))
        return false;
    // Player* master = botAI->GetMaster();
    // if (!master)
    //     return false;
    
    // if (!master->GetVehicle())
    //     return false;

    return true;
}

bool IccGunshipTeleportAllyTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "kor'kron battle-mage");
    if (!boss) { return false; }

    return true;
}

bool IccGunshipTeleportHordeTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "skybreaker sorcerer");
    if (!boss) { return false; }

    return true;
}

//DBS
bool IccDbsTankPositionTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "deathbringer saurfang");
    if (!boss) { return false; }

    return true;
}

bool IccDbsMainTankRuneOfBloodTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "deathbringer saurfang");

    if (!boss) 
        return false;
    
    if (!botAI->IsAssistTankOfIndex(bot, 0)) 
        return false;

    Unit* mt = AI_VALUE(Unit*, "main tank");
    if (!mt) 
        return false; 
    
    Aura* aura = botAI->GetAura("rune of blood", mt);
    if (!aura) 
        return false;
    
    return true;
}

bool IccAddsDbsTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "deathbringer saurfang");
    if (!boss) { return false; }

     return true;
}

//DOGS
bool IccStinkyPreciousMainTankMortalWoundTrigger::IsActive()
{
    bool bossPresent = false;
    if (AI_VALUE2(Unit*, "find target", "stinky") || AI_VALUE2(Unit*, "find target", "precious"))
        bossPresent = true;

    if (!bossPresent)
        return false;

    if (!botAI->IsAssistTankOfIndex(bot, 0))
    {
        return false;
    }
    Unit* mt = AI_VALUE(Unit*, "main tank");
    if (!mt)
    {
        return false;
    }
    Aura* aura = botAI->GetAura("mortal wound", mt, false, true);
    if (!aura || aura->GetStackAmount() < 8)
    {
        return false;
    }
    return true;
}

//FESTERGUT
bool IccFestergutTankPositionTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "festergut");
    if (!boss || !(botAI->IsTank(bot) || botAI->IsRanged(bot) || botAI->IsHeal(bot)))  
        return false;

    return true;
}

bool IccFestergutMainTankGastricBloatTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "festergut");
    if (!boss) 
    {
        return false;
    }
    if (!botAI->IsAssistTankOfIndex(bot, 0))
    {
        return false;
    }
    Unit* mt = AI_VALUE(Unit*, "main tank");
    if (!mt)
    {
        return false;
    }
    Aura* aura = botAI->GetAura("Gastric Bloat", mt, false, true);
    if (!aura || aura->GetStackAmount() < 6)
    {
        return false;
    }
    return true;
}

bool IccFestergutSporeTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "festergut");
    if (!boss || botAI->IsTank(bot)) 
        return false;

    // Check for spore aura (ID: 69279) on any bot in the group
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member)
            continue;

        if (member->HasAura(69279))  // Spore aura ID
            return true;
    }

    return false;
}

//ROTFACE
bool IccRotfaceTankPositionTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "rotface");
    if (!boss || !(botAI->IsTank(bot) || botAI->IsMainTank(bot) || botAI->IsAssistTank(bot))) 
        return false;

    return true;
}

bool IccRotfaceGroupPositionTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "rotface");
    if (!boss) { return false; }

    return true;
}

bool IccRotfaceMoveAwayFromExplosionTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "big ooze");
    if (!boss) { return false; }

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(69839);
}

//PP

bool IccPutricideGrowingOozePuddleTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "professor putricide");
    bool botHasAura = botAI->HasAura("Gaseous Bloat", bot) || botAI->HasAura("Volatile Ooze Adhesive", bot);
    
    if (!boss || botHasAura) 
        return false;

    // Check for nearby growing ooze puddles (37690) and slime puddles (70341)
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit)
            continue;

        uint32 entry = unit->GetEntry();
        if (entry == 37690 || entry == 70341)  // Growing Ooze Puddle or Slime Puddle
        {
                return true;
        }
    }

    return false;
}

bool IccPutricideVolatileOozeTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "volatile ooze");
    if (!boss)
        return false;

    if (botAI->HasAura("Gaseous Bloat", bot))
        return false;

    return true;
}

bool IccPutricideGasCloudTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "gas cloud");
    if (!boss)
        return false;

    return true;
}

bool IccPutricideMainTankMutatedPlagueTrigger::IsActive()
{
    bool bossPresent = false;
    if (AI_VALUE2(Unit*, "find target", "professor putricide"))
        bossPresent = true;

    if (!bossPresent)
        return false;

    if (!botAI->IsAssistTankOfIndex(bot, 0))
    {
        return false;
    }
    Unit* mt = AI_VALUE(Unit*, "main tank");
    if (!mt)
    {
        return false;
    }
    Aura* aura = botAI->GetAura("Mutated Plague", mt, false, true);
    if (!aura || aura->GetStackAmount() < 4)
    {
        return false;
    }
    return true;
}

bool IccPutricideMalleableGooTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "professor putricide");
    if (!boss) 
        return false;

    Unit* boss1 = AI_VALUE2(Unit*, "find target", "volatile ooze");
    if (boss1) 
        return false;

    Unit* boss2 = AI_VALUE2(Unit*, "find target", "gas cloud");
    if (boss2) 
        return false;

    return true;
}

//BPC
bool IccBpcKelesethTankTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "prince keleseth");
    if (!boss) 
        return false;

    if (!botAI->IsAssistTank(bot))
        return false;

    Aura* aura = botAI->GetAura("Shadow Prison", bot, false, true);
    if (aura) 
        if (aura->GetStackAmount() > 18)
            return false;

    // First priority is to check for nucleuses that need to be picked up
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && unit->IsAlive() && unit->GetEntry() == 38369) // Dark Nucleus entry
        {
            if (!unit->GetVictim() || unit->GetVictim() != bot)
                return false; // Don't tank Keleseth if there's a nucleus to grab
        }
    }

    return true;
}

bool IccBpcNucleusTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "prince keleseth");
    if (!boss) 
        return false;

    if (!botAI->IsAssistTank(bot))
        return false;

    Aura* aura = botAI->GetAura("Shadow Prison", bot, false, true);
    if (aura) 
        if (aura->GetStackAmount() > 18)
            return false;

    // Actively look for any nucleus that isn't targeting us
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && unit->IsAlive() && unit->GetEntry() == 38369) // Dark Nucleus entry
        {
            if (!unit->GetVictim() || unit->GetVictim() != bot)
                return true; // Found a nucleus that needs to be picked up
        }
    }

    return false;
}

bool IccBpcMainTankTrigger::IsActive()
{
    Unit* valanar = AI_VALUE2(Unit*, "find target", "prince valanar");
    Unit* taldaram = AI_VALUE2(Unit*, "find target", "prince taldaram");
    Unit* keleseth = AI_VALUE2(Unit*, "find target", "prince keleseth");

    return valanar != nullptr || taldaram != nullptr || keleseth != nullptr;
}

bool IccBpcEmpoweredVortexTrigger::IsActive()
{
    // Tanks should ignore this mechanic
    if (botAI->IsMainTank(bot) || botAI->IsAssistTank(bot))
        return false;

    Unit* valanar = AI_VALUE2(Unit*, "find target", "prince valanar");
    if (!valanar)
        return false;

    Aura* aura = botAI->GetAura("Shadow Prison", bot, false, true);
    if (aura)
        if (aura->GetStackAmount() > 12)
            return false;

    Aura* auraValanar = botAI->GetAura("Invocation of Blood", valanar);
    if (!auraValanar)
        return false;

    // For ranged, spread whenever Valanar is empowered
    //if (botAI->IsRanged(bot) && auraValanar)
        //return true;

    // For melee, only spread during vortex cast
    if (auraValanar && valanar->HasUnitState(UNIT_STATE_CASTING))
        return true;

    return false;
}

bool IccBpcKineticBombTrigger::IsActive()
{
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    Aura* aura = botAI->GetAura("Shadow Prison", bot, false, true);
    if (aura)
        if (aura->GetStackAmount() > 12)
            return false;

    // Check for hunters first
    bool hasHunter = false;
    Group* group = bot->GetGroup();
    if (group)
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (member && member->getClass() == CLASS_HUNTER && member->IsAlive())
            {
                hasHunter = true;
                break;
            }
        }
    }

    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetName() == "Kinetic Bomb" && 
            ((unit->GetPositionZ() - bot->GetPositionZ()) < 25.0f))
        {
            if (hasHunter)
            {
                return bot->getClass() == CLASS_HUNTER; // Only hunters can handle bombs
            }
            else if (botAI->IsRangedDps(bot))
            {
                return true; // If no hunters, any ranged DPS can handle bombs
            }
        }
    }
    return false;
}

 //BQL
bool IccBqlTankPositionTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "blood-queen lana'thel");
    if (!boss) 
        return false;

    return true;
}

bool IccBqlPactOfDarkfallenTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "blood-queen lana'thel");

    Aura* aura = botAI->GetAura("Pact of the Darkfallen", bot);
    if (!boss || !aura) 
        return false;

    return true;
}

bool IccBqlVampiricBiteTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "blood-queen lana'thel");
    if (!boss) 
        return false;

    Aura* aura = botAI->GetAura("Frenzied Bloodthirst", bot);
    // Only trigger when bot has Frenzied Bloodthirst
    if (!aura)
        return false;

    return true;
}

//VDW
bool IccValkyreSpearTrigger::IsActive()
{
    // Check if there's a spear nearby
    if (Creature* spear = bot->FindNearestCreature(38248, 100.0f))
        return true;

    return false;
}

bool IccSisterSvalnaTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sister svalna");
    if (!boss) 
    {
        return false;
    }   
    return true;
}

bool IccValithriaPortalTrigger::IsActive()
{

    Unit* boss = bot->FindNearestCreature(36789, 100.0f);
    if (!boss) 
        return false;

    //for gruop position for non healers
    if(!botAI->IsHeal(bot) && (bot->GetDistance(ICC_VDW_GROUP_POSITION) > 35.0f))
        return true;
    
    // Only healers should use portals
    if (!botAI->IsHeal(bot) || bot->HasAura(70766))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Count healers and collect their GUIDs
    std::vector<ObjectGuid> healerGuids;
    int healerCount = 0;

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive())
            continue;

        if (botAI->IsHeal(member))
        {
            healerCount++;
            healerGuids.push_back(member->GetGUID());
        }
    }

    // Sort GUIDs to ensure consistent ordering
    std::sort(healerGuids.begin(), healerGuids.end());

    // Find position of current bot's GUID in the sorted list
    auto botGuidPos = std::find(healerGuids.begin(), healerGuids.end(), bot->GetGUID());
    if (botGuidPos == healerGuids.end())
        return false;

    int healerIndex = std::distance(healerGuids.begin(), botGuidPos);

    // Determine if this healer should focus on raid
    bool shouldHealRaid = false;
    
    if (healerCount > 3)
    {
        // If more than 3 healers, 2 should heal raid
        if (bot->getClass() == CLASS_DRUID)
            shouldHealRaid = true;  // Druids prioritize raid healing
        else
            shouldHealRaid = (healerIndex >= (healerCount - 2));  // Last 2 healers (by GUID) heal raid if no druid
    }
    else
    {
        // If 3 or fewer healers, 1 should heal raid
        if (bot->getClass() == CLASS_DRUID)
            shouldHealRaid = true;  // Druids prioritize raid healing
        else
            shouldHealRaid = (healerIndex == (healerCount - 1));  // Last healer (by GUID) heals raid if no druid
    }

    // Raid healers should not use portals
    if (shouldHealRaid)
        return false;

    // Find the nearest portal creature
    Creature* portal = bot->FindNearestCreature(37945, 100.0f);  // Only check within 10 yards
    Creature* portal2 = bot->FindNearestCreature(38430, 100.0f); // Only check within 10 yards

    return portal || portal2;
}

bool IccValithriaHealTrigger::IsActive()
{
    Unit* boss = bot->FindNearestCreature(36789, 100.0f);
    if (!boss) 
        return false;
    
    // Only healers should use healing
    if (!botAI->IsHeal(bot) || bot->HasAura(70766))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Count healers and collect their GUIDs
    std::vector<ObjectGuid> healerGuids;
    int healerCount = 0;

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive())
            continue;

        if (botAI->IsHeal(member))
        {
            healerCount++;
            healerGuids.push_back(member->GetGUID());
        }
    }

    // Sort GUIDs to ensure consistent ordering
    std::sort(healerGuids.begin(), healerGuids.end());

    // Find position of current bot's GUID in the sorted list
    auto botGuidPos = std::find(healerGuids.begin(), healerGuids.end(), bot->GetGUID());
    if (botGuidPos == healerGuids.end())
        return false;

    int healerIndex = std::distance(healerGuids.begin(), botGuidPos);

    // Determine if this healer should focus on raid
    bool shouldHealRaid = false;
    
    if (healerCount > 3)
    {
        // If more than 3 healers, 2 should heal raid
        if (bot->getClass() == CLASS_DRUID)
            shouldHealRaid = true;  // Druids prioritize raid healing
        else
            shouldHealRaid = (healerIndex >= (healerCount - 2));  // Last 2 healers (by GUID) heal raid if no druid
    }
    else
    {
        // If 3 or fewer healers, 1 should heal raid
        if (bot->getClass() == CLASS_DRUID)
            shouldHealRaid = true;  // Druids prioritize raid healing
        else
            shouldHealRaid = (healerIndex == (healerCount - 1));  // Last healer (by GUID) heals raid if no druid
    }

    // If assigned to raid healing, return false to not heal Valithria
    if (shouldHealRaid)
        return false;

    // For Valithria healers, check portal logic
    // If no portal is found within 100 yards, we should heal
    if (!bot->FindNearestCreature(37945, 100.0f) && !bot->FindNearestCreature(38430, 100.0f))
        return true;

    // If portal is nearby (10 yards), don't heal
    if (bot->FindNearestCreature(37945, 20.0f) || bot->FindNearestCreature(38430, 10.0f))
        return false;

    // If portal is far but within 100 yards, heal while moving to it
    return true;
}

bool IccValithriaDreamCloudTrigger::IsActive()
{
    // Only active if we're in dream state
    if (!bot->HasAura(70766))
        return false;

    // Find nearest cloud of either type
    Creature* dreamCloud = bot->FindNearestCreature(37985, 100.0f);
    Creature* nightmareCloud = bot->FindNearestCreature(38421, 100.0f);

    return (dreamCloud || nightmareCloud);
}

//SINDRAGOSA
bool IccSindragosaTankPositionTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss || bot->HasAura(70126) /*|| bot->HasAura(69762)*/ || boss->HasUnitMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY))
        return false;

    return true;
}

bool IccSindragosaFrostBeaconTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    if (boss->HasUnitMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY) && !boss->HealthBelowPct(35))
        return true;

    Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
    for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); ++itr)
    {
        Player* member = ObjectAccessor::FindPlayer(itr->guid);
        if (!member || !member->IsAlive())
            continue;

        if (member->HasAura(70126))  // If any player has Frost Beacon, keep trigger active
            return true;
    }
    return false;
}

bool IccSindragosaBlisteringColdTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss)
        return false;

    if (botAI->IsMainTank(bot))
        return false;

    // Don't move if any bot in group has ice tomb
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    float dist = bot->GetDistance(boss);
    
    if (dist >= 30.0f)
        return false;

    bool isCasting = boss->HasUnitState(UNIT_STATE_CASTING);
    bool isBlisteringCold = boss->FindCurrentSpellBySpellId(70123) || boss->FindCurrentSpellBySpellId(71047) ||
                           boss->FindCurrentSpellBySpellId(71048) || boss->FindCurrentSpellBySpellId(71049);
    
    return isCasting && isBlisteringCold;
}

bool IccSindragosaUnchainedMagicTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss || (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(70123)))
        return false;

    return bot->HasAura(69762);
}

bool IccSindragosaChilledToTheBoneTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss || (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(70123)))
        return false;

    return bot->HasAura(70106);
}

bool IccSindragosaMysticBuffetTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss)
        return false;
    
    if (botAI->IsTank(bot) && boss->GetVictim() == bot)
        return false;

    Aura* aura = botAI->GetAura("mystic buffet", bot, false, true);
    if (!aura)
        return false;

    if (bot->HasAura(70126))  // Frost Beacon
        return false;

    if (aura->GetStackAmount() >= 2 && botAI->IsAssistTank(bot))
        return true;

    if (aura->GetStackAmount() >= 3)
        return true;

    return false;
}

bool IccSindragosaMainTankMysticBuffetTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss)
        return false;

    if (botAI->IsTank(bot) && boss->GetVictim() == bot)
        return false;

    // Only for assist tank
    if (!botAI->IsAssistTankOfIndex(bot, 0))
        return false;

    // Don't swap if we have frost beacon
    if (bot->HasAura(70126))   // Frost Beacon
        return false;

    Unit* mt = AI_VALUE(Unit*, "main tank");
    if (!mt)
        return false;

    // Check main tank stacks
    Aura* mtAura = botAI->GetAura("mystic buffet", mt, false, true);
    if (!mtAura || mtAura->GetStackAmount() < 9)
        return false;

    // Check our own stacks - don't taunt if we have too many
    Aura* selfAura = botAI->GetAura("mystic buffet", bot, false, true);
    if (selfAura && selfAura->GetStackAmount() > 6)
        return false;

    // Only taunt if we're in position
    float distToTankPos = bot->GetExactDist2d(ICC_SINDRAGOSA_TANK_POSITION);
    if (distToTankPos > 3.0f)
        return false;

    return true;
}

bool IccSindragosaTankSwapPositionTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss)
        return false;

    // Only for assist tank
    if (!botAI->IsAssistTankOfIndex(bot, 0))
        return false;

    // Don't move to position if we have frost beacon
    if (bot->HasAura(70126))   // Frost Beacon
        return false;

    // Check our own stacks - don't try to tank if we have too many
    Aura* selfAura = botAI->GetAura("mystic buffet", bot, false, true);
    if (selfAura && selfAura->GetStackAmount() > 6)
        return false;

    // Check if main tank has high stacks
    Unit* mt = AI_VALUE(Unit*, "main tank");
    if (!mt)    
        return false;

    Aura* mtAura = botAI->GetAura("mystic buffet", mt, false, true);
    if (!mtAura)
        return false;

    uint32 mtStacks = mtAura->GetStackAmount();
    if (mtStacks < 9)  // Only start moving when MT has 5+ stacks
        return false;

    // Check if we're already in position
    float distToTankPos = bot->GetExactDist2d(ICC_SINDRAGOSA_TANK_POSITION);
    if (distToTankPos <= 3.0f)
        return false;

    return true;  // Move to position if all conditions are met
}

bool IccSindragosaFrostBombTrigger::IsActive()
{
    if (!bot->IsAlive() || bot->HasAura(70157))  // Skip if dead or in Ice Tomb
        return false;

    // Simply check if frost bomb marker exists
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->HasAura(70022))  // Frost bomb visual
            return true;
    }

    return false;
}

//LK

bool IccLichKingShadowTrapTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "the lich king");
    if (!boss)
        return false;

    return true;
}

bool IccLichKingNecroticPlagueTrigger::IsActive()
{
    if (!bot || !bot->IsAlive())
        return false;

    // Check for plague by name instead of ID
    bool hasPlague = botAI->HasAura("Necrotic Plague", bot);

    return hasPlague;
}

bool IccLichKingWinterTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "the lich king");
    if (!boss) 
        return false;
    
    // Check for either Remorseless Winter
    bool hasWinterAura = boss->HasAura(72259) || boss->HasAura(74273) || boss->HasAura(74274) || boss->HasAura(74275);
    bool hasWinter2Aura = boss->HasAura(68981) || boss->HasAura(74270) || boss->HasAura(74271) || boss->HasAura(74272);
    bool isCasting = boss->HasUnitState(UNIT_STATE_CASTING);
    bool isWinter = boss->FindCurrentSpellBySpellId(77259) || boss->FindCurrentSpellBySpellId(74273) || boss->FindCurrentSpellBySpellId(68981) || boss->FindCurrentSpellBySpellId(74270) ||
                    boss->FindCurrentSpellBySpellId(74274) || boss->FindCurrentSpellBySpellId(74275) || boss->FindCurrentSpellBySpellId(74271) || boss->FindCurrentSpellBySpellId(74272);

    if (hasWinterAura || hasWinter2Aura)
        return true;   

    if (isCasting && isWinter)
        return true;

    return false;
}

bool IccLichKingAddsTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "the lich king");
    Unit* spiritWarden = AI_VALUE2(Unit*, "find target", "spirit warden");

    if (!boss && !spiritWarden)  
        return false;

    return true;
}