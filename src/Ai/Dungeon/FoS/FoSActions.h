/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_FOSACTIONS_H
#define PLAYERBOTS_FOSACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "FoSTriggers.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

const Position BRONJAHM_TANK_POSITION = Position(5297.920f, 2506.698f, 686.068f);

class MoveFromBronjahmAction : public MovementAction
{
public:
    MoveFromBronjahmAction(PlayerbotAI* botAI) : MovementAction(botAI, "move from bronjahm") {}
    bool Execute(Event event) override;
};

class AttackCorruptedSoulFragmentAction : public AttackAction
{
public:
    AttackCorruptedSoulFragmentAction(PlayerbotAI* botAI) : AttackAction(botAI, "attack corrupted soul fragment") {}
    bool Execute(Event event) override;
};

class BronjahmGroupPositionAction : public AttackAction
{
public:
    BronjahmGroupPositionAction(PlayerbotAI* botAI) : AttackAction(botAI, "bronjahm group position") {}

    bool Execute(Event event) override;

    bool isUseful() override;
};

class DevourerOfSoulsAction : public AttackAction
{
public:
    DevourerOfSoulsAction(PlayerbotAI* botAI, float distance = 10.0f, float delta_angle = M_PI / 8)
        : AttackAction(botAI, "devourer of souls")
    {
        this->distance = distance;
        this->delta_angle = delta_angle;
    }
    virtual bool Execute(Event event);

protected:
    float distance, delta_angle;
};

#endif
