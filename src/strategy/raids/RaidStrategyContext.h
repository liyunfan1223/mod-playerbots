#ifndef _PLAYERBOT_RAIDSTRATEGYCONTEXT_H_
#define _PLAYERBOT_RAIDSTRATEGYCONTEXT_H_

#include "Strategy.h"
#include "RaidNaxxStrategy.h"

class RaidStrategyContext : public NamedObjectContext<Strategy>
{
    public:
        RaidStrategyContext() : NamedObjectContext<Strategy>(false, true)
        {
            creators["naxx"] = &RaidStrategyContext::naxx;
        }
    private:
        static Strategy* naxx(PlayerbotAI* botAI) { return new RaidNaxxStrategy(botAI); }
};

#endif