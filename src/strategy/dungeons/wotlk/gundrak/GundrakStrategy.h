#ifndef _PLAYERBOT_WOTLKDUNGEONGDSTRATEGY_H
#define _PLAYERBOT_WOTLKDUNGEONGDSTRATEGY_H

#include "Multiplier.h"
#include "AiObjectContext.h"
#include "Strategy.h"


class WotlkDungeonGDStrategy : public Strategy
{
public:
    WotlkDungeonGDStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "gundrak"; }
    virtual void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
