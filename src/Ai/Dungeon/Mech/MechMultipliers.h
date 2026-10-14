/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MECHMULTIPLIERS_H
#define PLAYERBOTS_MECHMULTIPLIERS_H

#include "Multiplier.h"

class SepethreaKiteFlameMultiplier : public Multiplier
{
public:
    SepethreaKiteFlameMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "sepethrea kite flame") {}
    float GetValue(Action* action) override;
};

class SepethreaFocusBossMultiplier : public Multiplier
{
public:
    SepethreaFocusBossMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "sepethrea focus boss") {}
    float GetValue(Action* action) override;
};

#endif
