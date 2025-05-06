/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "HealthTriggers.h"

#include "Playerbots.h"

bool HealthInRangeTrigger::IsActive()
{
    return ValueInRangeTrigger::IsActive() && !AI_VALUE2(bool, "dead", GetTargetName());
}

float HealthInRangeTrigger::GetValue() { return AI_VALUE2(uint8, "health", GetTargetName()); }

bool PartyMemberDeadTrigger::IsActive() { return GetTarget(); }

bool CombatPartyMemberDeadTrigger::IsActive() { return GetTarget(); }

bool DeadTrigger::IsActive() { return AI_VALUE2(bool, "dead", GetTargetName()); }

bool AoeHealTrigger::IsActive() { return AI_VALUE2(uint8, "aoe heal", type) >= count; }

bool AoeInGroupTrigger::IsActive()
{
    int32 member = botAI->GetNearGroupMemberCount();
    if (member < 5)
        return false;
    int threshold = member * 0.5;
    if (member <= 5)
        threshold = 3;
    else if (member <= 10)
        threshold = std::min(threshold, 5);
    else if (member <= 25)
        threshold = std::min(threshold, 10);
    else
        threshold = std::min(threshold, 15);

    return AI_VALUE2(uint8, "aoe heal", type) >= threshold;
}