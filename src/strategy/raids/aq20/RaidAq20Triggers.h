#ifndef _PLAYERBOT_RAIDAQ20TRIGGERS_H
#define _PLAYERBOT_RAIDAQ20TRIGGERS_H

#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Trigger.h"

class Aq20MoveToCrystalTrigger : public Trigger
{
public:
    Aq20MoveToCrystalTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq20 move to crystal") {}
    bool IsActive() override;
};
#endif
