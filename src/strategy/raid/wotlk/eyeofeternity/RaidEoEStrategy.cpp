#include "RaidEoEStrategy.h"
#include "RaidEoEMultipliers.h"
#include "Strategy.h"

void RaidEoEStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("malygos",
        NextAction::array(0, new NextAction("malygos position", ACTION_MOVE), nullptr)));
    triggers.push_back(new TriggerNode("malygos",
        NextAction::array(0, new NextAction("malygos target", ACTION_RAID + 1), nullptr)));
    // triggers.push_back(new TriggerNode("power spark",
    //     NextAction::array(0, new NextAction("pull power spark", ACTION_RAID + 2), nullptr)));
    // triggers.push_back(new TriggerNode("power spark",
    //     NextAction::array(0, new NextAction("kill power spark", ACTION_RAID + 3), nullptr)));

    triggers.push_back(new TriggerNode("group flying",
        NextAction::array(0, new NextAction("eoe fly drake", ACTION_NORMAL + 1), nullptr)));
    triggers.push_back(new TriggerNode("drake combat",
        NextAction::array(0, new NextAction("eoe drake attack", ACTION_NORMAL + 5), nullptr)));
}

void RaidEoEStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    multipliers.push_back(new MalygosMultiplier(botAI));
}
