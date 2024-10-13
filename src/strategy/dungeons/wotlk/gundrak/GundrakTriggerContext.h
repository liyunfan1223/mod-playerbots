#ifndef _PLAYERBOT_WOTLKDUNGEONGDTRIGGERCONTEXT_H
#define _PLAYERBOT_WOTLKDUNGEONGDTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "AiObjectContext.h"
#include "GundrakTriggers.h"

class WotlkDungeonGDTriggerContext : public NamedObjectContext<Trigger> 
{
    public:
        WotlkDungeonGDTriggerContext()
        {
            creators["poison nova"] = &WotlkDungeonGDTriggerContext::poison_nova;
            creators["whirling slash"] = &WotlkDungeonGDTriggerContext::whirling_slash;
        }
    private:
        static Trigger* poison_nova(PlayerbotAI* ai) { return new SladranPoisonNovaTrigger(ai); }
        static Trigger* whirling_slash(PlayerbotAI* ai) { return new GaldarahWhirlingSlashTrigger(ai); }
};

#endif
