/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MECHACTIONS_H
#define PLAYERBOTS_MECHACTIONS_H

#include "AttackAction.h"
#include "MechTriggers.h"
#include "MovementActions.h"
#include "ObjectGuid.h"

class SepethreaKiteFlameAction : public MovementAction
{
public:
    SepethreaKiteFlameAction(PlayerbotAI* botAI) : MovementAction(botAI, "sepethrea kite flame") {}
    bool Execute(Event event) override;

private:
    uint32 _turnUntilMs = 0;
    float _turnSide = 1.0f;
    float _turnEnd = 1.0f;
    bool _turnLateral = false;
    ObjectGuid _turnFlame;
};

class SepethreaAvoidFlameAction : public MovementAction
{
public:
    SepethreaAvoidFlameAction(PlayerbotAI* botAI) : MovementAction(botAI, "sepethrea avoid flame") {}
    bool Execute(Event event) override;
};

class SepethreaAvoidTrailAction : public MovementAction
{
public:
    SepethreaAvoidTrailAction(PlayerbotAI* botAI) : MovementAction(botAI, "sepethrea avoid trail") {}
    bool Execute(Event event) override;
};

class SepethreaFocusBossAction : public AttackAction
{
public:
    SepethreaFocusBossAction(PlayerbotAI* botAI) : AttackAction(botAI, "sepethrea focus boss") {}
    bool Execute(Event event) override;
};

#endif
