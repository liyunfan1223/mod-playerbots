#ifndef _PLAYERBOT_WOTLKDUNGEONFOSACTIONS_H
#define _PLAYERBOT_WOTLKDUNGEONFOSACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "ForgeOfSoulsTriggers.h"

const Position BRONJAHM_TANK_POSITION = Position(5297.920f, 2506.698f, 686.068f);

class MoveFromBronjahmAction : public MovementAction
{
public:
    MoveFromBronjahmAction(PlayerbotAI* ai) : MovementAction(ai, "move from bronjahm") {}
    bool Execute(Event event) override;
};

class AttackCorruptedSoulFragmentAction : public AttackAction
{
public:
    AttackCorruptedSoulFragmentAction(PlayerbotAI* ai) : AttackAction(ai, "attack corrupted soul fragment") {}
    bool Execute(Event event) override;
};

class BronjahmGroupPositionAction : public AttackAction
{
public:
    BronjahmGroupPositionAction(PlayerbotAI* ai) : AttackAction(ai, "bronjahm group position") {}

    bool Execute(Event event) override;

    bool isUseful() override;
};


#endif
