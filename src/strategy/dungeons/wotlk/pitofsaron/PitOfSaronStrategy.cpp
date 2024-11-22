#include "PitOfSaronStrategy.h"
#include "PitOfSaronMultipliers.h"

void WotlkDungeonPoSStrategy::InitTriggers(std::vector<TriggerNode*>& triggers) {
    triggers.push_back(
        new TriggerNode("move from ick pursuit",
                        NextAction::array(0, new NextAction("move from ick pursuit", ACTION_MOVE + 5), nullptr)));
    triggers.push_back(
        new TriggerNode("move from poison nova",
                        NextAction::array(0, new NextAction("move from poison nova", ACTION_MOVE + 5), nullptr)));
    triggers.push_back(
        new TriggerNode("move from exploding orb",
                        NextAction::array(0, new NextAction("move from exploding orb", ACTION_MOVE + 5), nullptr)));
}

void WotlkDungeonPoSStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new IckMultiplier(botAI));
}
