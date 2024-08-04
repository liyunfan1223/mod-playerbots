/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_SELFTARGETVALUE_H
#define _PLAYERBOT_SELFTARGETVALUE_H

#include "Value.h"

class PlayerbotAI;
class Unit;

class SelfTargetValue : public UnitCalculatedValue
{
public:
    SelfTargetValue(PlayerbotAI* botAI, std::string const name = "self target") : UnitCalculatedValue(botAI, name) {}

    Unit* Calculate() override;
};

#endif
