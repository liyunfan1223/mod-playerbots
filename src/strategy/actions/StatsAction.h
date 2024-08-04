/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_STATSACTION_H
#define _PLAYERBOT_STATSACTION_H

#include "Action.h"

class PlayerbotAI;

class StatsAction : public Action
{
public:
    StatsAction(PlayerbotAI* botAI) : Action(botAI, "stats") {}

    bool Execute(Event event) override;

private:
    void ListBagSlots(std::ostringstream& out);
    void ListXP(std::ostringstream& out);
    void ListRepairCost(std::ostringstream& out);
    void ListGold(std::ostringstream& out);
    uint32 EstRepair(uint16 pos);
    double RepairPercent(uint16 pos);
};

#endif
