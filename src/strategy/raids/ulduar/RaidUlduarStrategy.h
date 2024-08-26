
#ifndef _PLAYERBOT_RAIDULDUARSTRATEGY_H
#define _PLAYERBOT_RAIDULDUARSTRATEGY_H

#include "AiObjectContext.h"
#include "Multiplier.h"
#include "Strategy.h"

class RaidUlduarStrategy : public Strategy
{
public:
    RaidUlduarStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "uld"; }
    virtual void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif