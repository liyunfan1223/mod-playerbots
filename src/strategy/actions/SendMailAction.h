/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_SENDMAILACTION_H
#define _PLAYERBOT_SENDMAILACTION_H

#include "InventoryAction.h"

class PlayerbotAI;

class SendMailAction : public InventoryAction
{
public:
    SendMailAction(PlayerbotAI* botAI) : InventoryAction(botAI, "sendmail") {}

    bool Execute(Event event) override;
};

#endif
