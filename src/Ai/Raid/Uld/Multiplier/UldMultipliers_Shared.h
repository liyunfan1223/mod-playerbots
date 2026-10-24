/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDMULTIPLIERS_SHARED_H
#define PLAYERBOTS_ULDMULTIPLIERS_SHARED_H

#include "Define.h"
#include "Multiplier.h"

// The class-generic Misdirection / Tricks nodes always redirect at the group main tank. On the
// encounters below he is not the tank holding what the raid is hitting, so the redirect is held.
class UldThreatRedirectMultiplier : public Multiplier
{
public:
    UldThreatRedirectMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "uld threat redirect") {}
    float GetValue(Action* action) override;
};

// Holds the offensive burst cooldowns until the encounter's real DPS check. One multiplier for every
// gated boss so the IsBurstCooldownAction early-out runs once per action rather than once per boss.
class UlduarBurstWindowMultiplier : public Multiplier
{
public:
    UlduarBurstWindowMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "ulduar burst window") {}
    float GetValue(Action* action) override;

private:
    // Lust is a 10-minute raid cooldown, so several bosses want it held past the point where the
    // personal cooldowns - back up within a phase - are already worth spending.
    struct BurstWindow
    {
        bool allowAll = true;
        bool allowLust = true;
    };

    // Sweeps the whole threat list, so the verdict is memoised for the rest of the tick instead of
    // being recomputed for every candidate action.
    BurstWindow EvaluateWindow();

    // Freya's final phase is six waves of adds away, so a health release keeps lust from being held
    // for the whole fight should the Attuned to Nature read ever miss.
    static constexpr float FREYA_LUST_FALLBACK_PCT = 25.0f;

    uint32 cachedAtMs = 0;
    BurstWindow cachedValue;

    // When this bot first saw all three Mimiron parts fighting in phase 4, 0 outside it.
    uint32 mimironPhase4LiveMs = 0;
};

#endif
