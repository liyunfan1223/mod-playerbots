#ifndef _PLAYERBOT_WOTLKDUNGEONTOCTRIGGERCONTEXT_H
#define _PLAYERBOT_WOTLKDUNGEONTOCTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "AiObjectContext.h"
#include "TrialOfTheChampionTriggers.h"

class WotlkDungeonToCTriggerContext : public NamedObjectContext<Trigger> 
{
    public:
        WotlkDungeonToCTriggerContext()
        {
            creators["toc lance"] = &WotlkDungeonToCTriggerContext::toc_lance;
            creators["toc ue lance"] = &WotlkDungeonToCTriggerContext::toc_ue_lance;
            creators["toc mount near"] = &WotlkDungeonToCTriggerContext::toc_mount_near;
            creators["toc mounted"] = &WotlkDungeonToCTriggerContext::toc_mounted;
        }
    private:
        static Trigger* toc_lance(PlayerbotAI* ai) { return new ToCLanceTrigger(ai); }
        static Trigger* toc_ue_lance(PlayerbotAI* ai) { return new ToCUELanceTrigger(ai); }
        static Trigger* toc_mount_near(PlayerbotAI* ai) { return new ToCMountNearTrigger(ai); }
        static Trigger* toc_mounted(PlayerbotAI* ai) { return new ToCMountedTrigger(ai); }
};

#endif
