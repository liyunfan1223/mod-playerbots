/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_CHATFILTER_H
#define _PLAYERBOT_CHATFILTER_H

#include <vector>

#include "Common.h"
#include "PlayerbotAIAware.h"

class PlayerbotAI;

class ChatFilter : public PlayerbotAIAware
{
public:
    ChatFilter(PlayerbotAI* botAI) : PlayerbotAIAware(botAI) {}
    virtual ~ChatFilter() {}

    virtual std::string const Filter(std::string& message);
};

class CompositeChatFilter : public ChatFilter
{
public:
    CompositeChatFilter(PlayerbotAI* botAI);

    virtual ~CompositeChatFilter();
    std::string const Filter(std::string& message) override;

private:
    std::vector<ChatFilter*> filters;
};

#endif
