/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GDMULTIPLIERS_H
#define PLAYERBOTS_GDMULTIPLIERS_H

#include "Multiplier.h"

class SladranMultiplier : public Multiplier
{
    public:
        SladranMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "slad'ran") {}

    public:
        float GetValue(Action* action) override;
};

class GaldarahMultiplier : public Multiplier
{
    public:
        GaldarahMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "gal'darah") {}

    public:
        float GetValue(Action* action) override;
};

#endif
