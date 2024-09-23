#ifndef _PLAYERBOT_RAIDICCACTIONCONTEXT_H
#define _PLAYERBOT_RAIDICCACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "RaidIccActions.h"

class RaidIccActionContext : public NamedObjectContext<Action>
{
public:
    RaidIccActionContext()
    {
        creators["icc cannon fire"] = &RaidIccActionContext::icc_cannon_fire;
        creators["icc gunship enter cannon"] = &RaidIccActionContext::icc_gunship_enter_cannon;
    }

private:
    static Action* icc_cannon_fire(PlayerbotAI* ai) { return new IccCannonFireAction(ai); }
    static Action* icc_gunship_enter_cannon(PlayerbotAI* ai) { return new IccGunshipEnterCannonAction(ai); }
};

#endif
