/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

 #ifndef _PLAYERBOT_OFFHEALDRUIDCATSTRATEGY_H
 #define _PLAYERBOT_OFFHEALDRUIDCATSTRATEGY_H

 #include "FeralDruidStrategy.h"

 class PlayerbotAI;

 class OffhealDruidCatStrategy : public FeralDruidStrategy
 {
 public:
    OffhealDruidCatStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "offheal"; }
    NextAction** getDefaultActions() override;
    uint32 GetType() const override
    {
        return STRATEGY_TYPE_COMBAT | STRATEGY_TYPE_DPS | STRATEGY_TYPE_HEAL | STRATEGY_TYPE_MELEE;
    }
 };

 #endif