/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldDefinitions.h"

#include "BossAuraActions.h"
#include "BossAuraTriggers.h"
#include "Strategy.h"
#include "UldActions_Vezax.h"
#include "UldEncounterGate.h"
#include "UldEncounter_Vezax.h"
#include "UldMultipliers_Vezax.h"
#include "UldTriggers_Vezax.h"

namespace Family = RaidEncounterRules::Family;
namespace Role = RaidEncounterRules::Role;

namespace
{
void DefineVezax(EncounterBuilder& e)
{
    // First action to return true wins, so this is a survival ranking. No ties: a tie falls to insertion order.
    e.Node<VezaxResetEncounterStateTrigger, VezaxResetEncounterStateAction>(ACTION_EMERGENCY + 10);
    // The only hazard here with a deadline, the missile's flight.
    e.Node<VezaxShadowCrashDodgeTrigger, VezaxShadowCrashDodgeAction>(ACTION_EMERGENCY + 9, EncounterRow::Mover);
    // A missed kick is 13875-16125 fire on the whole raid at once.
    e.Node<VezaxSearingFlamesInterruptTrigger, VezaxSearingFlamesInterruptAction>(ACTION_EMERGENCY + 8);
    // The mark drains over 10s instead of landing at once, so it sits under those two. Break outranks
    // carry: the holder is the one person its own leech skips.
    e.Node<VezaxMarkOfTheFacelessBreakTrigger, VezaxMarkOfTheFacelessBreakAction>(ACTION_EMERGENCY + 7,
                                                                                 EncounterRow::Mover);
    e.Node<VezaxMarkOfTheFacelessTrigger, VezaxMarkOfTheFacelessAction>(ACTION_EMERGENCY + 6, EncounterRow::Mover);
    e.Node<VezaxSurgeOfDarknessTrigger, VezaxSurgeOfDarknessAction>(ACTION_EMERGENCY + 5);
    // The RAID band is the reward half. The hold never claims the tick, it only has to run before the hold
    // target row. The redirect needs the bot on the Animus already, so it never competes with the switch.
    e.Node<VezaxDamageHoldTrigger, VezaxDamageHoldAction>(ACTION_RAID + 8);
    e.Node<VezaxAnimusRedirectTrigger, VezaxAnimusRedirectAction>(ACTION_RAID + 7);
    e.Node<VezaxSaroniteAnimusTrigger, VezaxSaroniteAnimusAction>(ACTION_RAID + 6);
    e.Node<VezaxHoldTargetTrigger, VezaxHoldTargetAction>(ACTION_RAID + 5);
    // Above the soak, or the Animus's victim leads it into the camp.
    e.Node<VezaxAnimusBringBackTrigger, VezaxAnimusBringBackAction>(ACTION_RAID + 4, EncounterRow::Mover);
    // Under the bring-back, which already ends inside his crash limit, and under the switch.
    e.Node<VezaxAnimusMeleeSpotTrigger, VezaxAnimusMeleeSpotAction>(ACTION_RAID + 3, EncounterRow::Mover);
    e.Node<VezaxShadowCrashSoakTrigger, VezaxShadowCrashSoakAction>(ACTION_RAID + 2, EncounterRow::Mover);
    e.Node(
        "vezax shadow resistance",
        [](PlayerbotAI* botAI) -> Trigger* { return new BossShadowResistanceTrigger(botAI, "general vezax"); },
        "vezax shadow resistance action",
        [](PlayerbotAI* botAI) -> Action* { return new BossShadowResistanceAction(botAI, "general vezax"); },
        ACTION_RAID + 1);
    // Last, and yields once parked so the class interrupts at ACTION_INTERRUPT (40) still get a tick.
    e.Node<VezaxRaidPositionTrigger, VezaxRaidPositionAction>(ACTION_RAID, EncounterRow::Mover);

    // Generic movers stand down for slot holders, or the camp is abandoned on alternate ticks. ReachHeal
    // stays so a healer can reach someone the formation can't.
    e.OwnMovement("vezax control movement multiplier", Role::Ranged | Role::MainTank, VezaxFormationActive,
                  Family::Attack | Family::Reach | Family::ReachHeal);

    // Set behind on the Animus walks melee round its back, away from the tank and Vezax beside him, and
    // into his Shadow Crash picks. The melee spot row puts them in between the two instead.
    e.OwnMovement("vezax animus melee movement multiplier", Role::Melee, VezaxAnimusPhaseActive);

    e.Multiplier<VezaxSuppressLifeTapMultiplier>(Family::Spell);

    // The generic pickers take vapors, and one kill ends hard mode. The debuff family DoTs whatever a caster
    // drifted onto. With these gone, the hold target row is the raid's only target source.
    e.OwnTargeting("vezax target guard multiplier", Role::Any, VezaxEncounterActive,
                   Family::DpsAssist | Family::TankAssist | Family::DebuffOnAttacker);

    // Lust waits for the Animus summon or a vapor death. So does Starfall: its stars pick random enemies the
    // target guard never sees, and a vapor can spawn under one already falling.
    e.Block("vezax hard mode hold", Role::Any, VezaxHardModePending, 0, {"heroism", "bloodlust", "starfall"});

    // Past the stop line nothing hits him until the Animus is up. Spells are the multiplier below, which
    // sees the summons as buffs, so they go by name here.
    e.Block("vezax damage hold", Role::Any, VezaxDamageHeld,
            Family::Melee | Family::PetAttack | Family::Charge | Family::DpsAoe | Family::AggressiveTarget |
                Family::AttackAnything | Family::AttackLeastHp | Family::AttackRti,
            {"army of the dead", "dancing rune weapon", "feral spirit", "fire elemental totem", "force of nature",
             "magma totem", "mirror image", "searing totem", "summon gargoyle"});
    e.Multiplier<VezaxDamageHoldMultiplier>(Family::Spell);

    // Kept off cooldown for the Animus, which only a tank's threat holds. Left alone, the generic
    // triggers spend them on Vezax, where the tank has had minutes to build threat, or on a dps.
    e.Block("vezax save redirects for the animus", Role::Any, VezaxAnimusDue, 0,
            {"misdirection on main tank", "tricks of the trade on main tank"});

    e.Multiplier<VezaxHoldCastOutsideFieldMultiplier>(Family::Melee | Family::Spell);

    // Nothing generic holds a dps bot under the tank's threat, since no bot ever gets the "threat"
    // strategy, and the Animus can't be taunted back once one passes him.
    e.Multiplier<VezaxAnimusThreatMultiplier>(Family::Melee | Family::Spell);
}
}  // namespace

EncounterDefinition const& UldVezaxDefinition()
{
    static EncounterDefinition const definition(ULD_BOSS_VEZAX, &BossStateGateOpen, &DefineVezax);
    return definition;
}
