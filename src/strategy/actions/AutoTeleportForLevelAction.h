/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_AUTOTELEPORTFORLEVELACTION_H
#define _PLAYERBOT_AUTOTELEPORTFORLEVELACTION_H

#include "Action.h"

class PlayerbotAI;

class AutoTeleportForLevelAction : public Action
{
    public:
        AutoTeleportForLevelAction(PlayerbotAI* botAI, std::string const name = "auto teleport for level") : Action(botAI, name) { }

        bool Execute(Event event);
    private:
        void AutoUpgradeEquip();
};

#endif
