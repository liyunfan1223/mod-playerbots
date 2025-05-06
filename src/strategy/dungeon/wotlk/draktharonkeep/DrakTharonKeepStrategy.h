#ifndef _PLAYERBOT_WOTLKDUNGEONDTKSTRATEGY_H
#define _PLAYERBOT_WOTLKDUNGEONDTKSTRATEGY_H

#include "Multiplier.h"
#include "AiObjectContext.h"
#include "Strategy.h"


class WotlkDungeonDTKStrategy : public Strategy
{
public:
    WotlkDungeonDTKStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "drak'tharon keep"; }
    virtual void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
