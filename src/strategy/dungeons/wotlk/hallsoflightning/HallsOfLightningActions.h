#ifndef _PLAYERBOT_WOTLKDUNGEONHOLACTIONS_H
#define _PLAYERBOT_WOTLKDUNGEONHOLACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "HallsOfLightningTriggers.h"

const Position IONAR_TANK_POSITION = Position(1078.860f, -261.928f, 61.226f);
const Position DISPERSE_POSITION = Position(1161.152f, -261.584f, 53.223f);

class BjarngrimTargetAction : public AttackAction
{
public:
    BjarngrimTargetAction(PlayerbotAI* ai) : AttackAction(ai, "bjarngrim target") {}
    bool Execute(Event event) override;
};

class AvoidWhirlwindAction : public MovementAction
{
public:
    AvoidWhirlwindAction(PlayerbotAI* ai) : MovementAction(ai, "avoid whirlwind") {}
    bool Execute(Event event) override;
};

class VolkhanTargetAction : public AttackAction
{
public:
    VolkhanTargetAction(PlayerbotAI* ai) : AttackAction(ai, "volkhan target") {}
    bool Execute(Event event) override;
};

class StaticOverloadSpreadAction : public MovementAction
{
public:
    StaticOverloadSpreadAction(PlayerbotAI* ai) : MovementAction(ai, "static overload spread") {}
    bool Execute(Event event) override;
};

class BallLightningSpreadAction : public MovementAction
{
public:
    BallLightningSpreadAction(PlayerbotAI* ai) : MovementAction(ai, "ball lightning spread") {}
    bool Execute(Event event) override;
};

class IonarTankPositionAction : public MovementAction
{
public:
    IonarTankPositionAction(PlayerbotAI* ai) : MovementAction(ai, "ionar tank position") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class DispersePositionAction : public MovementAction
{
public:
    DispersePositionAction(PlayerbotAI* ai) : MovementAction(ai, "disperse position") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class LokenStackAction : public MovementAction
{
public:
    LokenStackAction(PlayerbotAI* ai) : MovementAction(ai, "loken stack") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class AvoidLightningNovaAction : public MovementAction
{
public:
    AvoidLightningNovaAction(PlayerbotAI* ai) : MovementAction(ai, "avoid lightning nova") {}
    bool Execute(Event event) override;
};

#endif
