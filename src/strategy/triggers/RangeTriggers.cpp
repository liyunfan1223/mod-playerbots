/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "RangeTriggers.h"

#include "MoveSplineInit.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "ServerFacade.h"

static float GetSpeedInMotion(Unit* target)
{
    return target->GetSpeed(Movement::SelectSpeedType(target->GetUnitMovementFlags()));
}

bool EnemyTooCloseForSpellTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    return target && (target->GetVictim() != bot || target->isFrozen() || !target->CanFreeMove()) &&
           target->GetObjectSize() <= 10.0f && target->IsWithinCombatRange(bot, MIN_MELEE_REACH);
    //     Unit* target = AI_VALUE(Unit*, "current target");
    //     if (!target) {
    //         return false;
    //     }

    //     if (target->GetTarget() == bot->GetGUID() && !bot->GetGroup() && !target->HasUnitState(UNIT_STATE_ROOT) &&
    //     GetSpeedInMotion(target) > GetSpeedInMotion(bot) * 0.65f)
    //         return false;

    //     bool isBoss = false;
    //     bool isRaid = false;
    //     float combatReach = bot->GetCombatReach() + target->GetCombatReach();
    //     float targetDistance = sServerFacade->GetDistance2d(bot, target) + combatReach;
    //     if (target->GetTypeId() == TYPEID_UNIT)
    //     {
    //         Creature* creature = botAI->GetCreature(target->GetGUID());
    //             if (creature)
    //             {
    //                 isBoss = creature->isWorldBoss();
    //             }
    //     }

    //     if (bot->GetMap() && bot->GetMap()->IsRaid())
    //         isRaid = true;

    // //    if (isBoss || isRaid)
    // //        return sServerFacade->IsDistanceLessThan(targetDistance, (sPlayerbotAIConfig->tooCloseDistance +
    // combatReach) / 2);

    //     return sServerFacade->IsDistanceLessOrEqualThan(targetDistance, (sPlayerbotAIConfig->tooCloseDistance +
    //     combatReach / 2));
}

bool EnemyTooCloseForAutoShotTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");

    return target && (target->GetVictim() != bot || target->isFrozen() || !target->CanFreeMove()) &&
           bot->IsWithinMeleeRange(target);

    // if (target->GetTarget() == bot->GetGUID() && !bot->GetGroup() && !target->HasUnitState(UNIT_STATE_ROOT) &&
    // GetSpeedInMotion(target) > GetSpeedInMotion(bot) * 0.65f)
    //     return false;

    // bool isBoss = false;
    // bool isRaid = false;
    // float combatReach = bot->GetCombatReach() + target->GetCombatReach();
    // float targetDistance = sServerFacade->GetDistance2d(bot, target) + combatReach;
    // if (target->GetTypeId() == TYPEID_UNIT)
    // {
    //     Creature* creature = botAI->GetCreature(target->GetGUID());
    //     if (creature)
    //     {
    //         isBoss = creature->isWorldBoss();
    //     }
    // }

    // if (bot->GetMap() && bot->GetMap()->IsRaid())
    //     isRaid = true;

    // return sServerFacade->IsDistanceLessOrEqualThan(targetDistance, 5.0f);
}

bool EnemyTooCloseForShootTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    // target->IsWithinCombatRange()

    return target && (target->GetVictim() != bot || target->isFrozen() || !target->CanFreeMove()) &&
           target->IsWithinCombatRange(bot, MIN_MELEE_REACH);

    //     Unit* target = AI_VALUE(Unit*, "current target");
    //     if (!target)
    //         return false;

    //     if (target->GetTarget() == bot->GetGUID() && !bot->GetGroup() && !target->HasUnitState(UNIT_STATE_ROOT) &&
    //     GetSpeedInMotion(target) > GetSpeedInMotion(bot) * 0.65f)
    //         return false;

    //     bool isBoss = false;
    //     bool isRaid = false;
    //     float combatReach = bot->GetCombatReach() + target->GetCombatReach();
    //     float targetDistance = sServerFacade->GetDistance2d(bot, target) + combatReach;
    //     if (target->GetTypeId() == TYPEID_UNIT)
    //     {
    //         Creature* creature = botAI->GetCreature(target->GetGUID());
    //         if (creature)
    //         {
    //             isBoss = creature->isWorldBoss();
    //         }
    //     }

    //     if (bot->GetMap() && bot->GetMap()->IsRaid())
    //         isRaid = true;

    // //    if (isBoss || isRaid)
    // //        return sServerFacade->IsDistanceLessThan(targetDistance, botAI->GetRange("shoot") + combatReach);

    //     return sServerFacade->IsDistanceLessOrEqualThan(targetDistance, (botAI->GetRange("shoot") + combatReach /
    //     2));
}

bool EnemyTooCloseForMeleeTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (target && target->IsPlayer())
        return false;

    return target && AI_VALUE2(bool, "inside target", "current target");
}

bool EnemyIsCloseTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    return target && sServerFacade->IsDistanceLessOrEqualThan(AI_VALUE2(float, "distance", "current target"),
                                                              sPlayerbotAIConfig->tooCloseDistance);
}

bool EnemyWithinMeleeTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    return target && bot->IsWithinMeleeRange(target);
}

bool OutOfRangeTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, GetTargetName());
    // increase contact distance to prevent calculation error
    float dis = distance + CONTACT_DISTANCE;
    return target &&
           !bot->IsWithinCombatRange(
               target,
               dis);  // sServerFacade->IsDistanceGreaterThan(AI_VALUE2(float, "distance", GetTargetName()), distance);
}

EnemyOutOfSpellRangeTrigger::EnemyOutOfSpellRangeTrigger(PlayerbotAI* botAI)
    : OutOfRangeTrigger(botAI, "enemy out of spell range", botAI->GetRange("spell"))
{
}

// bool EnemyOutOfSpellRangeTrigger::IsActive()
// {
//     Unit* target = AI_VALUE(Unit*, GetTargetName());
//     if (!target)
//         return false;

//     float combatReach = bot->GetCombatReach() + target->GetCombatReach();
//     return target && (sServerFacade->GetDistance2d(bot, target) > (distance + combatReach +
//     sPlayerbotAIConfig->contactDistance) || !bot->IsWithinLOSInMap(target));
// }

// bool EnemyOutOfMeleeTrigger::IsActive()
// {
//     Unit* target = AI_VALUE(Unit*, GetTargetName());
//     if (!target)
//         return false;

//     float targetDistance = sServerFacade->GetDistance2d(bot, target);
//     return target && (targetDistance > std::max(5.0f, bot->GetCombatReach() + target->GetCombatReach()) ||
//     (!bot->IsWithinLOSInMap(target) && targetDistance > 5.0f));
// }

bool PartyMemberToHealOutOfSpellRangeTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, GetTargetName());
    if (!target)
        return false;

    float combatReach = bot->GetCombatReach() + target->GetCombatReach();
    return target && (sServerFacade->GetDistance2d(bot, target) > (distance + sPlayerbotAIConfig->contactDistance) ||
                      !bot->IsWithinLOSInMap(target));
}

PartyMemberToHealOutOfSpellRangeTrigger::PartyMemberToHealOutOfSpellRangeTrigger(PlayerbotAI* botAI)
    : OutOfRangeTrigger(botAI, "party member to heal out of spell range", botAI->GetRange("heal") + 1.0f)
{
}

bool FarFromMasterTrigger::IsActive()
{
    return sServerFacade->IsDistanceGreaterThan(AI_VALUE2(float, "distance", "master target"), distance);
}
