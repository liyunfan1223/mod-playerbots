/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_POSACTIONS_H
#define PLAYERBOTS_POSACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "PoSTriggers.h"

const Position ICKANDKRICK_TANK_POSITION = Position(816.8508f, 102.331505f, 509.1586f);

class IckAndKrickAction : public AttackAction
{
public:
    IckAndKrickAction(PlayerbotAI* botAI) : AttackAction(botAI, "ick and krick") {}
    bool Execute(Event event) override;

    bool TankPosition(Unit* boss);
    bool Pursuit(bool pursuit, Unit* boss);
    bool PoisonNova(bool poisonNova, Unit* boss);
    bool ExplosiveBarrage(bool explosiveBarrage, Unit* boss);
};

class TyrannusAction : public AttackAction
{
public:
    TyrannusAction(PlayerbotAI* botAI) : AttackAction(botAI, "tyrannus") {}
    bool Execute(Event event) override;

    bool RangedSpread(bool rangedSpread);
};
#endif
