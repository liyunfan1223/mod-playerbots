#ifndef _PLAYERBOT_WOTLKDUNGEONNEXACTIONS_H
#define _PLAYERBOT_WOTLKDUNGEONNEXACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "NexusTriggers.h"

#define ANGLE_45_DEG (static_cast<float>(M_PI) / 4.f)
#define ANGLE_90_DEG M_PI_2
#define ANGLE_120_DEG (2.f * static_cast<float>(M_PI) / 3.f)

// Slice of the circle that we want melee dps to attack from.
// Measured from boss orientation, on one side.

// Even though the breath cone is not the full 180 degrees,
// avoid melee dps from the front due to parry potential.
// Bots should learn good dps etiquette :)
#define DRAGON_MELEE_MIN_ANGLE ANGLE_90_DEG
// This leaves a danger zone of 60 degrees at the tail end on both sides.
// This is a total of 120 degrees tail arc that bots will avoid -
// number just happens to be the same in this case, but this is always measured from the front.
#define DRAGON_MELEE_MAX_ANGLE ANGLE_120_DEG

class MoveFromWhirlwindAction : public MovementAction
{
public:
    MoveFromWhirlwindAction(PlayerbotAI* ai) : MovementAction(ai, "move from whirlwind") {}
    bool Execute(Event event) override;
};

class FirebombSpreadAction : public MovementAction
{
public:
    FirebombSpreadAction(PlayerbotAI* ai) : MovementAction(ai, "firebomb spread") {}
    bool Execute(Event event) override;
};

class TelestraSplitTargetAction : public AttackAction
{
public:
    TelestraSplitTargetAction(PlayerbotAI* ai) : AttackAction(ai, "telestra split target") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class ChaoticRiftTargetAction : public AttackAction
{
public:
    ChaoticRiftTargetAction(PlayerbotAI* ai) : AttackAction(ai, "chaotic rift target") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class DodgeSpikesAction : public MovementAction
{
public:
    DodgeSpikesAction(PlayerbotAI* ai) : MovementAction(ai, "dodge spikes") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class IntenseColdJumpAction : public MovementAction
{
public:
    IntenseColdJumpAction(PlayerbotAI* ai) : MovementAction(ai, "intense cold jump") {}
    bool Execute(Event event) override;
};

class RearFlankPositionAction : public MovementAction
{
public:
    RearFlankPositionAction(PlayerbotAI* ai) : MovementAction(ai, "rear flank position") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
