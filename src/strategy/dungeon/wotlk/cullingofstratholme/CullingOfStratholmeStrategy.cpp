#include "CullingOfStratholmeStrategy.h"
#include "CullingOfStratholmeMultipliers.h"


void WotlkDungeonCoSStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    // Meathook
    // Can tank this in a fixed position to allow healer to LoS the stun, probably not necessary

    // Salramm the Fleshcrafter
    triggers.push_back(new TriggerNode("explode ghoul",
        NextAction::array(0, new NextAction("explode ghoul spread", ACTION_MOVE + 5), nullptr)));
    
    // Chrono-Lord Epoch
    // Not sure if this actually works, I think I've seen him charge melee characters..?
    triggers.push_back(new TriggerNode("epoch ranged",
        NextAction::array(0, new NextAction("epoch stack", ACTION_MOVE + 5), nullptr)));
    
    // Mal'Ganis

    // Infinite Corruptor (Heroic only)
}

void WotlkDungeonCoSStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    multipliers.push_back(new EpochMultiplier(botAI));
}
