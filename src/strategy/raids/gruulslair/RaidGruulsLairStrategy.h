#ifndef _PLAYERBOT_RAIDGRUULSLAIRSTRATEGY_H
#define _PLAYERBOT_RAIDGRUULSLAIRSTRATEGY_H

#include "Strategy.h"
#include "Multiplier.h"

class RaidGruulsLairStrategy : public Strategy
{
public:
    RaidGruulsLairStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}
    
    std::string const getName() override { return "gruulslair"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
