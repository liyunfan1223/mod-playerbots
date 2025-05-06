#ifndef _PLAYERBOT_WOTLKDUNGEONOCCSTRATEGY_H
#define _PLAYERBOT_WOTLKDUNGEONOCCSTRATEGY_H

#include "Multiplier.h"
#include "AiObjectContext.h"
#include "Strategy.h"


class WotlkDungeonOccStrategy : public Strategy
{
public:
    WotlkDungeonOccStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "oculus"; }
    virtual void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
