/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_DISTANCEVALUE_H
#define _PLAYERBOT_DISTANCEVALUE_H

#include "NamedObjectContext.h"
#include "Value.h"

class PlayerbotAI;

class DistanceValue : public FloatCalculatedValue, public Qualified
{
public:
    DistanceValue(PlayerbotAI* botAI, std::string const name = "distance") : FloatCalculatedValue(botAI, name) {}

    float Calculate() override;
};

class InsideTargetValue : public BoolCalculatedValue, public Qualified
{
public:
    InsideTargetValue(PlayerbotAI* botAI, std::string const name = "inside target") : BoolCalculatedValue(botAI, name)
    {
    }

    bool Calculate() override;
};

#endif
