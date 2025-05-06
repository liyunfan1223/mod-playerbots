#include "DrakTharonKeepStrategy.h"
#include "DrakTharonKeepMultipliers.h"


void WotlkDungeonDTKStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    // Trollgore
    triggers.push_back(new TriggerNode("corpse explode",
        NextAction::array(0, new NextAction("corpse explode spread", ACTION_MOVE + 5), nullptr)));
    
    // Novos the Summoner
    // TODO: Can be improved - it's a pretty easy fight but complex to program, revisit if needed
    triggers.push_back(new TriggerNode("arcane field",
        NextAction::array(0, new NextAction("avoid arcane field", ACTION_MOVE + 5), nullptr)));
    triggers.push_back(new TriggerNode("arcane field",
        NextAction::array(0, new NextAction("novos positioning", ACTION_MOVE + 4), nullptr)));
    triggers.push_back(new TriggerNode("arcane field",
        NextAction::array(0, new NextAction("novos target priority", ACTION_NORMAL + 1), nullptr)));
    
    // King Dred
    // TODO: Fear ward / tremor totem, or general anti-fear strat development

    //The Prophet Tharon'ja
    triggers.push_back(new TriggerNode("gift of tharon'ja",
        NextAction::array(0, new NextAction("touch of life", ACTION_NORMAL + 5), nullptr)));
    triggers.push_back(new TriggerNode("gift of tharon'ja",
        NextAction::array(0, new NextAction("bone armor", ACTION_NORMAL + 4), nullptr)));
    // Run ranged chars (who would normally stand at range) into melee, to dps in skeleton form
    triggers.push_back(new TriggerNode("tharon'ja out of melee",
        NextAction::array(0, new NextAction("reach melee", ACTION_NORMAL + 3), nullptr)));
    triggers.push_back(new TriggerNode("gift of tharon'ja",
        NextAction::array(0, new NextAction("taunt", ACTION_NORMAL + 2), nullptr)));
    triggers.push_back(new TriggerNode("gift of tharon'ja",
        NextAction::array(0, new NextAction("slaying strike", ACTION_NORMAL + 2), nullptr)));
}

void WotlkDungeonDTKStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    multipliers.push_back(new NovosMultiplier(botAI));
    multipliers.push_back(new TharonjaMultiplier(botAI));
}
