/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDTRIGGERS_MIMIRON_H
#define PLAYERBOTS_ULDTRIGGERS_MIMIRON_H

#include "EventMap.h"
#include "GenericTriggers.h"
#include "Trigger.h"

// Mimiron

// "disperse distance" is bot state, so without this it outlives the pull and the unstacker keeps
// walking bots in later fights. Also clears a "disperse enable" typed inside Ulduar: it can't tell who set it.
class MimironResetEncounterStateTrigger : public Trigger
{
public:
    static constexpr char const* Name = "mimiron reset encounter state trigger";

    MimironResetEncounterStateTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name, 5) {}
    bool IsActive() override;
};

class MimironShockBlastTrigger : public Trigger
{
public:
    static constexpr char const* Name = "mimiron shock blast trigger";

    MimironShockBlastTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class MimironPhase1PositioningTrigger : public Trigger
{
public:
    static constexpr char const* Name = "mimiron phase 1 positioning trigger";

    MimironPhase1PositioningTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class MimironP3Wx2LaserBarrageTrigger : public Trigger
{
public:
    static constexpr char const* Name = "mimiron p3wx2 laser barrage trigger";

    MimironP3Wx2LaserBarrageTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class MimironArcSpreadTrigger : public Trigger
{
public:
    static constexpr char const* Name = "mimiron arc spread trigger";

    MimironArcSpreadTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class MimironAerialCommandUnitTrigger : public Trigger
{
public:
    static constexpr char const* Name = "mimiron aerial command unit trigger";

    MimironAerialCommandUnitTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class MimironRocketStrikeTrigger : public Trigger
{
public:
    static constexpr char const* Name = "mimiron rocket strike trigger";

    MimironRocketStrikeTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class MimironPhase4FocusTrigger : public Trigger
{
public:
    static constexpr char const* Name = "mimiron phase 4 focus trigger";

    MimironPhase4FocusTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class MimironMagneticCoreTrigger : public Trigger
{
public:
    static constexpr char const* Name = "mimiron magnetic core trigger";

    MimironMagneticCoreTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Live while the cannon casts Plasma Blast: the button has to be down before the first of six ticks,
// and a health threshold only notices around the third.
class MimironPlasmaBlastDefensiveTrigger : public Trigger
{
public:
    static constexpr char const* Name = "mimiron plasma blast defensive trigger";

    MimironPlasmaBlastDefensiveTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Phase 1 only, to keep the MK II on the main tank through the walk west. Later phases split two mechs
// across two tanks.
class MimironRedirectThreatTrigger : public Trigger
{
public:
    static constexpr char const* Name = "mimiron redirect threat trigger";

    MimironRedirectThreatTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class MimironSetDpsPriorityTrigger : public Trigger
{
public:
    static constexpr char const* Name = "mimiron set dps priority trigger";

    MimironSetDpsPriorityTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class MimironProximityMineTrigger : public Trigger
{
public:
    static constexpr char const* Name = "mimiron proximity mine trigger";

    MimironProximityMineTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class MimironBombBotTrigger : public Trigger
{
public:
    static constexpr char const* Name = "mimiron bomb bot trigger";

    MimironBombBotTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// This bot has a pet and the encounter is in a phase where it needs telling where to go.
class MimironPetControlTrigger : public Trigger
{
public:
    static constexpr char const* Name = "mimiron pet control trigger";

    MimironPetControlTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// This bot is the phase 3 Frost Resistance Aura paladin and is not running it yet.
class MimironFrostResistanceTrigger : public Trigger
{
public:
    static constexpr char const* Name = "mimiron frost resistance trigger";

    MimironFrostResistanceTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Hard mode (Firefighter): bot is standing in a persistent ground-fire node and must step out.
class MimironDodgeFlamesTrigger : public Trigger
{
public:
    static constexpr char const* Name = "mimiron dodge flames trigger";

    MimironDodgeFlamesTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Hard mode (Firefighter): the bot has to close on its target and the spot the generic reach nodes
// would stop on is burning, but another bearing round the target is not.
class MimironApproachTargetTrigger : public Trigger
{
public:
    static constexpr char const* Name = "mimiron approach target trigger";

    MimironApproachTargetTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Hard mode (Firefighter): bot is inside VX-001's Frost Bomb radius and must clear it before it blows.
class MimironFrostBombTrigger : public Trigger
{
public:
    static constexpr char const* Name = "mimiron frost bomb trigger";

    MimironFrostBombTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Hard mode (Firefighter): bot is standing in a fire bot's spray line or, as a caster or healer in
// 25-man, inside its silence aura.
class MimironFireBotTrigger : public Trigger
{
public:
    static constexpr char const* Name = "mimiron fire bot trigger";

    MimironFireBotTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// This bot carries a ranged snare and is already shooting a Bomb Bot that still has ground to cover.
class MimironSlowBombBotTrigger : public Trigger
{
public:
    static constexpr char const* Name = "mimiron slow bomb bot trigger";

    MimironSlowBombBotTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

#endif
