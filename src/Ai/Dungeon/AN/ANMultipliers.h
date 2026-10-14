/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ANMULTIPLIERS_H
#define PLAYERBOTS_ANMULTIPLIERS_H

#include "Multiplier.h"

class KrikthirMultiplier : public Multiplier
{
    public:
        KrikthirMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "krik'thir the gatewatcher") {}

    public:
        float GetValue(Action* action) override;
};

#endif
