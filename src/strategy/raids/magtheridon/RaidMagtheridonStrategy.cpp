#include "RaidMagtheridonStrategy.h"
#include "RaidMagtheridonMultipliers.h"

void RaidMagtheridonStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("magtheridon", NextAction::array(0,
        new NextAction("magtheridon use manticron cube", ACTION_EMERGENCY + 10),
        new NextAction("magtheridon hellfire channeler warlock", ACTION_RAID + 3),
        new NextAction("magtheridon spread ranged", ACTION_RAID + 2),
        new NextAction("magtheridon spread healer", ACTION_RAID + 2),
        new NextAction("magtheridon hellfire channeler misdirection", ACTION_RAID + 2),
        new NextAction("magtheridon position boss", ACTION_RAID + 2),
        new NextAction("magtheridon hellfire channeler main tank", ACTION_RAID + 1),
        new NextAction("magtheridon hellfire channeler northwest channeler tank", ACTION_RAID + 1),
        new NextAction("magtheridon hellfire channeler northeast channeler tank", ACTION_RAID + 1),
        new NextAction("magtheridon hellfire channeler dps priority", ACTION_RAID + 1),
        nullptr)));
}

void RaidMagtheridonStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new MagtheridonMultiplier(botAI));
}
