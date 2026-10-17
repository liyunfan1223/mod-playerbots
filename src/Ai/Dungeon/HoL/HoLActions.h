/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HOLACTIONS_H
#define PLAYERBOTS_HOLACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "HoLTriggers.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

const Position IONAR_TANK_POSITION = Position(1078.860f, -261.928f, 61.226f);
const Position DISPERSE_POSITION = Position(1161.152f, -261.584f, 53.223f);

class BjarngrimTargetAction : public AttackAction
{
public:
    BjarngrimTargetAction(PlayerbotAI* botAI) : AttackAction(botAI, "bjarngrim target") {}
    bool Execute(Event event) override;
};

class AvoidWhirlwindAction : public MovementAction
{
public:
    AvoidWhirlwindAction(PlayerbotAI* botAI) : MovementAction(botAI, "avoid whirlwind") {}
    bool Execute(Event event) override;
};

class VolkhanTargetAction : public AttackAction
{
public:
    VolkhanTargetAction(PlayerbotAI* botAI) : AttackAction(botAI, "volkhan target") {}
    bool Execute(Event event) override;
};

class StaticOverloadSpreadAction : public MovementAction
{
public:
    StaticOverloadSpreadAction(PlayerbotAI* botAI) : MovementAction(botAI, "static overload spread") {}
    bool Execute(Event event) override;
};

class BallLightningSpreadAction : public MovementAction
{
public:
    BallLightningSpreadAction(PlayerbotAI* botAI) : MovementAction(botAI, "ball lightning spread") {}
    bool Execute(Event event) override;
};

class IonarTankPositionAction : public MovementAction
{
public:
    IonarTankPositionAction(PlayerbotAI* botAI) : MovementAction(botAI, "ionar tank position") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class DispersePositionAction : public MovementAction
{
public:
    DispersePositionAction(PlayerbotAI* botAI) : MovementAction(botAI, "disperse position") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class LokenStackAction : public MovementAction
{
public:
    LokenStackAction(PlayerbotAI* botAI) : MovementAction(botAI, "loken stack") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class AvoidLightningNovaAction : public MovementAction
{
public:
    AvoidLightningNovaAction(PlayerbotAI* botAI) : MovementAction(botAI, "avoid lightning nova") {}
    bool Execute(Event event) override;
};

#endif
