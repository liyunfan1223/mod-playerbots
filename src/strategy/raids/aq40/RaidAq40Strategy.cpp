#include "RaidAq40Strategy.h"

#include "Strategy.h"

void RaidAq40Strategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("aq40 should use resistance buffs",
                        NextAction::array(0, new NextAction("aq40 use resistance buffs", ACTION_RAID), nullptr)));

    triggers.push_back(
        new TriggerNode("aq40 has emperor aggro",
                        NextAction::array(0, new NextAction("aq40 move from other emperor", ACTION_EMERGENCY), nullptr)));

    triggers.push_back(
        new TriggerNode("aq40 warlock tank emperor",
                        NextAction::array(0, new NextAction("searing pain", ACTION_RAID), nullptr)));

    triggers.push_back(
        new TriggerNode("aq40 mage frostbolt viscidus", // would be rank 1, ideally.. supplying "frostbolt(rank 1)" seems to not work
                        NextAction::array(0, new NextAction("frostbolt", ACTION_RAID), nullptr)));

    triggers.push_back(
        new TriggerNode("aq40 melee viscidus",
                        NextAction::array(0, new NextAction("aq40 melee viscidus", ACTION_RAID + 1), nullptr)));

    triggers.push_back(
        new TriggerNode("aq40 emperor fight",
                        NextAction::array(0, new NextAction("aq40 decide emperor action", ACTION_RAID), nullptr)));

    triggers.push_back(
        new TriggerNode("aq40 ouro burrowed",
                        NextAction::array(0, new NextAction("aq40 ouro burrowed flee", ACTION_RAID), nullptr)));

    triggers.push_back(
        new TriggerNode("aq40 cthun1 started",
                        NextAction::array(0, new NextAction("aq40 cthun1 get positioned", ACTION_RAID), nullptr)));

    triggers.push_back(
        new TriggerNode("aq40 cthun2 started",
                        NextAction::array(0, new NextAction("aq40 cthun2 get positioned", ACTION_RAID), nullptr)));
}
