#include "PitOfSaronStrategy.h"
#include "PitOfSaronMultipliers.h"

void WotlkDungeonPoSStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("ick and krick",
        NextAction::array(0, new NextAction("ick and krick", ACTION_RAID + 5), nullptr)));

    triggers.push_back(new TriggerNode("tyrannus",
        NextAction::array(0, new NextAction("tyrannus", ACTION_RAID + 5), nullptr)));
}

void WotlkDungeonPoSStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new IckAndKrickMultiplier(botAI));
    //multipliers.push_back(new AttackFragmentMultiplier(botAI));
}
