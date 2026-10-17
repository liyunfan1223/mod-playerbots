/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_OCACTIONCONTEXT_H
#define PLAYERBOTS_OCACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "OCActions.h"

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
        static Action* avoid_unstable_sphere(PlayerbotAI* botAI) { return new AvoidUnstableSphereAction(botAI); }
        static Action* mount_drake(PlayerbotAI* botAI) { return new MountDrakeAction(botAI); }
        static Action* dismount_drake(PlayerbotAI* botAI) { return new DismountDrakeAction(botAI); }
        static Action* occ_fly_drake(PlayerbotAI* botAI) { return new OccFlyDrakeAction(botAI); }
        static Action* occ_drake_attack(PlayerbotAI* botAI) { return new OccDrakeAttackAction(botAI); }
        static Action* avoid_arcane_explosion(PlayerbotAI* botAI) { return new AvoidArcaneExplosionAction(botAI); }
        static Action* time_bomb_spread(PlayerbotAI* botAI) { return new TimeBombSpreadAction(botAI); }
};

#endif
