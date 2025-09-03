/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_MASTERTARGETVALUE_H
#define _PLAYERBOT_MASTERTARGETVALUE_H

#include "Value.h"

class PlayerbotAI;
class Unit;

class MasterTargetValue : public UnitCalculatedValue
{
public:
    MasterTargetValue(PlayerbotAI* botAI, std::string const name = "master target") : UnitCalculatedValue(botAI, name)
    {
    }

    Unit* Calculate() override;
};

#endif
