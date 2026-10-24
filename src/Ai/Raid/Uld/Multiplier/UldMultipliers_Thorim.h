/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDMULTIPLIERS_THORIM_H
#define PLAYERBOTS_ULDMULTIPLIERS_THORIM_H

#include "Define.h"
#include "Multiplier.h"

// Thorim: Runic Barrier answers every melee swing with 2000 arcane, so a bot backed out on the health band must
// not walk back in. Holds only the swing and the chase: the target stays and everything else carries on.
class ThorimRunicBarrierMultiplier : public Multiplier
{
public:
    ThorimRunicBarrierMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "thorim runic barrier") {}
    float GetValue(Action* action) override;
};

// Without this the leash and the chase take turns: the leash walks the bot back, the picker still
// holds the corridor mob, and it walks straight out again. Zeroing the attack drops the target instead.
class ThorimArenaTargetGuardMultiplier : public Multiplier
{
public:
    ThorimArenaTargetGuardMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "thorim arena target guard") {}
    float GetValue(Action* action) override;
};

// Thorim: the corridor squad on the platform. The generic picker reads threat, not range, so it holds the
// untouchable boss from far out and "reach melee" walks it up the middle into the Paralytic Field bunnies.
class ThorimBalconyGuardMultiplier : public Multiplier
{
public:
    ThorimBalconyGuardMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "thorim balcony guard") {}
    float GetValue(Action* action) override;
};

#endif
