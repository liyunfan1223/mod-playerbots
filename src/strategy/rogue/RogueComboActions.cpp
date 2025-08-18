/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RogueComboActions.h"

#include "Playerbots.h"

bool CastComboAction::isUseful()
{
    return CastMeleeSpellAction::isUseful() && AI_VALUE2(uint8, "combo", "current target") < 5;
}
