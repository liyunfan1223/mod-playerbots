/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "WarriorTriggers.h"

#include "Playerbots.h"

bool BloodrageBuffTrigger::IsActive()
{
    return AI_VALUE2(uint8, "health", "self target") >= sPlayerbotAIConfig->mediumHealth &&
           AI_VALUE2(uint8, "rage", "self target") < 20;
}

bool VigilanceTrigger::IsActive()
{
    // Check for an appropriate target for Vigilance
    Unit* target = context->GetValue<Unit*>("party member without aura", "vigilance")->Get();
    
    // Ensure the target exists, is not the bot itself, and the bot doesn't already have Vigilance active
    return target && target != bot && !botAI->HasAura("vigilance", bot);
}
