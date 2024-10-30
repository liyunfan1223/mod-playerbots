#ifndef _PLAYERBOT_WOTLKDUNGEONFOSSTRATEGY_H
#define _PLAYERBOT_WOTLKDUNGEONFOSSTRATEGY_H
#include "Multiplier.h"
#include "Strategy.h"

class WotlkDungeonFosStrategy : public Strategy
{
public:
    WotlkDungeonFosStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    std::string const getName() override { return "fos"; }
    void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    void InitMultipliers(std::vector<Multiplier*> &multipliers) override;

};

#endif  // !_PLAYERBOT_WOTLKDUNGEONFOSSTRATEGY_H
