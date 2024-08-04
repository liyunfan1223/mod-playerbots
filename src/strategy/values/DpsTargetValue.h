/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_DPSTARGETVALUE_H
#define _PLAYERBOT_DPSTARGETVALUE_H

#include "RtiTargetValue.h"

class PlayerbotAI;

class DpsTargetValue : public RtiTargetValue
{
public:
    DpsTargetValue(PlayerbotAI* botAI, std::string const type = "rti", std::string const name = "dps target")
        : RtiTargetValue(botAI, type, name)
    {
    }

    Unit* Calculate() override;
};

class DpsAoeTargetValue : public RtiTargetValue
{
public:
    DpsAoeTargetValue(PlayerbotAI* botAI, std::string const type = "rti", std::string const name = "dps aoe target")
        : RtiTargetValue(botAI, type, name)
    {
    }

    Unit* Calculate() override;
};

#endif
