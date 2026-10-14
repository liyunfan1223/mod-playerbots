/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "KaraStrategy.h"
#include "AiObjectContext.h"
#include "KaraHelpers.h"
#include "KaraMultipliers.h"
#include "Playerbots.h"

void RaidKarazhanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // General
    triggers.push_back(new TriggerNode("karazhan bot is not in combat",
        { NextAction("karazhan reset encounter states", ACTION_EMERGENCY + 10) }
    ));
    triggers.push_back(new TriggerNode("karazhan enemies cast fear",
        { NextAction("karazhan set tremor totem", ACTION_RAID) }
    ));

    // Trash
    triggers.push_back(new TriggerNode("mana warp is about to explode",
        { NextAction("mana warp stun creature before warp breach", ACTION_EMERGENCY + 6) }
    ));

    // Attumen the Huntsman
    triggers.push_back(new TriggerNode("attumen the huntsman phase one active",
        { NextAction("attumen the huntsman handle phase one", ACTION_RAID) }
    ));
    triggers.push_back(new TriggerNode("attumen the huntsman phase two active",
        { NextAction("attumen the huntsman handle phase two", ACTION_RAID) }
    ));
    triggers.push_back(new TriggerNode("attumen the huntsman phase transition",
        { NextAction("attumen the huntsman set dps timer", ACTION_EMERGENCY + 10) }
    ));

    // Moroes
    triggers.push_back(new TriggerNode("moroes should prioritize adds",
        { NextAction("moroes mark target", ACTION_RAID) }
    ));

    // Maiden of Virtue
    triggers.push_back(new TriggerNode("maiden of virtue boss engaged by tanks",
        { NextAction("maiden of virtue tank position boss", ACTION_RAID) }
    ));
    triggers.push_back(new TriggerNode("maiden of virtue holy wrath deals chain damage",
        { NextAction("maiden of virtue position ranged between pillars", ACTION_RAID + 1) }
    ));
    triggers.push_back(new TriggerNode("maiden of virtue grounding totem consumes holy fire",
        { NextAction("maiden of virtue set grounding totem", ACTION_RAID) }
    ));

    // The Big Bad Wolf
    triggers.push_back(new TriggerNode("big bad wolf boss is chasing little red riding hood",
        { NextAction("big bad wolf little red riding hood run away", ACTION_EMERGENCY + 6) }
    ));
    triggers.push_back(new TriggerNode("big bad wolf boss engaged by tank",
        { NextAction("big bad wolf position boss", ACTION_RAID) }
    ));

    // Romulo and Julianne
    triggers.push_back(new TriggerNode("romulo and julianne both bosses revived",
        { NextAction("romulo and julianne mark target", ACTION_RAID) }
    ));

    // The Wizard of Oz
    triggers.push_back(new TriggerNode("wizard of oz need target priority",
        { NextAction("wizard of oz mark target", ACTION_RAID) }
    ));
    triggers.push_back(new TriggerNode("wizard of oz strawman is vulnerable to fire",
        { NextAction("wizard of oz scorch strawman", ACTION_RAID + 1) }
    ));

    // The Curator
    triggers.push_back(new TriggerNode("the curator astral flare spawned",
        { NextAction("the curator mark astral flare", ACTION_RAID + 1) }
    ));
    triggers.push_back(new TriggerNode("the curator boss engaged by tanks",
        { NextAction("the curator position boss", ACTION_RAID) }
    ));
    triggers.push_back(new TriggerNode("the curator boss engaged by ranged",
        { NextAction("the curator spread ranged", ACTION_RAID) }
    ));

    // Terestian Illhoof
    triggers.push_back(new TriggerNode("terestian illhoof should prioritize chains",
        { NextAction("terestian illhoof mark target", ACTION_RAID) }
    ));

    // Shade of Aran
    triggers.push_back(new TriggerNode("shade of aran arcane explosion is casting",
        { NextAction("shade of aran run away from arcane explosion", ACTION_EMERGENCY + 6) }
    ));
    triggers.push_back(new TriggerNode("shade of aran flame wreath is active",
        { NextAction("shade of aran stop moving during flame wreath", ACTION_EMERGENCY + 7) }
    ));
    triggers.push_back(new TriggerNode("shade of aran conjured elementals summoned",
        { NextAction("shade of aran mark conjured elemental", ACTION_RAID) }
    ));
    triggers.push_back(new TriggerNode("shade of aran boss casts counterspell nearby",
        { NextAction("shade of aran ranged maintain distance", ACTION_RAID + 1) }
    ));

    // Netherspite
    triggers.push_back(new TriggerNode("netherspite red beam is active",
        { NextAction("netherspite block red beam", ACTION_EMERGENCY + 6) }
    ));
    triggers.push_back(new TriggerNode("netherspite blue beam is active",
        { NextAction("netherspite block blue beam", ACTION_EMERGENCY + 6) }
    ));
    triggers.push_back(new TriggerNode("netherspite green beam is active",
        { NextAction("netherspite block green beam", ACTION_EMERGENCY + 6) }
    ));
    triggers.push_back(new TriggerNode("netherspite bot is not beam blocker",
        { NextAction("netherspite avoid beam and void zone", ACTION_EMERGENCY + 6) }
    ));
    triggers.push_back(new TriggerNode("netherspite boss is banished",
        { NextAction("netherspite banish phase avoid void zone", ACTION_EMERGENCY + 1) }
    ));
    triggers.push_back(new TriggerNode("netherspite should manage timers and trackers",
        { NextAction("netherspite manage timers and trackers", ACTION_EMERGENCY + 10) }
    ));

    // Prince Malchezaar
    triggers.push_back(new TriggerNode("prince malchezaar bot is enfeebled",
        { NextAction("prince malchezaar enfeebled bot avoid hazard", ACTION_EMERGENCY + 6) }
    ));
    triggers.push_back(new TriggerNode("prince malchezaar engaged by non-tanks",
        { NextAction("prince malchezaar non-tank avoid infernal", ACTION_EMERGENCY + 1) }
    ));
    triggers.push_back(new TriggerNode("prince malchezaar boss engaged by tanks",
        { NextAction("prince malchezaar tanks position boss", ACTION_EMERGENCY + 6) }
    ));

    // Nightbane
    triggers.push_back(new TriggerNode("nightbane boss engaged by tanks",
        { NextAction("nightbane ground phase tanks position boss", ACTION_RAID) }
    ));
    triggers.push_back(new TriggerNode("nightbane ground phase engaged by ranged",
        { NextAction("nightbane ground phase coordinate ranged movement", ACTION_EMERGENCY + 1) }
    ));
    triggers.push_back(new TriggerNode("nightbane pets ignore collision to chase flying boss",
        { NextAction("nightbane control pet aggression", ACTION_RAID + 1) }
    ));
    triggers.push_back(new TriggerNode("nightbane boss is flying",
        { NextAction("nightbane flight phase stack and move", ACTION_RAID) }
    ));
    triggers.push_back(new TriggerNode("nightbane bot went out of bounds",
        { NextAction("nightbane teleport back to terrace", ACTION_EMERGENCY + 9) }
    ));
    triggers.push_back(new TriggerNode("nightbane should manage timers and trackers",
        { NextAction("nightbane manage timers and trackers", ACTION_EMERGENCY + 10) }
    ));
}

void RaidKarazhanStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new KarazhanSetTremorTotemMultiplier(botAI));
    multipliers.push_back(new AttumenTheHuntsmanDisableAutomaticTargetingMultiplier(botAI));
    multipliers.push_back(new AttumenTheHuntsmanStayStackedMultiplier(botAI));
    multipliers.push_back(new AttumenTheHuntsmanWaitForDpsMultiplier(botAI));
    multipliers.push_back(new MaidenOfVirtueDisableCombatFormationMoveMultiplier(botAI));
    multipliers.push_back(new MaidenOfVirtueSetGroundingTotemMultiplier(botAI));
    multipliers.push_back(new TheCuratorDisableTankAssistMultiplier(botAI));
    multipliers.push_back(new TheCuratorDisableCombatFormationMoveMultiplier(botAI));
    multipliers.push_back(new TheCuratorDelayBloodlustAndHeroismMultiplier(botAI));
    multipliers.push_back(new TerestianIllhoofDontDotFiendishImpsMultiplier(botAI));
    multipliers.push_back(new ShadeOfAranArcaneExplosionRunAwayMultiplier(botAI));
    multipliers.push_back(new ShadeOfAranFlameWreathDisableMovementMultiplier(botAI));
    multipliers.push_back(new NetherspiteKeepBlockingBeamMultiplier(botAI));
    multipliers.push_back(new NetherspiteWaitForDpsMultiplier(botAI));
    multipliers.push_back(new PrinceMalchezaarEnfeebleMultiplier(botAI));
    multipliers.push_back(new PrinceMalchezaarDelayBloodlustAndHeroismMultiplier(botAI));
    multipliers.push_back(new NightbaneDisablePetsMultiplier(botAI));
    multipliers.push_back(new NightbaneWaitForDpsMultiplier(botAI));
    multipliers.push_back(new NightbaneDisableAvoidAoeMultiplier(botAI));
    multipliers.push_back(new NightbaneDisableMovementMultiplier(botAI));
}

void RaidKarazhanStrategy::AppendTargetExclusions(
    GuidSet& exclusions, TargetValueExclusionType /*type*/)
{
    AiObjectContext* context = botAI->GetAiObjectContext();
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (nightbane && nightbane->GetPositionZ() > KaraHelpers::NIGHTBANE_FLIGHT_Z)
        exclusions.insert(nightbane->GetGUID());
}
