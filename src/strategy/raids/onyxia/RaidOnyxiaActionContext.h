#ifndef _PLAYERBOT_RAIDONYXIAACTIONS_CONTEXT_H
#define _PLAYERBOT_RAIDONYXIAACTIONS_CONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "RaidOnyxiaActions.h"

class RaidOnyxiaActionContext : public NamedObjectContext<Action>
{
public:
    RaidOnyxiaActionContext()
    {
        creators["ony position tank"] = &RaidOnyxiaActionContext::position_tank;
        creators["ony move to side"] = &RaidOnyxiaActionContext::move_to_side;
        creators["ony spread out"] = &RaidOnyxiaActionContext::spread_out;
        creators["ony avoid lava"] = &RaidOnyxiaActionContext::avoid_lava;
        creators["ony move to safe zone"] = &RaidOnyxiaActionContext::move_to_safe_zone;
    }

private:
    static Action* position_tank(PlayerbotAI* ai) { return new RaidOnyxiaPositionTankAction(ai); }
    static Action* move_to_side(PlayerbotAI* ai) { return new RaidOnyxiaMoveToSideAction(ai); }
    static Action* spread_out(PlayerbotAI* ai) { return new RaidOnyxiaSpreadOutAction(ai); }
    static Action* avoid_lava(PlayerbotAI* ai) { return new RaidOnyxiaAvoidLavaAction(ai); }
    static Action* move_to_safe_zone(PlayerbotAI* ai) { return new RaidOnyxiaMoveToSafeZoneAction(ai); }
};

#endif
