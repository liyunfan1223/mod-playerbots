#ifndef _PLAYERBOT_WOTLKDUNGEONOCCTRIGGERCONTEXT_H
#define _PLAYERBOT_WOTLKDUNGEONOCCTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "AiObjectContext.h"
#include "OculusTriggers.h"

class WotlkDungeonOccTriggerContext : public NamedObjectContext<Trigger> 
{
    public:
        WotlkDungeonOccTriggerContext()
        {
            creators["unstable sphere"] = &WotlkDungeonOccTriggerContext::unstable_sphere;
            creators["drake mount"] = &WotlkDungeonOccTriggerContext::drake_mount;
            creators["drake dismount"] = &WotlkDungeonOccTriggerContext::drake_dismount;
            creators["group flying"] = &WotlkDungeonOccTriggerContext::group_flying;
            creators["drake combat"] = &WotlkDungeonOccTriggerContext::drake_combat;
            creators["varos cloudstrider"] = &WotlkDungeonOccTriggerContext::varos_cloudstrider;
            creators["arcane explosion"] = &WotlkDungeonOccTriggerContext::arcane_explosion;
            creators["time bomb"] = &WotlkDungeonOccTriggerContext::time_bomb;
        }
    private:
        static Trigger* unstable_sphere(PlayerbotAI* ai) { return new DrakosUnstableSphereTrigger(ai); }
        static Trigger* drake_mount(PlayerbotAI* ai) { return new DrakeMountTrigger(ai); }
        static Trigger* drake_dismount(PlayerbotAI* ai) { return new DrakeDismountTrigger(ai); }
        static Trigger* group_flying(PlayerbotAI* ai) { return new GroupFlyingTrigger(ai); }
        static Trigger* drake_combat(PlayerbotAI* ai) { return new DrakeCombatTrigger(ai); }
        static Trigger* varos_cloudstrider(PlayerbotAI* ai) { return new VarosCloudstriderTrigger(ai); }
        static Trigger* arcane_explosion(PlayerbotAI* ai) { return new UromArcaneExplosionTrigger(ai); }
        static Trigger* time_bomb(PlayerbotAI* ai) { return new UromTimeBombTrigger(ai); }
};

#endif
