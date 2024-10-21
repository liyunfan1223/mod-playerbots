#include "UtgardePinnacleStrategy.h"
#include "UtgardePinnacleMultipliers.h"


void WotlkDungeonUPStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    // Svala Sorrowgrave

    // Gortok Palehoof

    // Skadi the Ruthless
    // TODO: Harpoons launchable via GameObject. For now players should do them
    triggers.push_back(new TriggerNode("freezing cloud",
        NextAction::array(0, new NextAction("avoid freezing cloud", ACTION_RAID + 5), nullptr)));
    triggers.push_back(new TriggerNode("skadi whirlwind",
        NextAction::array(0, new NextAction("avoid skadi whirlwind", ACTION_RAID + 4), nullptr)));
    
    // King Ymiron
    // May need to avoid orb.. unclear if the generic avoid AoE does this well
}

void WotlkDungeonUPStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    multipliers.push_back(new SkadiMultiplier(botAI));
}
