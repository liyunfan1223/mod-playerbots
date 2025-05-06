#ifndef _PLAYERBOT_WOTLKDUNGEONHOLSTRATEGY_H
#define _PLAYERBOT_WOTLKDUNGEONHOLSTRATEGY_H

#include "Multiplier.h"
#include "AiObjectContext.h"
#include "Strategy.h"


class WotlkDungeonHoLStrategy : public Strategy
{
public:
    WotlkDungeonHoLStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "halls of lightning"; }
    virtual void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
