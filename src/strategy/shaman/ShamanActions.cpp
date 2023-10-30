/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ShamanActions.h"
#include "Playerbots.h"

bool CastTotemAction::isUseful()
{
    if (needLifeTime > 0.1f) {
        Unit* target = AI_VALUE(Unit*, "current target");
        if (!target) {
            return false;
        }
        float dps = AI_VALUE(float, "expected group dps");
        if (target->GetHealth() / dps < needLifeTime) {
            return false;
        }
    }
    return CastBuffSpellAction::isUseful() && !AI_VALUE2(bool, "has totem", name) && !botAI->HasAura(buff, bot);
}

bool CastManaSpringTotemAction::isUseful()
{
    return CastTotemAction::isUseful() && !AI_VALUE2(bool, "has totem", "healing stream totem");
}

bool CastFlametongueTotemAction::isUseful()
{
    return CastTotemAction::isUseful() && !AI_VALUE2(bool, "has totem", "magma totem") && !botAI->HasAura("totem of wrath", bot);
}

bool CastSearingTotemAction::isUseful()
{
    return CastTotemAction::isUseful() && !AI_VALUE2(bool, "has totem", "flametongue totem");
}

bool CastMagmaTotemAction::isUseful()
{
    return CastTotemAction::isUseful() && !AI_VALUE2(bool, "has totem", name);
}

bool CastCleansingTotemAction::isUseful() 
{ 
    return CastTotemAction::isUseful() && !AI_VALUE2(bool, "has totem", "mana tide totem"); 
}