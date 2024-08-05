/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PLAYERbotAIAWARE_H
#define _PLAYERBOT_PLAYERbotAIAWARE_H

class PlayerbotAI;

class PlayerbotAIAware
{
public:
    PlayerbotAIAware(PlayerbotAI* botAI) : botAI(botAI) {}

protected:
    PlayerbotAI* botAI;
};

#endif
