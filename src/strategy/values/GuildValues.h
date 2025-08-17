/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_GUILDVALUES_H
#define _PLAYERBOT_GUILDVALUES_H

#include "Value.h"

class PlayerbotAI;

class PetitionSignsValue : public SingleCalculatedValue<uint8>
{
public:
    PetitionSignsValue(PlayerbotAI* botAI) : SingleCalculatedValue<uint8>(botAI, "petition signs") {}

    uint8 Calculate();
};

#endif
