#ifndef _PLAYERBOT_RAIDSTRATEGYCONTEXT_H_
#define _PLAYERBOT_RAIDSTRATEGYCONTEXT_H_

#include "RaidAq20Strategy.h"
#include "RaidBwlStrategy.h"
#include "RaidEoEStrategy.h"
#include "RaidIccStrategy.h"
#include "RaidMcStrategy.h"
#include "RaidNaxxStrategy.h"
#include "RaidOnyxiaStrategy.h"
#include "RaidOsStrategy.h"
#include "RaidUlduarStrategy.h"
#include "RaidVoAStrategy.h"
#include "Strategy.h"

class RaidStrategyContext : public NamedObjectContext<Strategy>
{
public:
    RaidStrategyContext() : NamedObjectContext<Strategy>(false, true)
    {
        // TODO should we give these prefixes (eg: "naxx" -> "raid naxx")? because if we don't it's going to end up
        // very crowded (with possible conflicts) once we have strats for all raids and some dungeons
        // (mc already very similiar to nc)
        creators["mc"] = &RaidStrategyContext::mc;
        creators["bwl"] = &RaidStrategyContext::bwl;
        creators["aq20"] = &RaidStrategyContext::aq20;
        creators["naxx"] = &RaidStrategyContext::naxx;
        creators["wotlk-os"] = &RaidStrategyContext::wotlk_os;
        creators["wotlk-eoe"] = &RaidStrategyContext::wotlk_eoe;
        creators["voa"] = &RaidStrategyContext::voa;
        creators["uld"] = &RaidStrategyContext::uld;
        creators["icc"] = &RaidStrategyContext::icc;
        creators["onyxia"] = &RaidStrategyContext::onyxia;
    }

private:
    static Strategy* mc(PlayerbotAI* botAI) { return new RaidMcStrategy(botAI); }
    static Strategy* bwl(PlayerbotAI* botAI) { return new RaidBwlStrategy(botAI); }
    static Strategy* aq20(PlayerbotAI* botAI) { return new RaidAq20Strategy(botAI); }
    static Strategy* naxx(PlayerbotAI* botAI) { return new RaidNaxxStrategy(botAI); }
    static Strategy* wotlk_os(PlayerbotAI* botAI) { return new RaidOsStrategy(botAI); }
    static Strategy* wotlk_eoe(PlayerbotAI* botAI) { return new RaidEoEStrategy(botAI); }
    static Strategy* voa(PlayerbotAI* botAI) { return new RaidVoAStrategy(botAI); }
    static Strategy* uld(PlayerbotAI* botAI) { return new RaidUlduarStrategy(botAI); }
    static Strategy* icc(PlayerbotAI* botAI) { return new RaidIccStrategy(botAI); }
    static Strategy* onyxia(PlayerbotAI* botAI) { return new RaidOnyxiaStrategy(botAI); }
};

#endif
