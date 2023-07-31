/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "HealthTriggers.h"
#include "Playerbots.h"

bool HealthInRangeTrigger::IsActive()
{
    return ValueInRangeTrigger::IsActive() && !AI_VALUE2(bool, "dead", GetTargetName());
}

float HealthInRangeTrigger::GetValue()
{
    return AI_VALUE2(uint8, "health", GetTargetName());
}

bool PartyMemberDeadTrigger::IsActive()
{
	return GetTarget();
}

bool CombatPartyMemberDeadTrigger::IsActive()
{
    return GetTarget();
}

bool DeadTrigger::IsActive()
{
    return AI_VALUE2(bool, "dead", GetTargetName());
}

bool AoeHealTrigger::IsActive()
{
    return AI_VALUE2(uint8, "aoe heal", type) >= count;
}

bool AoeInGroupTrigger::IsActive()
{
    Group *group = bot->GetGroup();
    return group && group->GetMembersCount() >= 5 && AI_VALUE2(uint8, "aoe heal", type) >= (group->GetMembersCount() * ratio);
}