#include "RaidOsStrategy.h"
#include "RaidOsMultipliers.h"
#include "Strategy.h"

void RaidOsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("sartharion tank",
                        NextAction::array(0, new NextAction("sartharion tank position", ACTION_MOVE), nullptr)));
    triggers.push_back(
        new TriggerNode("twilight fissure",
                        NextAction::array(0, new NextAction("avoid twilight fissure", ACTION_RAID + 2), nullptr)));
    triggers.push_back(
        new TriggerNode("flame tsunami",
                        NextAction::array(0, new NextAction("avoid flame tsunami", ACTION_RAID + 1), nullptr)));
    triggers.push_back(
        new TriggerNode("sartharion dps",
                        NextAction::array(0, new NextAction("sartharion attack priority", ACTION_RAID), nullptr)));
    // Flank dragon positioning
    triggers.push_back(new TriggerNode("sartharion melee positioning",
        NextAction::array(0, new NextAction("rear flank", ACTION_MOVE + 4), nullptr)));
    
    triggers.push_back(new TriggerNode("twilight portal enter",
        NextAction::array(0, new NextAction("enter twilight portal", ACTION_RAID + 1), nullptr)));
    triggers.push_back(new TriggerNode("twilight portal exit",
        NextAction::array(0, new NextAction("exit twilight portal", ACTION_RAID + 1), nullptr)));
}

void RaidOsStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    multipliers.push_back(new SartharionMultiplier(botAI));
}
