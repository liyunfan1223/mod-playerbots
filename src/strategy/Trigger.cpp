/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "Trigger.h"
#include "Event.h"
#include "Playerbots.h"

Trigger::Trigger(PlayerbotAI* botAI, std::string const name, int32 checkInterval) :
    AiNamedObject(botAI, name), checkInterval(checkInterval), lastCheckTime(time(nullptr) - rand() % checkInterval)
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

    time_t now = time(nullptr);
    if (!lastCheckTime || now - lastCheckTime >= checkInterval)
    {
        lastCheckTime = now;
        return true;
    }

    return false;
}
