#include "RaidMcStrategy.h"

#include "Strategy.h"

void RaidMcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("mc living bomb debuff",
                        NextAction::array(0, new NextAction("mc check should move from group", ACTION_RAID), nullptr)));
    triggers.push_back(
        new TriggerNode("mc baron geddon inferno",
                        NextAction::array(0, new NextAction("mc move from baron geddon", ACTION_RAID), nullptr)));
}
