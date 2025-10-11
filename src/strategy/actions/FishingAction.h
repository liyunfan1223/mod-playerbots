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

WorldPosition findWaterRadial(Player* bot, float x, float y, float z, Map* map, uint32 phaseMask, float minDistance, float maxDistance, float increment, bool checkLOS=false);
WorldPosition findFishingNode(PlayerbotAI* botAI);

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
private:
    Item* pole = nullptr;
};

class MoveNearWaterAction : public MovementAction, public Qualified
{
public:
    MoveNearWaterAction(PlayerbotAI* botAI): MovementAction(botAI, "move near water") {}
    bool Execute(Event event) override;
    bool isUseful() override;
    bool isPossible() override;

private:
    WorldPosition fishingPosition;
    WorldPosition landSpot = WorldPosition();
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
    bool isUseful() override;
};

class RemoveBobberStrategyAction : public Action
{
public:
    RemoveBobberStrategyAction(PlayerbotAI* botAI) : Action(botAI, "remove bobber strategy") {}
    bool Execute(Event event) override;
};
#endif
