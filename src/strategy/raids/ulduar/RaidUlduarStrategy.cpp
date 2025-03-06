#include "RaidUlduarStrategy.h"

#include "RaidUlduarMultipliers.h"

void RaidUlduarStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    //
    // Flame Leviathan
    //
    triggers.push_back(new TriggerNode(
        "flame leviathan vehicle near",
        NextAction::array(0, new NextAction("flame leviathan enter vehicle", ACTION_RAID + 2), nullptr)));

    triggers.push_back(new TriggerNode(
        "flame leviathan on vehicle",
        NextAction::array(0, new NextAction("flame leviathan vehicle", ACTION_RAID + 1), nullptr)));

    //
    // Razorscale
    //
    triggers.push_back(new TriggerNode(
        "razorscale avoid devouring flames",
        NextAction::array(0, new NextAction("razorscale avoid devouring flames", ACTION_RAID + 1), nullptr)));

    triggers.push_back(new TriggerNode(
        "razorscale avoid sentinel",
        NextAction::array(0, new NextAction("razorscale avoid sentinel", ACTION_RAID + 2), nullptr)));
    
    triggers.push_back(new TriggerNode(
        "razorscale flying alone",
    NextAction::array(0, new NextAction("razorscale ignore flying alone", ACTION_MOVE + 5), nullptr)));

    triggers.push_back(new TriggerNode(
        "razorscale avoid whirlwind",
        NextAction::array(0, new NextAction("razorscale avoid whirlwind", ACTION_RAID + 3), nullptr)));

    triggers.push_back(new TriggerNode(
        "razorscale grounded",
        NextAction::array(0, new NextAction("razorscale grounded", ACTION_RAID), nullptr)));

    triggers.push_back(new TriggerNode(
        "razorscale harpoon trigger",
        NextAction::array(0, new NextAction("razorscale harpoon action", ACTION_MOVE), nullptr)));

    triggers.push_back(new TriggerNode(
        "razorscale fuse armor trigger",
        NextAction::array(0, new NextAction("razorscale fuse armor action", ACTION_RAID + 2), nullptr)));

    //
    // Hodir
    //
    triggers.push_back(new TriggerNode(
        "hodir near snowpacked icicle",
        NextAction::array(0, new NextAction("hodir move snowpacked icicle", ACTION_RAID + 5), nullptr)));
    triggers.push_back(new TriggerNode(
        "hodir biting cold", NextAction::array(0, new NextAction("intense cold jump", ACTION_RAID + 4), nullptr)));
    
}

void RaidUlduarStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new FlameLeviathanMultiplier(botAI));
}
