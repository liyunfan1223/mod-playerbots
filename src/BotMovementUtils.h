/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */
 
#pragma once
#include "Player.h"
#include "MotionMaster.h"
#include "Unit.h"

inline bool CanStartMoveSpline(Unit* u)
{
    if (!u) return false;
    if (!u->IsAlive()) return false;

    // states that block movement
    if (u->HasUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED | UNIT_STATE_CONFUSED | UNIT_STATE_FLEEING))
        return false;

    // no spline if a "CONTROLLED" movement is in progress (knockback/fear, etc.)
    if (u->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_CONTROLLED) != NULL_MOTION_TYPE)
        return false;

    // MoveSpline constraint: speed > 0.01f
    if (u->GetSpeed(MOVE_RUN) <= 0.01f)
        return false;

    return true;
}


