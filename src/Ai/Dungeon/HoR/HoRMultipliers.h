/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HORMULTIPLIERS_H
#define PLAYERBOTS_HORMULTIPLIERS_H

#include "Multiplier.h"

enum HallsOfReflectionIDs
{
    SPELL_SHARED_SUFFERING    = 72368,
    SPELL_SHARED_SUFFERING_HC = 72369,
};

class MarwynSharedSufferingMultiplier : public Multiplier
{
public:
    MarwynSharedSufferingMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "marwyn shared suffering") {}

    float GetValue(Action* action) override;
};

#endif
