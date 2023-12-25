/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "AttackAction.h"
#include "Event.h"
#include "LootObjectStack.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "CreatureAI.h"
#include "Unit.h"

bool AttackAction::Execute(Event event)
{
    Unit* target = GetTarget();
    if (!target)
        return false;
    
    if (!target->IsInWorld()) {
        return false;
    }
    return Attack(target);
}

bool AttackMyTargetAction::Execute(Event event)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    ObjectGuid guid = master->GetTarget();
    if (!guid)
    {
        if (verbose)
            botAI->TellError("You have no target");

        return false;
    }

    bool result = Attack(botAI->GetUnit(guid));
    if (result)
        context->GetValue<ObjectGuid>("pull target")->Set(guid);

    return result;
}

bool AttackAction::Attack(Unit* target, bool with_pet /*true*/)
{
    if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == FLIGHT_MOTION_TYPE || bot->HasUnitState(UNIT_STATE_IN_FLIGHT))
    {
        if (verbose)
            botAI->TellError("I cannot attack in flight");

        return false;
    }

    if (!target)
    {
        if (verbose)
            botAI->TellError("I have no target");

        return false;
    }

    std::ostringstream msg;
    msg << target->GetName();

    if (bot->IsFriendlyTo(target))
    {
        msg << " is friendly to me";
        if (verbose)
            botAI->TellError(msg.str());

        return false;
    }

    if (!bot->IsWithinLOSInMap(target))
    {
        msg << " is not on my sight";
        if (verbose)
            botAI->TellError(msg.str());
    }

    if (target->isDead())
    {
        msg << " is dead";
        if (verbose)
            botAI->TellError(msg.str());

        return false;
    }

    if (bot->IsMounted() && bot->IsWithinLOSInMap(target) && sServerFacade->GetDistance2d(bot, target) < 40.0f)
    {
        WorldPacket emptyPacket;
        bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);
    }

    ObjectGuid guid = target->GetGUID();
    bot->SetSelection(target->GetGUID());

    Unit* oldTarget = context->GetValue<Unit*>("current target")->Get();
    context->GetValue<Unit*>("old target")->Set(oldTarget);

    context->GetValue<Unit*>("current target")->Set(target);
    context->GetValue<LootObjectStack*>("available loot")->Get()->Add(guid);
    
    bool attacked = bot->Attack(target, true);

    if (!attacked) {
        return false;
    }
    /* prevent pet dead immediately in group */
    if (bot->GetMap()->IsDungeon() && bot->GetGroup() && !target->IsInCombat()) {
        with_pet = false;
    }
    if (Pet* pet = bot->GetPet())
    {
        if (with_pet) {
            pet->SetReactState(REACT_DEFENSIVE);
            pet->SetTarget(target->GetGUID());
            pet->GetCharmInfo()->SetIsCommandAttack(true);
            pet->AI()->AttackStart(target);
            // pet->SetReactState(REACT_DEFENSIVE);
        } else {
            pet->SetReactState(REACT_PASSIVE);
            pet->GetCharmInfo()->SetIsCommandFollow(true);
            pet->GetCharmInfo()->IsReturning();
            // pet->GetMotionMaster()->MoveFollow(bot, PET_FOLLOW_DIST, pet->GetFollowAngle());
        }
    }

    if (IsMovingAllowed() && !bot->HasInArc(CAST_ANGLE_IN_FRONT, target)) {
        sServerFacade->SetFacingTo(bot, target);
    }
        // bot->SetFacingToObject(target);

    
    botAI->ChangeEngine(BOT_STATE_COMBAT);

    return true;
}

bool AttackDuelOpponentAction::isUseful()
{
    return AI_VALUE(Unit*, "duel target");
}

bool AttackDuelOpponentAction::Execute(Event event)
{
    return Attack(AI_VALUE(Unit*, "duel target"));
}
