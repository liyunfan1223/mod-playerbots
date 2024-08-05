
#ifndef _PLAYERBOT_RAIDNAXXSTRATEGY_H
#define _PLAYERBOT_RAIDNAXXSTRATEGY_H

#include "AiObjectContext.h"
#include "Multiplier.h"
#include "RaidNaxxScripts.h"
#include "Strategy.h"

class RaidNaxxStrategy : public Strategy
{
public:
    RaidNaxxStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "naxx"; }
    virtual void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif