/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_SAYACTION_H
#define _PLAYERBOT_SAYACTION_H

#include "Action.h"
#include "NamedObjectContext.h"

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
    ChatReplyAction(PlayerbotAI* ai) : Action(ai, "chat message") {}
    virtual bool Execute(Event event) { return true; }
    bool isUseful() { return true; }
    static void ChatReplyDo(Player* bot, uint32 type, uint32 guid1, uint32 guid2, std::string msg, std::string chanName,
                            std::string name);
};
#endif
