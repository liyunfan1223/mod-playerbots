#include "RaidIccMultipliers.h"

#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidActions.h"
#include "DruidBearActions.h"
#include "FollowActions.h"
#include "GenericActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "MovementActions.h"
#include "PaladinActions.h"
#include "PriestActions.h"
#include "RaidIccActions.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ScriptedCreature.h"
#include "ShamanActions.h"
#include "UseMeetingStoneAction.h"
#include "WarriorActions.h"
#include "PlayerbotAI.h"

// LK global variables
namespace
{
uint32 g_lastPlagueTime = 0;
bool g_plagueAllowedToCure = false;
std::map<ObjectGuid, uint32> g_plagueTimes;
std::map<ObjectGuid, bool> g_allowCure;
std::mutex g_plagueMutex;  // Add mutex for thread safety
}


float IccLadyDeathwhisperMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "lady deathwhisper");
    if (!boss)
    {
        return 1.0f;
    }

    if (dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FollowAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float IccAddsDbsMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "deathbringer saurfang");
    if (!boss)
    {
        return 1.0f;
    }

    if (dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FollowAction*>(action))
    {
        return 0.0f;
    }

    if (botAI->IsMainTank(bot))
    {
        Aura* aura = botAI->GetAura("rune of blood", bot);
        if (aura)
        {
            if (dynamic_cast<CastTauntAction*>(action) || dynamic_cast<CastDarkCommandAction*>(action) ||
                dynamic_cast<CastHandOfReckoningAction*>(action) || dynamic_cast<CastGrowlAction*>(action))
            {
                return 0.0f;
            }
        }
    }

    if (botAI->IsDps(bot))
    {
        GuidVector targets = AI_VALUE(GuidVector, "possible targets");
        bool hasAdds = false;
        for (auto& guid : targets)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->IsAlive() && (
                unit->GetEntry() == 38508 || //blood beast
                unit->GetEntry() == 38596 || 
                unit->GetEntry() == 38597 || 
                unit->GetEntry() == 38598))
            {
                hasAdds = true;
                break;
            }
        }

        if (hasAdds && !botAI->IsMainTank(bot))
        {
            if (dynamic_cast<IccAddsDbsAction*>(action))
                return 2.0f;
            else if (dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action) || 
                    dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FollowAction*>(action))
                return 0.0f;
        }
    }
    return 1.0f;
}

float IccDogsMultiplier::GetValue(Action* action)
{
    bool bossPresent = false;
    if (AI_VALUE2(Unit*, "find target", "stinky") || AI_VALUE2(Unit*, "find target", "precious"))
        bossPresent = true;

    if (!bossPresent)
        return 1.0f;

    if (botAI->IsMainTank(bot))
    {
        Aura* aura = botAI->GetAura("mortal wound", bot, false, true);
        if (aura && aura->GetStackAmount() >= 8)
        {
            if (dynamic_cast<CastTauntAction*>(action) || dynamic_cast<CastDarkCommandAction*>(action) ||
                dynamic_cast<CastHandOfReckoningAction*>(action) || dynamic_cast<CastGrowlAction*>(action))
            {
                return 0.0f;
            }
        }
    }
    return 1.0f;
}

float IccFestergutMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "festergut");
    if (!boss)
    {
        return 1.0f;
    }

    if (dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FollowAction*>(action))
    {
        return 0.0f;
    }

    if (botAI->IsMainTank(bot))
    {
        Aura* aura = botAI->GetAura("gastric bloat", bot, false, true);
        if (aura && aura->GetStackAmount() >= 6)
        {
            if (dynamic_cast<CastTauntAction*>(action) || dynamic_cast<CastDarkCommandAction*>(action) ||
                dynamic_cast<CastHandOfReckoningAction*>(action) || dynamic_cast<CastGrowlAction*>(action))
            {
                return 0.0f;
            }
        }
    }

    if (bot->HasAura(69279) && (bot->getClass() == CLASS_HUNTER))
    {
        if (dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action) ||
            dynamic_cast<CastDebuffSpellOnAttackerAction*>(action) || dynamic_cast<CombatFormationMoveAction*>(action) ||
            dynamic_cast<FleeAction*>(action))
        {
            return 0.0f;
        }
    }
    return 1.0f;
}

float IccRotfaceMultiplier::GetValue(Action* action)
{
    // If we're already executing the escape movement, don't interrupt it
    if (dynamic_cast<IccRotfaceMoveAwayFromExplosionAction*>(action))
        return 1.0f;

    Unit* boss = AI_VALUE2(Unit*, "find target", "big ooze");
    if (!boss) { return 1.0f; }

    if (dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FollowAction*>(action))
    {
        return 0.0f;
    }

    if (dynamic_cast<FleeAction*>(action)) 
    { 
        return 0.0f; 
    }

    static std::map<ObjectGuid, uint32> lastExplosionTimes;
    static std::map<ObjectGuid, bool> hasMoved;
    
    ObjectGuid botGuid = bot->GetGUID();

    // When cast starts, record the time
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(69839))
    {
        if (lastExplosionTimes[botGuid] == 0)  // Only set if not already set
        {
            lastExplosionTimes[botGuid] = time(nullptr);
            hasMoved[botGuid] = false;
        }
    }

    // If explosion cast is no longer active, reset the timers
    if (!boss->HasUnitState(UNIT_STATE_CASTING) || !boss->FindCurrentSpellBySpellId(69839))
    {
        if (lastExplosionTimes[botGuid] > 0 && time(nullptr) - lastExplosionTimes[botGuid] >= 16)
        {
            lastExplosionTimes[botGuid] = 0;
            hasMoved[botGuid] = false;
            return 1.0f;  // Allow normal actions to resume
        }
    }

    // If 9 seconds have passed since cast start and we haven't moved yet
    if (lastExplosionTimes[botGuid] > 0 && !hasMoved[botGuid] && time(nullptr) - lastExplosionTimes[botGuid] >= 9)
    {
        if (dynamic_cast<MovementAction*>(action)
            && !dynamic_cast<IccRotfaceMoveAwayFromExplosionAction*>(action))
        {
            return 0.0f;  // Block other movement actions
        }
        hasMoved[botGuid] = true;  // Mark that we've initiated movement
    }

    // Continue blocking other movements for 7 seconds after moving
    if (hasMoved[botGuid] && time(nullptr) - lastExplosionTimes[botGuid] < 16  // 9 seconds wait + 7 seconds stay
        && dynamic_cast<MovementAction*>(action)
        && !dynamic_cast<IccRotfaceMoveAwayFromExplosionAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

/*float IccRotfaceGroupPositionMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<IccRotfaceGroupPositionAction*>(action))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action))
        return 0.0f;

    return 1.0f;
}*/

float IccAddsPutricideMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "professor putricide");
    if (!boss)
    {
        return 1.0f;
    }

    if (botAI->IsMainTank(bot))
    {
        Aura* aura = botAI->GetAura("mutated plague", bot, false, true);
        if (aura && aura->GetStackAmount() >= 4)
        {
            if (dynamic_cast<CastTauntAction*>(action) || dynamic_cast<CastDarkCommandAction*>(action) ||
                dynamic_cast<CastHandOfReckoningAction*>(action) || dynamic_cast<CastGrowlAction*>(action))
            {
                return 0.0f;
            }
        }
    }

    if (dynamic_cast<IccPutricideVolatileOozeAction*>(action) || dynamic_cast<IccPutricideGasCloudAction*>(action))
    {
        if (dynamic_cast<AvoidMalleableGooAction*>(action) || dynamic_cast<IccPutricideGrowingOozePuddleAction*>(action))
            return 0.0f;
    }   

    if (botAI->IsDps(bot))
    {
        GuidVector targets = AI_VALUE(GuidVector, "possible targets");
        bool hasAdds = false;
        for (auto& guid : targets)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && (unit->GetEntry() == 37697 || unit->GetEntry() == 38604 || unit->GetEntry() == 38758 || unit->GetEntry() == 38759 ||//volatile ooze
                        unit->GetEntry() == 37562 || unit->GetEntry() == 38602 || unit->GetEntry() == 38760 || unit->GetEntry() == 38761)) //gas cloud
            {
                hasAdds = true;
                break;
            }
        }

        if (hasAdds)
        {
            if (dynamic_cast<IccPutricideVolatileOozeAction*>(action) || dynamic_cast<IccPutricideGasCloudAction*>(action))
                return 2.0f;
            else if (dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action) || dynamic_cast<AvoidMalleableGooAction*>(action))
                return 0.0f;
        }
    }
    return 1.0f;
}

//bpc
float IccBpcAssistMultiplier::GetValue(Action* action)
{
    if (!action)
        return 1.0f;

    Unit* keleseth = AI_VALUE2(Unit*, "find target", "prince keleseth");
    if (!keleseth || !keleseth->IsAlive())
        return 1.0f;

    Aura* aura = botAI->GetAura("Shadow Prison", bot, false, true);
    if (aura) 
    {
        if (aura->GetStackAmount() > 18 && botAI->IsTank(bot))
        {
            if (dynamic_cast<MovementAction*>(action))
                return 0.0f;
        }

        if (aura->GetStackAmount() > 12 && !botAI->IsTank(bot))
        {
            if (dynamic_cast<MovementAction*>(action))
                return 0.0f;
        }
    }

    Unit* Valanar = AI_VALUE2(Unit*, "find target", "prince valanar");
    if (!Valanar || !Valanar->IsAlive())
        return 1.0f;

    Aura* auraValanar = botAI->GetAura("Invocation of Blood", Valanar);

    if (!botAI->IsTank(bot) && auraValanar && Valanar->HasUnitState(UNIT_STATE_CASTING))
    {
        if (dynamic_cast<IccBpcEmpoweredVortexAction*>(action)) 
            return 1.0f;

        if (dynamic_cast<AttackRtiTargetAction*>(action) ||
            dynamic_cast<TankAssistAction*>(action) ||
            dynamic_cast<DpsAssistAction*>(action) ||
            dynamic_cast<IccBpcMainTankAction*>(action) ||
            dynamic_cast<CombatFormationMoveAction*>(action))
            return 0.0f;
    }

    if (botAI->IsRangedDps(bot))
    {
        GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
        for (auto& npc : npcs)
        {
            Unit* unit = botAI->GetUnit(npc);
            if (unit)
            {
                if (unit->GetName() == "Kinetic Bomb" && ((unit->GetPositionZ() - bot->GetPositionZ()) < 25.0f))
                {
                    if (dynamic_cast<IccBpcKineticBombAction*>(action))
                        return 1.0f;

                    if (dynamic_cast<AttackRtiTargetAction*>(action) ||
                        dynamic_cast<TankAssistAction*>(action) ||
                        dynamic_cast<DpsAssistAction*>(action) ||
                        dynamic_cast<IccBpcMainTankAction*>(action))
                        return 0.0f;
                }
            }
        }
    }

    // For assist tank during BPC fight
    if (botAI->IsAssistTank(bot))
    {
        // Allow BPC-specific actions
        if (dynamic_cast<IccBpcKelesethTankAction*>(action) || 
            dynamic_cast<IccBpcNucleusAction*>(action))
            return 1.0f;

        // Disable normal assist behavior
        if (dynamic_cast<TankAssistAction*>(action))
            return 0.0f;

        // Disable following
        if (dynamic_cast<FollowAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

//BQL
float IccBqlPactOfDarkfallenMultiplier::GetValue(Action* action)
{
    if (!action)
        return 1.0f;

    if (action->getName() == "icc bql pact of darkfallen")
        return 1.0f;

    // If bot has Pact of Darkfallen aura, return 0 for all other actions
    if (bot->HasAura(71340))
        return 0.0f;

    return 1.0f;
}

float IccBqlVampiricBiteMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "blood-queen lana'thel");
    if (!boss)
        return 1.0f;

    Aura* aura = botAI->GetAura("Frenzied Bloodthirst", bot);

    if (botAI->IsMelee(bot) && ((boss->GetPositionZ() - bot->GetPositionZ()) > 5.0f) && !aura)
        {
            if (dynamic_cast<DpsAssistAction*>(action) ||
                dynamic_cast<TankAssistAction*>(action) ||
                dynamic_cast<CastDebuffSpellOnAttackerAction*>(action) ||
                dynamic_cast<CombatFormationMoveAction*>(action))
                return 0.0f;
        }

    // If bot has frenzied bloodthirst, allow highest priority for bite action
    if (aura) // If bot has frenzied bloodthirst
    {
        if (dynamic_cast<IccBqlVampiricBiteAction*>(action))
            return 5.0f;  // Highest priority for bite action

        if (dynamic_cast<DpsAssistAction*>(action) || 
            dynamic_cast<IccBqlTankPositionAction*>(action) ||
            dynamic_cast<TankAssistAction*>(action) ||
            dynamic_cast<CastDebuffSpellOnAttackerAction*>(action) ||
            dynamic_cast<CombatFormationMoveAction*>(action))
            return 0.0f;  // Disable all formation/movement actions
    }

    return 1.0f;
}

//VDW
float IccValithriaDreamCloudMultiplier::GetValue(Action* action)
{
    if (!bot->HasAura(70766))
        return 1.0f;

    // If bot is in dream state, prioritize cloud collection over other actions
    if (bot->HasAura(70766) && dynamic_cast<IccValithriaDreamCloudAction*>(action))
        return 2.0f;
    else if (dynamic_cast<FollowAction*>(action))
        return 0.0f;

    return 1.0f;
}

//SINDRAGOSA
float IccSindragosaTankPositionMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss)
        return 1.0f;

    if (dynamic_cast<IccSindragosaTankPositionAction*>(action))
        return 1.0f;
    else if (dynamic_cast<CombatFormationMoveAction*>(action))
        return 0.0f;    
    return 1.0f;
}

float IccSindragosaFrostBeaconMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss)
        return 1.0f;

    if (!dynamic_cast<IccSindragosaFrostBeaconAction*>(action))
        return 1.0f;

    // Highest priority if we have beacon
    if (bot->HasAura(70126))
        return 2.0f;

    // Lower priority for non-beaconed players
    float const MIN_SAFE_DISTANCE = 11.0f;
    GuidVector members = AI_VALUE(GuidVector, "group members");
    
    for (auto& member : members)
    {
        Unit* player = botAI->GetUnit(member);
        if (!player || player->GetGUID() == bot->GetGUID())
            continue;
            
        if (player->HasAura(70126)) // Frost Beacon
        {
            float dist = bot->GetExactDist2d(player);
            if (dist < MIN_SAFE_DISTANCE)
                return 1.5f;  // Medium priority if too close to beaconed player
        }
    }

    return 1.0f;
}

float IccSindragosaBlisteringColdPriorityMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss)
        return 1.0f;

    // Check if boss is casting blistering cold (using both normal and heroic spell IDs)
    if (boss->HasUnitState(UNIT_STATE_CASTING) && 
        (boss->FindCurrentSpellBySpellId(70123) || boss->FindCurrentSpellBySpellId(71047) || 
         boss->FindCurrentSpellBySpellId(71048) || boss->FindCurrentSpellBySpellId(71049)))
    {
        // If this is the blistering cold action, give it highest priority
        if (dynamic_cast<IccSindragosaBlisteringColdAction*>(action) ||
            dynamic_cast<IccSindragosaTankSwapPositionAction*>(action))
            return 5.0f;

        // Disable all other actions while blistering cold is casting
        return 0.0f;
    }

    return 1.0f;
}

float IccSindragosaMysticBuffetMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss)
        return 1.0f;

    if (botAI->IsMainTank(bot))
    {
        Aura* aura = botAI->GetAura("mystic buffet", bot, false, true);
        if (aura && aura->GetStackAmount() >= 8)
        {
            if (dynamic_cast<CastTauntAction*>(action) || dynamic_cast<CastDarkCommandAction*>(action) ||
                dynamic_cast<CastHandOfReckoningAction*>(action) || dynamic_cast<CastGrowlAction*>(action))
            {
                return 0.0f;
            }
        }
    }

    if (boss->GetVictim() == bot)
        return 1.0f;

    // Only modify actions if we have buffet stacks
    Aura* aura = bot->GetAura(70127);
    Aura* aura2 = bot->GetAura(72528);
    
    // Return normal priority if no auras or not enough stacks
    if (!aura && !aura2)
        return 1.0f;
        
    bool hasEnoughStacks = (aura && aura->GetStackAmount() >= 3) || (aura2 && aura2->GetStackAmount() >= 3);
    if (!hasEnoughStacks)
        return 1.0f;

    if (dynamic_cast<IccSindragosaMysticBuffetAction*>(action) || 
        dynamic_cast<IccSindragosaTankSwapPositionAction*>(action))
        return 5.0f;
    else if (dynamic_cast<CombatFormationMoveAction*>(action) || 
             dynamic_cast<IccSindragosaTankPositionAction*>(action)
             || dynamic_cast<FollowAction*>(action) || dynamic_cast<TankAssistAction*>(action))
        return 0.0f;    
    return 1.0f;
}

float IccSindragosaFrostBombMultiplier::GetValue(Action* action)
{

    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss)
        return 1.0f;

    float const MAX_REACTION_RANGE = 200.0f;

    // Check if there's an active frost bomb marker within range
    bool hasMarkerInRange = false;
    float closestDist = std::numeric_limits<float>::max();
    
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->HasAura(70022))  // Frost bomb visual
        {
            float dist = bot->GetDistance(unit);
            if (dist <= MAX_REACTION_RANGE && dist < closestDist)
            {
                hasMarkerInRange = true;
                closestDist = dist;
            }
        }
    }

    if (!hasMarkerInRange)
        return 1.0f;

    if (dynamic_cast<IccSindragosaFrostBombAction*>(action))
        return 5.0f;
    else if (dynamic_cast<CombatFormationMoveAction*>(action) || 
             dynamic_cast<IccSindragosaTankPositionAction*>(action)
             || dynamic_cast<IccSindragosaBlisteringColdAction*>(action)
             || dynamic_cast<FollowAction*>(action) || dynamic_cast<AttackAction*>(action))
        return 0.0f;    
    return 1.0f;
}

float IccLichKingNecroticPlagueMultiplier::GetValue(Action* action)
{
    // Allow plague movement action to proceed
    if (dynamic_cast<IccLichKingNecroticPlagueAction*>(action))
        return 1.0f;
    // Block combat formation and cure actions by default
    else if (dynamic_cast<CombatFormationMoveAction*>(action))
        return 0.0f;

    // Handle cure actions
    if (dynamic_cast<CurePartyMemberAction*>(action))
    {
        Group* group = bot->GetGroup();
        if (!group)
            return 1.0f;

        // Check if any bot in the group has plague
        bool anyBotHasPlague = false;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            if (Player* member = ref->GetSource())
            {
                if (botAI->HasAura("Necrotic Plague", member))
                {
                    anyBotHasPlague = true;
                    break;
                }
            }
        }

        uint32 currentTime = getMSTime();

        // Reset state if no one has plague
        if (!anyBotHasPlague)
        {
            g_lastPlagueTime = 0;
            g_plagueAllowedToCure = false;
            return 1.0f;
        }

        // Start timer if this is a new plague
        if (g_lastPlagueTime == 0)
        {
            g_lastPlagueTime = currentTime;
            g_plagueAllowedToCure = false;
            return 0.0f;
        }

        // Once we allow cure, keep allowing it until plague is gone
        if (g_plagueAllowedToCure)
        {
            return 1.0f;
        }

        // Check if enough time has passed (3,5 seconds)
        if (currentTime - g_lastPlagueTime >= 3500)
        {
            g_plagueAllowedToCure = true;
            return 1.0f;
        }

        return 0.0f;
    }

    return 1.0f;
}

float IccLichKingAddsMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "the lich king");
    if (!boss)
        return 1.0f;
    Unit* currentTarget = AI_VALUE(Unit*, "current target");

    if (dynamic_cast<IccLichKingWinterAction*>(action))
    {
        if (currentTarget && currentTarget->GetGUID() == boss->GetGUID())
        { 
            if (dynamic_cast<ReachMeleeAction*>(action) || dynamic_cast<ReachSpellAction*>(action) || dynamic_cast<ReachTargetAction*>(action))
                return 0.0f;
        }

        if (dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<IccLichKingAddsAction*>(action))
            return 0.0f;

        return 1.0f;
    }

    //melee reach, spell reach, ranged reach

    if (botAI->IsRanged(bot))
    {
        // Check for defile presence
        GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
        bool defilePresent = false;
        for (auto& npc : npcs)
        {
            Unit* unit = botAI->GetUnit(npc);
            if (unit && unit->IsAlive() && unit->GetEntry() == 38757)  // Defile entry
            {
                defilePresent = true;
                break;
            }
        }

        // Only disable movement if defile is present
        if (defilePresent && (
            dynamic_cast<CombatFormationMoveAction*>(action) ||
            dynamic_cast<FollowAction*>(action) ||
            dynamic_cast<FleeAction*>(action) ||
            dynamic_cast<MoveRandomAction*>(action) ||
            dynamic_cast<MoveFromGroupAction*>(action)))
        {
            return 0.0f;
        }
    }

    if (botAI->IsAssistTank(bot) && !boss->HealthBelowPct(71))
    {
        // Allow BPC-specific actions
        if (dynamic_cast<IccLichKingAddsAction*>(action))
            return 1.0f;

        // Disable normal assist behavior
        if (dynamic_cast<TankAssistAction*>(action))
            return 0.0f;

        if (dynamic_cast<CurePartyMemberAction*>(action))
            return 0.0f;

        // Disable following
        if (dynamic_cast<FollowAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

//raging spirit, Ice sphere, valkyere shadowguard, sphere 36633 39305 39306 39307
