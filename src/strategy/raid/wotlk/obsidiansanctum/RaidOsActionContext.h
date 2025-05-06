#ifndef _PLAYERBOT_RAIDOSACTIONCONTEXT_H
#define _PLAYERBOT_RAIDOSACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "RaidOsActions.h"

class RaidOsActionContext : public NamedObjectContext<Action>
{
public:
    RaidOsActionContext()
    {
        creators["sartharion tank position"] = &RaidOsActionContext::tank_position;
        creators["avoid twilight fissure"] = &RaidOsActionContext::avoid_twilight_fissure;
        creators["avoid flame tsunami"] = &RaidOsActionContext::avoid_flame_tsunami;
        creators["sartharion attack priority"] = &RaidOsActionContext::attack_priority;
        creators["enter twilight portal"] = &RaidOsActionContext::enter_twilight_portal;
        creators["exit twilight portal"] = &RaidOsActionContext::exit_twilight_portal;
    }

private:
    static Action* tank_position(PlayerbotAI* ai) { return new SartharionTankPositionAction(ai); }
    static Action* avoid_twilight_fissure(PlayerbotAI* ai) { return new AvoidTwilightFissureAction(ai); }
    static Action* avoid_flame_tsunami(PlayerbotAI* ai) { return new AvoidFlameTsunamiAction(ai); }
    static Action* attack_priority(PlayerbotAI* ai) { return new SartharionAttackPriorityAction(ai); }
    static Action* enter_twilight_portal(PlayerbotAI* ai) { return new EnterTwilightPortalAction(ai); }
    static Action* exit_twilight_portal(PlayerbotAI* ai) { return new ExitTwilightPortalAction(ai); }
};

#endif
