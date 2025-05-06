/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_NEARESTUNITSVALUE_H
#define _PLAYERBOT_NEARESTUNITSVALUE_H

#include "PlayerbotAIConfig.h"
#include "Unit.h"
#include "Value.h"

class PlayerbotAI;

class NearestUnitsValue : public ObjectGuidListCalculatedValue
{
public:
    NearestUnitsValue(PlayerbotAI* botAI, std::string const name = "nearest units",
                      float range = sPlayerbotAIConfig->sightDistance, bool ignoreLos = false, uint32 checkInterval = 1)
        : ObjectGuidListCalculatedValue(botAI, name, checkInterval), range(range), ignoreLos(ignoreLos)
    {
    }

    GuidVector Calculate() override;

protected:
    virtual void FindUnits(std::list<Unit*>& targets) = 0;
    virtual bool AcceptUnit(Unit* unit) = 0;

    float range;
    bool ignoreLos;
};

#endif
