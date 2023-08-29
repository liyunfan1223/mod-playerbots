/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RAIDACTIONCONTEXTACTION_H
#define _PLAYERBOT_RAIDACTIONCONTEXTACTION_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "raids/blackwinglair/RaidBwlActions.h"

class RaidActionContext : public NamedObjectContext<Action>
{
    public:
        RaidActionContext()
        {
            creators["bwl check onyxia scale cloak"] = &RaidActionContext::bwl_check_onyxia_scale_cloak;
            creators["bwl turn off suppression device"] = &RaidActionContext::bwl_turn_off_suppression_device;
        }

    private:
        static Action* bwl_check_onyxia_scale_cloak(PlayerbotAI* botAI) { return new BwlOnyxiaScaleCloakAuraCheckAction(botAI); }
        static Action* bwl_turn_off_suppression_device(PlayerbotAI* botAI) { return new BwlTurnOffSuppressionDeviceAction(botAI); }
};

#endif
