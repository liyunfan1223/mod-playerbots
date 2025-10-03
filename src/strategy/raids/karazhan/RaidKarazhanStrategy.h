#ifndef _PLAYERBOT_RAIDKARAZHANSTRATEGY_H_
#define _PLAYERBOT_RAIDKARAZHANSTRATEGY_H_

#include "Strategy.h"
#include "Multiplier.h"

class RaidKarazhanStrategy : public Strategy
{
public:
    RaidKarazhanStrategy(PlayerbotAI* ai) : Strategy(ai) {}

    std::string const getName() override { return "karazhan"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
