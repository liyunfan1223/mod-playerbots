/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_NEARESTUNITSVALUE_H
#define PLAYERBOTS_NEARESTUNITSVALUE_H

#include "PlayerbotAIConfig.h"
#include "Unit.h"
#include "Value.h"

class PlayerbotAI;

class NearestUnitsValue : public ObjectGuidListCalculatedValue
{
public:
    NearestUnitsValue(PlayerbotAI* botAI, std::string const name = "nearest units",
                      float range = sPlayerbotAIConfig.SightDistance, bool ignoreLos = false, uint32 checkInterval = 1)
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
