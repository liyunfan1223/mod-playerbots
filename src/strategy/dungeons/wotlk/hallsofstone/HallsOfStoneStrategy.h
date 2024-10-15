#ifndef _PLAYERBOT_WOTLKDUNGEONHOSSTRATEGY_H
#define _PLAYERBOT_WOTLKDUNGEONHOSSTRATEGY_H

#include "Multiplier.h"
#include "AiObjectContext.h"
#include "Strategy.h"


class WotlkDungeonHoSStrategy : public Strategy
{
public:
    WotlkDungeonHoSStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "halls of stone"; }
    virtual void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
