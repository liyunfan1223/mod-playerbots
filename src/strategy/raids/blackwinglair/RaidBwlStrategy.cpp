#include "RaidBwlStrategy.h"

#include "Strategy.h"

void RaidBwlStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("often",
        NextAction::array(0, new NextAction("bwl check onyxia scale cloak", ACTION_RAID), NULL)));

    triggers.push_back(new TriggerNode("bwl suppression device",
                        NextAction::array(0, new NextAction("bwl turn off suppression device", ACTION_RAID), NULL)));

    triggers.push_back(new TriggerNode("bwl affliction bronze",
        NextAction::array(0, new NextAction("bwl use hourglass sand", ACTION_RAID), NULL)));
}
