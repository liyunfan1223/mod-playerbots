/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDMULTIPLIERS_XT002_H
#define PLAYERBOTS_ULDMULTIPLIERS_XT002_H

#include "Define.h"
#include "Multiplier.h"

// XT-002: holds burst cooldowns for the real window, the first exposed Heart in hard mode and the sub-25%
// push in normal. Bloodlust is separate: its one use has to land on the longest phase.
class XT002BurstWindowMultiplier : public Multiplier
{
public:
    XT002BurstWindowMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "xt002 burst window") {}
    float GetValue(Action* action) override;

private:
    struct Window
    {
        bool allowAll;
        bool allowLust;
    };

    // Runs against every candidate action, so the encounter sweep is memoised for the rest of the tick.
    Window EvaluateWindow();

    uint32 cachedAtMs = 0;
    Window cachedValue = {true, true};
};

// XT-002: normal mode Heart floor, so bots don't kill the Heart and flip the raid into hard mode. Also stands
// down generic targeting, movers for pinned bots, carriers' gap closers and generic threat redirects.
class XT002TargetGuardMultiplier : public Multiplier
{
public:
    XT002TargetGuardMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "xt002 target guard") {}
    float GetValue(Action* action) override;
};

#endif
