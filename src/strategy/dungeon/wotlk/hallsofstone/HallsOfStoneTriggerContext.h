#ifndef _PLAYERBOT_WOTLKDUNGEONHOSTRIGGERCONTEXT_H
#define _PLAYERBOT_WOTLKDUNGEONHOSTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "AiObjectContext.h"
#include "HallsOfStoneTriggers.h"

class WotlkDungeonHoSTriggerContext : public NamedObjectContext<Trigger> 
{
    public:
        WotlkDungeonHoSTriggerContext()
        {
            creators["ground slam"] = &WotlkDungeonHoSTriggerContext::ground_slam;
            creators["lightning ring"] = &WotlkDungeonHoSTriggerContext::lightning_ring;
        }
    private:
        static Trigger* ground_slam(PlayerbotAI* ai) { return new KrystallusGroundSlamTrigger(ai); }
        static Trigger* lightning_ring(PlayerbotAI* ai) { return new SjonnirLightningRingTrigger(ai); }
};

#endif
