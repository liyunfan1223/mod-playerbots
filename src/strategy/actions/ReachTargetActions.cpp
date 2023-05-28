/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ReachTargetActions.h"
#include "Event.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "ServerFacade.h"

bool ReachTargetAction::Execute(Event event)
{
    Unit* target = AI_VALUE(Unit*, GetTargetName());
    if (!target)
        return false;

    UpdateMovementState();

    float combatReach = bot->GetCombatReach() + target->GetCombatReach() + 4.0f / 3.0f;
    if (distance < std::max(5.0f, combatReach))
    {
        // return MoveTo(target, 0.0f);
        return ChaseTo(target, 0.0f, bot->GetAngle(target));
    }
    else
    {
        combatReach = bot->GetCombatReach() + target->GetCombatReach();
        bool inLos = bot->IsWithinLOSInMap(target);
        bool  isFriend  = bot->IsFriendlyTo(target);
        float chaseDist = inLos ? distance : isFriend ? distance / 2 : distance;
        return ChaseTo(target, chaseDist - sPlayerbotAIConfig->contactDistance, bot->GetAngle(target));
        // return MoveTo(target, chaseDist - sPlayerbotAIConfig->contactDistance);
    }
}

bool ReachTargetAction::isUseful()
{
    // do not move while casting
    if (bot->IsNonMeleeSpellCast(true))
        return false;

    Unit* target = AI_VALUE(Unit*, GetTargetName());
    return target && (!bot->IsWithinDistInMap(target, distance) || (bot->IsWithinDistInMap(target, distance) && !bot->IsWithinLOSInMap(target)));
}

std::string const ReachTargetAction::GetTargetName()
{
    return "current target";
}

bool CastReachTargetSpellAction::isUseful()
{
    return sServerFacade->IsDistanceGreaterThan(AI_VALUE2(float, "distance", "current target"), (distance + sPlayerbotAIConfig->contactDistance));
}

ReachSpellAction::ReachSpellAction(PlayerbotAI* botAI) : ReachTargetAction(botAI, "reach spell", botAI->GetRange("spell"))
{
}

ReachPartyMemberToHealAction::ReachPartyMemberToHealAction(PlayerbotAI* botAI) : ReachTargetAction(botAI, "reach party member to heal", botAI->GetRange("heal"))
{
}

std::string const ReachPartyMemberToHealAction::GetTargetName()
{
    return "party member to heal";
}
