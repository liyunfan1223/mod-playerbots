/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_TELLITEMCOUNTACTION_H
#define _PLAYERBOT_TELLITEMCOUNTACTION_H

#include "InventoryAction.h"

class PlayerbotAI;

class TellItemCountAction : public InventoryAction
{
public:
    TellItemCountAction(PlayerbotAI* botAI) : InventoryAction(botAI, "c") {}

    bool Execute(Event event) override;
};

#endif
