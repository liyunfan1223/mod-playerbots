/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_TOTEMSSHAMANSTRATEGY_H
#define _PLAYERBOT_TOTEMSSHAMANSTRATEGY_H

#include "GenericShamanStrategy.h"

class PlayerbotAI;

class TotemsShamanStrategy : public GenericShamanStrategy
{
public:
    TotemsShamanStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "totems"; }
};

#endif
