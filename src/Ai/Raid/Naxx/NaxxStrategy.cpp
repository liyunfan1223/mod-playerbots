/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "NaxxStrategy.h"
#include "NaxxMultipliers.h"

void RaidNaxxStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Grobbulus
    triggers.push_back(new TriggerNode("mutating injection melee",
        { NextAction("grobbulus move away", ACTION_RAID + 2) }
    ));

    triggers.push_back(new TriggerNode("mutating injection ranged",
        { NextAction("grobbulus go behind the boss", ACTION_RAID + 2) }
    ));

    triggers.push_back(new TriggerNode("mutating injection removed",
        { NextAction("grobbulus move center", ACTION_RAID + 1) }
    ));

    triggers.push_back(new TriggerNode("grobbulus cloud",
        { NextAction("rotate grobbulus", ACTION_RAID + 1) }
    ));

    // Heigan the Unclean
    triggers.push_back(new TriggerNode("heigan melee",
        { NextAction("heigan dance melee", ACTION_RAID + 1) }
    ));

    triggers.push_back(new TriggerNode("heigan ranged",
        { NextAction("heigan dance ranged", ACTION_RAID + 1) }
    ));

    // Kel'Thuzad
    triggers.push_back(
        new TriggerNode("kel'thuzad",
        {
            NextAction("kel'thuzad position", ACTION_RAID + 2),
            NextAction("kel'thuzad choose target", ACTION_RAID + 1)
        })
    );

    // Anub'Rekhan
    triggers.push_back(new TriggerNode("anub'rekhan",
        { NextAction("anub'rekhan position", ACTION_RAID + 1) }
    ));

     // Grand Widow Faerlina
     triggers.push_back(new TriggerNode("faerlina",
        { NextAction("avoid aoe", ACTION_RAID + 1) }
    ));

    // Maexxna
    triggers.push_back(
        new TriggerNode("maexxna",
        {
            NextAction("rear flank", ACTION_RAID + 1),
            NextAction("avoid aoe", ACTION_RAID + 1)
        })
    );

    // Patchwerk
    //triggers.push_back(new TriggerNode("patchwerk tank",
    //    { NextAction("tank face", ACTION_RAID + 2) }
    //));

    //triggers.push_back(new TriggerNode("patchwerk ranged",
    //    { NextAction("patchwerk ranged position", ACTION_RAID + 2) }
    //));

    //triggers.push_back(new TriggerNode("patchwerk non-tank",
    //    { NextAction("rear flank", ACTION_RAID + 1) }
    //));

    // Thaddius
    triggers.push_back(new TriggerNode("thaddius phase pet",
        { NextAction("thaddius attack nearest pet", ACTION_RAID + 1) }
    ));

    triggers.push_back(new TriggerNode("thaddius phase pet lose aggro",
        { NextAction("taunt spell", ACTION_RAID + 2) }
    ));

    triggers.push_back(new TriggerNode("thaddius phase transition",
        { NextAction("thaddius move to platform", ACTION_RAID + 1) }
    ));

    triggers.push_back(new TriggerNode("thaddius phase thaddius",
        { NextAction("thaddius move polarity", ACTION_RAID + 1) }
    ));

    // Instructor Razuvious
    triggers.push_back(new TriggerNode("razuvious tank",
        { NextAction("razuvious use obedience crystal", ACTION_RAID + 1) }
    ));

    triggers.push_back(new TriggerNode("razuvious nontank",
        { NextAction("razuvious target", ACTION_RAID + 1) }
    ));

    // four horsemen
    triggers.push_back(new TriggerNode("four horsemen attractors",
        { NextAction("four horsemen attract alternatively", ACTION_RAID + 1) }
    ));

    triggers.push_back(new TriggerNode("four horsemen except attractors",
        { NextAction("four horsemen attack in order", ACTION_RAID + 1) }
    ));

    // sapphiron
    triggers.push_back(new TriggerNode("sapphiron ground",
        { NextAction("sapphiron ground position", ACTION_RAID + 1) }
    ));

    triggers.push_back(new TriggerNode("sapphiron flight",
        { NextAction("sapphiron flight position", ACTION_RAID + 1) }
    ));

    // Gluth
    triggers.push_back(
        new TriggerNode("gluth",
        {
            NextAction("gluth choose target", ACTION_RAID + 1),
            NextAction("gluth position", ACTION_RAID + 1),
            NextAction("gluth slowdown", ACTION_RAID)
        })
    );

    triggers.push_back(new TriggerNode("gluth main tank mortal wound",
        { NextAction("taunt spell", ACTION_RAID + 1) }
    ));

    // Loatheb
    triggers.push_back(
        new TriggerNode("loatheb",
        {
            NextAction("loatheb position", ACTION_RAID + 1),
            NextAction("loatheb choose target", ACTION_RAID + 1)
        })
    );

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
    multipliers.push_back(new FourHorsemenGenericMultiplier(botAI));
    // multipliers.push_back(new GothikGenericMultiplier(botAI));
    multipliers.push_back(new GluthGenericMultiplier(botAI));
}
