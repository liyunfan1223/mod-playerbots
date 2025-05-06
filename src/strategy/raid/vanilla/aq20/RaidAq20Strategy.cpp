#include "RaidAq20Strategy.h"

#include "Strategy.h"

void RaidAq20Strategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("aq20 move to crystal",
            NextAction::array(0, new NextAction("aq20 use crystal", ACTION_RAID), nullptr)));

}
