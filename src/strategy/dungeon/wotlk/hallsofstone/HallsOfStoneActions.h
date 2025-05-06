#ifndef _PLAYERBOT_WOTLKDUNGEONHOSACTIONS_H
#define _PLAYERBOT_WOTLKDUNGEONHOSACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "HallsOfStoneTriggers.h"

class ShatterSpreadAction : public MovementAction
{
public:
    ShatterSpreadAction(PlayerbotAI* ai) : MovementAction(ai, "shatter spread") {}
    bool Execute(Event event) override;
};

class AvoidLightningRingAction : public MovementAction
{
public:
    AvoidLightningRingAction(PlayerbotAI* ai) : MovementAction(ai, "avoid lightning ring") {}
    bool Execute(Event event) override;
};

#endif
