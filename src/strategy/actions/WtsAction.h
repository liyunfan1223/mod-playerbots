/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_WTSACTION_H
#define _PLAYERBOT_WTSACTION_H

#include "InventoryAction.h"

class PlayerbotAI;

class WtsAction : public InventoryAction
{
public:
    WtsAction(PlayerbotAI* botAI) : InventoryAction(botAI, "wts") {}

    bool Execute(Event event) override;
};

#endif
