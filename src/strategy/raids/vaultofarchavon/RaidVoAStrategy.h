
#ifndef _PLAYERBOT_RAIDVOASTRATEGY_H
#define _PLAYERBOT_RAIDVOASTRATEGY_H

#include "Strategy.h"
#include "PlayerbotAI.h"
#include "string"
#include "Trigger.h"
#include "vector"

class RaidVoAStrategy : public Strategy
{
public:
    RaidVoAStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "voa"; }
    virtual void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
