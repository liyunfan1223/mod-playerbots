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
#include "UldActions_Kologarn.h"
#include "UldEncounterGate.h"
#include "UldEncounter_Kologarn.h"
#include "UldTriggers_Kologarn.h"

namespace Family = RaidEncounterRules::Family;
namespace Role = RaidEncounterRules::Role;

namespace
{
// Engaged, not merely nearby: before the pull the per-role pickers are inert, so silencing the generic
// ones would leave the bot with none, and trash within 100yd of him would be unfightable.
bool KologarnOwnsTargets(PlayerbotAI* botAI)
{
    return botAI->GetState() != BOT_STATE_NON_COMBAT && KologarnEncounterActive(botAI);
}

bool KologarnStoneGripRide(PlayerbotAI* botAI) { return IsKologarnStoneGripped(botAI->GetBot()) && GetKologarn(botAI); }

void DefineKologarn(EncounterBuilder& e)
{
    // Targets are picked per role in code, with no raid icons: Skull means "everyone DPS this" and
    // Moon is the CC channel, so borrowing them for a per-role split leaks into the generic engine.
    e.Node<KologarnBodyTankTrigger, KologarnBodyTankAction>(ACTION_RAID);
    e.Node<KologarnOffTankTrigger, KologarnOffTankAction>(ACTION_RAID);
    e.Node<KologarnDpsTargetTrigger, KologarnDpsTargetAction>(ACTION_RAID);

    // Rubble outrun players, so the off-tank holds them clear of the raid instead of kiting.
    e.Node<KologarnRubbleTankTrigger, KologarnRubbleTankAction>(ACTION_RAID + 1);
    e.Node<KologarnRubbleSlowdownTrigger, KologarnRubbleSlowdownAction>(ACTION_RAID);

    // Overhead Smash stacks Crunch Armor on whoever holds the body; the pair trade it at 2 stacks.
    e.Node<KologarnSmashSwapTrigger, KologarnSmashSwapAction>(ACTION_RAID + 2);

    e.Node(
        "kologarn nature resistance trigger",
        [](PlayerbotAI* botAI) -> Trigger* { return new BossNatureResistanceTrigger(botAI, "kologarn"); },
        "kologarn nature resistance action",
        [](PlayerbotAI* botAI) -> Action* { return new BossNatureResistanceAction(botAI, "kologarn"); }, ACTION_RAID);
    e.Node<KologarnFallFromFloorTrigger, KologarnFallFromFloorAction>(ACTION_RAID + 1);

    // An uncovered body means Petrifying Breath on the whole raid, so this outranks the dodges.
    e.Node<KologarnBodyUncoveredTrigger, KologarnBodyUncoveredAction>(ACTION_EMERGENCY + 1);
    e.Node<KologarnEyebeamTrigger, KologarnEyebeamAction>(ACTION_EMERGENCY);

    // The rows above pick every bot's target. "dps target" falls back to a smart target with no icon set,
    // so it's never null and would fight them on alternating ticks.
    e.OwnTargeting("kologarn disable automatic targeting", Role::Any, KologarnOwnsTargets,
                   Family::DpsAssist | Family::TankAssist | Family::DebuffOnAttacker);

    // A gripped bot is a stunned passenger on the right arm until the arm lets go or it dies. Movement
    // orders only fight the ride and leave it facing the wrong way when it drops.
    e.Exclusive("kologarn", Role::Any, KologarnStoneGripRide);

    e.Multiplier<BossNatureAspectHoldMultiplier>(Family::Spell, "kologarn");
}
}  // namespace

EncounterDefinition const& UldKologarnDefinition()
{
    static EncounterDefinition const definition(ULD_BOSS_KOLOGARN, &BossStateGateOpen, &DefineKologarn);
    return definition;
}
