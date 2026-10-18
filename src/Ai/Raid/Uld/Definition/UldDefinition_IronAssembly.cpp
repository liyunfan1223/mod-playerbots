/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldDefinitions.h"

#include "PlayerbotAI.h"
#include "Strategy.h"
#include "UldActions_IronAssembly.h"
#include "UldEncounterGate.h"
#include "UldEncounter_IronAssembly.h"
#include "UldMultipliers_IronAssembly.h"
#include "UldScripts.h"
#include "UldTriggers_IronAssembly.h"

namespace Family = RaidEncounterRules::Family;
namespace Role = RaidEncounterRules::Role;

namespace
{
bool IronAssemblyOwnsTargets(PlayerbotAI* botAI)
{
    return botAI->GetState() == BOT_STATE_COMBAT && IronAssemblyFormationActive(botAI);
}

// Only while this bot is actually committed to a hazard. Outside that window the generic movers are
// what bring it back to the formation, and the position row has already yielded.
bool IronAssemblyHazardMove(PlayerbotAI* botAI)
{
    return IronAssemblyFormationActive(botAI) && IronAssemblyMemberMustMove(botAI, botAI->GetBot());
}

// The whole channel, not just while inside: the escape parks at the clearance, and Charge and
// Intercept reach 25 yd from there, right back into the blast.
bool IronAssemblyBrundirHazard(PlayerbotAI* botAI)
{
    if (!IronAssemblyFormationActive(botAI))
        return false;

    Unit* brundir = GetIronAssemblyMember(botAI, NPC_BRUNDIR);
    return brundir && (IronAssemblyOverloadActive(brundir) || IronAssemblyTendrilsActive(brundir));
}

// Only a tank holding an assignment, which is the one that has a spot to be pulled off.
bool IronAssemblyTankPinned(PlayerbotAI* botAI)
{
    return IronAssemblyFormationActive(botAI) && IronAssemblyAssignedBoss(botAI, botAI->GetBot());
}

void DefineIronAssembly(EncounterBuilder& e)
{
    // First success ends the tick, so this is a survival order. Interrupt sits above every class interrupt
    // (ACTION_INTERRUPT, 40) since Whirl has no positional answer. Position goes last, yielding once parked.
    e.Node<IronAssemblyResetEncounterStateTrigger, IronAssemblyResetEncounterStateAction>(ACTION_EMERGENCY + 10);
    e.Node<IronAssemblyOverloadTrigger, IronAssemblyOverloadAction>(ACTION_EMERGENCY + 6, EncounterRow::Mover);
    e.Node<IronAssemblyLightningTendrilsTrigger, IronAssemblyLightningTendrilsAction>(ACTION_EMERGENCY + 6,
                                                                                     EncounterRow::Mover);
    e.Node<IronAssemblyRuneOfDeathTrigger, IronAssemblyRuneOfDeathAction>(ACTION_EMERGENCY + 5, EncounterRow::Mover);
    e.Node<IronAssemblyInterruptTrigger, IronAssemblyInterruptAction>(ACTION_EMERGENCY + 4);
    e.Node<IronAssemblyTankAssignmentTrigger, IronAssemblyTankAssignmentAction>(ACTION_RAID + 6);
    e.Node<IronAssemblyShieldOfRunesTrigger, IronAssemblyShieldOfRunesAction>(ACTION_RAID + 4);
    e.Node<IronAssemblyFusionPunchDispelTrigger, IronAssemblyFusionPunchDispelAction>(ACTION_RAID + 3);
    e.Node<IronAssemblyRedirectThreatTrigger, IronAssemblyRedirectThreatAction>(ACTION_RAID + 2);
    e.Node<IronAssemblyRuneOfPowerSoakTrigger, IronAssemblyRuneOfPowerSoakAction>(ACTION_RAID + 1,
                                                                                 EncounterRow::Mover);
    e.Node<IronAssemblySetDpsPriorityTrigger, IronAssemblySetDpsPriorityAction>(ACTION_RAID + 1);
    e.Node<IronAssemblyRaidPositionTrigger, IronAssemblyRaidPositionAction>(ACTION_RAID, EncounterRow::Mover);

    // Targets picked in code for every role, all fight: the fallback is always a living member. No role
    // filter on purpose, IsRanged() is true for healers.
    e.OwnTargeting("iron assembly disable automatic targeting", Role::Any, IronAssemblyOwnsTargets);

    // Hazard window only: the position row yields once parked, so a permanent guard strands the raid
    // wherever its last dodge ended. ReachHeal would walk a healer to someone the formation can't reach.
    e.OwnMovement("iron assembly movement guard", Role::Any, IronAssemblyHazardMove,
                  Family::Attack | Family::Reach | Family::ReachHeal);

    // A gap-closer runs straight, reads nothing about the ground and isn't a MovementAction, so the
    // movement guard can't see it.
    e.Block("iron assembly charge guard", Role::Any, IronAssemblyBrundirHazard, Family::Charge);

    // Tank face is a second mover at the tank spot's priority, and the two alternate. Set facing is a
    // separate node.
    e.Block("iron assembly disable tank face", Role::Any, IronAssemblyTankPinned, Family::TankFace);

    // Potions and tinkers are on the burst list.
    e.Multiplier<IronAssemblyHoldDpsCooldownsMultiplier>(Family::AnyAction);
    e.Multiplier<IronAssemblyTauntGuardMultiplier>(Family::Spell);
}
}  // namespace

EncounterDefinition const& UldIronAssemblyDefinition()
{
    static EncounterDefinition const definition(ULD_BOSS_ASSEMBLY, &BossStateGateOpen, &DefineIronAssembly);
    return definition;
}
