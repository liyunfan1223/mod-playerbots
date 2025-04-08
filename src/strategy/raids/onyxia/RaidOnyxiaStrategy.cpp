#include "RaidOnyxiaStrategy.h"

void RaidOnyxiaStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // ----------- Phase 1 (100% - 65%) -----------
    triggers.push_back(new TriggerNode(
        "ony phase1 start", NextAction::array(0, new NextAction("ony position tank", ACTION_RAID + 5), nullptr)));

    triggers.push_back(new TriggerNode(
        "ony near tail", NextAction::array(0, new NextAction("ony move to side", ACTION_RAID + 2), nullptr)));

    // ----------- Phase 2 (65% - 40%) -----------
    triggers.push_back(
        new TriggerNode("ony deep breath warning",
                        NextAction::array(0, new NextAction("ony move to safe zone", ACTION_EMERGENCY + 5), nullptr)));

    triggers.push_back(
        new TriggerNode("ony fireball splash incoming",
                        NextAction::array(0, new NextAction("ony spread out", ACTION_MOVE + 2), nullptr)));

    // ----------- Phase 3 (40% - 0%) -----------
    triggers.push_back(new TriggerNode(
        "ony phase3 start", NextAction::array(0, new NextAction("ony position tank", ACTION_RAID + 5), nullptr)));

    triggers.push_back(new TriggerNode(
        "ony lava pulse", NextAction::array(0, new NextAction("ony avoid lava", ACTION_MOVE + 3), nullptr)));
}
