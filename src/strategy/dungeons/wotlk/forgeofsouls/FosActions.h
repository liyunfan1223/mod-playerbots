#ifndef _PLAYERBOT_WOTLKDUNGEONFOSACTIONS_H
#define _PLAYERBOT_WOTLKDUNGEONFOSACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "FosTriggers.h"

const Position BRONJAHM_TANK_POSITION = Position(5297.9204f, 2506.698f, 686.06793f);

class MoveFromBronjahmAction : public MovementAction
{
public:
    MoveFromBronjahmAction(PlayerbotAI* ai) : MovementAction(ai, "move from bronjahm") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class AttackCorruptedSoulFragmentAction : public AttackAction
{
public:
    AttackCorruptedSoulFragmentAction(PlayerbotAI* ai) : AttackAction(ai, "attack corrupted soul fragment") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class BronjahmTankPositionAction : public MovementAction
{
public:
    BronjahmTankPositionAction(PlayerbotAI* ai) : MovementAction(ai, "bronjahm tank position") {}

    bool Execute(Event event) override;

    bool isUseful() override;
};

class BronjahmDpsPositionAction : public MovementAction
{
public:
    BronjahmDpsPositionAction(PlayerbotAI* ai) : MovementAction(ai, "bronjahm dps position") {}

    bool Execute(Event event) override;

    bool isUseful() override;
};

class BronjahmTankTargetAction : public AttackAction
{
public:
    BronjahmTankTargetAction(PlayerbotAI* ai) : AttackAction(ai, "bronjahm tank target") {}
    bool Execute(Event event) override;
};

#endif
