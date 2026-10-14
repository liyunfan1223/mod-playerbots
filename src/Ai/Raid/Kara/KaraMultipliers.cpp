/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "KaraMultipliers.h"
#include "AttackAction.h"
#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidActions.h"
#include "FollowActions.h"
#include "HunterActions.h"
#include "KaraActions.h"
#include "KaraHelpers.h"
#include "MageActions.h"
#include "PaladinActions.h"
#include "PetsAction.h"
#include "Playerbots.h"
#include "PriestActions.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "WarriorActions.h"
#include <ctime>

using namespace KaraHelpers;

// General

float KarazhanSetTremorTotemMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!dynamic_cast<CastStrengthOfEarthTotemAction*>(action) &&
        !dynamic_cast<CastStoneskinTotemAction*>(action) &&
        !dynamic_cast<CastStoneclawTotemAction*>(action) &&
        !dynamic_cast<CastEarthbindTotemAction*>(action))
    {
        return 1.0f;
    }

    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (nightbane && nightbane->GetPositionZ() <= NIGHTBANE_FLIGHT_Z)
        return 0.0f;

    if (AI_VALUE2(Unit*, "find target", "spectral charger") ||
        AI_VALUE2(Unit*, "find target", "the big bad wolf") ||
        AI_VALUE2(Unit*, "find target", "roar"))
    {
        return 0.0f;
    }

    return 1.0f;
}

// Attumen the Huntsman

float AttumenTheHuntsmanDisableAutomaticTargetingMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<TankAssistAction*>(action) &&
        !dynamic_cast<DpsAssistAction*>(action))
    {
        return 1.0f;
    }

    if (AI_VALUE2(Unit*, "find target", "midnight"))
        return 0.0f;

    return 1.0f;
}

float AttumenTheHuntsmanStayStackedMultiplier::GetValue(Action* action)
{
    if (PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action) ||
        dynamic_cast<AttumenTheHuntsmanHandlePhaseTwoAction*>(action))
    {
        return 1.0f;
    }

    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastBlinkBackAction*>(action) &&
        !dynamic_cast<CastDisengageAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (!AI_VALUE2(Unit*, "find target", "midnight"))
        return 1.0f;

    if (GetAttumenMounted(bot))
        return 0.0f;

    return 1.0f;
}

// Give the main tank 5 seconds to grab aggro when Attumen mounts Midnight
float AttumenTheHuntsmanWaitForDpsMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    if (!dynamic_cast<AttackAction*>(action) &&
        !dynamic_cast<CastSpellAction*>(action))
    {
        return 1.0f;
    }

    if (!AI_VALUE2(Unit*, "find target", "midnight"))
        return 1.0f;

    Unit* attumen = GetAttumenMounted(bot);
    if (!attumen)
        return 1.0f;

    if (PlayerbotAI::IsMainTank(bot))
        return 1.0f;

    time_t const now = std::time(nullptr);
    constexpr uint8 dpsWaitSeconds = 5;

    auto it = attumenDpsWaitTimer.find(attumen->GetMap()->GetInstanceId());
    if (it == attumenDpsWaitTimer.end() || (now - it->second) < dpsWaitSeconds)
        return 0.0f;

    return 1.0f;
}

// Maiden of Virtue

float MaidenOfVirtueDisableCombatFormationMoveMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<SetBehindTargetAction*>(action) ||
        !dynamic_cast<CombatFormationMoveAction*>(action))
    {
        return 1.0f;
    }

    if (AI_VALUE2(Unit*, "find target", "maiden of virtue"))
        return 0.0f;

    return 1.0f;
}

float MaidenOfVirtueSetGroundingTotemMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!dynamic_cast<CastWrathOfAirTotemAction*>(action) &&
        !dynamic_cast<CastNatureResistanceTotemAction*>(action) &&
        !dynamic_cast<CastWindfuryTotemAction*>(action))
    {
        return 1.0f;
    }

    if (AI_VALUE2(Unit*, "find target", "maiden of virtue"))
        return 0.0f;

    return 1.0f;
}

// The Curator

float TheCuratorDisableTankAssistMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<TankAssistAction*>(action))
        return 1.0f;

    if (AI_VALUE2(Unit*, "find target", "the curator"))
        return 0.0f;

    return 1.0f;
}

float TheCuratorDisableCombatFormationMoveMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<SetBehindTargetAction*>(action) ||
        !dynamic_cast<CombatFormationMoveAction*>(action))
    {
        return 1.0f;
    }

    if (AI_VALUE2(Unit*, "find target", "the curator"))
        return 0.0f;

    return 1.0f;
}

// Save Bloodlust/Heroism for Evocation (100% increased damage)
float TheCuratorDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!dynamic_cast<CastBloodlustAction*>(action) &&
        !dynamic_cast<CastHeroismAction*>(action))
    {
        return 1.0f;
    }

    Unit* curator = AI_VALUE2(Unit*, "find target", "the curator");
    if (curator && !curator->HasAura(Id(KaraSpells::SPELL_CURATOR_EVOCATION)))
        return 0.0f;

    return 1.0f;
}

// Terestian Illhoof

float TerestianIllhoofDontDotFiendishImpsMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<CastDebuffSpellOnAttackerAction*>(action))
        return 1.0f;

    Unit* imp = AI_VALUE2(Unit*, "find target", "fiendish imp");
    if (imp && AI_VALUE(Unit*, "current target") == imp)
        return 0.0f;

     return 1.0f;
}

// Shade of Aran

float ShadeOfAranArcaneExplosionRunAwayMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<AttackAction*>(action) ||
        dynamic_cast<ShadeOfAranRunAwayFromArcaneExplosionAction*>(action))
    {
        return 1.0f;
    }

    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastBlinkBackAction*>(action) &&
        !dynamic_cast<CastDisengageAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");
    if (!aran)
        return 1.0f;

    if (IsAranCastingArcaneExplosion(aran))
        return 0.0f;

    return 1.0f;
}

// I will not move when Flame Wreath is cast or the raid blows up
float ShadeOfAranFlameWreathDisableMovementMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<AttackAction*>(action) ||
        dynamic_cast<ShadeOfAranStopMovingDuringFlameWreathAction*>(action))
    {
        return 1.0f;
    }

    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastBlinkBackAction*>(action) &&
        !dynamic_cast<CastDisengageAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action) &&
        !dynamic_cast<CastKillingSpreeAction*>(action))
    {
        return 1.0f;
    }

    if (!AI_VALUE2(Unit*, "find target", "shade of aran"))
        return 1.0f;

    if (IsFlameWreathActive(bot))
        return 0.0f;

    return 1.0f;
}

// Netherspite

float NetherspiteKeepBlockingBeamMultiplier::GetValue(Action* action)
{
    bool const isReachTargetAction =
        dynamic_cast<ReachTargetAction*>(action) ||
        dynamic_cast<CastReachTargetSpellAction*>(action);

    bool const isBlockedAction =
        isReachTargetAction ||
        dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<AvoidAoeAction*>(action) ||
        dynamic_cast<CastKillingSpreeAction*>(action);

    if (!isBlockedAction)
        return 1.0f;

    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite || IsBanishPhase(netherspite))
        return 1.0f;

    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(bot);

    if (bot == greenBlocker || bot == blueBlocker)
        return 0.0f;

    if (bot == redBlocker && !isReachTargetAction)
        return 0.0f;

    return 1.0f;
}

// Give tanks 5 seconds to get aggro during phase transitions
float NetherspiteWaitForDpsMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    if (!dynamic_cast<AttackAction*>(action) &&
        !dynamic_cast<CastSpellAction*>(action))
    {
        return 1.0f;
    }

    if (PlayerbotAI::IsTank(bot))
        return 1.0f;

    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite || IsBanishPhase(netherspite))
        return 1.0f;

    time_t const now = std::time(nullptr);
    constexpr uint8 dpsWaitSeconds = 5;

    auto it = netherspiteDpsWaitTimer.find(netherspite->GetMap()->GetInstanceId());
    if (it == netherspiteDpsWaitTimer.end() || (now - it->second) < dpsWaitSeconds)
        return 0.0f;

     return 1.0f;
}

// Prince Malchezaar

float PrinceMalchezaarEnfeebleMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<AttackAction*>(action) ||
        dynamic_cast<PrinceMalchezaarEnfeebledBotAvoidHazardAction*>(action))
    {
        return 1.0f;
    }

    if (!bot->HasAura(Id(KaraSpells::SPELL_ENFEEBLE)))
        return 1.0f;

    // Disable movement other than escaping Shadow Nova range
    if (dynamic_cast<MovementAction*>(action) ||
        dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 0.0f;
    }

    // Disable some cooldowns that are used upon low/critical health
    if (dynamic_cast<CastIceBlockAction*>(action) ||
        dynamic_cast<CastManaShieldAction*>(action) ||
        dynamic_cast<CastDivineShieldAction*>(action) ||
        dynamic_cast<CastLayOnHandsAction*>(action) ||
        dynamic_cast<CastCloakOfShadowsAction*>(action) ||
        dynamic_cast<CastEvasionAction*>(action) ||
        dynamic_cast<CastShamanisticRageAction*>(action) ||
        dynamic_cast<CastBindingHealAction*>(action) ||
        dynamic_cast<CastDesperatePrayerAction*>(action) ||
        dynamic_cast<CastPainSuppressionAction*>(action) ||
        dynamic_cast<CastDispersionAction*>(action) ||
        dynamic_cast<CastEnragedRegenerationAction*>(action) ||
        dynamic_cast<CastLastStandAction*>(action) ||
        dynamic_cast<CastShieldWallAction*>(action) ||
        dynamic_cast<CastSurvivalInstinctsAction*>(action) ||
        dynamic_cast<CastDeterrenceAction*>(action) ||
        dynamic_cast<CastIceboundFortitudeAction*>(action) ||
        dynamic_cast<CastRuneTapAction*>(action) ||
        dynamic_cast<CastVampiricBloodAction*>(action) ||
        dynamic_cast<CastDeathStrikeAction*>(action) ||
        dynamic_cast<CastDeathPactAction*>(action) ||
        dynamic_cast<UseItemAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

// Wait until Phase 3 to use Bloodlust/Heroism
float PrinceMalchezaarDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!dynamic_cast<CastBloodlustAction*>(action) &&
        !dynamic_cast<CastHeroismAction*>(action))
    {
        return 1.0f;
    }

    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    if (malchezaar && malchezaar->GetHealthPct() > 30.0f)
        return 0.0f;

    return 1.0f;
}

// Nightbane

// Pets tend to run out of bounds and cause skeletons to spawn off the map
// Pets also tend to pull adds from inside of the tower through the floor
// This multiplier DOES NOT impact Hunter and Warlock pets
// Hunter and Warlock pets are addressed in ControlPetAggressionAction
float NightbaneDisablePetsMultiplier::GetValue(Action* action)
{
    bool const isTemporarySummonSpell =
        dynamic_cast<CastForceOfNatureAction*>(action) ||
        dynamic_cast<CastFeralSpiritAction*>(action) ||
        dynamic_cast<CastFireElementalTotemAction*>(action) ||
        dynamic_cast<CastFireElementalTotemMeleeAction*>(action) ||
        dynamic_cast<CastSummonWaterElementalAction*>(action) ||
        dynamic_cast<CastShadowfiendAction*>(action);

    bool const isPetAttackAction = dynamic_cast<PetAttackAction*>(action);

    if (!isTemporarySummonSpell && !isPetAttackAction)
        return 1.0f;

    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane)
        return 1.0f;

    if (isTemporarySummonSpell)
        return 0.0f;

    if (nightbane->GetPositionZ() <= NIGHTBANE_FLIGHT_Z)
        return 1.0f;

    if (isPetAttackAction)
        return 0.0f;

    return 1.0f;
}

// Give the main tank 8 seconds to get aggro during phase transitions
float NightbaneWaitForDpsMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    if (!dynamic_cast<AttackAction*>(action) &&
        !dynamic_cast<CastSpellAction*>(action))
    {
        return 1.0f;
    }

    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane || nightbane->GetPositionZ() > NIGHTBANE_FLIGHT_Z)
        return 1.0f;

    if (PlayerbotAI::IsMainTank(bot))
        return 1.0f;

    time_t const now = std::time(nullptr);
    constexpr uint8 dpsWaitSeconds = 8;

    auto it = nightbaneDpsWaitTimer.find(nightbane->GetMap()->GetInstanceId());
    if (it == nightbaneDpsWaitTimer.end() || (now - it->second) < dpsWaitSeconds)
        return 0.0f;

    return 1.0f;
}

float NightbaneDisableAvoidAoeMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<AvoidAoeAction*>(action))
        return 1.0f;

    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane || nightbane->GetPositionZ() > NIGHTBANE_FLIGHT_Z)
        return 1.0f;

    if (PlayerbotAI::IsRanged(bot) || PlayerbotAI::IsMainTank(bot))
        return 0.0f;

    return 1.0f;
}

float NightbaneDisableMovementMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    bool const isFollowAction = dynamic_cast<FollowAction*>(action);

    bool const isReachAction =
        dynamic_cast<ReachTargetAction*>(action) ||
        dynamic_cast<CastReachTargetSpellAction*>(action);

    bool const isBlockedMovement =
        dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<FleeAction*>(action);

    if (!isFollowAction && !isReachAction && !isBlockedMovement)
        return 1.0f;

    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane)
        return 1.0f;

    if (isBlockedMovement)
        return 0.0f;

    if (nightbane->GetPositionZ() <= NIGHTBANE_FLIGHT_Z)
        return 1.0f;

    if (isReachAction)
        return 0.0f;

    // After 35s, Nightbane goes to land, and bots freely follow their master
    time_t const now = std::time(nullptr);
    constexpr uint8 flightPhaseDurationSeconds = 35;

    auto const it = nightbaneFlightPhaseStartTimer.find(nightbane->GetMap()->GetInstanceId());
    if (it == nightbaneFlightPhaseStartTimer.end() ||
        (now - it->second) >= flightPhaseDurationSeconds)
    {
        return 1.0f;
    }

    if (isFollowAction)
        return 0.0f;

    return 1.0f;
}
