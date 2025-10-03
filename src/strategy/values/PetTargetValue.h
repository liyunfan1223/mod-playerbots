/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PETTARGETVALUE_H
#define _PLAYERBOT_PETTARGETVALUE_H

#include "Value.h"

class PlayerbotAI;
class Unit;

class PetTargetValue : public UnitCalculatedValue
{
public:
    PetTargetValue(PlayerbotAI* botAI, std::string const name = "pet target") : UnitCalculatedValue(botAI, name) {}

    Unit* Calculate() override;
};

#endif
