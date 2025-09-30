/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_NEARESTADSVALUE_H
#define _PLAYERBOT_NEARESTADSVALUE_H

#include "PlayerbotAIConfig.h"
#include "PossibleTargetsValue.h"

class PlayerbotAI;

class NearestAddsValue : public PossibleTargetsValue
{
public:
    NearestAddsValue(PlayerbotAI* botAI, float range = sPlayerbotAIConfig->tooCloseDistance)
        : PossibleTargetsValue(botAI, "nearest adds", range, true)
    {
    }

protected:
    bool AcceptUnit(Unit* unit) override;
};

#endif
