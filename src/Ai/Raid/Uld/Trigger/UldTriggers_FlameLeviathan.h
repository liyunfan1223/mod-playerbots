/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDTRIGGERS_FLAMELEVIATHAN_H
#define PLAYERBOTS_ULDTRIGGERS_FLAMELEVIATHAN_H

#include "EventMap.h"
#include "GenericTriggers.h"
#include "Trigger.h"

// Flame Leviathan
class FlameLeviathanOnVehicleTrigger : public Trigger
{
public:
    static constexpr char const* Name = "flame leviathan on vehicle";

    FlameLeviathanOnVehicleTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class FlameLeviathanVehicleNearTrigger : public Trigger
{
public:
    static constexpr char const* Name = "flame leviathan vehicle near";

    FlameLeviathanVehicleNearTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Flame Vents is a 10 s channel and Electroshock cancels it, so the channel is the whole reaction window:
// this stays unthrottled.
class FlameLeviathanFlameVentsTrigger : public Trigger
{
public:
    static constexpr char const* Name = "flame leviathan flame vents";

    FlameLeviathanFlameVentsTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Raises the drive action above the rotation for the two cases where steering cannot wait a tick:
// being chased, and sitting in a hard-mode ground hazard.
class FlameLeviathanDriveUrgentTrigger : public Trigger
{
public:
    static constexpr char const* Name = "flame leviathan drive urgent";

    FlameLeviathanDriveUrgentTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

#endif
