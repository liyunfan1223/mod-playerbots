/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_POSMULTIPLIERS_H
#define PLAYERBOTS_POSMULTIPLIERS_H

#include "Multiplier.h"

class IckAndKrickMultiplier : public Multiplier
{
    public:
    IckAndKrickMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "ick and krick") {}

    public:
        float GetValue(Action* action) override;
};

class GarfrostMultiplier : public Multiplier
{
public:
    GarfrostMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "garfrost") { }

    float GetValue(Action* action) override;
};

#endif
