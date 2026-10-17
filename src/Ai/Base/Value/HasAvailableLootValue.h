/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HASAVAILABLELOOTVALUE_H
#define PLAYERBOTS_HASAVAILABLELOOTVALUE_H

#include "Value.h"

class PlayerbotAI;

class HasAvailableLootValue : public BoolCalculatedValue
{
public:
    HasAvailableLootValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "has available loot") {}

    bool Calculate() override;
};

#endif
