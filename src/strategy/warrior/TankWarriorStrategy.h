/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_TANKWARRIORSTRATEGY_H
#define _PLAYERBOT_TANKWARRIORSTRATEGY_H

#include "GenericWarriorStrategy.h"
#include "WarriorTriggers.h"

class PlayerbotAI;

class TankWarriorStrategy : public GenericWarriorStrategy
{
public:
    TankWarriorStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "tank"; }
    NextAction** getDefaultActions() override;
    uint32 GetType() const override { return STRATEGY_TYPE_TANK | STRATEGY_TYPE_MELEE; }
};

#endif
