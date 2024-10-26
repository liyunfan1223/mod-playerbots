#include "OculusStrategy.h"
#include "OculusMultipliers.h"


void WotlkDungeonOccStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    // Drakos the Interrogator
    // TODO: May need work, TBA.
    triggers.push_back(new TriggerNode("unstable sphere",
        NextAction::array(0, new NextAction("avoid unstable sphere", ACTION_MOVE + 5), nullptr)));
    
    // DRAKES
    triggers.push_back(new TriggerNode("drake mount",
        NextAction::array(0, new NextAction("mount drake", ACTION_RAID + 5), nullptr)));
    triggers.push_back(new TriggerNode("drake dismount",
        NextAction::array(0, new NextAction("dismount drake", ACTION_RAID + 5), nullptr)));
    triggers.push_back(new TriggerNode("group flying",
        NextAction::array(0, new NextAction("fly drake", ACTION_NORMAL + 1), nullptr)));
    triggers.push_back(new TriggerNode("drake combat",
        NextAction::array(0, new NextAction("drake attack", ACTION_NORMAL + 5), nullptr)));

    // Varos Cloudstrider
    // Seems to be no way to identify the marked cores, may need to hook boss AI..
    // triggers.push_back(new TriggerNode("varos cloudstrider",
    //     NextAction::array(0, new NextAction("avoid energize cores", ACTION_RAID + 5), nullptr)));

    // Mage-Lord Urom
    triggers.push_back(new TriggerNode("arcane explosion",
        NextAction::array(0, new NextAction("avoid arcane explosion", ACTION_MOVE + 5), nullptr)));
    triggers.push_back(new TriggerNode("time bomb",
        NextAction::array(0, new NextAction("time bomb spread", ACTION_MOVE + 4), nullptr)));
    
    // Ley-Guardian Eregos
}

void WotlkDungeonOccStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    multipliers.push_back(new MountingDrakeMultiplier(botAI));
    multipliers.push_back(new FlyingMultiplier(botAI));
    multipliers.push_back(new UromMultiplier(botAI));
    multipliers.push_back(new EregosMultiplier(botAI));
}
