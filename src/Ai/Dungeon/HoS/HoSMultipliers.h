/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HOSMULTIPLIERS_H
#define PLAYERBOTS_HOSMULTIPLIERS_H

#include "Multiplier.h"

class KrystallusMultiplier : public Multiplier
{
    public:
        KrystallusMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "krystallus") {}

    public:
        float GetValue(Action* action) override;
};

class SjonnirMultiplier : public Multiplier
{
    public:
        SjonnirMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "sjonnir the ironshaper") {}

    public:
        float GetValue(Action* action) override;
};

#endif
