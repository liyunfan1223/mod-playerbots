/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPMultipliers.h"
#include "ChooseTargetActions.h"
#include "DruidActions.h"
#include "EncounterHelpers.h"
#include "FollowActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "InstanceScript.h"
#include "MageActions.h"
#include "NonCombatActions.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "SWPActions.h"
#include "SWPEncounter_Felmyst.h"
#include "SWPEncounter_Kalec.h"
#include "SWPEncounter_KJ.h"
#include "SWPEncounter_Muru.h"
#include "SWPEncounter_Twins.h"
#include "SWPShared.h"
#include "ThreatManager.h"
#include "Timer.h"
#include "WipeAction.h"

using namespace SwpHelpers;
using namespace EncounterHelpers;

// Shared

// Without this, bots are able to drink after Kil'jaeden's Hands go down. I assume it's due to the
// issue reported here: https://github.com/azerothcore/azerothcore-wotlk/issues/22122.
float SunwellNoEncounterDrinkingMultiplier::GetValueInEncounter(Action* action)
{
    return dynamic_cast<DrinkAction*>(action) || dynamic_cast<EatAction*>(action) ? 0.0f : 1.0f;
}

// Trash

float VolatileFiendRestrictApproachMultiplier::GetValue(Action* action)
{
    if (IsEncounterInProgress(bot, SwpHelpers::SWP_MAP_ID))
        return 1.0f;

    if (!dynamic_cast<CastReachTargetSpellAction*>(action) &&
        !dynamic_cast<ReachTargetAction*>(action))
    {
        return 1.0f;
    }

    if (PlayerbotAI::IsTank(bot))
        return 1.0f;

    Creature* volatileFiend = botAI->GetCreature(AI_VALUE(ObjectGuid, "swp volatile fiend"));
    if (!volatileFiend || !volatileFiend->IsAlive())
        return 1.0f;

    return bot->GetExactDist2d(volatileFiend) < VOLATILE_FIEND_APPROACH_SUPPRESSION_RADIUS ?
        0.0f : 1.0f;
}

// Shared Boss

float SunwellControlMisdirectionMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 1.0f;

    bool isHeldBoss = false;
    for (auto const& [guid, ref] : bot->GetThreatMgr().GetThreatenedByMeList())
    {
        Unit* unit = ref->GetOwner();
        if (!unit)
            continue;

        switch (unit->GetEntry())
        {
            case Id(SwpNpcs::NPC_ENTROPIUS):
                return 1.0f;

            case Id(SwpNpcs::NPC_KALECGOS_DRAGON):
            case Id(SwpNpcs::NPC_BRUTALLUS):
            case Id(SwpNpcs::NPC_GRAND_WARLOCK_ALYTHESS):
            case Id(SwpNpcs::NPC_MURU):
            case Id(SwpNpcs::NPC_KILJAEDEN):
                isHeldBoss = true;
                break;

            default:
                break;
        }
    }

    return isHeldBoss ? 0.0f : 1.0f;
}

float SunwellDelayDpsCooldownsMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!IsDpsCooldownAction(bot, action))
        return 1.0f;

    // TEMP LOG (Kil'jaeden cooldown hold), remove after testing
    if (getMSTimeDiff(_tempLogMs, getMSTime()) >= 1000)
    {
        _tempLogMs = getMSTime();
        Unit* tempHand = AI_VALUE2(Unit*, "find target", "hand of the deceiver");
        Unit* tempKiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
        if (tempHand || tempKiljaeden)
        {
            LOG_INFO("playerbots",
                "[SWP cooldowns] {} {}: hand {}, kil'jaeden {} (entry {}, {:.0f}%)",
                bot->GetName(), action->getName(), tempHand ? "found" : "none",
                tempKiljaeden ? "found" : "none", tempKiljaeden ? tempKiljaeden->GetEntry() : 0,
                tempKiljaeden ? tempKiljaeden->GetHealthPct() : 0.0f);
        }
    }

    Unit* hand = nullptr;
    Unit* kiljaeden = nullptr;
    Unit* muru = nullptr;
    Unit* entropius = nullptr;
    Unit* alythess = nullptr;
    Unit* sacrolash = nullptr;
    Unit* felmyst = nullptr;
    Unit* brutallus = nullptr;
    Unit* kalecgos = nullptr;
    for (auto const& [guid, ref] : bot->GetThreatMgr().GetThreatenedByMeList())
    {
        Unit* unit = ref->GetOwner();
        if (!unit)
            continue;

        switch (unit->GetEntry())
        {
            case Id(SwpNpcs::NPC_HAND_OF_THE_DECEIVER):
                hand = unit;
                break;

            case Id(SwpNpcs::NPC_KILJAEDEN):
                kiljaeden = unit;
                break;

            case Id(SwpNpcs::NPC_MURU):
                muru = unit;
                break;

            case Id(SwpNpcs::NPC_ENTROPIUS):
                entropius = unit;
                break;

            case Id(SwpNpcs::NPC_GRAND_WARLOCK_ALYTHESS):
                alythess = unit;
                break;

            case Id(SwpNpcs::NPC_LADY_SACROLASH):
                sacrolash = unit;
                break;

            case Id(SwpNpcs::NPC_FELMYST):
                felmyst = unit;
                break;

            case Id(SwpNpcs::NPC_BRUTALLUS):
                brutallus = unit;
                break;

            case Id(SwpNpcs::NPC_KALECGOS_DRAGON):
                kalecgos = unit;
                break;

            default:
                break;
        }
    }

    // Kil'jaeden is summoned only when the last Hand dies.
    if (hand)
        return 0.0f;

    bool const isBloodlust = bot->getClass() == CLASS_SHAMAN &&
        (dynamic_cast<CastHeroismAction*>(action) || dynamic_cast<CastBloodlustAction*>(action));

    if (kiljaeden)
    {
        if (kiljaeden->GetHealthPct() <= KILJAEDEN_PHASE5_HP_THRESHOLD)
            return 1.0f;

        if (isBloodlust)
            return 0.0f;

        return kiljaeden->GetHealthPct() > KILJAEDEN_PHASE3_HP_THRESHOLD ? 0.0f : 1.0f;
    }

    if (muru)
    {
        if (entropius && entropius->GetHealthPct() < BOSS_ENGAGED_HEALTH_PCT)
            return 1.0f;

        if (isBloodlust) // Bloodlust is saved for Entropius.
            return 0.0f;

        // Other dps cooldowns can be used on M'uru after the pull.
        return muru->GetHealthPct() > MURU_MAX_DPS_HP_PERCENT ? 0.0f : 1.0f;
    }

    if (alythess)
    {
        return sacrolash && sacrolash->GetHealthPct() > EREDAR_TWINS_MAX_DPS_HP_PERCENT ?
            0.0f : 1.0f;
    }

    if (felmyst)
    {
        if (felmyst->IsFlying())
            return 0.0f;

        return felmyst->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT ? 0.0f : 1.0f;
    }

    if (brutallus)
        return brutallus->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT ? 0.0f : 1.0f;

    if (kalecgos) // Use cooldowns against Sathrovarr only.
        return IsInSpectralRealm(bot) ? 1.0f : 0.0f;

    return 1.0f;
}

// Kalecgos

float KalecgosWaitToDecurseMultiplier::GetValueInEncounter(Action* action)
{
    if (bot->getClass() != CLASS_DRUID && bot->getClass() != CLASS_MAGE &&
        bot->getClass() != CLASS_SHAMAN)
    {
        return 1.0f;
    }

    if (!dynamic_cast<CastRemoveCurseAction*>(action) &&
        !dynamic_cast<CastRemoveCurseOnPartyAction*>(action) &&
        !dynamic_cast<CastCleanseSpiritAction*>(action) &&
        !dynamic_cast<CastCleanseSpiritCurseOnPartyAction*>(action) &&
        !dynamic_cast<CastDruidRemoveCurseOnPartyAction*>(action))
    {
        return 1.0f;
    }

    if (!AI_VALUE2(Unit*, "find target", "kalecgos"))
        return 1.0f;

    Unit* target = AI_VALUE2(Unit*, "party member to dispel", DISPEL_CURSE);
    if (!target)
        return 1.0f;

    // Like with Illidan's Shadowfiends, the spread from player-to-player is a separate spell.
    Aura* curse = target->GetAura(Id(SwpSpells::SPELL_CURSE_OF_BOUNDLESS_AGONY));
    if (!curse)
        curse = target->GetAura(Id(SwpSpells::SPELL_CURSE_OF_BOUNDLESS_AGONY_SEC));

    return curse && curse->GetDuration() >= KALECGOS_DISPEL_REMAINING_MS ? 0.0f : 1.0f;
}

float KalecgosControlMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<FollowAction*>(action) && !dynamic_cast<FleeAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    return kalecgos && !kalecgos->IsFriendlyTo(bot) ? 0.0f : 1.0f;
}

// Avoid dueling taunts in the surface and spectral realms
float KalecgosRestrictTauntMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!IsTauntAction(bot, action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "kalecgos"))
        return 1.0f;

    if (!IsInSpectralRealm(bot))
        return FindKalecgosDesignatedTank(bot) == bot ? 1.0f : 0.0f;

    Unit* sathrovarr = AI_VALUE2(Unit*, "find target", "sathrovarr the corruptor");
    if (!sathrovarr)
        return 1.0f;

    Unit* victim = sathrovarr->GetVictim();
    Player* victimPlayer = victim ? victim->ToPlayer() : nullptr;
    return victimPlayer && PlayerbotAI::IsTank(victimPlayer) ? 0.0f : 1.0f;
}

float KalecgosSuppressAssistTankPullThreatMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "kalecgos"))
        return 1.0f;

    if (!PlayerbotAI::IsAssistTank(bot))
        return 1.0f;

    KalecgosEncounterState const* state = FindKalecgosState(bot->GetInstanceId());
    if (!state || !state->encounterStartMs)
        return 1.0f;

    return getMSTimeDiff(state->encounterStartMs, getMSTime()) <
        KALECGOS_PULL_THREAT_SUPPRESSION_MS ? 0.0f : 1.0f;
}

float KalecgosEnterSpectralRiftMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<KalecgosEnterSpectralRiftAction*>(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "kalecgos"))
        return 1.0f;

    if (!ShouldEnterKalecgosPortal(bot))
        return 1.0f;

    return botAI->GetGameObject(AI_VALUE(ObjectGuid, "kalecgos spectral rift")) ? 0.0f : 1.0f;
}

// Brutallus

float BrutallusControlMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<FollowAction*>(action) &&
        !dynamic_cast<ReachTargetAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<FleeAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action) &&
        !dynamic_cast<CastDisengageAction*>(action) &&
        !dynamic_cast<CastBlinkBackAction*>(action))
    {
        return 1.0f;
    }

    return AI_VALUE2(Unit*, "find target", "brutallus") ? 0.0f : 1.0f;
}

// Don't use KS if any melee member (other than the Brutallus tanks) has Burn
float BrutallusNoKillingSpreeWhenNearbyBurnMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_ROGUE)
        return 1.0f;

    if (!dynamic_cast<CastKillingSpreeAction*>(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "brutallus"))
        return 1.0f;

    Group* group = bot->GetGroup();
    if (!group)
        return 1.0f;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->HasAura(Id(SwpSpells::SPELL_BURN)))
            continue;

        if (PlayerbotAI::IsMelee(member) && !PlayerbotAI::IsMainTank(member) &&
            !PlayerbotAI::IsAssistTankOfIndex(member, 0, true))
        {
            return 0.0f;
        }
    }

    return 1.0f;
}

float BrutallusRestrictTauntMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!IsTauntAction(bot, action))
        return 1.0f;

    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    if (!brutallus)
        return 1.0f;

    Unit* victim = brutallus->GetVictim();
    if (!victim)
        return 1.0f;

    Player* playerVictim = victim->ToPlayer();
    return playerVictim && PlayerbotAI::IsTank(playerVictim) ? 0.0f : 1.0f;
}

// Felmyst

float FelmystControlMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<FleeAction*>(action) &&
        !dynamic_cast<CastDisengageAction*>(action) &&
        !dynamic_cast<CastBlinkBackAction*>(action))
    {
        return 1.0f;
    }

    return AI_VALUE2(Unit*, "find target", "felmyst") ? 0.0f : 1.0f;
}

float FelmystWaitForLandingDpsMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return 1.0f;

    if (PlayerbotAI::IsMainTank(bot))
        return 1.0f;

    FelmystEncounterState const* state = FindFelmystState(felmyst->GetInstanceId());
    return state && state->landingDpsWaitStartMs ? 0.0f : 1.0f;
}

float FelmystPrioritizeEncapsulateAvoidanceMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<FelmystRunAwayFromEncapsulatedPlayerAction*>(action))
        return 1.0f;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || felmyst->IsFlying())
        return 1.0f;

    return GetFelmystEncapsulateTarget(bot) ? 0.0f : 1.0f;
}

float FelmystPrioritizeFogAvoidanceMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<FelmystMoveToSafeFogLaneAction*>(action))
        return 1.0f;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || !felmyst->IsFlying())
        return 1.0f;

    return IsFelmystFogMovementSuppressed(felmyst) ? 0.0f : 1.0f;
}

float FelmystPrioritizeDemonicVaporAvoidanceMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<FollowAction*>(action) &&
        !dynamic_cast<ReachTargetAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || !felmyst->IsFlying())
        return 1.0f;

    if (IsFelmystFogActiveForBot(bot, felmyst))
        return 1.0f;

    return IsFelmystLanding(felmyst) ? 1.0f : 0.0f;
}

float FelmystFocusAttacksOnCharmedPlayerMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsDps(bot))
        return 1.0f;

    // The charmed player is still friendly to group members so is considered to be an
    // invalid target by bots; blocking "drop target" allows them to be attacked
    if (!dynamic_cast<DpsAssistAction*>(action) && !dynamic_cast<DropTargetAction*>(action))
        return 1.0f;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return 1.0f;

    Player* charmedPlayer = GetFelmystCharmedTarget(bot, felmyst);
    if (!charmedPlayer)
        return 1.0f;

    // Melee: attack only during flight phase when the charmed player is in melee range
    if (PlayerbotAI::IsMelee(bot) &&
        (!felmyst->IsFlying() || !bot->IsWithinMeleeRange(charmedPlayer)))
    {
        return 0.0f;
    }

    // Ranged: attack at any time the charmed player is in general spell range
    return PlayerbotAI::IsRanged(bot) &&
        bot->GetExactDist2d(charmedPlayer) < FELMYST_CHARMED_TARGET_RANGE ? 0.0f : 1.0f;
}

float FelmystDontDotAddsMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<CastDebuffSpellOnAttackerAction*>(action))
        return 1.0f;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || !felmyst->IsFlying())
        return 1.0f;

    return action->GetTarget() == felmyst ? 1.0f : 0.0f;
}

// Eredar Twins

float EredarTwinsDisableAutoTargetingMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<DpsAssistAction*>(action) && !dynamic_cast<TankAssistAction*>(action) &&
        !dynamic_cast<CastDebuffSpellOnAttackerAction*>(action))
    {
        return 1.0f;
    }

    return AI_VALUE2(Unit*, "find target", "grand warlock alythess") ? 0.0f : 1.0f;
}

float EredarTwinsHoldDpsAtStartMultiplier::GetValueInEncounter(Action* action)
{
    if (PlayerbotAI::IsTank(bot))
        return 1.0f;

    // No AttackAction block. Commencing auto-attack activates combat engines and gets bots
    // positioned, but don't use abilities.
    if (!dynamic_cast<CastSpellAction*>(action))
        return 1.0f;

    if (dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    if (PlayerbotAI::IsMelee(bot) && bot->GetPositionZ() > EREDAR_TWINS_BALCONY_Z)
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "lady sacrolash"))
        return 1.0f;

    EredarTwinsEncounterState const* state = FindEredarTwinsState(bot->GetInstanceId());
    if (!state || !state->dpsHoldStartMs)
        return 0.0f;

    return getMSTimeDiff(*state->dpsHoldStartMs, getMSTime()) <
        EREDAR_TWINS_DPS_HOLD_MS ? 0.0f : 1.0f;
}

float EredarTwinsControlThreatMultiplier::GetValueInEncounter(Action* action)
{
    if (PlayerbotAI::IsHeal(bot))
        return 1.0f;

    if (!dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<EredarTwinsDpsPrioritizeSacrolashAction*>(action))
        return 1.0f;

    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");

    bool const shouldHoldSacrolashThreat =
        sacrolash && !PlayerbotAI::IsTank(bot) && ShouldHoldSacrolashThreat(bot, sacrolash);
    bool const shouldHoldAlythessThreat = alythess && ShouldHoldAlythessThreat(bot, alythess);

    if (!shouldHoldSacrolashThreat && !shouldHoldAlythessThreat)
        return 1.0f;

    Unit* actionTarget = action->GetTarget();
    bool const suppressSacrolashAttack = shouldHoldSacrolashThreat &&
        (actionTarget == sacrolash || AI_VALUE(Unit*, "current target") == sacrolash);
    bool const suppressAlythessAttack = shouldHoldAlythessThreat &&
        (actionTarget == alythess || AI_VALUE(Unit*, "current target") == alythess);

    return suppressSacrolashAttack || suppressAlythessAttack ? 0.0f : 1.0f;
}

float EredarTwinsControlMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    bool const isReachAction =
        dynamic_cast<ReachTargetAction*>(action) ||
        dynamic_cast<CastReachTargetSpellAction*>(action);

    if (!isReachAction &&
        !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<FleeAction*>(action) &&
        !dynamic_cast<CastDisengageAction*>(action) &&
        !dynamic_cast<CastBlinkBackAction*>(action) &&
        !dynamic_cast<CastKillingSpreeAction*>(action) &&
        !(PlayerbotAI::IsTank(bot) && dynamic_cast<AvoidAoeAction*>(action)))
    {
        return 1.0f;
    }

    if (!AI_VALUE2(Unit*, "find target", "grand warlock alythess"))
        return 1.0f;

    if (!isReachAction)
        return 0.0f;

    return PlayerbotAI::IsRanged(bot) || IsAlythessTank(bot) ? 0.0f : 1.0f;
}

float EredarTwinsIsolateConflagrationMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<ReachTargetAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<EredarTwinsConflagrationTargetMoveFromGroupAction*>(action) ||
        dynamic_cast<EredarTwinsMoveAwayFromSacrolashVictimAction*>(action))
    {
        return 1.0f;
    }

    if (!AI_VALUE2(Unit*, "find target", "grand warlock alythess"))
        return 1.0f;

    Player* conflagTarget = GetEredarTwinsConflagrationTarget(bot);
    if (!conflagTarget)
        return 1.0f;

    // Block movement for bot targeted by Conflagration, unless the target is a Rogue that has
    // vanished and caused Alythess to drop the target.
    if (conflagTarget == bot)
        return bot->getClass() == CLASS_ROGUE && botAI->HasAura("vanish", bot) ? 1.0f : 0.0f;

    if (IsAlythessTank(bot))
        return 1.0f;

    // If Sacrolash's victim is targeted by Conflagration, block actions that move toward Sacrolash.
    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");
    if (!sacrolash)
        return 1.0f;

    Unit* victim = sacrolash->GetVictim();
    return victim && victim != bot && conflagTarget == victim ? 0.0f : 1.0f;
}

// M'uru

float MuruDisableDefaultTargetingMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    bool const isDpsAssist = dynamic_cast<DpsAssistAction*>(action);
    bool const isTankAssist = dynamic_cast<TankAssistAction*>(action);

    if (!isDpsAssist && !isTankAssist && !dynamic_cast<CastDebuffSpellOnAttackerAction*>(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "m'uru"))
        return 1.0f;

    if (isDpsAssist)
        return 0.0f;

    if (isTankAssist && PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) &&
        AI_VALUE2(Unit*, "find target", "void sentinel"))
    {
        return 0.0f;
    }

    // Disable secondary dots on void spawn
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    return currentTarget && currentTarget->GetEntry() == Id(SwpNpcs::NPC_VOID_SPAWN) ? 0.0f : 1.0f;
}

float MuruControlMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    bool const isReachAction =
        dynamic_cast<ReachTargetAction*>(action) ||
        dynamic_cast<CastReachTargetSpellAction*>(action);

    if (!isReachAction &&
        !dynamic_cast<FollowAction*>(action) &&
        !dynamic_cast<FleeAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<CastDisengageAction*>(action) &&
        !dynamic_cast<CastBlinkBackAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    if (!muru)
        return 1.0f;

    if (!isReachAction)
        return 0.0f;

    // Remainder is checking only for validity of reach actions

    if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) &&
        AI_VALUE2(Unit*, "find target", "void sentinel"))
    {
        return 1.0f;
    }

    if (!PeekMuruDarknessActiveState(bot))
        return 1.0f;

    auto const isReachTargetSafeFromDarkness = [&](Action* action) -> bool
    {
        Unit* actionTarget = action->GetTarget();
        if (!actionTarget)
            return false;

        float const targetDistFromMuru = muru->GetExactDist2d(actionTarget);
        Position const& refPosition = PlayerbotAI::IsAssistTankOfIndex(bot, 1, true) ?
            MURU_ENTRANCE_POSITION : MURU_STACK_POSITION;
        float const targetDistFromRef = actionTarget->GetExactDist2d(refPosition);

        return targetDistFromMuru > MURU_DARKNESS_SAFE_DISTANCE &&
            targetDistFromRef < MURU_HOLDING_POSITION_RADIUS;
    };

    if (isReachTargetSafeFromDarkness(action))
        return 1.0f;

    return PlayerbotAI::IsTank(bot) && !PeekMuruDarknessEarlyState(bot) ? 1.0f : 0.0f;
}

// Kil'jaeden <The Deceiver>

float KiljaedenSingleTargetHandsMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    // The only Shaman spell classified as ActionThreatType::Aoe is Chain Lightning, which is a
    // strong single-target spell in addition to providing AoE damage.
    if (bot->getClass() == CLASS_SHAMAN)
        return 1.0f;

    if (!PlayerbotAI::IsDps(bot))
        return 1.0f;

    if (!dynamic_cast<CastDebuffSpellOnAttackerAction*>(action) &&
        action->getThreatType() != Action::ActionThreatType::Aoe)
    {
        return 1.0f;
    }

    return AI_VALUE2(Unit*, "find target", "hand of the deceiver") ? 0.0f : 1.0f;
}

float KiljaedenControlMovementAndTargetingMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<FleeAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<CastDisengageAction*>(action) &&
        !dynamic_cast<CastBlinkBackAction*>(action) &&
        !(dynamic_cast<TankAssistAction*>(action) && PlayerbotAI::IsMainTank(bot)))
    {
        return 1.0f;
    }

    return AI_VALUE2(Unit*, "find target", "kil'jaeden") ? 0.0f : 1.0f;
}

float KiljaedenPrioritizeDarknessProtectionMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<KiljaedenStackForShieldOfTheBlueAction*>(action))
        return 1.0f;

    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden)
        return 1.0f;

    if (HasKiljaedenDragonAura(bot))
        return 1.0f;

    return IsKiljaedenCastingDarknessOfAThousandSouls(kiljaeden) ? 0.0f : 1.0f;
}

float KiljaedenControlDragonMultiplier::GetValueInEncounter(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "kil'jaeden"))
        return 1.0f;

    if (dynamic_cast<KiljaedenDragonBuffAndProtectRaidAction*>(action))
        return 1.0f;

    if (dynamic_cast<WipeAction*>(action))
        return 1.0f;

    return HasKiljaedenDragonAura(bot) ? 0.0f : 1.0f;
}
