/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "CombatActions.h"

#include "Event.h"
#include "LastMovementValue.h"
#include "Playerbots.h"
#include "ServerFacade.h"

bool SwitchToMeleeAction::Execute(Event event)
{
    // botAI->TellMasterNoFacing("Switching to melee!");
    return ChangeCombatStrategyAction::Execute(event);
}

bool SwitchToMeleeAction::isUseful()
{
    if (bot->getClass() == CLASS_HUNTER)
    {
        Unit* target = AI_VALUE(Unit*, "current target");
        time_t lastFlee = AI_VALUE(LastMovement&, "last movement").lastFlee;
        return botAI->HasStrategy("ranged", BOT_STATE_COMBAT) &&
               ((bot->IsInCombat() && target &&
                 (target->GetVictim() == bot && (!bot->GetGroup() || lastFlee) &&
                  sServerFacade->IsDistanceLessOrEqualThan(AI_VALUE2(float, "distance", "current target"), 8.0f))) ||
                (!bot->IsInCombat()));
    }

    return botAI->HasStrategy("ranged", BOT_STATE_COMBAT);
}

bool SwitchToRangedAction::Execute(Event event)
{
    // botAI->TellMasterNoFacing("Switching to ranged!");
    return ChangeCombatStrategyAction::Execute(event);
}

bool SwitchToRangedAction::isUseful()
{
    if (bot->getClass() == CLASS_HUNTER)
    {
        Unit* target = AI_VALUE(Unit*, "current target");
        bool hasAmmo = AI_VALUE2(uint32, "item count", "ammo");
        return botAI->HasStrategy("close", BOT_STATE_COMBAT) && hasAmmo &&
               ((bot->IsInCombat() && target &&
                 ((target->GetVictim() != bot || target->GetTarget() != bot->GetGUID()) ||
                  sServerFacade->IsDistanceGreaterThan(AI_VALUE2(float, "distance", "current target"), 8.0f))) ||
                (!bot->IsInCombat()));
    }

    return botAI->HasStrategy("close", BOT_STATE_COMBAT);
}
