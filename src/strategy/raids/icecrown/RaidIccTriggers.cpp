#include "RaidIccTriggers.h"

#include "PlayerbotAIConfig.h"
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

    return botAI->IsRangedDps(bot);
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
    if (!boss) { return false; }

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
    if (!boss || !botAI->IsTank(bot)) 
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
    
    if (!mt->HasAura(72410))
    {
        return false;
    }
    
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
    if (!aura || aura->GetStackAmount() < 6)
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
    Aura* aura = botAI->GetAura("gastric bloat", mt, false, true);
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
    if (!boss || !(botAI->IsTank(bot) || botAI->IsMainTank(bot) || botAI->IsAssistTank(bot))) { return false; }

    return true;
}

bool IccRotfaceGroupPositionTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "rotface");
    if (!boss || botAI->IsMelee(bot) || botAI->IsTank(bot) || botAI->IsAssistTank(bot) || botAI->IsMainTank(bot)) { return false; }

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
    if (!boss) { return false; }

    return true;
}

bool IccPutricideVolatileOozeTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "volatile ooze");
    if (!boss) { return false; }

    return true;
}

bool IccPutricideGasCloudTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "gas cloud");
    if (!boss) { return false; }

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
    Aura* aura = botAI->GetAura("mutated plague", mt, false, true);
    if (!aura || aura->GetStackAmount() < 4)
    {
        return false;
    }
    return true;
}

//BPC
bool IccBpcKelesethTankTrigger::IsActive()
{
    if (!botAI->IsAssistTank(bot))
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

    Unit* boss = AI_VALUE2(Unit*, "find target", "prince keleseth");
    if (!boss || boss->GetEntry() != 37972) // Verify it's actually Keleseth
        return false;

    return true;
}

bool IccBpcNucleusTrigger::IsActive()
{
    if (!botAI->IsAssistTank(bot))
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
    if (!botAI->IsMainTank(bot))
        return false;

    Unit* valanar = AI_VALUE2(Unit*, "find target", "prince valanar");
    Unit* taldaram = AI_VALUE2(Unit*, "find target", "prince taldaram");
    
    return valanar != nullptr || taldaram != nullptr;
}

bool IccBpcEmpoweredVortexTrigger::IsActive()
{
    // Tanks should ignore this mechanic
    if (botAI->IsMainTank(bot) || botAI->IsAssistTank(bot))
        return false;

    Unit* valanar = AI_VALUE2(Unit*, "find target", "prince valanar");
    if (!valanar || !valanar->IsAlive())
        return false;

    // For ranged, spread whenever Valanar is empowered
    if (botAI->IsRanged(bot))
        return valanar->HasAura(70952); // Invocation of Blood

    // For melee, only spread during vortex cast
    if (valanar->HasAura(70952) && // Invocation of Blood
        valanar->GetCurrentSpell(CURRENT_GENERIC_SPELL) && 
        valanar->GetCurrentSpell(CURRENT_GENERIC_SPELL)->m_spellInfo->Id == 72039)
    {
        return true;
    }

    return false;
}

//BQL
bool IccBqlTankPositionTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "blood-queen lana'thel");
    if (!boss || !(botAI->IsTank(bot) || botAI->IsMainTank(bot) || botAI->IsAssistTank(bot) || botAI->IsRanged(bot))) 
        return false;

    return true;
}

bool IccBqlPactOfDarkfallenTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "blood-queen lana'thel");
    if (!boss || !bot->HasAura(71340)) 
        return false;

    return true;
}

bool IccBqlVampiricBiteTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "blood-queen lana'thel");
    if (!boss) 
        return false;

    // Only trigger when bot has Frenzied Bloodthirst
    if (!bot->HasAura(70877))
        return false;

    // Add a debug yell when trigger activates
    bot->Yell("Vampiric Bite Trigger Active!", LANG_UNIVERSAL);
    return true;
}
