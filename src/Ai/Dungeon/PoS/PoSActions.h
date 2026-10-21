/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_POSACTIONS_H
#define PLAYERBOTS_POSACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "PoSTriggers.h"

class IckAndKrickAction : public AttackAction
{
public:
    IckAndKrickAction(PlayerbotAI* botAI) : AttackAction(botAI, "ick and krick") {}
    bool Execute(Event event) override;

private:
    bool TankPosition(Unit* boss);
    bool Pursuit(Unit* boss);
    bool PoisonNova(Unit* boss);
    bool ExplosiveBarrage(Unit* boss);
};

class TyrannusAction : public AttackAction
{
public:
    TyrannusAction(PlayerbotAI* botAI) : AttackAction(botAI, "tyrannus") {}
    bool Execute(Event event) override;

private:
    bool RangedSpread();
};

class GarfrostTakeCoverAction : public MovementAction
{
public:
    GarfrostTakeCoverAction(PlayerbotAI* botAI) : MovementAction(botAI, "garfrost take cover") {}
    bool Execute(Event event) override;
};

class GarfrostTankPositionAction : public MovementAction
{
public:
    GarfrostTankPositionAction(PlayerbotAI* botAI) : MovementAction(botAI, "garfrost tank position") {}
    bool Execute(Event event) override;
};

class GarfrostDodgeSaroniteAction : public MovementAction
{
public:
    GarfrostDodgeSaroniteAction(PlayerbotAI* botAI) : MovementAction(botAI, "garfrost dodge saronite") {}
    bool Execute(Event event) override;
};

class PoSAvoidGroundHazardAction : public MovementAction
{
public:
    PoSAvoidGroundHazardAction(PlayerbotAI* botAI) : MovementAction(botAI, "pos avoid ground hazard") {}
    bool Execute(Event event) override;
};

class TyrannusBrandStopAction : public Action
{
public:
    TyrannusBrandStopAction(PlayerbotAI* botAI) : Action(botAI, "tyrannus brand stop") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class TyrannusBrandRestorePetsAction : public Action
{
public:
    TyrannusBrandRestorePetsAction(PlayerbotAI* botAI) : Action(botAI, "tyrannus brand restore pets") {}
    bool Execute(Event event) override;
};

// A reaction: the engine interrupts the cast in progress before running it
class TyrannusBrandStopCastAction : public Action
{
public:
    TyrannusBrandStopCastAction(PlayerbotAI* botAI) : Action(botAI, "tyrannus brand stop cast") {}
    bool Execute(Event /*event*/) override { return true; }
    bool ShouldReactionInterruptCast() const override { return true; }
};

#endif
