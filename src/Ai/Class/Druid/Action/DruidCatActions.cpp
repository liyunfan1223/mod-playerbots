/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "DruidCatActions.h"

#include "Playerbots.h"

bool CastCowerAction::isUseful()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    return CastBuffSpellAction::isUseful() && !(target && target->IsPlayer());  // players have no threat
}
