/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDMULTIPLIERS_IRONASSEMBLY_H
#define PLAYERBOTS_ULDMULTIPLIERS_IRONASSEMBLY_H

#include "Define.h"
#include "Multiplier.h"

// Hard mode only: burst waits until Steelbreaker is last, where Supercharge and Electrical Charge compound.
// Before that it's one shared health pool, so holding costs nothing.
class IronAssemblyHoldDpsCooldownsMultiplier : public Multiplier
{
public:
    IronAssemblyHoldDpsCooldownsMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "iron assembly hold dps cooldowns") {}
    float GetValue(Action* action) override;
};

// Whoever carries Overwhelming Power keeps the empowered Steelbreaker until Meltdown, so this zeroes
// any class taunt while another grouped tank holds him.
class IronAssemblyTauntGuardMultiplier : public Multiplier
{
public:
    IronAssemblyTauntGuardMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "iron assembly taunt guard") {}
    float GetValue(Action* action) override;
};

#endif
