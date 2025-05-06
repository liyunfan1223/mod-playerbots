/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_SPELLIDVALUE_H
#define _PLAYERBOT_SPELLIDVALUE_H

#include "NamedObjectContext.h"
#include "Value.h"

class PlayerbotAI;

class SpellIdValue : public CalculatedValue<uint32>, public Qualified
{
public:
    SpellIdValue(PlayerbotAI* botAI);

    uint32 Calculate() override;
};

class VehicleSpellIdValue : public CalculatedValue<uint32>, public Qualified
{
public:
    VehicleSpellIdValue(PlayerbotAI* botAI);

    uint32 Calculate() override;
};

#endif
