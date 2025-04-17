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
    // Iron Assembly
    //
    triggers.push_back(new TriggerNode(
        "iron assembly lightning tendrils trigger",
        NextAction::array(0, new NextAction("iron assembly lightning tendrils action", ACTION_RAID), nullptr)));

    triggers.push_back(new TriggerNode(
        "iron assembly overload trigger",
        NextAction::array(0, new NextAction("iron assembly overload action", ACTION_RAID), nullptr)));

    //
    // Kologarn
    //
    triggers.push_back(new TriggerNode(
        "kologarn fall from floor trigger",
        NextAction::array(0, new NextAction("kologarn fall from floor action", ACTION_RAID + 1), nullptr)));

    triggers.push_back(new TriggerNode(
        "kologarn mark dps target trigger",
        NextAction::array(0, new NextAction("kologarn mark dps target action", ACTION_RAID), nullptr)));

    triggers.push_back(new TriggerNode(
        "kologarn nature resistance trigger",
        NextAction::array(0, new NextAction("kologarn nature resistance action", ACTION_RAID), nullptr)));

    //
    // Hodir
    //
    triggers.push_back(new TriggerNode(
        "hodir near snowpacked icicle",
        NextAction::array(0, new NextAction("hodir move snowpacked icicle", ACTION_RAID + 1), nullptr)));

    triggers.push_back(new TriggerNode(
        "hodir biting cold",
        NextAction::array(0, new NextAction("hodir biting cold jump", ACTION_RAID), nullptr)));

    //
    // Freya
    //
    triggers.push_back(new TriggerNode(
        "freya tank near eonars gift",
        NextAction::array(0, new NextAction("freya mark eonars gift", ACTION_RAID + 1), nullptr)));

    triggers.push_back(new TriggerNode(
        "freya near nature bomb",
        NextAction::array(0, new NextAction("freya move away nature bomb", ACTION_RAID), nullptr)));
}

void RaidUlduarStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new FlameLeviathanMultiplier(botAI));
}
