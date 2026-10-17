/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_NEARESTNONBOTPLAYERSVALUE_H
#define PLAYERBOTS_NEARESTNONBOTPLAYERSVALUE_H

#include "NearestUnitsValue.h"
#include "PlayerbotAIConfig.h"

class PlayerbotAI;

class NearestNonBotPlayersValue : public NearestUnitsValue
{
public:
    NearestNonBotPlayersValue(PlayerbotAI* botAI, float range = sPlayerbotAIConfig.GrindDistance)
        : NearestUnitsValue(botAI, "nearest non bot players", range, true)
    {
    }

protected:
    void FindUnits(std::list<Unit*>& targets) override;
    bool AcceptUnit(Unit* unit) override;
};

#endif
