#ifndef _PLAYERBOT_WOTLKDUNGEONUPTRIGGERCONTEXT_H
#define _PLAYERBOT_WOTLKDUNGEONUPTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "AiObjectContext.h"
#include "UtgardePinnacleTriggers.h"

class WotlkDungeonUPTriggerContext : public NamedObjectContext<Trigger> 
{
    public:
        WotlkDungeonUPTriggerContext()
        {
            creators["freezing cloud"] = &WotlkDungeonUPTriggerContext::freezing_cloud;
            creators["skadi whirlwind"] = &WotlkDungeonUPTriggerContext::whirlwind;
            creators["ymiron bane"] = &WotlkDungeonUPTriggerContext::bane;
        }
    private:
        static Trigger* freezing_cloud(PlayerbotAI* ai) { return new SkadiFreezingCloudTrigger(ai); }
        static Trigger* whirlwind(PlayerbotAI* ai) { return new SkadiWhirlwindTrigger(ai); }
        static Trigger* bane(PlayerbotAI* ai) { return new YmironBaneTrigger(ai); }
};

#endif
