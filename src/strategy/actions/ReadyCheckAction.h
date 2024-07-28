/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_READYCHECKACTION_H
#define _PLAYERBOT_READYCHECKACTION_H

#include "InventoryAction.h"

class PlayerbotAI;

class ReadyCheckAction : public InventoryAction
{
public:
    ReadyCheckAction(PlayerbotAI* botAI, std::string const name = "ready check") : InventoryAction(botAI, name) {}

    bool Execute(Event event) override;

protected:
    bool ReadyCheck();
};

class FinishReadyCheckAction : public ReadyCheckAction
{
public:
    FinishReadyCheckAction(PlayerbotAI* botAI) : ReadyCheckAction(botAI, "finish ready check") {}

    bool Execute(Event event) override;
};

#endif
