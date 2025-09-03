/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_NEARESTNONBOTPLAYERSVALUE_H
#define _PLAYERBOT_NEARESTNONBOTPLAYERSVALUE_H

#include "NearestUnitsValue.h"
#include "PlayerbotAIConfig.h"

class PlayerbotAI;

class NearestNonBotPlayersValue : public NearestUnitsValue
{
public:
    NearestNonBotPlayersValue(PlayerbotAI* botAI, float range = sPlayerbotAIConfig->grindDistance)
        : NearestUnitsValue(botAI, "nearest non bot players", range, true)
    {
    }

protected:
    void FindUnits(std::list<Unit*>& targets) override;
    bool AcceptUnit(Unit* unit) override;
};

#endif
