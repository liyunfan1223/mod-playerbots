/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SAYACTION_H
#define PLAYERBOTS_SAYACTION_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "PlayerbotAI.h"

class PlayerbotAI;
class SayAction : public Action, public Qualified
{
public:
    SayAction(PlayerbotAI* botAI);

    bool Execute(Event event) override;
    bool isUseful() override;
    std::string const getName() override { return "say::" + qualifier; }

private:
    static std::map<std::string, std::vector<std::string>> stringTable;
    static std::map<std::string, uint32> probabilityTable;
};

class ChatReplyAction : public Action
{
public:
    ChatReplyAction(PlayerbotAI* botAI) : Action(botAI, "chat message") {}
    virtual bool Execute(Event /*event*/) { return true; }
    bool isUseful() { return true; }

    static void ChatReplyDo(Player* bot, uint32& type, uint32& guid1, std::string& msg, std::string& chanName, std::string& name);
    static bool HandleThunderfuryReply(Player* bot, ChatChannelSource chatChannelSource);
    static bool HandleToxicLinksReply(Player* bot, ChatChannelSource chatChannelSource);
    static bool HandleWTBItemsReply(Player* bot, ChatChannelSource chatChannelSource, std::string& msg, std::string& name);
    static bool HandleLFGQuestsReply(Player* bot, ChatChannelSource chatChannelSource, std::string& msg, std::string& name);
    static bool SendGeneralResponse(Player* bot, ChatChannelSource chatChannelSource, std::string& responseMessage, std::string& name);
    static std::string GenerateReplyMessage(Player* bot, std::string& incomingMessage, uint32& guid1, std::string& name);
};
#endif
