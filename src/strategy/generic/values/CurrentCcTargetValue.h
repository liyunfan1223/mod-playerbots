/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_CURRENTCCTARGETVALUE_H
#define _PLAYERBOT_CURRENTCCTARGETVALUE_H

#include "NamedObjectContext.h"
#include "TargetValue.h"

class PlayerbotAI;
class Unit;

class CurrentCcTargetValue : public TargetValue, public Qualified
{
public:
    CurrentCcTargetValue(PlayerbotAI* botAI, std::string const name = "current cc target") : TargetValue(botAI, name) {}

    Unit* Calculate() override;
};

#endif
