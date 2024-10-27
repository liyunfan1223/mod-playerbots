#ifndef _PLAYERBOT_WOTLKDUNGEONOCCACTIONS_H
#define _PLAYERBOT_WOTLKDUNGEONOCCACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "OculusTriggers.h"
#include "UseItemAction.h"
#include "GenericSpellActions.h"

const Position uromSafePositions[3] =
{
    Position(1138.88f, 1052.22f, 508.36f),
    Position(1084.62f, 1079.71f, 508.36f),
    Position(1087.42f, 1020.132f, 508.36f)
};

class AvoidUnstableSphereAction : public MovementAction
{
public:
    AvoidUnstableSphereAction(PlayerbotAI* ai) : MovementAction(ai, "avoid unstable sphere") {}
    bool Execute(Event event) override;
};

class MountDrakeAction : public UseItemAction
{
public:
    MountDrakeAction(PlayerbotAI* ai) : UseItemAction(ai, "mount drake") {}
    bool Execute(Event event) override;
    bool isPossible() override;
};

class DismountDrakeAction : public Action
{
public:
    DismountDrakeAction(PlayerbotAI* ai) : Action(ai, "dismount drake") {}
    bool Execute(Event event) override;
};

class FlyDrakeAction : public MovementAction
{
public:
    FlyDrakeAction(PlayerbotAI* ai) : MovementAction(ai, "fly drake") {}
    bool Execute(Event event) override;
};

class DrakeAttackAction : public Action
{
public:
    DrakeAttackAction(PlayerbotAI* botAI) : Action(botAI, "drake attack") {}
    bool Execute(Event event) override;

protected:
    Unit* vehicleBase;
    bool CastDrakeSpellAction(Unit* target, uint32 spellId, uint32 cooldown);
    bool AmberDrakeAction(Unit* target);
    bool EmeraldDrakeAction(Unit* target);
    bool RubyDrakeAction(Unit* target);

};

class AvoidArcaneExplosionAction : public MovementAction
{
public:
    AvoidArcaneExplosionAction(PlayerbotAI* ai) : MovementAction(ai, "avoid arcane explosion") {}
    bool Execute(Event event) override;
};

class TimeBombSpreadAction : public MovementAction
{
public:
    TimeBombSpreadAction(PlayerbotAI* ai) : MovementAction(ai, "time bomb spread") {}
    bool Execute(Event event) override;
};

#endif
