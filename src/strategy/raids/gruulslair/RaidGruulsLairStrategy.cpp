#include "RaidGruulsLairStrategy.h"
#include "RaidGruulsLairMultipliers.h"

void RaidGruulsLairStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("high king maulgar", NextAction::array(0,
        new NextAction("high king maulgar whirlwind run away", ACTION_EMERGENCY + 6),
        new NextAction("high king maulgar hunter misdirection", ACTION_RAID + 2),
        new NextAction("high king maulgar banish felstalker", ACTION_RAID + 2),
        new NextAction("high king maulgar maulgar tank", ACTION_RAID + 1),
        new NextAction("high king maulgar olm tank", ACTION_RAID + 1),
        new NextAction("high king maulgar blindeye tank", ACTION_RAID + 1),
        new NextAction("high king maulgar krosh mage tank", ACTION_RAID + 1),
        new NextAction("high king maulgar kiggler moonkin tank", ACTION_RAID + 1),
        new NextAction("high king maulgar melee dps", ACTION_RAID + 1),
        new NextAction("high king maulgar ranged dps", ACTION_RAID + 1),
        new NextAction("high king maulgar healer avoidance", ACTION_RAID + 1),
        nullptr)));

    triggers.push_back(new TriggerNode("gruul the dragonkiller", NextAction::array(0,
        new NextAction("gruul the dragonkiller shatter spread", ACTION_EMERGENCY + 6),
        new NextAction("gruul the dragonkiller position boss", ACTION_RAID + 1),
        new NextAction("gruul the dragonkiller spread ranged", ACTION_RAID + 1),
        nullptr)));
}

void RaidGruulsLairStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new HighKingMaulgarMultiplier(botAI));
    multipliers.push_back(new GruulTheDragonkillerMultiplier(botAI));
}
