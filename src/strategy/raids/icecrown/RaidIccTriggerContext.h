#ifndef _PLAYERBOT_RAIDICCTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDICCTRIGGERCONTEXT_H

#include "AiObjectContext.h"
#include "NamedObjectContext.h"
#include "RaidIccTriggers.h"

class RaidIccTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidIccTriggerContext()
    {
        creators["icc in cannon"] = &RaidIccTriggerContext::icc_in_cannon;
        creators["icc gunship cannon near"] = &RaidIccTriggerContext::icc_gunship_cannon_near;
    }

private:
    static Trigger* icc_in_cannon(PlayerbotAI* ai) { return new IccInCannonTrigger(ai); }
    static Trigger* icc_gunship_cannon_near(PlayerbotAI* ai) { return new IccGunshipCannonNearTrigger(ai); }
};

#endif
