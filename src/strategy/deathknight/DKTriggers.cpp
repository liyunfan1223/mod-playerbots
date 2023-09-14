/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "DKTriggers.h"
#include "Playerbots.h"
#include "SharedDefines.h"
#include <string>

bool DKPresenceTrigger::IsActive()
{
    Unit* target = GetTarget();
    return !botAI->HasAura("blood presence", target) && !botAI->HasAura("unholy presence", target) && !botAI->HasAura("frost presence", target);
}

bool PestilenceTrigger::IsActive() {
    if (!SpellTrigger::IsActive()) {
        return false;
    }
    if (!bot->HasAura(63334)) {
        return false;
    }
    Aura *blood_plague = botAI->GetAura("blood plague", GetTarget(), true, true);
    Aura *frost_fever = botAI->GetAura("frost fever", GetTarget(), true, true);
    if ((blood_plague && blood_plague->GetDuration() <= 3000) ||
        (frost_fever && frost_fever->GetDuration() <= 3000)) {
            return true;
    }
    return false;
}

bool HighBloodRuneTrigger::IsActive() {
    // bot->Say(std::to_string(bot->GetBaseRune(0)) + "_" + std::to_string(bot->GetRuneCooldown(0)) + " " + std::to_string(bot->GetBaseRune(1)) + "_" + std::to_string(bot->GetRuneCooldown(1)), LANG_UNIVERSAL);
    return !bot->GetRuneCooldown(0) && !bot->GetRuneCooldown(1);
}