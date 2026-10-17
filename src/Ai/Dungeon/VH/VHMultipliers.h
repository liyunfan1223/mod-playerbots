/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_VHMULTIPLIERS_H
#define PLAYERBOTS_VHMULTIPLIERS_H

#include "Multiplier.h"

class ErekemMultiplier : public Multiplier
{
    public:
        ErekemMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "erekem") {}

    public:
        float GetValue(Action* action) override;
};

class IchoronMultiplier : public Multiplier
{
    public:
        IchoronMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "ichoron") {}

    public:
        float GetValue(Action* action) override;
};

class ZuramatMultiplier : public Multiplier
{
    public:
        ZuramatMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "zuramat the obliterator") {}

    public:
        float GetValue(Action* action) override;
};

#endif
