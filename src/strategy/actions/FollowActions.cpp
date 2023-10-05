/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "FollowActions.h"
#include "Event.h"
#include "Formations.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "SharedDefines.h"
#include <cstddef>

bool FollowAction::Execute(Event event)
{
    Formation* formation = AI_VALUE(Formation*, "formation");
    std::string const target = formation->GetTargetName();

    bool moved = false;
    if (!target.empty())
    {
        moved = Follow(AI_VALUE(Unit*, target));
    }
    else
    {
        WorldLocation loc = formation->GetLocation();
        if (Formation::IsNullLocation(loc) || loc.GetMapId() == -1)
            return false;

        moved = MoveTo(loc.GetMapId(), loc.GetPositionX(), loc.GetPositionY(), loc.GetPositionZ());
    }

    if (Pet* pet = bot->GetPet())
    {
        if (CreatureAI* creatureAI = ((Creature*)pet)->AI())
        {
            pet->SetReactState(REACT_PASSIVE);
            pet->GetCharmInfo()->SetIsCommandFollow(true);
            pet->GetCharmInfo()->IsReturning();
            pet->GetMotionMaster()->MoveFollow(bot, PET_FOLLOW_DIST, pet->GetFollowAngle());
            // pet->GetCharmInfo()->SetCommandState(COMMAND_FOLLOW);
            // pet->GetCharmInfo()->SetIsFollowing(true);
            // pet->AttackStop();
            // pet->GetCharmInfo()->IsReturning();
            // pet->GetMotionMaster()->MoveFollow(bot, PET_FOLLOW_DIST, pet->GetFollowAngle());
        }
    }
    //if (moved)
        //botAI->SetNextCheckDelay(sPlayerbotAIConfig->reactDelay);

    return moved;
}

bool FollowAction::isUseful()
{
    if (bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr) {
        return false;
    }
    Formation* formation = AI_VALUE(Formation*, "formation");
    std::string const target = formation->GetTargetName();

    Unit* fTarget = nullptr;
    if (!target.empty())
        fTarget = AI_VALUE(Unit*, target);
    else
        fTarget = AI_VALUE(Unit*, "master target");

    if (fTarget)
    {
        if (fTarget->HasUnitState(UNIT_STATE_IN_FLIGHT))
            return false;

        if (!CanDeadFollow(fTarget))
            return false;

        if (fTarget->GetGUID() == bot->GetGUID())
            return false;
    }

    float distance = 0.f;
    if (!target.empty())
    {
        distance = AI_VALUE2(float, "distance", target);
    }
    else
    {
        WorldLocation loc = formation->GetLocation();
        if (Formation::IsNullLocation(loc) || bot->GetMapId() != loc.GetMapId())
            return false;

        distance = bot->GetDistance(loc.GetPositionX(), loc.GetPositionY(), loc.GetPositionZ());
    }

    return sServerFacade->IsDistanceGreaterThan(distance, formation->GetMaxDistance());
}

bool FollowAction::CanDeadFollow(Unit* target)
{
    // Move to corpse when dead and player is alive or not a ghost.
    if (!bot->IsAlive() && (target->IsAlive() || !target->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST)))
        return false;

    return true;
}

bool FleeToMasterAction::Execute(Event event)
{
    Unit* fTarget = AI_VALUE(Unit*, "master target");
    bool canFollow = Follow(fTarget);
    if (!canFollow)
    {
        //botAI->SetNextCheckDelay(5000);
        return false;
    }

    WorldPosition targetPos(fTarget);
    WorldPosition bosPos(bot);
    float distance = bosPos.fDist(targetPos);

    if (distance < sPlayerbotAIConfig->reactDistance * 3)
    {
        if (!urand(0, 3))
            botAI->TellMaster("I am close, wait for me!");
    }
    else if (distance < 1000)
    {
        if (!urand(0, 10))
            botAI->TellMaster("I heading to your position.");
    }
    else
        if (!urand(0,20))
            botAI->TellMaster("I am traveling to your position.");

    botAI->SetNextCheckDelay(3000);

    return true;
}

bool FleeToMasterAction::isUseful()
{
    if (!botAI->GetGroupMaster())
        return false;

    if (botAI->GetGroupMaster() == bot)
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (target && botAI->GetGroupMaster()->GetTarget() == target->GetGUID())
        return false;

    if (!botAI->HasStrategy("follow", BOT_STATE_NON_COMBAT))
        return false;

    Unit* fTarget = AI_VALUE(Unit*, "master target");

    if (!CanDeadFollow(fTarget))
        return false;

    return true;
}
