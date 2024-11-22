#ifndef _PLAYERBOT_WOTLKDUNGEONPOSACTIONS_H
#define _PLAYERBOT_WOTLKDUNGEONPOSACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "PitOfSaronTriggers.h"

class MoveFromIckPursuitAction : public MovementAction
{
public:
    MoveFromIckPursuitAction(PlayerbotAI* ai) : MovementAction(ai, "move from ick pursuit") {}
    bool Execute(Event event) override;
};

class MoveFromPoisonNovaAction : public MovementAction
{
public:
    MoveFromPoisonNovaAction(PlayerbotAI* ai) : MovementAction(ai, "move from poison nova") {}
    bool Execute(Event event) override;
};

class MoveFromExplodingOrbAction : public MovementAction
{
public:
    MoveFromExplodingOrbAction(PlayerbotAI* ai) : MovementAction(ai, "move from exploding orb") {}
    bool Execute(Event event) override;
};


#endif
