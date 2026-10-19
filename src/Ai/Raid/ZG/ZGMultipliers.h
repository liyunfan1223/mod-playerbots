/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ZGMULTIPLIERS_H
#define PLAYERBOTS_ZGMULTIPLIERS_H

#include "Multiplier.h"

class HakkarKeepPoisonousBloodMultiplier : public Multiplier
{
public:
    HakkarKeepPoisonousBloodMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "hakkar keep poisonous blood") {}
    float GetValue(Action* action) override;
};

#endif
