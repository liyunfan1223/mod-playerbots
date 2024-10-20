#include "HallsOfLightningStrategy.h"
#include "HallsOfLightningMultipliers.h"


void WotlkDungeonHoLStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    // General Bjarngrim
    triggers.push_back(new TriggerNode("stormforged lieutenant",
        NextAction::array(0, new NextAction("bjarngrim target", ACTION_RAID + 5), nullptr)));
    triggers.push_back(new TriggerNode("whirlwind",
        NextAction::array(0, new NextAction("avoid whirlwind", ACTION_RAID + 4), nullptr)));

    // Volkhan
    triggers.push_back(new TriggerNode("volkhan",
        NextAction::array(0, new NextAction("volkhan target", ACTION_RAID + 5), nullptr)));
    
    // Ionar
    triggers.push_back(new TriggerNode("ionar disperse",
        NextAction::array(0, new NextAction("disperse position", ACTION_MOVE + 5), nullptr)));
    triggers.push_back(new TriggerNode("ionar tank aggro",
        NextAction::array(0, new NextAction("ionar tank position", ACTION_MOVE + 4), nullptr)));
    triggers.push_back(new TriggerNode("static overload",
        NextAction::array(0, new NextAction("static overload spread", ACTION_MOVE + 3), nullptr)));
    // TODO: Targeted player can dodge the ball, but a single player soaking it isn't too bad to heal
    triggers.push_back(new TriggerNode("ball lightning",
        NextAction::array(0, new NextAction("ball lightning spread", ACTION_MOVE + 2), nullptr)));
    
    // Loken
    triggers.push_back(new TriggerNode("lightning nova",
        NextAction::array(0, new NextAction("avoid lightning nova", ACTION_MOVE + 5), nullptr)));
    triggers.push_back(new TriggerNode("loken ranged",
        NextAction::array(0, new NextAction("loken stack", ACTION_MOVE + 4), nullptr)));
}

void WotlkDungeonHoLStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    multipliers.push_back(new BjarngrimMultiplier(botAI));
    multipliers.push_back(new VolkhanMultiplier(botAI));
    multipliers.push_back(new IonarMultiplier(botAI));
    multipliers.push_back(new LokenMultiplier(botAI));
}
