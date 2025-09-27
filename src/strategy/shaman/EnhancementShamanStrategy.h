/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ENHANCEMENTSHAMANSTRATEGY_H
#define _PLAYERBOT_ENHANCEMENTSHAMANSTRATEGY_H

#include "GenericShamanStrategy.h"

class PlayerbotAI;

class EnhancementShamanStrategy : public GenericShamanStrategy
{
public:
    EnhancementShamanStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    NextAction** getDefaultActions() override;
    std::string const getName() override { return "enh"; }
    uint32 GetType() const override { return STRATEGY_TYPE_COMBAT | STRATEGY_TYPE_DPS | STRATEGY_TYPE_MELEE; }
};

#endif
