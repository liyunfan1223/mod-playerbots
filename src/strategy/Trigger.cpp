/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "Trigger.h"
#include "Event.h"
#include "Playerbots.h"
#include "Timer.h"

Trigger::Trigger(PlayerbotAI* botAI, std::string const name, int32 checkInterval) :
    AiNamedObject(botAI, name), checkInterval(checkInterval == 1 ? 1 : (checkInterval < 100 ? checkInterval * 1000 : checkInterval)), lastCheckTime(0)
{
}

Event Trigger::Check()
{
	if (IsActive())
	{
		Event event(getName());
		return event;
	}

	Event event;
	return event;
}

Value<Unit*>* Trigger::GetTargetValue()
{
    return context->GetValue<Unit*>(GetTargetName());
}

Unit* Trigger::GetTarget()
{
    return GetTargetValue()->Get();
}

bool Trigger::needCheck()
{
    if (checkInterval < 2)
        return true;

    uint32 now = getMSTime();
    if (!lastCheckTime || now - lastCheckTime >= checkInterval)
    {
        lastCheckTime = now;
        return true;
    }

    return false;
}
