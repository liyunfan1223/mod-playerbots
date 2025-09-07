/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "AcceptBattlegroundInvitationAction.h"

#include "Event.h"
#include "Playerbots.h"

bool AcceptBgInvitationAction::Execute(Event event)
{
    uint8 type = 0;                      // arenatype if arena
    uint8 unk2 = 0;                      // unk, can be 0x0 (may be if was invited?) and 0x1
    uint32 bgTypeId_ = BATTLEGROUND_WS;  // type id from dbc
    uint16 unk = 0x1F90;                 // 0x1F90 constant?*/
    uint8 action = 1;

    WorldPacket packet(CMSG_BATTLEFIELD_PORT, 20);
    packet << type << unk2 << (uint32)bgTypeId_ << unk << action;
    // packet << bgTypeId_ << action;
    bot->GetSession()->HandleBattleFieldPortOpcode(packet);

    botAI->ResetStrategies();
    return true;
}
