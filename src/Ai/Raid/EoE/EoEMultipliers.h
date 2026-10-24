/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_EOEMULTIPLIERS_H
#define PLAYERBOTS_EOEMULTIPLIERS_H

#include "Multiplier.h"

// P2: Scions hover 20 yd up, where only a disk rider reaches them, so a tank that picks one ends up
// standing under it.
class MalygosScionTankAssistMultiplier : public Multiplier
{
public:
    MalygosScionTankAssistMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "malygos") {}

    float GetValue(Action* action) override;
};

#endif
