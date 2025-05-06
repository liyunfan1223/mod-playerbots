/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_XPGAINACTION_H
#define _PLAYERBOT_XPGAINACTION_H

#include "Action.h"

class PlayerbotAI;
class Unit;

class XpGainAction : public Action
{
public:
    XpGainAction(PlayerbotAI* botAI) : Action(botAI, "xp gain") {}

    bool Execute(Event event) override;

private:
    void GiveXP(uint32 xp, Unit* victim);
};

#endif
