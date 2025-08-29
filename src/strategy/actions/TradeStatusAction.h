/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_TRADESTATUSACTION_H
#define _PLAYERBOT_TRADESTATUSACTION_H

#include "QueryItemUsageAction.h"

class Player;
class PlayerbotAI;

class TradeStatusAction : public QueryItemUsageAction
{
public:
    TradeStatusAction(PlayerbotAI* botAI) : QueryItemUsageAction(botAI, "accept trade") {}

    bool Execute(Event event) override;

private:
    void BeginTrade();
    bool CheckTrade();
    int32 CalculateCost(Player* player, bool sell);
};

#endif
