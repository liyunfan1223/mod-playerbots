#ifndef _PLAYERBOT_RAIDBWLTRIGGERS_H
#define _PLAYERBOT_RAIDBWLTRIGGERS_H

#include "Trigger.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

class BwlSuppressionDeviceTrigger : public Trigger 
{
    public:
        BwlSuppressionDeviceTrigger(PlayerbotAI* botAI) : Trigger(botAI, "bwl suppression device") {}
        bool IsActive() override;
};

#endif