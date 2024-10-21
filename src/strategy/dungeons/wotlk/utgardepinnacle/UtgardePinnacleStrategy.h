#ifndef _PLAYERBOT_WOTLKDUNGEONUPSTRATEGY_H
#define _PLAYERBOT_WOTLKDUNGEONUPSTRATEGY_H

#include "Multiplier.h"
#include "AiObjectContext.h"
#include "Strategy.h"


class WotlkDungeonUPStrategy : public Strategy
{
public:
    WotlkDungeonUPStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "utgarde pinnacle"; }
    virtual void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
