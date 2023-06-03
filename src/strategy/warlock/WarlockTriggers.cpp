/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "WarlockTriggers.h"
#include "Playerbots.h"

bool DemonArmorTrigger::IsActive()
{
	Unit* target = GetTarget();
	return !botAI->HasAura("demon skin", target) && !botAI->HasAura("demon armor", target) && !botAI->HasAura("fel armor", target);
}

bool SpellstoneTrigger::IsActive()
{
    return BuffTrigger::IsActive() && AI_VALUE2(uint32, "item count", getName()) > 0;
}

bool WarlockConjuredItemTrigger::IsActive()
{
    return ItemCountTrigger::IsActive() && AI_VALUE2(uint32, "item count", "soul shard") > 0;
}

bool ImmolateOnAttackerTrigger::IsActive()
{
    return DebuffOnAttackerTrigger::IsActive() && 
        // !botAI->HasAura("immolate", GetTarget(), false, true) && 
        !botAI->HasAura("unstable affliction", GetTarget(), false, true);
}

bool UnstableAfflictionTrigger::IsActive()
{
    return DebuffTrigger::IsActive() && 
        !botAI->HasAura("immolate", GetTarget(), false, true);
        // !botAI->HasAura("unstable affliction", GetTarget(), false, true);
}

bool UnstableAfflictionOnAttackerTrigger::IsActive()
{
    return DebuffOnAttackerTrigger::IsActive() && 
        !botAI->HasAura("immolate", GetTarget(), false, true);
        // !botAI->HasAura("unstable affliction", GetTarget(), false, true);
}

