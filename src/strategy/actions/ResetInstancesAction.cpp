/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ResetInstancesAction.h"

#include "Playerbots.h"

bool ResetInstancesAction::Execute(Event event)
{
    WorldPacket packet(CMSG_RESET_INSTANCES, 0);
    bot->GetSession()->HandleResetInstancesOpcode(packet);

    botAI->TellMaster("Resetting all instances");
    return true;
}

bool ResetInstancesAction::isUseful() { return botAI->GetGroupMaster() == bot; };
