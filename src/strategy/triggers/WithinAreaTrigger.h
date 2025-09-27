/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_WITHINAREATRIGGER_H
#define _PLAYERBOT_WITHINAREATRIGGER_H

#include "Trigger.h"

class PlayerbotAI;

struct AreaTrigger;

class WithinAreaTrigger : public Trigger
{
public:
    WithinAreaTrigger(PlayerbotAI* botAI) : Trigger(botAI, "within area trigger") {}

    bool IsActive() override;

private:
    bool IsPointInAreaTriggerZone(AreaTrigger const* atEntry, uint32 mapid, float x, float y, float z, float delta);
};

#endif
