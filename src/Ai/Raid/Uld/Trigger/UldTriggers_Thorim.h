/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDTRIGGERS_THORIM_H
#define PLAYERBOTS_ULDTRIGGERS_THORIM_H

#include "EventMap.h"
#include "GenericTriggers.h"
#include "Trigger.h"

class ThorimUnbalancingStrikeSwapTrigger : public Trigger
{
public:
    static constexpr char const* Name = "thorim unbalancing strike swap trigger";

    ThorimUnbalancingStrikeSwapTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Only ever true for one bot at a time. Two tanks both answering it is the ping-pong that walks the
// boss across the arena, so a tank already holding him hands the trade to the swap node instead.
class ThorimTankPickupTrigger : public Trigger
{
public:
    static constexpr char const* Name = "thorim tank pickup trigger";

    ThorimTankPickupTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class ThorimDpsPriorityTrigger : public Trigger
{
public:
    static constexpr char const* Name = "thorim dps priority trigger";

    ThorimDpsPriorityTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class ThorimGauntletPositioningTrigger : public Trigger
{
public:
    static constexpr char const* Name = "thorim gauntlet positioning trigger";

    ThorimGauntletPositioningTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Corridor squad member up in the hallway, Rune Giant dead. The lane waypoints all sit down at z 412, so without
// this plain follow walks the squad over both Paralytic Field bunnies on the centre line.
class ThorimBalconyAdvanceTrigger : public Trigger
{
public:
    static constexpr char const* Name = "thorim balcony advance trigger";

    ThorimBalconyAdvanceTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class ThorimArenaPositioningTrigger : public Trigger
{
public:
    static constexpr char const* Name = "thorim arena positioning trigger";

    ThorimArenaPositioningTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class ThorimFallFromFloorTrigger : public Trigger
{
public:
    static constexpr char const* Name = "thorim fall from floor trigger";

    ThorimFallFromFloorTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class ThorimPhase2PositioningTrigger : public Trigger
{
public:
    static constexpr char const* Name = "thorim phase 2 positioning trigger";

    ThorimPhase2PositioningTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// A Runic Smash telegraph is up and this bot is not in the lane it spares.
class ThorimRunicSmashTrigger : public Trigger
{
public:
    static constexpr char const* Name = "thorim runic smash trigger";

    ThorimRunicSmashTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class ThorimRunicBarrierBailTrigger : public Trigger
{
public:
    static constexpr char const* Name = "thorim runic barrier bail trigger";

    ThorimRunicBarrierBailTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// A bot inside the field of the Thunder Orb Thorim has charged, phase 1's largest avoidable damage.
class ThorimChargedOrbTrigger : public Trigger
{
public:
    static constexpr char const* Name = "thorim charged orb trigger";

    ThorimChargedOrbTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// The elected paladin, until the aura is actually up. Its own buff strategy is held off every competing aura,
// so nothing takes the slot back once this lands.
class ThorimFrostAuraTrigger : public Trigger
{
public:
    static constexpr char const* Name = "thorim frost aura trigger";

    ThorimFrostAuraTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// An arena squad member whose pet has left the room. Pets are under no leash of their own and nothing
// recalls them, so one that wanders down the corridor opens packs the gauntlet squad has not reached.
class ThorimPetLeashTrigger : public Trigger
{
public:
    static constexpr char const* Name = "thorim pet leash trigger";

    ThorimPetLeashTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// An arena squad member that has left the box the boss script scans, or is on its way out of it.
class ThorimArenaLeashTrigger : public Trigger
{
public:
    static constexpr char const* Name = "thorim arena leash trigger";

    ThorimArenaLeashTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class ThorimResetEncounterStateTrigger : public Trigger
{
public:
    static constexpr char const* Name = "thorim reset encounter state trigger";

    ThorimResetEncounterStateTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Hard mode: bot standing inside Sif's moving Blizzard ground AoE.
class ThorimSifBlizzardTrigger : public Trigger
{
public:
    static constexpr char const* Name = "thorim sif blizzard trigger";

    ThorimSifBlizzardTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

#endif
