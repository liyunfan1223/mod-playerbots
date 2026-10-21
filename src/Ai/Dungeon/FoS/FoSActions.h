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
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

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

class DevourerWailingSoulsAction : public MovementAction
{
public:
    DevourerWailingSoulsAction(PlayerbotAI* botAI) : MovementAction(botAI, "devourer wailing souls") {}
    bool Execute(Event event) override;
};

class DevourerWellOfSoulsAction : public MovementAction
{
public:
    DevourerWellOfSoulsAction(PlayerbotAI* botAI) : MovementAction(botAI, "devourer well of souls") {}
    bool Execute(Event event) override;

private:
    bool StepOut(Unit* boss, Unit* well);
    // The tank pulls him off a well he stands in, so melee can reach him without stepping in
    bool DragBossClear(Unit* boss);
};

class DevourerStopDamageAction : public Action
{
public:
    DevourerStopDamageAction(PlayerbotAI* botAI) : Action(botAI, "devourer stop damage") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class DevourerRestorePetsAction : public Action
{
public:
    DevourerRestorePetsAction(PlayerbotAI* botAI) : Action(botAI, "devourer restore pets") {}
    bool Execute(Event event) override;
};

class DevourerInterruptPhantomBlastAction : public Action
{
public:
    DevourerInterruptPhantomBlastAction(PlayerbotAI* botAI) : Action(botAI, "devourer interrupt phantom blast") {}
    bool Execute(Event event) override;
};

// A reaction: the engine interrupts the cast in progress before running it
class DevourerStopCastAction : public Action
{
public:
    DevourerStopCastAction(PlayerbotAI* botAI) : Action(botAI, "devourer stop cast") {}
    bool Execute(Event /*event*/) override { return true; }
    bool ShouldReactionInterruptCast() const override { return true; }
};

#endif
