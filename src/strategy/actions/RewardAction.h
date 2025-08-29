/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_REWARDACTION_H
#define _PLAYERBOT_REWARDACTION_H

#include "InventoryAction.h"

class PlayerbotAI;
class Object;

class RewardAction : public InventoryAction
{
public:
    RewardAction(PlayerbotAI* botAI) : InventoryAction(botAI, "reward") {}

    bool Execute(Event event) override;

private:
    bool Reward(uint32 itemId, Object* pNpc);
};

#endif
