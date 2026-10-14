/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "HyjalMultipliers.h"
#include "ChooseTargetActions.h"
#include "EncounterHelpers.h"
#include "HunterActions.h"
#include "HyjalActions.h"
#include "HyjalHelpers.h"
#include "ReachTargetActions.h"
#include "ShamanActions.h"

using namespace HyjalHelpers;
using namespace EncounterHelpers;

// Note: BOT_STATE_NON_COMBAT checks cannot be used by any multiplier that could result in a bot
// having no valid targets as it will then swap to the non-combat engine, even during a boss fight.
// This concern implicates any avoidance action that could hold the bot out of attack range.

float HyjalDelayDpsCooldownsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->GetMapId() != HYJAL_MAP_ID) // In case strategy persists outside (e.g., server reset)
        return 1.0f;

    if (!IsDpsCooldownAction(bot, action))
        return 1.0f;

    // Suppress Bloodlust/Heroism during all trash waves. It's blown on CD otherwise.
    Unit* boss = AI_VALUE(Unit*, "boss target");
    if (!boss)
    {
        return bot->getClass() == CLASS_SHAMAN &&
            (dynamic_cast<CastBloodlustAction*>(action) ||
             dynamic_cast<CastHeroismAction*>(action)) ? 0.0f : 1.0f;
    }

    return boss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT ? 0.0f : 1.0f;
}

float HyjalDisableDisperseAndTankFaceMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    if (HasProtectionOfElune(bot))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "archimonde") ||
        AI_VALUE2(Unit*, "find target", "kaz'rogal") ||
        AI_VALUE2(Unit*, "find target", "rage winterchill") ? 0.0f : 1.0f;
}

// Rage Winterchill

float RageWinterchillMeleeControlAvoidanceMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsMelee(bot))
        return 1.0f;

    bool const isAvoidAoe = dynamic_cast<AvoidAoeAction*>(action);

    if (!isAvoidAoe &&
        !dynamic_cast<ReachTargetAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
    {
        return 1.0f;
    }

    Unit* winterchill = AI_VALUE2(Unit*, "find target", "rage winterchill");
    if (!winterchill)
        return 1.0f;

    if (!IsNearDeathAndDecay(botAI, DEATH_AND_DECAY_CONTROL_RADIUS))
        return 1.0f;

    if (isAvoidAoe)
        return 0.0f;

    if (winterchill->GetVictim() == bot)
        return 1.0f;

    return PlayerbotAI::IsMainTank(bot) ? 1.0f : 0.0f;
}

float RageWinterchillRangedControlAvoidanceMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsRanged(bot))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    if (dynamic_cast<RageWinterchillRangedGetOutOfDeathAndDecayAction*>(action))
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "rage winterchill"))
        return 1.0f;

    if (dynamic_cast<AvoidAoeAction*>(action))
        return 0.0f;

    return IsNearDeathAndDecay(botAI, DEATH_AND_DECAY_CONTROL_RADIUS) ? 0.0f : 1.0f;
}

// Anetheron

float AnetheronDisableAssistTargetingMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    bool const isTankAssist = dynamic_cast<TankAssistAction*>(action);

    if (!isTankAssist && !dynamic_cast<DpsAssistAction*>(action))
        return 1.0f;

    if (isTankAssist && IsInfernalTank(bot))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "anetheron") ? 0.0f : 1.0f;
}

// Keep non-Infernal tanks from inadvertently grabbing aggro with Consecration, Thunder Clap, etc.
float AnetheronAvoidAccidentalInfernalAggroMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!IsAoeThreatAction(bot, action))
        return 1.0f;

    constexpr float holdTankAoeRadius = 20.0f; // arbitrary, but > AoE threat ability radii
    Unit* infernal = GetNearestInfernal(botAI);
    if (!infernal || infernal->GetExactDist2d(bot) > holdTankAoeRadius)
        return 1.0f;

    return IsInfernalTank(bot) ? 1.0f : 0.0f;
}

float AnetheronInfernalTargetRunToPositionMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<AnetheronBringInfernalToInfernalTankAction*>(action))
        return 1.0f;

    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron || anetheron->GetVictim() == bot)
        return 1.0f;

    if (IsInfernalTank(bot))
        return 1.0f;

    return GetInfernoTarget(anetheron) == bot || GetInfernalTargetingBot(botAI) ? 0.0f : 1.0f;
}

float AnetheronControlMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    bool const isTankAvoidAoe =
        PlayerbotAI::IsTank(bot) && dynamic_cast<AvoidAoeAction*>(action);

    if (!isTankAvoidAoe && !dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "anetheron") ? 0.0f : 1.0f;
}

float AnetheronControlMisdirectionMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "anetheron") ? 0.0f : 1.0f;
}

// Kaz'rogal

float KazrogalControlLowManaMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsKazrogalManaUser(botAI) || bot->getClass() == CLASS_HUNTER)
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<KazrogalMoveAwayFromGroupAction*>(action))
        return 1.0f;

    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
    if (!kazrogal || kazrogal->GetVictim() == bot)
        return 1.0f;

    return AI_VALUE(bool, "kaz'rogal below mana threshold") ? 0.0f : 1.0f;
}

float KazrogalKeepAspectOfTheViperActiveMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!dynamic_cast<CastAspectOfTheHawkAction*>(action) &&
        !dynamic_cast<CastAspectOfTheWildAction*>(action) &&
        !dynamic_cast<CastAspectOfTheDragonhawkAction*>(action) &&
        !dynamic_cast<CastAspectOfTheCheetahAction*>(action) &&
        !dynamic_cast<CastAspectOfThePackAction*>(action) &&
        !dynamic_cast<CastAspectOfTheMonkeyAction*>(action))
    {
        return 1.0f;
    }

    if (!AI_VALUE2(Unit*, "find target", "kaz'rogal"))
        return 1.0f;

    return bot->GetPower(POWER_MANA) <= MARK_REJOIN_MANA ? 0.0f : 1.0f;
}

// Azgalor

float AzgalorDisableAutoTargetingAndPositioningMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<DpsAssistAction*>(action) &&
        !dynamic_cast<TankAssistAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<AvoidAoeAction*>(action))
    {
        return 1.0f;
    }

    // Set Behind Target is still disabled in RoF (in AzgalorMeleeDpsControlAvoidanceMultiplier)
    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "azgalor") ? 0.0f : 1.0f;
}

float AzgalorDoomedBotPrioritizePositioningMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsDoomed(bot))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    return dynamic_cast<AzgalorMoveToDoomguardTankAction*>(action) ? 1.0f : 0.0f;
}

float AzgalorMeleeDpsControlAvoidanceMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsMelee(bot) || PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (IsDoomed(bot))
        return 1.0f;

    if (dynamic_cast<AzgalorMeleeManeuverThroughFireAction*>(action))
        return 1.0f;

    if (dynamic_cast<AzgalorDetermineDpsPriorityAction*>(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "azgalor"))
        return 1.0f;

    return IsNearRainOfFire(botAI, RAIN_OF_FIRE_CONTROL_RADIUS) ? 0.0f : 1.0f;
}

float AzgalorRangedControlAvoidanceMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsRanged(bot))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    if (IsDoomed(bot))
        return 1.0f;

    if (dynamic_cast<AzgalorRangedGetOutOfRainOfFireAction*>(action))
        return 1.0f;

    if (dynamic_cast<AzgalorDetermineDpsPriorityAction*>(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "azgalor"))
        return 1.0f;

    return IsNearRainOfFire(botAI, RAIN_OF_FIRE_CONTROL_RADIUS) ? 0.0f : 1.0f;
}

// Archimonde

float ArchimondeControlDoomfireAvoidanceMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<ArchimondeAvoidDoomfireAction*>(action) ||
        dynamic_cast<ArchimondeKeepAirBurstAwayFromTankAction*>(action))
    {
        return 1.0f;
    }

    if (!AI_VALUE2(Unit*, "find target", "archimonde"))
        return 1.0f;

    if (dynamic_cast<AvoidAoeAction*>(action))
        return 0.0f;

    if (HasProtectionOfElune(bot))
        return 1.0f;

    return IsNearDoomfire(botAI, DOOMFIRE_CONTROL_RADIUS) ? 0.0f : 1.0f;
}

float ArchimondeSetTremorTotemMultiplier::GetValueInEncounter(Action* action)
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

    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    if (!archimonde || archimonde->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT)
        return 1.0f;

    return !HasProtectionOfElune(bot) ? 0.0f : 1.0f;
}
