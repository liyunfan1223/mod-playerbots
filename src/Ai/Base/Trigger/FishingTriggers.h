/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_FISHINGTRIGGERS_H
#define PLAYERBOTS_FISHINGTRIGGERS_H

#include "GenericTriggers.h"

class CanFishTrigger : public Trigger
{
public:
    CanFishTrigger(PlayerbotAI* botAI) : Trigger(botAI, "can fish") {};
    bool IsActive() override;
};

class CanUseFishingBobberTrigger : public Trigger
{
public:
    CanUseFishingBobberTrigger(PlayerbotAI* botAI) : Trigger(botAI, "can use fishing bobber") {};
    bool IsActive() override;
};

#endif
