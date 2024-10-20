#include "HallsOfStoneStrategy.h"
#include "HallsOfStoneMultipliers.h"


void WotlkDungeonHoSStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    // Maiden of Grief
    // TODO: Jump into damage during shock of sorrow?

    // Krystallus
    // TODO: I think bots need to dismiss pets on this, or they nuke players they are standing close to
    triggers.push_back(new TriggerNode("ground slam",
        NextAction::array(0, new NextAction("shatter spread", ACTION_RAID + 5), nullptr)));
    
    // Tribunal of Ages
    // Seems fine, maybe add focus targeting strat if needed on heroic.
    // Main issue is dps will immediately rambo in and sometimes die before tank gets aggro,
    // this is mostly an issue with the bot AI as they do it on every fight

    // Sjonnir The Ironshaper
    // Possibly tank in place in the middle of the room, assign a dps to adds?
    triggers.push_back(new TriggerNode("lightning ring",
        NextAction::array(0, new NextAction("avoid lightning ring", ACTION_RAID + 5), nullptr)));
}

void WotlkDungeonHoSStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    multipliers.push_back(new KrystallusMultiplier(botAI));
    multipliers.push_back(new SjonnirMultiplier(botAI));
}
