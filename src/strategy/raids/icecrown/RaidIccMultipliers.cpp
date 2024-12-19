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

float IccAddsDbsMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "deathbringer saurfang");
    if (!boss)
    {
        return 1.0f;
    }

    if (botAI->IsDps(bot))
    {
        GuidVector targets = AI_VALUE(GuidVector, "possible targets");
        bool hasAdds = false;
        for (auto& guid : targets)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && (unit->GetEntry() == 38508 || //blood beast
                        unit->GetEntry() == 38596 || 
                        unit->GetEntry() == 38597 || 
                        unit->GetEntry() == 38598))
            {
                hasAdds = true;
                break;
            }
        }

        if (hasAdds)
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

float FestergutMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "festergut");
    if (!boss)
    {
        return 1.0f;
    }

    // Check if any bot has spore aura
    if (bot->HasAura(69279) || botAI->IsMelee(bot))
    {
        if (dynamic_cast<CombatFormationMoveAction*>(action))
        {
            return 0.0f;
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

float RotfaceMultiplier::GetValue(Action* action)
{
    // If we're already executing the escape movement, don't interrupt it
    if (dynamic_cast<IccRotfaceMoveAwayFromExplosionAction*>(action))
        return 1.0f;

    Unit* boss = AI_VALUE2(Unit*, "find target", "big ooze");
    if (!boss) { return 1.0f; }

    if (dynamic_cast<FleeAction*>(action)) { return 0.0f; }

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

/*float PutricideVolatileOozeMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "volatile ooze");
    if (!boss)
        return 1.0f;

    // Ignore formation actions when volatile ooze mechanic is active
    if (dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FleeAction*>(action) || dynamic_cast<FollowAction*>(action))
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

    if (botAI->IsDps(bot))
    {
        GuidVector targets = AI_VALUE(GuidVector, "possible targets");
        bool hasAdds = false;
        for (auto& guid : targets)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && (unit->GetEntry() == 37697 || //volatile ooze
                        unit->GetEntry() == 37562))    //gas cloud
            {
                hasAdds = true;
                break;
            }
        }

        if (hasAdds)
        {
            if (dynamic_cast<IccPutricideVolatileOozeAction*>(action) || dynamic_cast<IccPutricideGasCloudAction*>(action))
                return 2.0f;
            else if (dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action))
                return 0.0f;
        }
    }
    return 1.0f;
}

//bpc
float BpcAssistMultiplier::GetValue(Action* action)
{
    if (!action)
        return 1.0f;

    Unit* keleseth = AI_VALUE2(Unit*, "find target", "prince keleseth");
    if (!keleseth || !keleseth->IsAlive())
        return 1.0f;

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
    if (!bot->HasAura(70867)) // If bot doesn't have vampire buff
        return 1.0f;

    if (bot->HasAura(70877)) // If bot has frenzied bloodthirst
    {
        if (dynamic_cast<IccBqlVampiricBiteAction*>(action))
            return 5.0f;  // Highest priority for bite action
        else if (dynamic_cast<DpsAssistAction*>(action) ||
                 dynamic_cast<TankAssistAction*>(action) ||
                 dynamic_cast<CastDebuffSpellOnAttackerAction*>(action) ||
                 dynamic_cast<CombatFormationMoveAction*>(action))
            return 0.0f;  // Disable all formation/movement actions
        else
            return 0.0f;  // Disable all other actions
    }

    return 1.0f;
}
