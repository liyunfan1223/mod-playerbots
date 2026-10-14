/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ACMULTIPLIERS_H
#define PLAYERBOTS_ACMULTIPLIERS_H

#include "Multiplier.h"

class ShirrakFleeFocusFireMultiplier : public Multiplier
{
public:
    ShirrakFleeFocusFireMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "shirrak flee focus fire") {}
    float GetValue(Action* action) override;
};

#endif
