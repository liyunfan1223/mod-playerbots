#ifndef _PLAYERBOT_RAIDAQ20STRATEGY_H
#define _PLAYERBOT_RAIDAQ20STRATEGY_H

#include "AiObjectContext.h"
#include "Multiplier.h"
#include "Strategy.h"

class RaidAq20Strategy : public Strategy
{
public:
    RaidAq20Strategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "aq20"; }
    virtual void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    // virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
