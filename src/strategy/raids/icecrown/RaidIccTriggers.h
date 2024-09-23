#ifndef _PLAYERBOT_RAIDICCTRIGGERS_H
#define _PLAYERBOT_RAIDICCTRIGGERS_H

#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Trigger.h"


class IccInCannonTrigger : public Trigger
{
public:
    IccInCannonTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc in cannon") {}
    bool IsActive() override;
};

class IccGunshipCannonNearTrigger : public Trigger
{
public:
    IccGunshipCannonNearTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc in cannon") {}
    bool IsActive() override;
};

#endif
