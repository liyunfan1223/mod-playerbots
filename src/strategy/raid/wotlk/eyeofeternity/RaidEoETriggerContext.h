#ifndef _PLAYERBOT_RAIDEOETRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDEOETRIGGERCONTEXT_H

#include "AiObjectContext.h"
#include "NamedObjectContext.h"
#include "RaidEoETriggers.h"

class RaidEoETriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidEoETriggerContext()
    {
        creators["malygos"] = &RaidEoETriggerContext::malygos;
        creators["power spark"] = &RaidEoETriggerContext::power_spark;
    }

private:
    static Trigger* power_spark(PlayerbotAI* ai) { return new PowerSparkTrigger(ai); }
    static Trigger* malygos(PlayerbotAI* ai) { return new MalygosTrigger(ai); }
};

#endif
