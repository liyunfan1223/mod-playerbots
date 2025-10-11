#ifndef _PLAYERBOT_RAIDMAGTHERIDONTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDMAGTHERIDONTRIGGERCONTEXT_H

#include "RaidMagtheridonTriggers.h"
#include "AiObjectContext.h"

class RaidMagtheridonTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidMagtheridonTriggerContext() : NamedObjectContext<Trigger>()
    {
        creators["magtheridon"] = &RaidMagtheridonTriggerContext::magtheridon;
    }

private:
    static Trigger* magtheridon(PlayerbotAI* botAI) { return new MagtheridonTrigger(botAI); }
};

#endif
