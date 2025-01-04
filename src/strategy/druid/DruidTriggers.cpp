/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "DruidTriggers.h"

#include "Playerbots.h"

bool MarkOfTheWildOnPartyTrigger::IsActive()
{
    // Check both Gift & Mark auras
    return BuffOnPartyTrigger::IsActive() &&
           !botAI->HasAnyAuraOf(GetTarget(), "gift of the wild", "mark of the wild", nullptr);
}

bool MarkOfTheWildTrigger::IsActive()
{
    // Same check for single-target scenario
    return BuffTrigger::IsActive() &&
           !botAI->HasAnyAuraOf(GetTarget(), "gift of the wild", "mark of the wild", nullptr);
}

bool ThornsOnPartyTrigger::IsActive()
{
    return BuffOnPartyTrigger::IsActive() && !botAI->HasAura("thorns", GetTarget());
}

bool EntanglingRootsKiteTrigger::IsActive()
{
    return DebuffTrigger::IsActive() && AI_VALUE(uint8, "attacker count") < 3 && !GetTarget()->GetPower(POWER_MANA);
}

bool ThornsTrigger::IsActive() { return BuffTrigger::IsActive() && !botAI->HasAura("thorns", GetTarget()); }

bool BearFormTrigger::IsActive() { return !botAI->HasAnyAuraOf(bot, "bear form", "dire bear form", nullptr); }

bool TreeFormTrigger::IsActive() { return !botAI->HasAura(33891, bot); }

bool CatFormTrigger::IsActive() { return !botAI->HasAura("cat form", bot); }
