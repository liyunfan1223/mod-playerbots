/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */
 
#pragma once
#include "Unit.h"
#include "Player.h"
#include "MotionMaster.h"

inline bool CanStartMoveSpline(Player* bot) {
    if (!bot) return false;
    if (!bot->IsAlive()) return false;
    if (bot->IsBeingTeleported() || bot->IsInFlight()) return false;
    if (bot->HasUnitState(UNIT_STATE_LOST_CONTROL) || bot->HasRootAura() ||
        bot->HasStunAura() || bot->IsCharmed() || bot->isFrozen() || bot->IsPolymorphed())
        return false;
    if (bot->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_CONTROLLED) != NULL_MOTION_TYPE)
        return false;
    if (bot->GetSpeed(MOVE_RUN) <= 0.01f) return false;
    return true;
}

inline bool CanStartMoveSpline(Unit* u) {
    if (!u) return false;
    if (!u->IsAlive()) return false;
    if (u->HasUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED | UNIT_STATE_CONFUSED | UNIT_STATE_FLEEING))
        return false;
    if (u->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_CONTROLLED) != NULL_MOTION_TYPE)
        return false;
    if (u->GetSpeed(MOVE_RUN) <= 0.01f) return false;
    return true;
}


