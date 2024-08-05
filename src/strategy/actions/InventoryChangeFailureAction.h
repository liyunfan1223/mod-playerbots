/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_INVENTORYCHANGEFAILUREACTION_H
#define _PLAYERBOT_INVENTORYCHANGEFAILUREACTION_H

#include "Action.h"
#include "Item.h"

class PlayerbotAI;

class InventoryChangeFailureAction : public Action
{
public:
    InventoryChangeFailureAction(PlayerbotAI* botAI) : Action(botAI, "inventory change failure") {}

    bool Execute(Event event) override;

private:
    static std::map<InventoryResult, std::string> messages;
};

#endif
