/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_COSMULTIPLIERS_H
#define PLAYERBOTS_COSMULTIPLIERS_H

#include "Multiplier.h"

class EpochMultiplier : public Multiplier
{
    public:
        EpochMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "chrono-lord epoch") {}

    public:
        float GetValue(Action* action) override;
};

#endif
