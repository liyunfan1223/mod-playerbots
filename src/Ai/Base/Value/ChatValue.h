/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CHATVALUE_H
#define PLAYERBOTS_CHATVALUE_H

#include "Value.h"

class PlayerbotAI;

#include "SharedDefines.h"

class ChatValue : public ManualSetValue<ChatMsg>
{
public:
    ChatValue(PlayerbotAI* botAI, std::string const name = "chat")
        : ManualSetValue<ChatMsg>(botAI, CHAT_MSG_WHISPER, name)
    {
    }
};

#endif
