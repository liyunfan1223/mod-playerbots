
// RaidOnyxiaStrategy.h
#ifndef _PLAYERBOT_RAIDONYXIASTRATEGY_H_
#define _PLAYERBOT_RAIDONYXIASTRATEGY_H_

#include "Strategy.h"

class RaidOnyxiaStrategy : public Strategy
{
public:
    RaidOnyxiaStrategy(PlayerbotAI* ai) : Strategy(ai) {}

    const std::string& getName() const;

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;

private:
    static const std::string name;
};

#endif
