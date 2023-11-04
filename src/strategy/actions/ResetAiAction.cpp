/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ResetAiAction.h"
#include "Event.h"
#include "ObjectGuid.h"
#include "Playerbots.h"
#include "PlayerbotDbStore.h"
#include "WorldPacket.h"
#include "Group.h"

bool ResetAiAction::Execute(Event event)
{
    if (!event.getPacket().empty()) {
        WorldPacket packet = event.getPacket();
        if (packet.GetOpcode() == SMSG_GROUP_LIST) {
            uint8 groupType;
            Group::MemberSlot slot;
            packet >> groupType;
            packet >> slot.group;
            packet >> slot.flags;
            packet >> slot.roles;
            if (groupType & GROUPTYPE_LFG)
            {
                uint8 status;
                uint32 dungeon;
                packet >> status;
                packet >> dungeon;
            }
            ObjectGuid guid;
            uint32 counter;
            uint32 membersCount;
            packet >> guid;
            packet >> counter;
            packet >> membersCount;
            if (membersCount != 0) {
                return false;
            }
        }
    }
    sPlayerbotDbStore->Reset(botAI);
    botAI->ResetStrategies(false);
    botAI->TellMaster("AI was reset to defaults");
    return true;
}
