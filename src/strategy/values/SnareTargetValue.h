/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_SNARETARGETVALUE_H
#define _PLAYERBOT_SNARETARGETVALUE_H

#include "NamedObjectContext.h"
#include "Value.h"

class PlayerbotAI;
class Unit;

class SnareTargetValue : public UnitCalculatedValue, public Qualified
{
public:
    SnareTargetValue(PlayerbotAI* botAI) : UnitCalculatedValue(botAI, "snare target", 1) {}

protected:
    Unit* Calculate() override;
};

#endif
