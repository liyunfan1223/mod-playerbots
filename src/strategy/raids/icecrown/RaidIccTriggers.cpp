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
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Vehicle.h"

//Lord Marrogwar
bool IccLmTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "lord marrowgar");
    if (!boss)
        return false;

    if (bot->HasAura(SPELL_EXPERIENCED))
        bot->RemoveAura(SPELL_EXPERIENCED);

    return true;
}

//Lady Deathwhisper
bool IccDarkReckoningTrigger::IsActive()
{
    if (bot->HasAura(SPELL_DARK_RECKONING)) 
        return true;

    return false;
}

bool IccLadyDeathwhisperTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "lady deathwhisper");
    if (!boss) 
        return false;

    if (bot->HasAura(SPELL_EXPERIENCED))
        bot->RemoveAura(SPELL_EXPERIENCED);

    return true;
}

//Gunship Battle
bool IccRottingFrostGiantTankPositionTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "rotting frost giant");
    if (!boss)
        return false;

    if (bot->HasAura(SPELL_EXPERIENCED))
        bot->RemoveAura(SPELL_EXPERIENCED);

    return true;
}

bool IccInCannonTrigger::IsActive()
{
    Unit* vehicleBase = bot->GetVehicleBase();
    Vehicle* vehicle = bot->GetVehicle();
    if (!vehicleBase || !vehicle)
        return false;

    uint32 entry = vehicleBase->GetEntry();
    return entry == NPC_CANNONA || entry == NPC_CANNONH;
}

bool IccGunshipCannonNearTrigger::IsActive()
{
    if (bot->GetVehicle())
        return false;

    Unit* mount1 = bot->FindNearestCreature(NPC_CANNONA, 100.0f);

    Unit* mount2 = bot->FindNearestCreature(NPC_CANNONH, 100.0f);

    if (!mount1 && !mount2)
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
    Unit* boss = bot->FindNearestCreature(NPC_HIGH_OVERLORD_SAURFANG, 100.0f);
    if (!boss)
        return false;

    if (!boss->IsHostileTo(bot))
        return false;

    return true;
}

bool IccGunshipTeleportHordeTrigger::IsActive()
{
    Unit* boss = bot->FindNearestCreature(NPC_MURADIN_BRONZEBEARD, 100.0f);
    if (!boss)
        return false;

    if (!boss->IsHostileTo(bot))
        return false;

    return true;
}

//DBS
bool IccDbsTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "deathbringer saurfang");
    if (!boss)
        return false;

    if (bot->HasAura(SPELL_EXPERIENCED))
        bot->RemoveAura(SPELL_EXPERIENCED);

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

//DOGS
bool IccStinkyPreciousMainTankMortalWoundTrigger::IsActive()
{
    bool bossPresent = false;
    if (AI_VALUE2(Unit*, "find target", "stinky") || AI_VALUE2(Unit*, "find target", "precious"))
        bossPresent = true;

    if (!bossPresent)
        return false;

    if (!botAI->IsAssistTankOfIndex(bot, 0))
        return false;

    Unit* mt = AI_VALUE(Unit*, "main tank");
    if (!mt)
        return false;

    Aura* aura = botAI->GetAura("mortal wound", mt, false, true);
    if (!aura || aura->GetStackAmount() < 8)
        return false;

    return true;
}

//FESTERGUT
bool IccFestergutGroupPositionTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "festergut");
    if (!boss)  
        return false;

    if (bot->HasAura(SPELL_EXPERIENCED))
        bot->RemoveAura(SPELL_EXPERIENCED);

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

        if (member->HasAura(SPELL_GAS_SPORE))
            return true;
    }

    return false;
}

//ROTFACE
bool IccRotfaceTankPositionTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "rotface");
    if (!boss || !(botAI->IsTank(bot) || botAI->IsMelee(bot))) 
        return false;

    if (bot->HasAura(SPELL_EXPERIENCED))
        bot->RemoveAura(SPELL_EXPERIENCED);

    return true;
}

bool IccRotfaceGroupPositionTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "rotface");
    if (!boss)
        return false;

    return true;
}

bool IccRotfaceMoveAwayFromExplosionTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "big ooze");
    if (!boss)
        return false;

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_UNSTABLE_OOZE_EXPLOSION);
}

//PP

bool IccPutricideGrowingOozePuddleTrigger::IsActive()
{
    // Early return if boss doesn't exist
    Unit* boss = AI_VALUE2(Unit*, "find target", "professor putricide");
    if (!boss)
        return false;

    Difficulty diff = bot->GetRaidDifficulty();

    if (sPlayerbotAIConfig->EnableICCBuffs && diff && (diff == RAID_DIFFICULTY_10MAN_HEROIC || diff == RAID_DIFFICULTY_25MAN_HEROIC))
    {
        //-------CHEAT-------
        if (!bot->HasAura(SPELL_EXPERIENCED))
            bot->AddAura(SPELL_EXPERIENCED, bot);

        if (!bot->HasAura(SPELL_AGEIS_OF_DALARAN))
            bot->AddAura(SPELL_AGEIS_OF_DALARAN, bot);

        if (!bot->HasAura(SPELL_NO_THREAT) && botAI->HasAggro(boss) && !botAI->IsTank(bot))
            bot->AddAura(SPELL_NO_THREAT, bot);

        if (botAI->IsMainTank(bot) && !bot->HasAura(SPELL_SPITEFULL_FURY) && boss->GetVictim() != bot)
            bot->AddAura(SPELL_SPITEFULL_FURY, bot);
        //-------CHEAT-------
    }

    const GuidVector& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (const auto& npc : npcs)
    {
        if (Unit* unit = botAI->GetUnit(npc))
        {
            const uint32 entry = unit->GetEntry();
            if (entry == NPC_GROWING_OOZE_PUDDLE)
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

    Unit* boss1 = AI_VALUE2(Unit*, "find target", "volatile ooze");

    bool hasGaseousBloat = botAI->HasAura("Gaseous Bloat", bot);

    if (hasGaseousBloat && boss1)
        return true;

    if (boss1)
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

    if (botAI->IsTank(bot))
        return true;

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

    if (bot->HasAura(SPELL_EXPERIENCED))
        bot->RemoveAura(SPELL_EXPERIENCED);

    if (!botAI->IsAssistTank(bot))
        return false;

    Aura* aura = botAI->GetAura("Shadow Prison", bot, false, true);
    if (aura) 
        if (aura->GetStackAmount() > 18)
            return false;

    return true;
}

bool IccBpcMainTankTrigger::IsActive()
{
    if (!botAI->IsTank(bot))
        return false;

    Unit* valanar = AI_VALUE2(Unit*, "find target", "prince valanar");
    Unit* taldaram = AI_VALUE2(Unit*, "find target", "prince taldaram");
    Unit* keleseth = AI_VALUE2(Unit*, "find target", "prince keleseth");

    if (!(valanar || taldaram || keleseth))
        return false;

    if (bot->HasAura(SPELL_EXPERIENCED))
        bot->RemoveAura(SPELL_EXPERIENCED);

    return true;
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

    return true;
}

bool IccBpcKineticBombTrigger::IsActive()
{
    Unit* valanar = AI_VALUE2(Unit*, "find target", "prince valanar");
    Unit* taldaram = AI_VALUE2(Unit*, "find target", "prince taldaram");
    Unit* keleseth = AI_VALUE2(Unit*, "find target", "prince keleseth");

    if (!(valanar || taldaram || keleseth))
        return false;

    if (!botAI->IsRanged(bot) || botAI->IsHeal(bot))
        return false;

    // Early exit condition - if Shadow Prison has too many stacks
    if (Aura* aura = botAI->GetAura("Shadow Prison", bot, false, true))
    {
        if (aura->GetStackAmount() > 12)
            return false;
    }

    static const std::array<uint32, 4> bombEntries = {NPC_KINETIC_BOMB1, NPC_KINETIC_BOMB2, NPC_KINETIC_BOMB3,
                                                      NPC_KINETIC_BOMB4};
    const GuidVector bombs = AI_VALUE(GuidVector, "possible targets no los");

    bool bombFound = false;

    for (const auto entry : bombEntries)
    {
        for (const auto& guid : bombs)
        {
            if (Unit* unit = botAI->GetUnit(guid))
            {
                if (unit->GetEntry() == entry)
                {
                    // Check if bomb is within valid Z-axis range
                    if (unit->GetPositionZ() - bot->GetPositionZ() < 25.0f)
                    {
                        bombFound = true;
                        break;
                    }
                }
            }
        }
        if (bombFound)
            break;
    }

    return botAI->IsRangedDps(bot) && bombFound;
}

bool IccBpcBallOfFlameTrigger::IsActive()
{
    Unit* valanar = AI_VALUE2(Unit*, "find target", "prince valanar");
    Unit* taldaram = AI_VALUE2(Unit*, "find target", "prince taldaram");
    Unit* keleseth = AI_VALUE2(Unit*, "find target", "prince keleseth");

    if (!(valanar || taldaram || keleseth))
        return false;

    Aura* auraTaldaram = botAI->GetAura("Invocation of Blood", taldaram);
    if (!auraTaldaram)
        return false;

     return true;
}

//BQL
bool IccBqlGroupPositionTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "blood-queen lana'thel");
    if (!boss) 
        return false;

    if (bot->HasAura(SPELL_EXPERIENCED))
        bot->RemoveAura(SPELL_EXPERIENCED);

    return true;
}

bool IccBqlPactOfDarkfallenTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "blood-queen lana'thel");
    if (!boss)
        return false;

    Aura* aura = botAI->GetAura("Pact of the Darkfallen", bot);
    if (!aura) 
        return false;

    return true;
}

bool IccBqlVampiricBiteTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "blood-queen lana'thel");
    if (!boss) 
        return false;

    Aura* aura = botAI->GetAura("Frenzied Bloodthirst", bot);
    if (!aura)
        return false;

    return true;
}

// Sister Svalna
bool IccValkyreSpearTrigger::IsActive()
{
    // Check if there's a spear nearby
    if (Creature* spear = bot->FindNearestCreature(NPC_SPEAR, 100.0f))
        return true;

    return false;
}

bool IccSisterSvalnaTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sister svalna");
    if (!boss) 
        return false;

    if (bot->GetExactDist2d(boss) > 30.0f)
        return false;

    return true;
}

// VDW
bool IccValithriaGroupTrigger::IsActive()
{
    Unit* boss = bot->FindNearestCreature(NPC_VALITHRIA_DREAMWALKER, 100.0f);
    if (!boss)
        return false;

    if (bot->HasAura(SPELL_EXPERIENCED))
        bot->RemoveAura(SPELL_EXPERIENCED);

    return true;
}

bool IccValithriaPortalTrigger::IsActive()
{
    Unit* boss = bot->FindNearestCreature(NPC_VALITHRIA_DREAMWALKER, 100.0f);
    if (!boss)
        return false;

    Aura* aura = botAI->GetAura("Twisted Nightmares", bot, false, true);
    if (aura && aura->GetStackAmount() >= 25)
        return false;

    // Only healers should use portals
    if (!botAI->IsHeal(bot) || bot->HasAura(SPELL_DREAM_STATE))
        return false;

    Creature* worm = bot->FindNearestCreature(NPC_ROT_WORM, 100.0f);
    Creature* zombie = bot->FindNearestCreature(NPC_BLISTERING_ZOMBIE, 100.0f);

    if ((worm && worm->GetVictim() == bot) || (zombie && zombie->GetVictim() == bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Collect healer GUIDs and check for druids
    std::vector<ObjectGuid> healerGuids;
    std::vector<ObjectGuid> druidGuids;
    int healerCount = 0;

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive() || botAI->IsRealPlayer())
            continue;

        if (botAI->IsHeal(member) && !botAI->IsRealPlayer())
        {
            healerCount++;
            healerGuids.push_back(member->GetGUID());
            // Check if druid (class 11)
            if (member->getClass() == CLASS_DRUID)
                druidGuids.push_back(member->GetGUID());
        }
    }

    // Sort GUIDs to ensure consistent ordering
    std::sort(healerGuids.begin(), healerGuids.end());
    std::sort(druidGuids.begin(), druidGuids.end());

    // Find position of current bot's GUID in the sorted list
    auto botGuidPos = std::find(healerGuids.begin(), healerGuids.end(), bot->GetGUID());
    if (botGuidPos == healerGuids.end())
        return false;

    int healerIndex = std::distance(healerGuids.begin(), botGuidPos);

    // Find if this bot is a druid
    bool isDruid = (bot->getClass() == CLASS_DRUID);

    // Determine raid healer assignment
    bool shouldHealRaid = false;
    int druidCount = druidGuids.size();

    if (druidCount > 0)
    {
        // If we have druids, they should heal raid
        if (isDruid)
        {
            // If there are more druids than raid healers needed, extra druids can heal boss
            int raidHealersNeeded = healerCount > 3 ? 2 : 1;
            int druidIndex =
                std::distance(druidGuids.begin(), std::find(druidGuids.begin(), druidGuids.end(), bot->GetGUID()));
            if (druidIndex < raidHealersNeeded)
                shouldHealRaid = true;
            else
                shouldHealRaid = false;
        }
        else if (healerCount > 3 && druidCount == 1)
        {
            // If only 1 druid and need 2 raid healers, pick the last non-druid healer as well
            if (healerIndex == (healerCount - 1) && !isDruid)
                shouldHealRaid = true;
        }
    }
    else
    {
        // No druids, assign raid healers as before
        if (healerCount > 3)
            shouldHealRaid = (healerIndex >= (healerCount - 2));
        else
            shouldHealRaid = (healerIndex == (healerCount - 1));
    }

    // Raid healers should not use portals
    if (shouldHealRaid)
        return false;

    // Find the nearest portal creature
    Creature* portal1 = bot->FindNearestCreature(NPC_DREAM_PORTAL, 100.0f);
    if (!portal1)
        portal1 = bot->FindNearestCreature(NPC_DREAM_PORTAL_PRE_EFFECT, 100.0f);

    Creature* portal2 = bot->FindNearestCreature(NPC_NIGHTMARE_PORTAL, 100.0f);
    if (!portal2)
        portal2 = bot->FindNearestCreature(NPC_NIGHTMARE_PORTAL_PRE_EFFECT, 100.0f);

    return portal1 || portal2;
}

bool IccValithriaHealTrigger::IsActive()
{
    Unit* boss = bot->FindNearestCreature(NPC_VALITHRIA_DREAMWALKER, 100.0f);
    if (!boss)
        return false;

    // Only healers should use healing
    if (!botAI->IsHeal(bot) || bot->HasAura(SPELL_DREAM_STATE) || bot->HealthBelowPct(50))
        return false;

    Creature* worm = bot->FindNearestCreature(NPC_ROT_WORM, 100.0f);
    Creature* zombie = bot->FindNearestCreature(NPC_BLISTERING_ZOMBIE, 100.0f);

    if ((worm && worm->GetVictim() == bot) || (zombie && zombie->GetVictim() == bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Collect healer GUIDs and check for druids
    std::vector<ObjectGuid> healerGuids;
    std::vector<ObjectGuid> druidGuids;
    int healerCount = 0;

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive() || botAI->IsRealPlayer())
            continue;

        if (botAI->IsHeal(member) && !botAI->IsRealPlayer())
        {
            healerCount++;
            healerGuids.push_back(member->GetGUID());
            // Check if druid (class 11)
            if (member->getClass() == CLASS_DRUID)
                druidGuids.push_back(member->GetGUID());
        }
    }

    // Sort GUIDs to ensure consistent ordering
    std::sort(healerGuids.begin(), healerGuids.end());
    std::sort(druidGuids.begin(), druidGuids.end());

    // Find position of current bot's GUID in the sorted list
    auto botGuidPos = std::find(healerGuids.begin(), healerGuids.end(), bot->GetGUID());
    if (botGuidPos == healerGuids.end())
        return false;

    int healerIndex = std::distance(healerGuids.begin(), botGuidPos);

    // Find if this bot is a druid
    bool isDruid = (bot->getClass() == CLASS_DRUID);

    // Determine raid healer assignment
    bool shouldHealRaid = false;
    int druidCount = druidGuids.size();

    if (druidCount > 0)
    {
        // If we have druids, they should heal raid
        if (isDruid)
        {
            // If there are more druids than raid healers needed, extra druids can heal boss
            int raidHealersNeeded = healerCount > 3 ? 2 : 1;
            int druidIndex =
                std::distance(druidGuids.begin(), std::find(druidGuids.begin(), druidGuids.end(), bot->GetGUID()));
            if (druidIndex < raidHealersNeeded)
                shouldHealRaid = true;
            else
                shouldHealRaid = false;  // extra druids can heal boss
        }
        else if (healerCount > 3 && druidCount == 1)
        {
            // If only 1 druid and need 2 raid healers, pick the last non-druid healer as well
            if (healerIndex == (healerCount - 1) && !isDruid)
                shouldHealRaid = true;
        }
    }
    else
    {
        // No druids, assign raid healers as before
        if (healerCount > 3)
            shouldHealRaid = (healerIndex >= (healerCount - 2));
        else
            shouldHealRaid = (healerIndex == (healerCount - 1));
    }

    // If assigned to raid healing, return false to not heal Valithria
    if (shouldHealRaid)
        return false;

    if (bot->GetHealthPct() < 50.0f)
        return false;

    // For Valithria healers, check portal logic
    // If no portal is found within 100 yards, we should heal
    if (!bot->FindNearestCreature(NPC_DREAM_PORTAL, 100.0f) && !bot->FindNearestCreature(NPC_NIGHTMARE_PORTAL, 100.0f))
        return true;

    if (bot->FindNearestCreature(NPC_DREAM_PORTAL, 10.0f) || bot->FindNearestCreature(NPC_NIGHTMARE_PORTAL, 10.0f))
        return false;

    // If portal is far but within 100 yards, heal while moving to it
    return true;
}

bool IccValithriaDreamCloudTrigger::IsActive()
{
    // Only active if we're in dream state
    if (!bot->HasAura(SPELL_DREAM_STATE) || bot->HealthBelowPct(50))
        return false;

    // Find nearest cloud of either type
    Creature* dreamCloud = bot->FindNearestCreature(NPC_DREAM_CLOUD, 100.0f);
    Creature* nightmareCloud = bot->FindNearestCreature(NPC_NIGHTMARE_CLOUD, 100.0f);

    return (dreamCloud || nightmareCloud);
}

//SINDRAGOSA
bool IccSindragosaGroupPositionTrigger::IsActive()
{
    Unit* boss = bot->FindNearestCreature(NPC_SINDRAGOSA, 200.0f);  // sindra
    if (!boss)
        return false;

    Difficulty diff = bot->GetRaidDifficulty();

    if (sPlayerbotAIConfig->EnableICCBuffs && diff && (diff == RAID_DIFFICULTY_10MAN_HEROIC || diff == RAID_DIFFICULTY_25MAN_HEROIC))
    {
        //-------CHEAT-------
        if (!bot->HasAura(SPELL_EXPERIENCED))
            bot->AddAura(SPELL_EXPERIENCED, bot);

        if (!bot->HasAura(SPELL_AGEIS_OF_DALARAN))
            bot->AddAura(SPELL_AGEIS_OF_DALARAN, bot);

        if (!bot->HasAura(SPELL_NO_THREAT) && botAI->HasAggro(boss) && !botAI->IsTank(bot))
            bot->AddAura(SPELL_NO_THREAT, bot);

        if (botAI->IsMainTank(bot) && !bot->HasAura(SPELL_SPITEFULL_FURY) && boss->GetVictim() != bot)
            bot->AddAura(SPELL_SPITEFULL_FURY, bot);
        //-------CHEAT-------
    }

    if (!boss || bot->HasAura(SPELL_FROST_BEACON) /*|| bot->HasAura(69762)*/ || boss->GetExactDist2d(ICC_SINDRAGOSA_FLYING_POSITION.GetPositionX(), ICC_SINDRAGOSA_FLYING_POSITION.GetPositionY()) < 50.0f)
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

    if (bot->HasAura(SPELL_FROST_BEACON))
        return true;

    Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
    for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); ++itr)
    {
        Player* member = ObjectAccessor::FindPlayer(itr->guid);
        if (!member || !member->IsAlive())
            continue;

        if (member->HasAura(SPELL_FROST_BEACON))  // If any player has Frost Beacon, keep trigger active
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

    float dist = bot->GetExactDist2d(boss->GetPositionX(), boss->GetPositionY());
    
    if (dist >= 33.0f)
        return false;

    bool isCasting = false;
    if (boss && boss->HasUnitState(UNIT_STATE_CASTING))
        isCasting = true;

    bool isBlisteringCold = false;
    if (boss && (boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD1) ||
                 boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD2) ||
                 boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD3) ||
                 boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD4)))
        isBlisteringCold = true;
    
    return isCasting && isBlisteringCold;
}

bool IccSindragosaUnchainedMagicTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss)
        return false;

    Aura* aura = botAI->GetAura("unchained magic", bot, false, false);
    if (!aura)
        return false;

    bool isBlisteringCold = false;
    if (boss && (boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD1) ||
                 boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD2) ||
                 boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD3) ||
                 boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD4)))
        isBlisteringCold = true;

    if (boss && boss->HasUnitState(UNIT_STATE_CASTING) && isBlisteringCold)
        return false;

    return true;
}

bool IccSindragosaChilledToTheBoneTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss)
        return false;

    Aura* aura = botAI->GetAura("Chilled to the Bone", bot, false, false);
    if (!aura)
        return false;

    bool isBlisteringCold = false;
    if (boss && (boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD1) ||
                 boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD2) ||
                 boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD3) ||
                 boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD4)))
        isBlisteringCold = true;

    if (boss && boss->HasUnitState(UNIT_STATE_CASTING) && isBlisteringCold)
        return false;

    return true;
}

bool IccSindragosaMysticBuffetTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss)
        return false;
    
    if (boss->GetVictim() == bot)
        return false;

    Aura* aura = botAI->GetAura("mystic buffet", bot, false, true);
    if (!aura)
        return false;

    if (bot->HasAura(SPELL_FROST_BEACON))
        return false;

    if (aura->GetStackAmount() >= 1)
        return true;

    return false;
}

bool IccSindragosaMainTankMysticBuffetTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss)
        return false;

    Aura* aura = botAI->GetAura("mystic buffet", bot, false, false);
    if (botAI->IsTank(bot) && aura) //main tank will delete mystic buffet until I find a better way to swap tanks, atm it is not great since while swapping they will wipe group 7/10 times.
        bot->RemoveAura(aura->GetId());

    if (botAI->IsTank(bot) && boss->GetVictim() == bot)
        return false;

    // Only for assist tank
    if (!botAI->IsAssistTankOfIndex(bot, 0))
        return false;

    // Don't swap if we have frost beacon
    if (bot->HasAura(SPELL_FROST_BEACON))   // Frost Beacon
        return false;

    Unit* mt = AI_VALUE(Unit*, "main tank");
    if (!mt)
        return false;

    // Check main tank stacks
    Aura* mtAura = botAI->GetAura("mystic buffet", mt, false, true);
    if (!mtAura || mtAura->GetStackAmount() < 6)
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

    if (boss && boss->GetVictim() == bot)
        return false;

    // Only for assist tank
    if (!botAI->IsAssistTankOfIndex(bot, 0))
        return false;

    // Don't move to position if we have frost beacon
    if (bot->HasAura(SPELL_FROST_BEACON))
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
    if (mtStacks < 6)  // Only start moving when MT has 5+ stacks
        return false;

    // Check if we're already in position
    float distToTankPos = bot->GetExactDist2d(ICC_SINDRAGOSA_TANK_POSITION);
    if (distToTankPos <= 3.0f)
        return false;

    return true;  // Move to position if all conditions are met
}

bool IccSindragosaFrostBombTrigger::IsActive()
{
    Unit* boss = bot->FindNearestCreature(NPC_SINDRAGOSA, 200.0f);
    if (!boss)
        return false;

    if (!bot->IsAlive() || bot->HasAura(SPELL_ICE_TOMB))  // Skip if dead or in Ice Tomb
        return false;

    if (boss->GetExactDist2d(ICC_SINDRAGOSA_FLYING_POSITION.GetPositionX(), ICC_SINDRAGOSA_FLYING_POSITION.GetPositionY()) < 50.0f && !boss->HealthBelowPct(25) && !boss->HealthAbovePct(99))
        return true;

    return false;
}

//LK

bool IccLichKingShadowTrapTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "the lich king");
    if (!boss)
        return false;

    bool hasPlague = botAI->HasAura("Necrotic Plague", bot);
    if (hasPlague)
        return false;

    if (!botAI->IsMainTank(bot))
        return false;

    if (boss->HealthBelowPct(65))
        return false;

    // search for all nearby traps
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    std::vector<Unit*> nearbyTraps;
    bool needToMove = false;

    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_SHADOW_TRAP && bot->GetExactDist2d(unit) < 11.0f)  // shadow trap
            return true;
    }

    return false;
}

bool IccLichKingNecroticPlagueTrigger::IsActive()
{
    bool hasPlague = botAI->HasAura("Necrotic Plague", bot);

    return hasPlague;
}

bool IccLichKingWinterTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "the lich king");
    if (!boss) 
        return false;
    
    // Check for either Remorseless Winter
    bool hasWinterAura = false;
    if (boss && (boss->HasAura(SPELL_REMORSELESS_WINTER1) || boss->HasAura(SPELL_REMORSELESS_WINTER2) ||
        boss->HasAura(SPELL_REMORSELESS_WINTER3) || boss->HasAura(SPELL_REMORSELESS_WINTER4)))
        hasWinterAura = true;

    bool hasWinter2Aura = false;
    if (boss && (boss->HasAura(SPELL_REMORSELESS_WINTER5) || boss->HasAura(SPELL_REMORSELESS_WINTER6) ||
        boss->HasAura(SPELL_REMORSELESS_WINTER7) || boss->HasAura(SPELL_REMORSELESS_WINTER8)))
        hasWinter2Aura = true;

    bool isCasting = false;
    if (boss && boss->HasUnitState(UNIT_STATE_CASTING))
        isCasting = true;

    bool isWinter = false;
    if (boss && boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER1) ||
        boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER2) ||
        boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER5) ||
        boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER6) ||
        boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER3) ||
        boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER4) ||
        boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER7) ||
        boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER8))
        isWinter = true;

    if (hasWinterAura || hasWinter2Aura)
        return true;   

    if (isCasting && isWinter)
        return true;

    return false;
}

bool IccLichKingAddsTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "the lich king");

    bool hasPlague = botAI->HasAura("Necrotic Plague", bot);
    if (hasPlague)
        return false;

    Unit* terenasMenethilHC = bot->FindNearestCreature(NPC_TERENAS_MENETHIL_HC, 55.0f);
    Unit* terenasMenethil = bot->FindNearestCreature(NPC_TERENAS_MENETHIL, 55.0f);
    
    if (terenasMenethilHC)
        return true;

     if (terenasMenethil)
        return true;

    if (!boss) 
        return false;

    return true;
}
