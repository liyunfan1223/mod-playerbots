/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "DruidBearActions.h"

#include "Playerbots.h"

bool CastMaulAction::isUseful()
{
    return CastMeleeSpellAction::isUseful() && AI_VALUE2(uint8, "rage", "self target") >= 45;
}
