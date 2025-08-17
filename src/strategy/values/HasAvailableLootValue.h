/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_HASAVAILABLELOOTVALUE_H
#define _PLAYERBOT_HASAVAILABLELOOTVALUE_H

#include "Value.h"

class PlayerbotAI;

class HasAvailableLootValue : public BoolCalculatedValue
{
public:
    HasAvailableLootValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI) {}

    bool Calculate() override;
};

#endif
