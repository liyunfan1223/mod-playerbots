/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_HUNTERAIOBJECTCONTEXT_H
#define _PLAYERBOT_HUNTERAIOBJECTCONTEXT_H

#include "AiObjectContext.h"

class PlayerbotAI;

class HunterAiObjectContext : public AiObjectContext
{
public:
    HunterAiObjectContext(PlayerbotAI* botAI);
};

#endif
