// /*
//  * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
//  */

#ifndef _PLAYERBOT_RAIDTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "AiObjectContext.h"
#include "raids/naxxramas/RaidNaxxTriggers.h"
#include "raids/blackwinglair/RaidBwlTriggers.h"

class RaidTriggerContext : public NamedObjectContext<Trigger> 
{
    public:
        RaidTriggerContext() 
        {
            creators["bwl suppression device"] = &RaidTriggerContext::bwl_suppression_device;
        }
    private:
        static Trigger* bwl_suppression_device(PlayerbotAI* ai) { return new BwlSuppressionDeviceTrigger(ai); }
};

#endif