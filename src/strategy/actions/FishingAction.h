/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_FISHINGACTION_H
#define _PLAYERBOT_FISHINGACTION_H

#include "Action.h"
#include "MovementActions.h"
#include "Event.h"
#include "Playerbots.h"

extern const uint32 FISHING_SPELL;
extern const uint32 FISHING_POLE;
extern const uint32 FISHING_BOBBER;

class PlayerbotAI;

class FishingAction : public Action, public Qualified
{
public:
    FishingAction(PlayerbotAI* botAI) : Action(botAI, "go fishing"){}
    bool Execute(Event event) override;
    bool isUseful() override;
  
};

class EquipFishingPoleAction : public Action, public Qualified
{
public:
    EquipFishingPoleAction(PlayerbotAI* botAI) : Action(botAI, "equip fishing pole") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class MoveToFishAction : public MovementAction, public Qualified
{
public:
    MoveToFishAction(PlayerbotAI* botAI) : MovementAction(botAI, "move to fish") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class UseBobber : public Action
{
public:
    UseBobber(PlayerbotAI* botAI) : Action(botAI, "use fishing bobber") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class EndFishing : public Action
{
public:
    EndFishing(PlayerbotAI* botAI) : Action(botAI, "end fishing") {}
    bool Execute(Event event) override;
};

class RemoveBobberStrategyAction : public Action
{
public:
    RemoveBobberStrategyAction(PlayerbotAI* botAI) : Action(botAI, "remove bobber strategy") {}
    bool Execute(Event event) override;
};
#endif
