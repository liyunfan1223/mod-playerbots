#ifndef _PLAYERBOT_RAIDSTRATEGYCONTEXT_H_
#define _PLAYERBOT_RAIDSTRATEGYCONTEXT_H_

#include "RaidUlduarStrategy.h"
#include "Strategy.h"
#include "RaidBwlStrategy.h"
#include "RaidNaxxStrategy.h"

class RaidStrategyContext : public NamedObjectContext<Strategy>
{
public:
    RaidStrategyContext() : NamedObjectContext<Strategy>(false, true)
    {
        creators["naxx"] = &RaidStrategyContext::naxx;
        creators["bwl"] = &RaidStrategyContext::bwl;
        creators["uld"] = &RaidStrategyContext::uld;
    }

private:
    static Strategy* naxx(PlayerbotAI* botAI) { return new RaidNaxxStrategy(botAI); }
    static Strategy* bwl(PlayerbotAI* botAI) { return new RaidBwlStrategy(botAI); }
    static Strategy* uld(PlayerbotAI* botAI) { return new RaidUlduarStrategy(botAI); }
};

#endif