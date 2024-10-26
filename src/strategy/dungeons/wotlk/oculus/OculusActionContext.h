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
            creators["fly drake"] = &WotlkDungeonOccActionContext::fly_drake;
            creators["drake attack"] = &WotlkDungeonOccActionContext::drake_attack;
            creators["avoid arcane explosion"] = &WotlkDungeonOccActionContext::avoid_arcane_explosion;
            creators["time bomb spread"] = &WotlkDungeonOccActionContext::time_bomb_spread;
        }
    private:
        static Action* avoid_unstable_sphere(PlayerbotAI* ai) { return new AvoidUnstableSphereAction(ai); }
        static Action* mount_drake(PlayerbotAI* ai) { return new MountDrakeAction(ai); }
        static Action* dismount_drake(PlayerbotAI* ai) { return new DismountDrakeAction(ai); }
        static Action* fly_drake(PlayerbotAI* ai) { return new FlyDrakeAction(ai); }
        static Action* drake_attack(PlayerbotAI* ai) { return new DrakeAttackAction(ai); }
        static Action* avoid_arcane_explosion(PlayerbotAI* ai) { return new AvoidArcaneExplosionAction(ai); }
        static Action* time_bomb_spread(PlayerbotAI* ai) { return new TimeBombSpreadAction(ai); }
};

#endif
