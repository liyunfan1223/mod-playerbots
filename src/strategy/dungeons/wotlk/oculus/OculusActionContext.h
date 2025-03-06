#ifndef _PLAYERBOT_WOTLKDUNGEONOCCACTIONCONTEXT_H
#define _PLAYERBOT_WOTLKDUNGEONOCCACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "OculusActions.h"

class WotlkDungeonOccActionContext : public NamedObjectContext<Action>
{
    public:
        WotlkDungeonOccActionContext() {
            creators["avoid unstable sphere"] = &WotlkDungeonOccActionContext::avoid_unstable_sphere;
            creators["mount drake"] = &WotlkDungeonOccActionContext::mount_drake;
            creators["dismount drake"] = &WotlkDungeonOccActionContext::dismount_drake;
            creators["occ fly drake"] = &WotlkDungeonOccActionContext::occ_fly_drake;
            creators["occ drake attack"] = &WotlkDungeonOccActionContext::occ_drake_attack;
            creators["avoid arcane explosion"] = &WotlkDungeonOccActionContext::avoid_arcane_explosion;
            creators["time bomb spread"] = &WotlkDungeonOccActionContext::time_bomb_spread;
        }
    private:
        static Action* avoid_unstable_sphere(PlayerbotAI* ai) { return new AvoidUnstableSphereAction(ai); }
        static Action* mount_drake(PlayerbotAI* ai) { return new MountDrakeAction(ai); }
        static Action* dismount_drake(PlayerbotAI* ai) { return new DismountDrakeAction(ai); }
        static Action* occ_fly_drake(PlayerbotAI* ai) { return new OccFlyDrakeAction(ai); }
        static Action* occ_drake_attack(PlayerbotAI* ai) { return new OccDrakeAttackAction(ai); }
        static Action* avoid_arcane_explosion(PlayerbotAI* ai) { return new AvoidArcaneExplosionAction(ai); }
        static Action* time_bomb_spread(PlayerbotAI* ai) { return new TimeBombSpreadAction(ai); }
};

#endif
