#ifndef _PLAYERBOT_RAIDEOESTRATEGY_H
#define _PLAYERBOT_RAIDEOESTRATEGY_H

#include "AiObjectContext.h"
#include "Multiplier.h"
#include "Strategy.h"

class RaidEoEStrategy : public Strategy
{
public:
    RaidEoEStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "wotlk-eoe"; }
    virtual void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
