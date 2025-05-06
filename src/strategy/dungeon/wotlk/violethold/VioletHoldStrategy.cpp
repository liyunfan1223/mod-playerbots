#include "VioletHoldStrategy.h"
#include "VioletHoldMultipliers.h"


void WotlkDungeonVHStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    // Erekem
    // This boss has many purgable buffs, purging/dispels could be merged into generic strats though
    triggers.push_back(new TriggerNode("erekem target",
        NextAction::array(0, new NextAction("attack erekem", ACTION_RAID + 1), nullptr)));
    
    // Moragg
    // TODO: This guy has Optic Link which may require moving, add if needed

    // Ichoron
    triggers.push_back(new TriggerNode("ichoron target",
        NextAction::array(0, new NextAction("attack ichor globule", ACTION_RAID + 1), nullptr)));
    
    // Xevozz
    // TODO: Revisit in heroics, waypoints back and forth on stairs. Need to test with double beacon spawn

    // Lavanthor
    // Tank & spank

    // Zuramat the Obliterator
    triggers.push_back(new TriggerNode("shroud of darkness",
        NextAction::array(0, new NextAction("stop attack", ACTION_HIGH + 5), nullptr)));
    triggers.push_back(new TriggerNode("void shift",
        NextAction::array(0, new NextAction("attack void sentry", ACTION_RAID + 1), nullptr)));
    
    // Cyanigosa
    triggers.push_back(new TriggerNode("cyanigosa positioning",
        NextAction::array(0, new NextAction("rear flank", ACTION_MOVE + 5), nullptr)));
}

void WotlkDungeonVHStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    multipliers.push_back(new ErekemMultiplier(botAI));
    multipliers.push_back(new IchoronMultiplier(botAI));
    multipliers.push_back(new ZuramatMultiplier(botAI));
}
