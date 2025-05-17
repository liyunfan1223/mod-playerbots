#ifndef _PLAYERBOT_WOTLKDUNGEONPOSACTIONS_H
#define _PLAYERBOT_WOTLKDUNGEONPOSACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "PitOfSaronTriggers.h"

const Position ICKANDKRICK_TANK_POSITION = Position(816.8508f, 102.331505f, 509.1586f);

class IckAndKrickAction : public AttackAction
{
public:
    IckAndKrickAction(PlayerbotAI* ai) : AttackAction(ai, "ick and krick") {}
    bool Execute(Event event) override;

    bool TankPosition(Unit* boss);
    bool Pursuit(bool pursuit, Unit* boss);
    bool PoisonNova(bool poisonNova, Unit* boss);
    bool ExplosiveBarrage(bool explosiveBarrage, Unit* boss);
};

class TyrannusAction : public AttackAction
{
public:
    TyrannusAction(PlayerbotAI* ai) : AttackAction(ai, "tyrannus") {}
    bool Execute(Event event) override;

    bool RangedSpread(bool rangedSpread);
};
#endif
