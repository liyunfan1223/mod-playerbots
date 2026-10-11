/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ENEMYPLAYERVALUE_H
#define PLAYERBOTS_ENEMYPLAYERVALUE_H

#include "PlayerbotAIConfig.h"
#include "PossibleTargetsValue.h"
#include "TargetValue.h"

class PlayerbotAI;
class Unit;

class NearestEnemyPlayersValue : public PossibleTargetsValue
{
public:
    NearestEnemyPlayersValue(PlayerbotAI* botAI, float range = sPlayerbotAIConfig.GrindDistance)
        : PossibleTargetsValue(botAI, "nearest enemy players", range)
    {
    }

public:
    bool AcceptUnit(Unit* unit) override;
};

class EnemyPlayerValue : public UnitCalculatedValue
{
public:
    EnemyPlayerValue(PlayerbotAI* botAI, std::string const name = "enemy player")
        : UnitCalculatedValue(botAI, name, 1 * IN_MILLISECONDS)
    {
    }

    Unit* Calculate() override;

private:
    float GetMaxAttackDistance();
};

#endif
