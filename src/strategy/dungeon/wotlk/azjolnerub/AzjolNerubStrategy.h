#ifndef _PLAYERBOT_WOTLKDUNGEONANSTRATEGY_H
#define _PLAYERBOT_WOTLKDUNGEONANSTRATEGY_H

#include "Multiplier.h"
#include "AiObjectContext.h"
#include "Strategy.h"


class WotlkDungeonANStrategy : public Strategy
{
public:
    WotlkDungeonANStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "azjol'nerub"; }
    virtual void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif
