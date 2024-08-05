/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_LOOTTRIGGERS_H
#define _PLAYERBOT_LOOTTRIGGERS_H

#include "Trigger.h"

class PlayerbotAI;

class LootAvailableTrigger : public Trigger
{
public:
    LootAvailableTrigger(PlayerbotAI* botAI) : Trigger(botAI, "loot available") {}

    bool IsActive() override;
};

class FarFromCurrentLootTrigger : public Trigger
{
public:
    FarFromCurrentLootTrigger(PlayerbotAI* botAI) : Trigger(botAI, "far from current loot") {}

    bool IsActive() override;
};

class CanLootTrigger : public Trigger
{
public:
    CanLootTrigger(PlayerbotAI* botAI) : Trigger(botAI, "can loot") {}

    bool IsActive() override;
};

#endif
