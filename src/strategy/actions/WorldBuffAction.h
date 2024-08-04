/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_WORLDBUFFACTION_H
#define _PLAYERBOT_WORLDBUFFACTION_H

#include "Action.h"

class PlayerbotAI;
class Unit;

class WorldBuffAction : public Action
{
public:
    WorldBuffAction(PlayerbotAI* botAI) : Action(botAI, "world buff") {}

    bool Execute(Event event) override;

    static std::vector<uint32> NeedWorldBuffs(Unit* unit);
};

#endif
