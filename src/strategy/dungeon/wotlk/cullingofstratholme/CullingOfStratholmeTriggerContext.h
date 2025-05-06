#ifndef _PLAYERBOT_WOTLKDUNGEONCOSTRIGGERCONTEXT_H
#define _PLAYERBOT_WOTLKDUNGEONCOSTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "AiObjectContext.h"
#include "CullingOfStratholmeTriggers.h"

class WotlkDungeonCoSTriggerContext : public NamedObjectContext<Trigger> 
{
    public:
        WotlkDungeonCoSTriggerContext()
        {
            creators["explode ghoul"] = &WotlkDungeonCoSTriggerContext::explode_ghoul;
            creators["epoch ranged"] = &WotlkDungeonCoSTriggerContext::epoch_ranged;

        }
    private:
        static Trigger* explode_ghoul(PlayerbotAI* ai) { return new ExplodeGhoulTrigger(ai); }
        static Trigger* epoch_ranged(PlayerbotAI* ai) { return new EpochRangedTrigger(ai); }
};

#endif
