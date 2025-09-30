/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ELEMENTALSHAMANSTRATEGY_H
#define _PLAYERBOT_ELEMENTALSHAMANSTRATEGY_H

#include "GenericShamanStrategy.h"

class PlayerbotAI;

class ElementalShamanStrategy : public GenericShamanStrategy
{
public:
    ElementalShamanStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    NextAction** getDefaultActions() override;
    std::string const getName() override { return "ele"; }
    uint32 GetType() const override { return STRATEGY_TYPE_COMBAT | STRATEGY_TYPE_DPS | STRATEGY_TYPE_RANGED; }
};

#endif
