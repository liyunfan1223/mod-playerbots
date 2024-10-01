#ifndef _PLAYERBOT_WOTLKDUNGEONUKSTRATEGY_H_
#define _PLAYERBOT_WOTLKDUNGEONUKSTRATEGY_H_

#include "Multiplier.h"
#include "AiObjectContext.h"
#include "Strategy.h"


class WotlkDungeonUKStrategy : public Strategy
{
public:
    WotlkDungeonUKStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "utgarde keep"; }
    virtual void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
