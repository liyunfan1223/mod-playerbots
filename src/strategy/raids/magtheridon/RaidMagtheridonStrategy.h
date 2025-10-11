#ifndef _PLAYERBOT_RAIDMAGTHERIDONSTRATEGY_H
#define _PLAYERBOT_RAIDMAGTHERIDONSTRATEGY_H

#include "Strategy.h"
#include "Multiplier.h"

class RaidMagtheridonStrategy : public Strategy
{
public:
    RaidMagtheridonStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "magtheridon"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
