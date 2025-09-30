/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#pragma once

#include <string>
#include <functional>
#include "Common.h"
#include "Group.h"
#include "Chat.h"
#include "Language.h"

class Player;
class PlayerbotAI;

namespace ai::buff
{

    // Build an aura qualifier "single + greater" to avoid double-buffing
    std::string MakeAuraQualifierForBuff(std::string const& name);

    // Returns the group spell name for a given single-target buff.
    // If no group equivalent exists, returns "".
    std::string GroupVariantFor(std::string const& name);

    // Checks if the bot has the required reagents to cast a spell (by its spellId).
    // Returns false if the spellId is invalid.
    bool HasRequiredReagents(Player* bot, uint32 spellId);


    // Applies the "switch to group buff" policy if: the bot is in a group of size x+,
    // the group variant is known/useful, and reagents are available. Otherwise, returns baseName.
    // If announceOnMissing == true and reagents are missing, calls the 'announce' callback
    // (if provided) to notify the party/raid.
    std::string UpgradeToGroupIfAppropriate(
        Player* bot,
        PlayerbotAI* botAI,
        std::string const& baseName,
        bool announceOnMissing = false,
        std::function<void(std::string const&)> announce = {}
    );
}

namespace ai::chat {
    inline std::function<void(std::string const&)> MakeGroupAnnouncer(Player* me)
    {
        return [me](std::string const& msg)
        {
            if (Group* g = me->GetGroup())
            {
                WorldPacket data;
                ChatMsg type = g->isRaidGroup() ? CHAT_MSG_RAID : CHAT_MSG_PARTY;
                ChatHandler::BuildChatPacket(data, type, LANG_UNIVERSAL, me, /*receiver=*/nullptr, msg.c_str());
                g->BroadcastPacket(&data, true, -1, me->GetGUID());
            }
            else
            {
                me->Say(msg, LANG_UNIVERSAL);
            }
        };
    }
}
