/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_CURRENTTARGETVALUE_H
#define _PLAYERBOT_CURRENTTARGETVALUE_H

#include "Value.h"

class PlayerbotAI;
class Unit;

class CurrentTargetValue : public UnitManualSetValue
{
public:
    CurrentTargetValue(PlayerbotAI* botAI, std::string const name = "current target")
        : UnitManualSetValue(botAI, nullptr, name)
    {
    }

    Unit* Get() override;
    void Set(Unit* unit) override;

private:
    ObjectGuid selection;
};

#endif
