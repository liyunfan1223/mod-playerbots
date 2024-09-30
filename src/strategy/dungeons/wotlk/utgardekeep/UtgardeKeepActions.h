#ifndef _PLAYERBOT_WOTLKDUNGEONUKACTIONS_H
#define _PLAYERBOT_WOTLKDUNGEONUKACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "UtgardeKeepTriggers.h"

class AttackFrostTombAction : public AttackAction
{
public:
    AttackFrostTombAction(PlayerbotAI* ai) : AttackAction(ai, "attack frost tomb") {}
    bool Execute(Event event) override;
};

class AttackDalronnAction : public AttackAction
{
public:
    AttackDalronnAction(PlayerbotAI* ai) : AttackAction(ai, "attack dalronn") {}
    bool Execute(Event event) override;
};

class IngvarStopCastingAction : public Action
{
public:
    IngvarStopCastingAction(PlayerbotAI* ai) : Action(ai, "ingvar stop casting") {}
    bool Execute(Event event) override;
};

class IngvarDodgeSmashAction : public MovementAction
{
public:
    IngvarDodgeSmashAction(PlayerbotAI* ai) : MovementAction(ai, "ingvar dodge smash") {}
    bool isUseful() override;
    bool Execute(Event event) override;
};

class IngvarSmashReturnAction : public MovementAction
{
public:
    IngvarSmashReturnAction(PlayerbotAI* ai) : MovementAction(ai, "ingvar smash return") {}
    bool isUseful() override;
    bool Execute(Event event) override;
};

#endif
