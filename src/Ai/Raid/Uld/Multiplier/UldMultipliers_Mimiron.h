/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDMULTIPLIERS_MIMIRON_H
#define PLAYERBOTS_ULDMULTIPLIERS_MIMIRON_H

#include "Define.h"
#include "Multiplier.h"

// Mimiron hard mode phase 1: holds the main tank's "reach melee" and tank face while the MK II is his, since both
// fight the formation's anchor and the MK II has no frontal to aim. Phase 3 holds "reach melee" alone.
class MimironTankAnchorGuardMultiplier : public Multiplier
{
public:
    MimironTankAnchorGuardMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "mimiron tank anchor guard") {}
    float GetValue(Action* action) override;
};

// Mimiron hard mode, phase 3: bots cannot aim AoE away from a unit, so damage AoE is held near the
// Emergency Fire Bots the raid is keeping alive to put the fire out. Heals are left alone.
class MimironFireBotAoeGuardMultiplier : public Multiplier
{
public:
    MimironFireBotAoeGuardMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "mimiron fire bot aoe guard") {}
    float GetValue(Action* action) override;
};

// Mimiron phase 1: the main tank's own cooldowns go out only through the Plasma Blast window claim.
// The class nodes fire on health, so they'd double up on one window and leave the next one bare.
class MimironPlasmaDefensiveHoldMultiplier : public Multiplier
{
public:
    MimironPlasmaDefensiveHoldMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "mimiron plasma defensive hold") {}
    float GetValue(Action* action) override;
};

#endif
