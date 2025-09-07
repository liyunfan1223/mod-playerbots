/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_FROSTFIREMAGESTRATEGY_H
#define _PLAYERBOT_FROSTFIREMAGESTRATEGY_H

#include "GenericMageStrategy.h"

class PlayerbotAI;

class FrostFireMageStrategy : public GenericMageStrategy
{
public:
    FrostFireMageStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "frostfire"; }
    NextAction** getDefaultActions() override;
};

#endif
