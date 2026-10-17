/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_EXTERNALEVENTHELPER_H
#define PLAYERBOTS_EXTERNALEVENTHELPER_H

#include "Common.h"
#include <map>

class AiObjectContext;
class Player;
class WorldPacket;

class ExternalEventHelper
{
public:
    ExternalEventHelper(AiObjectContext* aiObjectContext) : aiObjectContext(aiObjectContext) {}

    bool ParseChatCommand(std::string const command, Player* owner = nullptr);
    // Classify with the same progressive trigger and item-link rules as
    // ParseChatCommand, but never dispatch an action.
    bool IsChatCommand(std::string const command) const;
    void HandlePacket(std::map<uint16, std::string>& handlers, WorldPacket const& packet, Player* owner = nullptr);
    bool HandleCommand(std::string const name, std::string const param, Player* owner = nullptr);

private:
    AiObjectContext* aiObjectContext;
};

#endif
