/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDMULTIPLIERS_AURIAYA_H
#define PLAYERBOTS_ULDMULTIPLIERS_AURIAYA_H

#include "RaidAntiFear.h"

// The earth totem slot holds one totem, so the shaman's own Stoneskin / Strength of Earth nodes have
// to be held for as long as the boss can fear, or Tremor is replaced on the next GCD.
class AuriayaAntiFearTotemGuardMultiplier : public RaidAntiFearTotemGuardMultiplier
{
public:
    AuriayaAntiFearTotemGuardMultiplier(PlayerbotAI* botAI)
        : RaidAntiFearTotemGuardMultiplier(botAI, "auriaya anti fear totem guard multiplier")
    {
    }

protected:
    bool FearWindowActive() override;
};

#endif
