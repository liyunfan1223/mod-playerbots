/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "EoEDefinitions.h"

#include "EoEActions.h"
#include "EoEData.h"
#include "EoEEncounter_Malygos.h"
#include "EoEMultipliers.h"
#include "EoETriggers.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Strategy.h"

namespace Family = RaidEncounterRules::Family;
namespace Role = RaidEncounterRules::Role;

namespace
{
bool Phase1(PlayerbotAI* botAI) { return GetMalygosPhase(botAI->GetBot()) == MalygosPhase::P1; }
bool Phase2(PlayerbotAI* botAI) { return GetMalygosPhase(botAI->GetBot()) == MalygosPhase::P2; }
bool Phase3(PlayerbotAI* botAI) { return GetMalygosPhase(botAI->GetBot()) == MalygosPhase::P3; }
bool PhaseTransition(PlayerbotAI* botAI) { return GetMalygosPhase(botAI->GetBot()) == MalygosPhase::Transition; }

bool Phase1TankedAndNotMainTank(PlayerbotAI* botAI)
{
    return Phase1(botAI) && !GetMalygosAggro(botAI).mainTank && GetMalygosAggro(botAI).victimIsTank;
}

// Whoever Malygos is actually hitting behaves as the tank, assigned or not.
bool Phase1NotBossTank(PlayerbotAI* botAI)
{
    return Phase1(botAI) && !GetMalygosAggro(botAI).mainTank && !GetMalygosAggro(botAI).victim;
}

bool Phase1BossVictim(PlayerbotAI* botAI) { return Phase1(botAI) && GetMalygosAggro(botAI).victim; }
bool Phase1NotMainTank(PlayerbotAI* botAI) { return Phase1(botAI) && !GetMalygosAggro(botAI).mainTank; }
bool Phase2Riding(PlayerbotAI* botAI) { return Phase2(botAI) && botAI->GetBot()->GetVehicle(); }
bool Phase2OnFoot(PlayerbotAI* botAI) { return Phase2(botAI) && !botAI->GetBot()->GetVehicle(); }

// The platform stays gone after the kill, so a rider keeps the P3 flight: generic follow and loot
// clamp a passenger's destination to the ground, which is Z 0 here.
bool MalygosGateOpen(PlayerbotAI* botAI, uint32 encounterId)
{
    if (BossStateGateOpen(botAI, encounterId))
        return true;

    Unit* drake = botAI->GetBot()->GetVehicleBase();
    return drake && drake->GetEntry() == NPC_WYRMREST_SKYTALON;
}

void DefineMalygos(EncounterBuilder& e)
{
    e.Node<MalygosTrigger, MalygosPositionAction>(ACTION_MOVE);
    e.Node<MalygosTrigger, MalygosTargetAction>(ACTION_RAID + 1);
    // P2 spellsteal gates itself on class and phase.
    e.Node<MalygosTrigger, MalygosSpellstealAction>(ACTION_RAID + 2);

    // P1 Power Sparks: DK grips them away, everyone else kills them.
    e.Node<PowerSparkTrigger, PullPowerSparkAction>(ACTION_RAID + 3);
    e.Node<PowerSparkTrigger, KillPowerSparkAction>(ACTION_RAID + 2);

    // P2 shelter. The bubble beats the surge dodge, which is only a fallback.
    e.Node<MalygosBubbleTrigger, MalygosSeekBubbleAction>(ACTION_EMERGENCY + 2);
    e.Node<SurgeOfPowerTrigger, AvoidSurgeOfPowerAction>(ACTION_EMERGENCY);

    // P2 hover disks: melee ride a freed disk up to the Scions.
    e.Node<MalygosFreeDiskTrigger, MalygosBoardDiskAction>(ACTION_RAID + 4);
    e.Node<MalygosOnDiskTrigger, MalygosRideDiskAction>(ACTION_RAID + 4);

    // P3 drake flight, at emergency relevance because a moving or off-arc vehicle cannot cast at
    // all, so it has to settle before the rotation runs.
    e.Node<MalygosDrakeFlightTrigger, EoEFlyDrakeAction>(ACTION_EMERGENCY);
    e.Node<MalygosDrakeFlightTrigger, EoEDrakeAttackAction>(ACTION_NORMAL + 5);
    e.Node<DrakeSurgeTrigger, DrakeSurgeShieldAction>(ACTION_EMERGENCY + 5);

    // Arcane Breath cones whoever he's hitting, so an off-tank taunting from the stack swings it
    // through the raid. Only while a tank holds him; with nobody on him the taunt is the rescue.
    e.Block("malygos", Role::Any, Phase1TankedAndNotMainTank, Family::Taunt);

    // "enemy too close for spell" stays active next to a 20-reach boss, and every class wires it to
    // an escape at 34-50 relevance, above the position action.
    e.Block("malygos", Role::Any, Phase1, Family::Flee | Family::RunAway | Family::Blink | Family::Disengage);
    e.Block("malygos", Role::Any, Phase1, Family::Follow);

    // Any dps may be holding a Power Spark rather than the boss.
    e.Block("malygos", Role::Dps, Phase1, Family::DropTarget);

    // He parks ~21y short of his victim, just outside what ReachMeleeAction calls melee, so this
    // would walk the tank in every tick while the position action walks him back out.
    e.Block("malygos", Role::Any, Phase1BossVictim, Family::Reach | Family::ReachHeal);

    e.OwnTargeting("malygos", Role::Dps, Phase1, Family::DpsAssist);

    // In P1 only the boss's victim moves on its own. Attacks move too, so the EoE ones are named;
    // closing on a heal target is the one exception, reach spells get none.
    e.OwnMovement("malygos", Role::Any, Phase1NotBossTank, Family::ReachHeal,
                  {MalygosPositionAction::Name, MalygosTargetAction::Name, KillPowerSparkAction::Name});
    e.Block("malygos", Role::Any, Phase1NotBossTank, Family::Charge);

    e.Block("malygos", Role::Any, Phase1NotMainTank, Family::TankAssist);

    // A chase that casts on arrival steers the disk exactly like a chase that walks.
    e.Block("malygos", Role::Any, Phase2Riding, Family::Charge);

    e.OwnTargeting("malygos", Role::Dps, Phase2, Family::DpsAssist);

    // Keep the generic flee from walking bots off the edge; the position action recentres.
    e.Block("malygos", Role::Any, Phase2, Family::Flee);

    // A rider's chase actions steer the disk and dive it whenever it parks by a Scion.
    // Leaving the vehicle stays open as a manual override.
    e.Exclusive("malygos", Role::Any, Phase2Riding, 0,
                {MalygosRideDiskAction::Name, MalygosTargetAction::Name, "leave vehicle"});

    // Nothing may chase ranged or healers out of shelter; melee still walk to the Nexus Lords.
    // Closing on someone to heal them stays open, or a raider out of range never gets one.
    e.Block("malygos", Role::Ranged | Role::Heal, Phase2OnFoot, Family::Reach | Family::Follow);

    e.Multiplier<MalygosScionTankAssistMultiplier>(Family::TankAssist);

    // The fly action is the only thing allowed to steer a Skytalon. The drake rotation and the surge
    // shield are plain Actions, so they are untouched.
    e.Exclusive("malygos", Role::Any, Phase3, 0, {EoEFlyDrakeAction::Name});

    // Phase transition: hold the gather at centre and stay off the default strategy.
    e.Exclusive("malygos", Role::Any, PhaseTransition, 0, {MalygosPositionAction::Name});
}
}  // namespace

EncounterDefinition const& EoEMalygosDefinition()
{
    static EncounterDefinition const definition(EOE_DATA_MALYGOS, &MalygosGateOpen, &DefineMalygos);
    return definition;
}
