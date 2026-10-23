/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_OSMULTIPLIERS_H
#define PLAYERBOTS_OSMULTIPLIERS_H

#include "Multiplier.h"

class SartharionMultiplier : public Multiplier
{
public:
    SartharionMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "sartharion") {}

public:
    float GetValue(Action* action) override;
};

#endif
