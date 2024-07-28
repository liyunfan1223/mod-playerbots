#include "RaidNaxxStrategy.h"

#include "RaidNaxxMultipliers.h"

void RaidNaxxStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Grobbulus
    triggers.push_back(new TriggerNode(
        "mutating injection",
        NextAction::array(0, new NextAction("grobbulus go behind the boss", ACTION_RAID + 2), nullptr)));

    triggers.push_back(
        new TriggerNode("mutating injection removed",
                        NextAction::array(0, new NextAction("grobbulus move center", ACTION_RAID + 1), nullptr)));

    triggers.push_back(new TriggerNode(
        "grobbulus cloud", NextAction::array(0, new NextAction("rotate grobbulus", ACTION_RAID + 1), nullptr)));

    // Heigan the Unclean
    triggers.push_back(new TriggerNode(
        "heigan melee", NextAction::array(0, new NextAction("heigan dance melee", ACTION_RAID + 1), nullptr)));

    triggers.push_back(new TriggerNode(
        "heigan ranged", NextAction::array(0, new NextAction("heigan dance ranged", ACTION_RAID + 1), nullptr)));

    // Kel'Thuzad
    triggers.push_back(new TriggerNode(
        "kel'thuzad", NextAction::array(0, new NextAction("kel'thuzad position", ACTION_RAID + 2),
                                        new NextAction("kel'thuzad choose target", ACTION_RAID + 1), nullptr)));

    // Anub'Rekhan
    triggers.push_back(new TriggerNode(
        "anub'rekhan", NextAction::array(0, new NextAction("anub'rekhan position", ACTION_RAID + 1), nullptr)));

    // Thaddius
    triggers.push_back(
        new TriggerNode("thaddius phase pet",
                        NextAction::array(0, new NextAction("thaddius attack nearest pet", ACTION_RAID + 1), nullptr)));

    triggers.push_back(new TriggerNode("thaddius phase pet lose aggro",
                                       NextAction::array(0, new NextAction("taunt spell", ACTION_RAID + 2), nullptr)));

    triggers.push_back(
        new TriggerNode("thaddius phase transition",
                        NextAction::array(0, new NextAction("thaddius move to platform", ACTION_RAID + 1), nullptr)));

    triggers.push_back(
        new TriggerNode("thaddius phase thaddius",
                        NextAction::array(0, new NextAction("thaddius move polarity", ACTION_RAID + 1), nullptr)));

    // Instructor Razuvious
    triggers.push_back(new TriggerNode(
        "razuvious tank",
        NextAction::array(0, new NextAction("razuvious use obedience crystal", ACTION_RAID + 1), nullptr)));

    triggers.push_back(new TriggerNode(
        "razuvious nontank", NextAction::array(0, new NextAction("razuvious target", ACTION_RAID + 1), nullptr)));

    // four horseman
    triggers.push_back(new TriggerNode(
        "horseman attractors",
        NextAction::array(0, new NextAction("horseman attract alternatively", ACTION_RAID + 1), nullptr)));

    triggers.push_back(
        new TriggerNode("horseman except attractors",
                        NextAction::array(0, new NextAction("horseman attack in order", ACTION_RAID + 1), nullptr)));

    // sapphiron
    triggers.push_back(
        new TriggerNode("sapphiron ground",
                        NextAction::array(0, new NextAction("sapphiron ground position", ACTION_RAID + 1), nullptr)));

    triggers.push_back(
        new TriggerNode("sapphiron flight",
                        NextAction::array(0, new NextAction("sapphiron flight position", ACTION_RAID + 1), nullptr)));

    // Gluth
    triggers.push_back(
        new TriggerNode("gluth", NextAction::array(0, new NextAction("gluth choose target", ACTION_RAID + 1),
                                                   new NextAction("gluth position", ACTION_RAID + 1),
                                                   new NextAction("gluth slowdown", ACTION_RAID), nullptr)));

    triggers.push_back(new TriggerNode("gluth main tank mortal wound",
                                       NextAction::array(0, new NextAction("taunt spell", ACTION_RAID + 1), nullptr)));

    // Loatheb
    triggers.push_back(new TriggerNode(
        "loatheb", NextAction::array(0, new NextAction("loatheb position", ACTION_RAID + 1),
                                     new NextAction("loatheb choose target", ACTION_RAID + 1), nullptr)));
}

void RaidNaxxStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new GrobbulusMultiplier(botAI));
    multipliers.push_back(new HeiganDanceMultiplier(botAI));
    multipliers.push_back(new LoathebGenericMultiplier(botAI));
    multipliers.push_back(new ThaddiusGenericMultiplier(botAI));
    multipliers.push_back(new SapphironGenericMultiplier(botAI));
    multipliers.push_back(new InstructorRazuviousGenericMultiplier(botAI));
    multipliers.push_back(new KelthuzadGenericMultiplier(botAI));
    multipliers.push_back(new AnubrekhanGenericMultiplier(botAI));
    multipliers.push_back(new FourhorsemanGenericMultiplier(botAI));
    // multipliers.push_back(new GothikGenericMultiplier(botAI));
    multipliers.push_back(new GluthGenericMultiplier(botAI));
}
