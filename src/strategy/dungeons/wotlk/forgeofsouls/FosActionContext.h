#ifndef _PLAYERBOT_WOTLKDUNGEONFOSACTIONCONTEXT_H
#define _PLAYERBOT_WOTLKDUNGEONFOSACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "FosActions.h"

class WotlkDungeonFosActionContext : public NamedObjectContext<Action>
{
    public:
        WotlkDungeonFosActionContext() {
            creators["move from bronjahm"] = &WotlkDungeonFosActionContext::move_from_bronjahm;
            creators["attack corrupted soul fragment"] = &WotlkDungeonFosActionContext::attack_corrupted_soul_fragment;
            creators["bronjahm tank position"] = &WotlkDungeonFosActionContext::bronjahm_tank_position;
            creators["bronjahm tank target"] = &WotlkDungeonFosActionContext::bronjahm_tank_target;
            creators["bronjahm dps position"] = &WotlkDungeonFosActionContext::bronjahm_dps_position;
        }
    private:
        static Action* move_from_bronjahm(PlayerbotAI* ai) { return new MoveFromBronjahmAction(ai); }
        static Action* attack_corrupted_soul_fragment(PlayerbotAI* ai) { return new AttackCorruptedSoulFragmentAction(ai); }
        static Action* bronjahm_tank_position(PlayerbotAI* ai) { return new BronjahmTankPositionAction(ai); }
        static Action* bronjahm_tank_target(PlayerbotAI* ai) { return new BronjahmTankTargetAction(ai); }
        static Action* bronjahm_dps_position(PlayerbotAI* ai) { return new BronjahmDpsPositionAction(ai); }
};

#endif
