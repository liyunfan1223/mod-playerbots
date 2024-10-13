#include "GundrakStrategy.h"
#include "GundrakMultipliers.h"


void WotlkDungeonGDStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    // Moorabi

    // Drakkari Colossus

    // Slad'ran
    // TODO: Might need to add target priority for heroic on the snakes or to burn down boss.
    // Will re-test in heroic, decent dps groups should be able to blast him down with no funky strats.
    triggers.push_back(new TriggerNode("poison nova",
        NextAction::array(0, new NextAction("avoid poison nova", ACTION_RAID + 5), nullptr)));
    
    // Gal'darah
    triggers.push_back(new TriggerNode("whirling slash",
        NextAction::array(0, new NextAction("avoid whirling slash", ACTION_RAID + 5), nullptr)));
    
    // Eck the Ferocious (Heroic only)
    // TODO
}

void WotlkDungeonGDStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    multipliers.push_back(new SladranMultiplier(botAI));
    multipliers.push_back(new GaldarahMultiplier(botAI));
}
