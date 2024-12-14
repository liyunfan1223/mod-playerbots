
#ifndef _PLAYERBOT_ASSASSINATIONROGUESTRATEGY_H
#define _PLAYERBOT_ASSASSINATIONROGUESTRATEGY_H

#include "MeleeCombatStrategy.h"

class AssassinationRogueStrategy : public MeleeCombatStrategy
{
public:
    AssassinationRogueStrategy(PlayerbotAI* ai);

public:
    virtual void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    virtual std::string const getName() override { return "melee"; }
    virtual NextAction** getDefaultActions() override;
    uint32 GetType() const override { return MeleeCombatStrategy::GetType() | STRATEGY_TYPE_DPS; }
};

#endif