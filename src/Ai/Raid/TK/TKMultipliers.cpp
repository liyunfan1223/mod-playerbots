/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TKMultipliers.h"
#include "ChooseTargetActions.h"
#include "EncounterHelpers.h"
#include "EquipAction.h"
#include "FollowActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "Playerbots.h"
#include "PriestActions.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "TKActions.h"
#include "TKHelpers.h"
#include "WarlockActions.h"

using namespace TkHelpers;
using namespace EncounterHelpers;

// Al'ar <Phoenix God>

float AlarSuppressGapClosersMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    bool const isBlockedMovement =
        dynamic_cast<TankFaceAction*>(action) ||
        dynamic_cast<CastKillingSpreeAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<ReachTargetAction*>(action);

    if (!isBlockedMovement && !dynamic_cast<CastReachTargetSpellAction*>(action))
        return 1.0f;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar || IsAlarInPhase2(alar->GetInstanceId()))
        return 1.0f;

    if (isBlockedMovement)
        return 0.0f;

    if (PlayerbotAI::IsTank(bot))
        return 1.0f;

    // Block Charge, etc. for non-tanks when not at a platform.
    int8 const currentLocationIndex = GetAlarCurrentLocationIndex(alar);
    return currentLocationIndex < PLATFORM_0_IDX ||
        currentLocationIndex > PLATFORM_3_IDX ? 0.0f : 1.0f;
}

float AlarControlMovementMultiplier::GetValueInEncounter(Action* action)
{
    bool const isDisperseOrFlee =
        dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FleeAction*>(action);

    if (!isDisperseOrFlee && !dynamic_cast<FollowAction*>(action))
        return 1.0f;

    if (dynamic_cast<TankFaceAction*>(action) || dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return 1.0f;

    if (isDisperseOrFlee)
        return 0.0f;

    if (!IsAlarInPhase2(alar->GetInstanceId()))
        return 1.0f;

    // Enable FollowAction only in the non-combat engine in Phase 2.
    return botAI->GetState() == BOT_STATE_COMBAT ? 0.0f : 1.0f;
}

float AlarDisableAutomaticTargetingMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<TankAssistAction*>(action) && !dynamic_cast<DpsAssistAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "al'ar") ? 0.0f : 1.0f;
}

float AlarStayAwayFromRebirthMultiplier::GetValueInEncounter(Action* action)
{
    if (PlayerbotAI::IsRanged(bot) || PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    if (dynamic_cast<AlarMoveAwayFromRebirthAction*>(action))
        return 1.0f;

    // Don't block Flame Quills avoidance in case of bad timing for the transition.
    if (dynamic_cast<AlarJumpFromPlatformAction*>(action))
        return 1.0f;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar || IsAlarInPhase2(alar->GetInstanceId()))
        return 1.0f;

    Creature* alarCreature = alar->ToCreature();
    if (alarCreature && alarCreature->GetReactState() == REACT_PASSIVE)
        return 0.0f;

    constexpr float phase1AlmostEndedHpThreshold = 5.0f;
    return alar->GetHealthPct() <= phase1AlmostEndedHpThreshold ? 0.0f : 1.0f;
}

float AlarControlTauntingMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!IsTauntAction(bot, action))
        return 1.0f;

    bool const isFirstAlarTank = IsFirstAlarTank(bot);

    if (!isFirstAlarTank && !IsSecondAlarTank(bot))
        return 1.0f;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return 1.0f;

    if (bot->HasAura(Id(TkSpells::SPELL_MELT_ARMOR)) && AI_VALUE(Unit*, "current target") == alar)
        return 0.0f;

    if (IsAlarInPhase2(alar->GetInstanceId()))
        return 1.0f;

    int8 platformIndex = GetAlarPlatformIndex(alar);
    if (isFirstAlarTank)
    {
        if (platformIndex != PLATFORM_0_IDX && platformIndex != PLATFORM_2_IDX)
            return 0.0f;
    }
    else // isSecondAlarTank
    {
        if (platformIndex != PLATFORM_1_IDX && platformIndex != PLATFORM_3_IDX)
            return 0.0f;
    }

    return 1.0f;
}

// Void Reaver

float VoidReaverMaintainPositionsMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "void reaver") ? 0.0f : 1.0f;
}

// High Astromancer Solarian

float HighAstromancerSolarianWrathStayAwayMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<HighAstromancerSolarianMoveAwayFromGroupAction*>(action))
        return 1.0f;

    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!astromancer || astromancer->HasAura(Id(TkSpells::SPELL_SOLARIAN_TRANSFORM)))
        return 1.0f;

    return HasWrathOfTheAstromancer(bot) ? 0.0f : 1.0f;
}

float HighAstromancerSolarianDisableMeleeTargetingMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsMelee(bot))
        return 1.0f;

    if (!dynamic_cast<TankAssistAction*>(action) && !dynamic_cast<DpsAssistAction*>(action))
        return 1.0f;

    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!astromancer)
        return 1.0f;

    if (PlayerbotAI::IsMainTank(bot))
    {
        Creature* astromancerCreature = astromancer->ToCreature();
        if (astromancerCreature && astromancerCreature->GetReactState() != REACT_PASSIVE)
            return 0.0f;

        return 1.0f;
    }

    return AI_VALUE2(Unit*, "find target", "solarium priest") ? 0.0f : 1.0f;
}

// Kael'thas Sunstrider <Lord of the Blood Elves>

float KaelthasSunstriderWaitForDpsMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<CastHealingSpellAction*>(action) ||
        dynamic_cast<CastFearWardOnMainTankAction*>(action))
    {
        return 1.0f;
    }

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    if (GetKaelthasTkPhase(kaelthas) != PHASE_SINGLE_ADVISOR)
        return 1.0f;

    constexpr uint32 dpsWaitMs = 10 * IN_MILLISECONDS;
    auto it = advisorDpsWaitTimer.find(kaelthas->GetInstanceId());
    if (it != advisorDpsWaitTimer.end() && it->second != ADVISOR_DPS_WAIT_NOT_STARTED &&
        getMSTimeDiff(it->second, getMSTime()) >= dpsWaitMs)
    {
        return 1.0f;
    }

    // Only the applicable tank may attack during the first 10 seconds of an advisor in phase 1.
    if (IsAdvisorActive(AI_VALUE2(Unit*, "find target", "lord sanguinar")))
        return PlayerbotAI::IsMainTank(bot) ? 1.0f : 0.0f;

    if (IsAdvisorActive(AI_VALUE2(Unit*, "find target", "grand astromancer capernian")))
        return IsCapernianTank(bot) ? 1.0f : 0.0f;

    if (IsAdvisorActive(AI_VALUE2(Unit*, "find target", "master engineer telonicus")))
        return PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) ? 1.0f : 0.0f;

    return 1.0f;
}

float KaelthasSunstriderKiteThaladredMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<KaelthasSunstriderKiteThaladredAction*>(action))
        return 1.0f;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    uint32 const phase = GetKaelthasTkPhase(kaelthas);
    if (phase == PHASE_NONE)
        return 1.0f;

    if (PlayerbotAI::IsTank(bot) && phase == PHASE_ALL_ADVISORS)
        return 1.0f;

    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");
    return thaladred && thaladred->GetVictim() == bot ? 0.0f : 1.0f;
}

float KaelthasSunstriderControlMisdirectionMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 1.0f;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    uint32 const phase = GetKaelthasTkPhase(kaelthas);
    return phase != PHASE_NONE && phase != PHASE_FINAL ? 0.0f : 1.0f;
}

// This multiplier is not needed right now because Soulshatter is cast only when there are
// multiple enemies. That's probably not the right approach and should be fixed, so this multiplier
// remains here but commented out in anticipation of a future correction to Soulshatter usage.
// float KaelthasSunstriderDisableWarlockTankSoulshatterMultiplier::GetValueInEncounter(Action* action)
// {
//     if (botAI->GetState() == BOT_STATE_NON_COMBAT)
//         return 1.0f;
//
//     if (bot->getClass() != CLASS_WARLOCK)
//         return 1.0f;
//
//     if (!dynamic_cast<CastSoulshatterAction*>(action))
//         return 1.0f;
//
//     Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
//     if (!kaelthas)
//         return 1.0f;
//
//     uint32 const phase = GetKaelthasTkPhase(kaelthas);
//     if (phase != PHASE_SINGLE_ADVISOR && phase != PHASE_ALL_ADVISORS)
//         return 1.0f;
//
//     Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
//     if (!IsAdvisorActive(capernian))
//         return 1.0f;
//
//     return IsCapernianTank(bot) ? 0.0f : 1.0f;
// }

float KaelthasSunstriderKeepDistanceFromCapernianMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<KaelthasSunstriderSpreadAndMoveAwayFromCapernianAction*>(action))
        return 1.0f;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    if (GetKaelthasTkPhase(kaelthas) != PHASE_SINGLE_ADVISOR)
        return 1.0f;

    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    if (!capernian)
        return 1.0f;

    return IsAdvisorActive(capernian) ? 0.0f : 1.0f;
}

float KaelthasSunstriderManageWeaponTankingMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    // Try to keep main tank from grabbing aggro on any weapon other than the axe.
    if (!IsTauntAction(bot, action) && !IsAoeThreatAction(bot, action))
        return 1.0f;

    if (!PlayerbotAI::IsMainTank(bot))
        return 1.0f;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    return GetKaelthasTkPhase(kaelthas) == PHASE_WEAPONS ? 0.0f : 1.0f;
}

float KaelthasSunstriderSuppressEquipUpgradeMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<EquipUpgradeAction*>(action) &&
        !dynamic_cast<EquipUpgradesPacketAction*>(action))
    {
        return 1.0f;
    }

    return AI_VALUE2(Unit*, "find target", "kael'thas sunstrider") ? 0.0f : 1.0f;
}

float KaelthasSunstriderManageAutomaticTargetingMultiplier::GetValueInEncounter(Action* action)
{
    bool const isDpsAssist = dynamic_cast<DpsAssistAction*>(action);

    if (!isDpsAssist && !dynamic_cast<TankAssistAction*>(action))
        return 1.0f;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    uint32 const phase = GetKaelthasTkPhase(kaelthas);
    if (phase == PHASE_NONE)
        return 1.0f;

    // Kael drops every attacker when he becomes unattackable for his RP scene.
    if (botAI->GetState() == BOT_STATE_NON_COMBAT &&
        !(phase == PHASE_FINAL && kaelthas->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE)))
    {
        return 1.0f;
    }

    if (isDpsAssist)
        return 0.0f;

    // TankAssistAction
    // All tanks are blocked during advisor and final phases.
    if (phase == PHASE_SINGLE_ADVISOR || phase == PHASE_ALL_ADVISORS || phase == PHASE_FINAL)
        return 0.0f;

    // Only main tank is blocked during weapons/transition phases.
    return PlayerbotAI::IsMainTank(bot) ? 0.0f : 1.0f;
}

float KaelthasSunstriderDisableDisperseMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "kael'thas sunstrider") ? 0.0f : 1.0f;
}

float KaelthasSunstriderPrepareForPhase3Multiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    if (dynamic_cast<KaelthasSunstriderHandleAdvisorRolesInPhase3Action*>(action))
        return 1.0f;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    if (GetKaelthasTkPhase(kaelthas) != PHASE_ALL_ADVISORS)
        return 1.0f;

    Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");
    if (PlayerbotAI::IsAssistHealOfIndex(bot, 0, true))
    {
        if (dynamic_cast<KaelthasSunstriderKiteThaladredAction*>(action))
            return 1.0f;

        return sanguinar && sanguinar->IsAlive() ? 0.0f : 1.0f;
    }

    if (!sanguinar || !sanguinar->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        return 1.0f;

    if (PlayerbotAI::IsMainTank(bot) || PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) ||
        IsCapernianTank(bot))
    {
        return 0.0f;
    }

    return 1.0f;
}

// Bloodlust/Heroism and other major cooldowns should be saved until Phase 3.
float KaelthasSunstriderDelayCooldownsMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!IsDpsCooldownAction(bot, action))
        return 1.0f;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    uint32 const phase = GetKaelthasTkPhase(kaelthas);
    if (phase == PHASE_NONE)
        return 1.0f;

    bool const isLustAction = bot->getClass() == CLASS_SHAMAN &&
        (dynamic_cast<CastBloodlustAction*>(action) ||
         dynamic_cast<CastHeroismAction*>(action));

    if (isLustAction && phase == PHASE_WEAPONS)
        return 0.0f;

    return phase == PHASE_SINGLE_ADVISOR || phase == PHASE_TRANSITION ? 0.0f : 1.0f;
}

float KaelthasSunstriderStaySpreadDuringGravityLapseMultiplier::GetValueInEncounter(Action* action)
{
    if (!bot->HasAura(Id(TkSpells::SPELL_GRAVITY_LAPSE_AURA)))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (PlayerbotAI::IsRanged(bot) && dynamic_cast<AttackAction*>(action))
        return 1.0f;

    return dynamic_cast<KaelthasSunstriderSpreadOutInMidairAction*>(action) ? 1.0f : 0.0f;
}
