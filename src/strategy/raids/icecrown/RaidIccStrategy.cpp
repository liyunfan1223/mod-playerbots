#include "RaidIccStrategy.h"

#include "Strategy.h"

void RaidIccStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "icc gunship cannon near",
        NextAction::array(0, new NextAction("icc gunship enter cannon", ACTION_RAID + 2), nullptr)));

    triggers.push_back(
        new TriggerNode("icc in cannon",
                        NextAction::array(0, new NextAction("icc cannon fire", ACTION_RAID), nullptr)));
}