#ifndef _PLAYERBOT_WOTLKDUNGEONCOSACTIONS_H
#define _PLAYERBOT_WOTLKDUNGEONCOSACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "GenericSpellActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "CullingOfStratholmeTriggers.h"

class ExplodeGhoulSpreadAction : public MovementAction
{
public:
    ExplodeGhoulSpreadAction(PlayerbotAI* ai) : MovementAction(ai, "explode ghoul spread") {}
    bool Execute(Event event) override;
};

class EpochStackAction : public MovementAction
{
public:
    EpochStackAction(PlayerbotAI* ai) : MovementAction(ai, "epoch stack") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
