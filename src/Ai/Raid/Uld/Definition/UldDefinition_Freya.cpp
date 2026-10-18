/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldDefinitions.h"

#include "BossAuraActions.h"
#include "BossAuraTriggers.h"
#include "BossResistanceMultipliers.h"
#include "PlayerbotAI.h"
#include "Strategy.h"
#include "UldActions_Freya.h"
#include "UldEncounterGate.h"
#include "UldEncounter_Freya.h"
#include "UldMultipliers_Freya.h"
#include "UldTriggers_Freya.h"

namespace Family = RaidEncounterRules::Family;
namespace Role = RaidEncounterRules::Role;

namespace
{
bool FreyaAlive(PlayerbotAI* botAI)
{
    Unit* freya = GetFreyaScan(botAI).Boss();
    return freya && freya->IsAlive();
}

bool FreyaOwnsTargets(PlayerbotAI* botAI) { return botAI->GetState() == BOT_STATE_COMBAT && FreyaAlive(botAI); }

void DefineFreya(EncounterBuilder& e)
{
    // Survival first, then the pacify counter, then targeting. A bot that is dead, blown up or
    // silenced contributes nothing to the trio wave it is being steered at.
    e.Node<FreyaNearNatureBombTrigger, FreyaMoveAwayNatureBombAction>(ACTION_RAID + 4);

    // The main tank answers a bomb by moving Freya, not itself. Bombs land at players' feet and the
    // melee stack is on the boss, so a volley buries her melee ring and the rest of the melee lose it.
    e.Node<FreyaTankNatureBombTrigger, FreyaTankNatureBombAction>(ACTION_RAID + 4);

    // The two 8 yd circles. Both sit with the other escapes, above the spore node, which would otherwise
    // walk the carrier straight back into the ball.
    e.Node<FreyaNaturesFuryBailTrigger, FreyaNaturesFuryBailAction>(ACTION_RAID + 4);
    e.Node<FreyaStepOutOfSunbeamTrigger, FreyaStepOutOfSunbeamAction>(ACTION_RAID + 4);

    // Detonating Lasher wave, which nothing can tank or outrun. Leave a blast first (a dead bot does no
    // CC), then root, snare, ghouls, and gather last, only when nothing urgent is asking.
    e.Node<FreyaLasherAboutToBlowTrigger, FreyaLasherAboutToBlowAction>(ACTION_RAID + 3);
    e.Node<FreyaFrostNovaLashersTrigger, FreyaFrostNovaLashersAction>(ACTION_RAID + 2);
    e.Node<FreyaTrapLashersTrigger, FreyaTrapLashersAction>(ACTION_RAID + 2);
    e.Node<FreyaSummonArmyTrigger, FreyaSummonArmyAction>(ACTION_RAID + 2);
    e.Node<FreyaRangedCampTrigger, FreyaRangedCampAction>(ACTION_RAID);

    // Conservator's Grip is raid-wide and cannot be outranged, so a spore outranks attacking: a
    // pacified bot cannot swing at anything anyway.
    e.Node<FreyaMoveToHealingSporeTrigger, FreyaMoveToHealingSporeAction>(ACTION_RAID + 2);

    e.Node<FreyaTankAddsTrigger, FreyaTankAddsAction>(ACTION_RAID + 1);

    // Freya walks after whoever holds her, so every tank escape moves her. Under the bomb escape, which
    // has to be able to leave the leash.
    e.Node<FreyaTankHoldFreyaTrigger, FreyaTankHoldFreyaAction>(ACTION_RAID + 1);

    e.Node<FreyaRedirectThreatTrigger, FreyaRedirectThreatAction>(ACTION_RAID + 1);
    e.Node<FreyaSetDpsPriorityTrigger, FreyaSetDpsPriorityAction>(ACTION_RAID);
    e.Node(
        "freya nature resistance trigger",
        [](PlayerbotAI* botAI) -> Trigger* { return new BossNatureResistanceTrigger(botAI, "freya"); },
        "freya nature resistance action",
        [](PlayerbotAI* botAI) -> Action* { return new BossNatureResistanceAction(botAI, "freya"); }, ACTION_RAID);
    e.Node(
        "freya fire resistance trigger",
        [](PlayerbotAI* botAI) -> Trigger* { return new BossFireResistanceTrigger(botAI, "freya"); },
        "freya fire resistance action",
        [](PlayerbotAI* botAI) -> Action* { return new BossFireResistanceAction(botAI, "freya"); }, ACTION_RAID);

    // Hard mode (config-gated): break out of Iron Roots and dodge the Unstable Sun Beam. Breaking the
    // root outranks the dodge: a rooted bot can't move, so it has to free itself before it can step out.
    e.Node<FreyaBreakIronRootsTrigger, FreyaBreakIronRootsAction>(ACTION_RAID + 5);
    e.Node<FreyaDodgeUnstableSunBeamTrigger, FreyaDodgeUnstableSunBeamAction>(ACTION_RAID + 4);
    e.Node<FreyaGroundTremorHoldCastTrigger, FreyaGroundTremorHoldCastAction>(ACTION_EMERGENCY + 2);

    // The generic picker would reclaim the trio split on alternating ticks. Tank assist is off for every
    // tank all fight: gating it on the ladder lets an off-tank drag a lasher wave into the raid stack.
    e.OwnTargeting("freya disable automatic targeting", Role::Dps, FreyaOwnsTargets, Family::DpsAssist);
    e.OwnTargeting("freya disable automatic targeting", Role::Tank, FreyaOwnsTargets, Family::TankAssist);

    e.Multiplier<FreyaTrioSyncMultiplier>(Family::Melee | Family::Spell);

    // Whether an action is AoE is its threat type, which any action can override.
    e.Multiplier<FreyaLasherFinishAoeMultiplier>(Family::AnyAction);

    e.Multiplier<FreyaLasherTrapReserveMultiplier>(Family::Spell);

    // Avoid AoE sits at ACTION_EMERGENCY above every Freya escape, and swaps their all-hazard answer for
    // a flat FleeDistance hop that clears none of them.
    e.Block("freya avoid aoe hold", Role::Any, FreyaAlive, Family::AvoidAoe);

    e.Multiplier<FreyaGroundTremorCastGateMultiplier>(Family::Spell);
    e.Multiplier<BossNatureAspectHoldMultiplier>(Family::Spell, "freya");
}
}  // namespace

EncounterDefinition const& UldFreyaDefinition()
{
    static EncounterDefinition const definition(ULD_BOSS_FREYA, &BossStateGateOpen, &DefineFreya);
    return definition;
}
