/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TRADESTATUSACTION_H
#define PLAYERBOTS_TRADESTATUSACTION_H

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
    void CancelTrade();
    bool CheckTrade();
    int32 CalculateCost(Player* player, bool sell);
};

#endif
