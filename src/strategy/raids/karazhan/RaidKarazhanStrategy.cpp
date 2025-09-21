#include "RaidKarazhanStrategy.h"
#include "RaidKarazhanMultipliers.h"

void RaidKarazhanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "karazhan attumen the huntsman", NextAction::array(0,
        new NextAction("karazhan attumen the huntsman stack behind", ACTION_RAID + 1),
        nullptr)));

    triggers.push_back(new TriggerNode(
        "karazhan moroes", NextAction::array(0,
        new NextAction("karazhan moroes mark target", ACTION_RAID + 1),
        nullptr)));

    triggers.push_back(new TriggerNode(
        "karazhan maiden of virtue", NextAction::array(0,
        new NextAction("karazhan maiden of virtue position ranged", ACTION_RAID + 1),
        new NextAction("karazhan maiden of virtue position boss", ACTION_RAID + 1),
        nullptr)));

    triggers.push_back(new TriggerNode(
        "karazhan big bad wolf", NextAction::array(0,
        new NextAction("karazhan big bad wolf run away", ACTION_EMERGENCY + 6),
        new NextAction("karazhan big bad wolf position boss", ACTION_RAID + 1),
        nullptr)));

    triggers.push_back(new TriggerNode(
        "karazhan romulo and julianne", NextAction::array(0,
        new NextAction("karazhan romulo and julianne mark target", ACTION_RAID + 1),
        nullptr)));

    triggers.push_back(new TriggerNode(
        "karazhan wizard of oz", NextAction::array(0,
        new NextAction("karazhan wizard of oz scorch strawman", ACTION_RAID + 2),
        new NextAction("karazhan wizard of oz mark target", ACTION_RAID + 1),
        nullptr)));

    triggers.push_back(new TriggerNode(
        "karazhan the curator", NextAction::array(0,
        new NextAction("karazhan the curator spread ranged", ACTION_RAID + 2),
        new NextAction("karazhan the curator position boss", ACTION_RAID + 2),
        new NextAction("karazhan the curator mark target", ACTION_RAID + 1),
        nullptr)));

    triggers.push_back(new TriggerNode(
        "karazhan terestian illhoof", NextAction::array(0,
        new NextAction("karazhan terestian illhoof mark target", ACTION_RAID + 1),
        nullptr)));

    triggers.push_back(new TriggerNode(
        "karazhan shade of aran", NextAction::array(0,
        new NextAction("karazhan shade of aran flame wreath stop movement", ACTION_EMERGENCY + 7),
        new NextAction("karazhan shade of aran arcane explosion run away", ACTION_EMERGENCY + 6),
        new NextAction("karazhan shade of aran spread ranged", ACTION_RAID + 2),
        new NextAction("karazhan shade of aran mark conjured elemental", ACTION_RAID + 1),
        nullptr)));

    triggers.push_back(new TriggerNode(
        "karazhan netherspite", NextAction::array(0,
        new NextAction("karazhan netherspite block red beam", ACTION_EMERGENCY + 8),
        new NextAction("karazhan netherspite block blue beam", ACTION_EMERGENCY + 8),
        new NextAction("karazhan netherspite block green beam", ACTION_EMERGENCY + 8),
        new NextAction("karazhan netherspite avoid beam and void zone", ACTION_EMERGENCY + 7),
        new NextAction("karazhan netherspite banish phase avoid void zone", ACTION_RAID + 1),
        nullptr)));

    triggers.push_back(new TriggerNode(
        "karazhan prince malchezaar", NextAction::array(0,
        new NextAction("karazhan prince malchezaar non tank avoid hazard", ACTION_EMERGENCY + 6),
        new NextAction("karazhan prince malchezaar tank avoid hazard", ACTION_EMERGENCY + 6),
        nullptr)));
}

void RaidKarazhanStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new KarazhanShadeOfAranMultiplier(botAI));
    multipliers.push_back(new KarazhanNetherspiteBlueAndGreenBeamMultiplier(botAI));
    multipliers.push_back(new KarazhanNetherspiteRedBeamMultiplier(botAI));
    multipliers.push_back(new KarazhanPrinceMalchezaarMultiplier(botAI));
}
