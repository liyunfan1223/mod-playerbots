/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RTSCACTION_H
#define _PLAYERBOT_RTSCACTION_H

#include "SeeSpellAction.h"

class PlayerbotAI;

#define RTSC_MOVE_SPELL 30758  // Aedm (Awesome Energetic do move)

class RTSCAction : public SeeSpellAction
{
public:
    RTSCAction(PlayerbotAI* botAI) : SeeSpellAction(botAI, "rtsc") {}

    bool Execute(Event event) override;
};

#endif
