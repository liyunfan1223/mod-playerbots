/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "NonCombatActions.h"
#include "Event.h"
#include "Playerbots.h"

bool DrinkAction::Execute(Event event)
{
    if (bot->IsInCombat())
        return false;

    return UseItemAction::Execute(event);
}

bool DrinkAction::isUseful()
{
    return UseItemAction::isUseful() && AI_VALUE2(uint8, "mana", "self target") < 85;
}

bool EatAction::Execute(Event event)
{
    if (bot->IsInCombat())
        return false;

    return UseItemAction::Execute(event);
}

bool EatAction::isUseful()
{
    return UseItemAction::isUseful() && AI_VALUE2(uint8, "health", "self target") < sPlayerbotAIConfig->lowHealth;
}
