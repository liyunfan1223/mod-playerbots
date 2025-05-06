#ifndef _PLAYERBOT_RAIDVOAACTIONS_H
#define _PLAYERBOT_RAIDVOAACTIONS_H

#include "Action.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Event.h"

//
//  Emalon the Storm Watcher
//

class EmalonMarkBossAction : public MovementAction
{
public:
    EmalonMarkBossAction(PlayerbotAI* botAI) : MovementAction(botAI, "emalon mark boss action") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class EmalonLightingNovaAction : public MovementAction
{
public:
    EmalonLightingNovaAction(PlayerbotAI* botAI) : MovementAction(botAI, "emalon lighting nova action") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class EmalonOverchargeAction : public Action
{
public:
    EmalonOverchargeAction(PlayerbotAI* botAI) : Action(botAI, "emalon overcharge action") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class EmalonFallFromFloorAction : public Action
{
public:
    EmalonFallFromFloorAction(PlayerbotAI* botAI) : Action(botAI, "emalon fall from floor action") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
