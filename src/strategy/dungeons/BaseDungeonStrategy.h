#ifndef _PLAYERBOT_BASEDUNGEONSTRATEGY_H
#define _PLAYERBOT_BASEDUNGEONSTRATEGY_H

#include "Strategy.h"

class BaseDungeonStrategy : public Strategy
{
public:
    BaseDungeonStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "base dungeon"; }
    virtual void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
