#ifndef _PLAYERBOT_WOTLKDUNGEONOKSTRATEGY_H
#define _PLAYERBOT_WOTLKDUNGEONOKSTRATEGY_H

#include "Multiplier.h"
#include "AiObjectContext.h"
#include "Strategy.h"


class WotlkDungeonOKStrategy : public Strategy
{
public:
    WotlkDungeonOKStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "old kingdom"; }
    virtual void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
