/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ACTIVESPELLVALUE_H
#define _PLAYERBOT_ACTIVESPELLVALUE_H

#include "Value.h"

class PlayerbotAI;

class ActiveSpellValue : public CalculatedValue<uint32>
{
public:
    ActiveSpellValue(PlayerbotAI* botAI, std::string const name = "active spell") : CalculatedValue<uint32>(botAI, name)
    {
    }

    uint32 Calculate() override;
};

#endif
