/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_NEARESTCORPSESVALUE_H
#define PLAYERBOTS_NEARESTCORPSESVALUE_H

#include "NearestUnitsValue.h"
#include "PlayerbotAIConfig.h"

class PlayerbotAI;

class NearestCorpsesValue : public NearestUnitsValue
{
public:
    NearestCorpsesValue(PlayerbotAI* botAI, float range = sPlayerbotAIConfig.SightDistance)
        : NearestUnitsValue(botAI, "nearest corpses", range, true)
    {
    }

protected:
    void FindUnits(std::list<Unit*>& targets) override;
    bool AcceptUnit(Unit* unit) override;
};

#endif
