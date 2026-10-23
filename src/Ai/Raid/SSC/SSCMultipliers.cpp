/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SSCMultipliers.h"
#include "ChooseTargetActions.h"
#include "DruidActions.h"
#include "DruidBearActions.h"
#include "DruidCatActions.h"
#include "DruidShapeshiftActions.h"
#include "EncounterHelpers.h"
#include "FishingAction.h"
#include "FollowActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "LootAction.h"
#include "MageActions.h"
#include "MoveSpline.h"
#include "NonCombatActions.h"
#include "PaladinActions.h"
#include "Playerbots.h"
#include "PriestActions.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "SSCActions.h"
#include "SSCHelpers.h"
#include "ShamanActions.h"
#include "WarlockActions.h"
#include "WarriorActions.h"
#include <algorithm>

using namespace SscHelpers;
using namespace EncounterHelpers;

namespace
{

bool IsEnchantedElemental(Unit* unit)
{
    return unit && unit->GetEntry() == Id(SscNpcs::NPC_ENCHANTED_ELEMENTAL);
}

bool IsRepositionAction(Player* bot, Action* action)
{
    return (bot->getClass() == CLASS_HUNTER && dynamic_cast<CastDisengageAction*>(action)) ||
        (bot->getClass() == CLASS_MAGE && dynamic_cast<CastBlinkBackAction*>(action));
}

bool IsAoeTauntAction(Player* bot, Action* action)
{
    switch (bot->getClass())
    {
        case CLASS_DRUID:
            return dynamic_cast<CastChallengingRoarAction*>(action);
        case CLASS_PALADIN:
            return dynamic_cast<CastRighteousDefenseAction*>(action);
        case CLASS_WARRIOR:
            return dynamic_cast<CastChallengingShoutAction*>(action);
        default:
            return false;
    }
}

bool IsMeleeReachSpell(Player* bot, Action* action)
{
    return dynamic_cast<CastReachTargetSpellAction*>(action) ||
        (bot->getClass() == CLASS_ROGUE && dynamic_cast<CastKillingSpreeAction*>(action));
}

bool IsDpsHoldCandidate(Player* bot, Action* action)
{
    if (!dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<AttackAction*>(action))
        return false;

    if (!PlayerbotAI::IsTank(bot) && dynamic_cast<CastHealingSpellAction*>(action))
        return false;

    // AttackAction for healers is used only to keep them in their combat engines. Their damage,
    // including from wanding, all comes from CastSpellAction.
    return !dynamic_cast<AttackAction*>(action) || !PlayerbotAI::IsHeal(bot);
}

// Healing, buffs etc. go through a dps hold. Totems don't because some are offensive.
float GetDpsHoldValue(Player* bot, Action* action)
{
    if (bot->getClass() == CLASS_SHAMAN && dynamic_cast<CastTotemAction*>(action))
        return 0.0f;

    bool const castOnRaidSpell = dynamic_cast<CastBuffSpellAction*>(action) ||
        dynamic_cast<CastCureSpellAction*>(action) ||
        dynamic_cast<CurePartyMemberAction*>(action) ||
        dynamic_cast<ResurrectPartyMemberAction*>(action) ||
        dynamic_cast<CastProtectSpellAction*>(action);

    return castOnRaidSpell ? 1.0f : 0.0f;
}

bool IsAnyVashjAddUntanked(PlayerbotAI* botAI)
{
    auto const& adds =
        botAI->GetAiObjectContext()->GetValue<VashjAddGuids>("ssc vashj adds")->RefGet();
    for (GuidVector const* guids : { &adds.elites, &adds.striders })
    {
        for (ObjectGuid const& guid : *guids)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->IsAlive() && !IsVashjAddHeldByTank(unit))
                return true;
        }
    }

    return false;
}

} // end anonymous namespace

// Shared

float SscControlMisdirectionMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 1.0f;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (vashj && GetLadyVashjPhase(vashj) != 1)
        return 0.0f;

    if (Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind"))
    {
        if (HasInnerDemon(bot))
            return 0.0f;

        if (IsLeotherasChannelingWhirlwind(leotheras))
            return 0.0f;

        if (GetLeotherasDemonOrShadow(botAI) && GetLeotherasWarlockTank(bot))
            return 0.0f;
    }

    return AI_VALUE2(Unit*, "find target", "fathom-lord karathress") ||
        AI_VALUE2(Unit*, "find target", "hydross the unstable") ? 0.0f : 1.0f;
}

float SscDelayDpsCooldownsMultiplier::GetValue(Action* action)
{
    if (bot->GetMapId() != SSC_MAP_ID || botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!IsDpsCooldownAction(bot, action))
        return 1.0f;

    bool const isBloodlust = bot->getClass() == CLASS_SHAMAN &&
        (dynamic_cast<CastBloodlustAction*>(action) || dynamic_cast<CastHeroismAction*>(action));

    // Vashj: Bloodlust/Heroism are phase 3 only; other dps cooldowns can be used from phase 2.
    if (Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj"))
    {
        int8 const phase = GetLadyVashjPhase(vashj);
        if (phase == 3)
            return 1.0f;

        return !isBloodlust && phase == 2 ? 1.0f : 0.0f;
    }

    // Tidewalker: Bloodlust/Heroism are phase 2 only, once the raid is stacked in the corner.
    if (Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker"))
    {
        if (isBloodlust)
            return tidewalker->GetHealthPct() <= TIDEWALKER_PHASE_2_HEALTH_PCT ? 1.0f : 0.0f;

        return tidewalker->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT ? 0.0f : 1.0f;
    }

    // Karathress: Hold until Tidalvess, the first council member in the kill order, is under 95%.
    if (AI_VALUE2(Unit*, "find target", "fathom-lord karathress"))
    {
        Unit* tidalvess = AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");
        return tidalvess && tidalvess->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT ? 0.0f : 1.0f;
    }

    // Lurker: No dps cooldowns while spouting or submerged.
    if (Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below"))
    {
        if (!IsLurkerSurfacedAndCalm(lurker))
            return 0.0f;

        return lurker->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT ? 0.0f : 1.0f;
    }

    // Hydross and Leotheras: Only the standard hold on pull until 95%.
    if (Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable"))
        return hydross->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT ? 0.0f : 1.0f;

    if (bot->GetExactDist(LEOTHERAS_SPAWN_POSITION) > LEOTHERAS_SEARCH_DISTANCE)
        return 1.0f;

    Unit* leotheras = GetLeotheras(botAI);
    return leotheras && leotheras->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT ? 0.0f : 1.0f;
}

// Practically, just for Lurker. Bots can briefly transition to non-combat engines during avoidance,
// and if they have +master fishing, they can drift to look for water to fish in.
float SscNoFishingDuringEncounterMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_COMBAT)
        return 1.0f;

    return dynamic_cast<MoveNearWaterAction*>(action) || dynamic_cast<FishingAction*>(action) ?
        0.0f : 1.0f;
}

// Trash

float UnderbogColossusHoldNearToxicPoolMultiplier::GetValue(Action* action)
{
    if (bot->GetMapId() != SSC_MAP_ID || IsEncounterInProgress(bot, SSC_MAP_ID))
        return 1.0f;

    if (dynamic_cast<DrinkAction*>(action) || dynamic_cast<EatAction*>(action))
        return IsNearToxicPool(botAI, TOXIC_POOL_HOLDING_RADIUS) ? 0.0f : 1.0f;

    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<UnderbogColossusEscapeToxicPoolAction*>(action))
        return 1.0f;

    return IsNearToxicPool(botAI, TOXIC_POOL_HOLDING_RADIUS) ? 0.0f : 1.0f;
}

// Hydross the Unstable <Duke of Currents>

float HydrossTheUnstableDisableOffPhaseTankActionsMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (dynamic_cast<HydrossTheUnstablePositionAndSwapTanksAction*>(action))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) && !IsTauntAction(bot, action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return 1.0f;

    bool const isOffPhaseTank =
        IsHydrossInFrostPhase(hydross) ? IsHydrossNatureTank(bot) : IsHydrossFrostTank(bot);
    return isOffPhaseTank ? 0.0f : 1.0f;
}

float HydrossTheUnstableDisablePhaseTankAssistMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<TankAssistAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action))
    {
        return 1.0f;
    }

    if (!AI_VALUE2(Unit*, "find target", "hydross the unstable"))
        return 1.0f;

    return IsHydrossPhaseTank(bot) ? 0.0f : 1.0f;
}

// Phase changes reset threat. Hold dps from 1s after Marks hit 100% until 5s post-phase change.
float HydrossTheUnstableWaitForDpsMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsDpsHoldCandidate(bot, action))
        return 1.0f;

    if (dynamic_cast<HydrossTheUnstablePositionAndSwapTanksAction*>(action))
        return 1.0f;

    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross || GetHydrossDpsHoldWindow(hydross) == HydrossDpsHoldWindow::None)
        return 1.0f;

    if (PlayerbotAI::IsTank(bot) &&
        !(IsHydrossInFrostPhase(hydross) ? IsHydrossNatureTank(bot) : IsHydrossFrostTank(bot)))
    {
        return 1.0f;
    }

    return GetDpsHoldValue(bot, action);
}

// The Lurker Below

float TheLurkerBelowStayAwayFromSpoutMultiplier::GetValueInEncounter(Action* action)
{
    bool const castTotem =
        bot->getClass() == CLASS_SHAMAN && dynamic_cast<CastTotemAction*>(action);

    if (!castTotem && !dynamic_cast<MovementAction*>(action) &&
        !IsMeleeReachSpell(bot, action) && !IsRepositionAction(bot, action))
    {
        return 1.0f;
    }

    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<TheLurkerBelowRunAroundBehindBossAction*>(action))
        return 1.0f;

    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    return lurker && IsLurkerSpouting(lurker) ? 0.0f : 1.0f;
}

float TheLurkerBelowMaintainPositionsMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<CombatFormationMoveAction*>(action) && !dynamic_cast<FleeAction*>(action) &&
        !dynamic_cast<FollowAction*>(action) && !IsRepositionAction(bot, action))
    {
        return 1.0f;
    }

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "the lurker below") ? 0.0f : 1.0f;
}

float TheLurkerBelowTanksFocusAssignedGuardianMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<TankAssistAction*>(action) &&
        !IsTauntAction(bot, action) && !IsAoeThreatAction(bot, action))
    {
        return 1.0f;
    }

    std::optional<LurkerGuardianTankAssignments> const& assignments =
        SscState(bot->GetInstanceId()).lurkerGuardianTankAssignments;
    if (!assignments)
        return 1.0f;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || !target->IsAlive())
        return 1.0f;

    return std::find(assignments->begin(), assignments->end(), target->GetGUID()) !=
        assignments->end() ? 0.0f : 1.0f;
}

// Killing Spree puts bots right at the center of Lurker, and then they don't move back.
float TheLurkerBelowDisableKillingSpreeMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_ROGUE)
        return 1.0f;

    if (!dynamic_cast<CastKillingSpreeAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "the lurker below") ? 0.0f : 1.0f;
}

float TheLurkerBelowMeleeWaitToSetBehindMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsMelee(bot))
        return 1.0f;

    if (!dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return 1.0f;

    if (bot->GetExactDist2d(lurker) > LURKER_ISLET_DISTANCE)
        return 0.0f;

    if (AI_VALUE(Unit*, "current target") != lurker)
        return 1.0f;

    constexpr float tankSpotTolerance = 3.0f;
    Unit* victim = lurker->GetVictim();
    return victim && victim->GetExactDist2d(LURKER_MAIN_TANK_POSITION) <= tankSpotTolerance ?
        1.0f : 0.0f;
}

float TheLurkerBelowMeleeDisableReachMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsMelee(bot))
        return 1.0f;

    if (!dynamic_cast<ReachMeleeAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "the lurker below") ? 0.0f : 1.0f;
}

// Leotheras the Blind

float LeotherasTheBlindAvoidWhirlwindMultiplier::GetValueInEncounter(Action* action)
{
    if (PlayerbotAI::IsTank(bot) || HasInnerDemon(bot))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<LeotherasTheBlindRunAwayFromWhirlwindAction*>(action))
        return 1.0f;

    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    return leotheras && IsLeotherasChannelingWhirlwind(leotheras) ? 0.0f : 1.0f;
}

float LeotherasTheBlindDisableTankActionsMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsTank(bot) || HasInnerDemon(bot))
        return 1.0f;

    if (!dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<TankAssistAction*>(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "leotheras the blind"))
        return 1.0f;

    // Disable all spells from tanks if there is a Warlock tank. Instead, just auto-attack to build
    // rage in case the tank gets an Inner Demon.
    if (GetLeotherasDemon(botAI) && GetLeotherasWarlockTank(bot))
    {
        return bot->getClass() == CLASS_DRUID &&
            (dynamic_cast<CastDireBearFormAction*>(action) ||
             dynamic_cast<CastBearFormAction*>(action)) ? 1.0f : 0.0f;
    }

    if (bot->getClass() == CLASS_WARRIOR && dynamic_cast<CastVigilanceAction*>(action) &&
        GetLeotherasDemonOrShadow(botAI))
    {
        Player* warlockTank = GetLeotherasWarlockTank(bot);
        if (warlockTank && action->GetTarget() == warlockTank)
            return 0.0f;
    }

    if (bot->getClass() == CLASS_DRUID && dynamic_cast<CastBerserkAction*>(action))
        return GetShadowOfLeotheras(botAI) ? 1.0f : 0.0f; // Save Berserk for Inner Demon pre-P3.

    return 1.0f;
}

float LeotherasTheBlindMeleeAvoidChaosBlastMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsMelee(bot))
        return 1.0f;

    if (!dynamic_cast<AttackAction*>(action) && !dynamic_cast<ReachTargetAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action) && !IsMeleeReachSpell(bot, action))
    {
        return 1.0f;
    }

    if (dynamic_cast<LeotherasTheBlindDestroyInnerDemonAction*>(action))
        return 1.0f;

    if (!HasTooManyChaosBlastStacks(bot))
        return 1.0f;

    Creature* leotherasDemon = GetLeotherasDemonOrShadow(botAI);
    return leotherasDemon && leotherasDemon->GetVictim() != bot ? 0.0f : 1.0f;
}

float LeotherasTheBlindFocusOnInnerDemonMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!HasInnerDemon(bot))
        return 1.0f;

    if (action->getThreatType() == Action::ActionThreatType::Aoe)
        return 0.0f;

    if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<MeleeAction*>(action) &&
        !dynamic_cast<LeotherasTheBlindDestroyInnerDemonAction*>(action) &&
        !dynamic_cast<LeotherasTheBlindMeleeRunFromChaosBlastAction*>(action) &&
        !dynamic_cast<LeotherasTheBlindRangedKeepDistanceAction*>(action))
    {
        return 0.0f;
    }

    if (IsRepositionAction(bot, action))
        return 0.0f;

    // Exclude spells that prevent attacking or drop threat, AoEs with no threat type, useless
    // spells, and spells that need to be blocked to facilitate the custom Inner Demon action.
    switch (bot->getClass())
    {
        case CLASS_DRUID:
        {
            if (dynamic_cast<CastTreeFormAction*>(action))
                return 0.0f;
            break;
        }
        case CLASS_HUNTER:
        {
            if (dynamic_cast<CastDeterrenceAction*>(action) ||
                dynamic_cast<CastFeignDeathAction*>(action) ||
                dynamic_cast<CastWingClipAction*>(action) ||
                dynamic_cast<CastFreezingTrap*>(action) ||
                dynamic_cast<CastExplosiveTrapAction*>(action) ||
                dynamic_cast<CastImmolationTrapAction*>(action) ||
                dynamic_cast<CastAspectOfTheHawkAction*>(action) ||
                dynamic_cast<CastAspectOfTheWildAction*>(action) ||
                dynamic_cast<CastAspectOfTheDragonhawkAction*>(action) ||
                dynamic_cast<CastAspectOfThePackAction*>(action) ||
                dynamic_cast<CastAspectOfTheCheetahAction*>(action) ||
                dynamic_cast<CastAspectOfTheMonkeyAction*>(action))
            {
                return 0.0f;
            }
            break;
        }
        case CLASS_MAGE:
        {
            if (dynamic_cast<CastIceBlockAction*>(action) ||
                dynamic_cast<CastInvisibilityAction*>(action))
            {
                return 0.0f;
            }
            break;
        }
        case CLASS_PALADIN:
        {
            if (dynamic_cast<CastDivineShieldAction*>(action))
                return 0.0f;
            break;
        }
        case CLASS_PRIEST:
        {
            if (dynamic_cast<CastFadeAction*>(action))
                return 0.0f;
            break;
        }
        case CLASS_ROGUE:
        {
            if (dynamic_cast<CastFeintAction*>(action) || dynamic_cast<CastVanishAction*>(action))
                return 0.0f;
            break;
        }
        case CLASS_WARLOCK:
        {
            if (dynamic_cast<CastCurseOfDoomAction*>(action))
                return 0.0f;
            break;
        }
        case CLASS_WARRIOR:
        {
            if (dynamic_cast<CastThunderClapAction*>(action) ||
                dynamic_cast<CastCleaveAction*>(action) ||
                dynamic_cast<CastChallengingShoutAction*>(action) ||
                dynamic_cast<CastDemoralizingShoutAction*>(action) ||
                dynamic_cast<CastDemoralizingShoutWithoutLifeTimeCheckAction*>(action) ||
                dynamic_cast<CastShockwaveAction*>(action) ||
                dynamic_cast<CastPiercingHowlAction*>(action) ||
                dynamic_cast<CastIntimidatingShoutAction*>(action) ||
                dynamic_cast<CastSweepingStrikesAction*>(action) ||
                dynamic_cast<CastVigilanceAction*>(action))
            {
                return 0.0f;
            }
            break;
        }
        default:
            break;
    }

    // Exclude abilities with a target that isn't the bot or the Inner Demon, plus self heals.
    return dynamic_cast<DpsAssistAction*>(action) ||
        dynamic_cast<TankAssistAction*>(action) ||
        dynamic_cast<CastSnareSpellAction*>(action) ||
        dynamic_cast<CastHealingSpellAction*>(action) ||
        dynamic_cast<CastCureSpellAction*>(action) ||
        dynamic_cast<CurePartyMemberAction*>(action) ||
        dynamic_cast<ResurrectPartyMemberAction*>(action) ||
        dynamic_cast<PartyMemberActionNameSupport*>(action) ||
        dynamic_cast<MainTankActionNameSupport*>(action) ||
        dynamic_cast<GroupBuffSpellAction*>(action) ||
        dynamic_cast<CastProtectSpellAction*>(action) ||
        dynamic_cast<CastInnervateOnHealerAction*>(action) ||
        dynamic_cast<CastDebuffSpellOnAttackerAction*>(action) ||
        dynamic_cast<CastDebuffSpellOnMeleeAttackerAction*>(action) ? 0.0f : 1.0f;
}

float LeotherasTheBlindWaitForDpsMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsDpsHoldCandidate(bot, action))
        return 1.0f;

    if (HasInnerDemon(bot))
        return 1.0f;

    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras || !IsLeotherasDpsHoldActive(botAI, leotheras))
        return 1.0f;

    return GetDpsHoldValue(bot, action);
}

// Soulshatter is eligible to be cast when there are at least two attackers, which is the case in
// the final phase. This is needed to keep the Warlock tank from dropping threat on the Shadow.
float LeotherasTheBlindDisableTankSoulshatterMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_WARLOCK)
        return 1.0f;

    if (!dynamic_cast<CastSoulshatterAction*>(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "leotheras the blind"))
        return 1.0f;

    return GetLeotherasDemonOrShadow(botAI) && IsLeotherasWarlockTank(bot) ? 0.0f : 1.0f;
}

// Fathom-Lord Karathress

float FathomLordKarathressDisableTankActionsMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    bool const isStockMove =
        dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<AvoidAoeAction*>(action);
    if (!isStockMove && !IsAoeThreatAction(bot, action) && !IsAoeTauntAction(bot, action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "fathom-lord karathress"))
        return 1.0f;

    if (isStockMove)
        return 0.0f;

    return IsAnotherCouncilMemberWithin(botAI, KARATHRESS_AOE_THREAT_CLEARANCE) ? 0.0f : 1.0f;
}

float FathomLordKarathressDisableAutoTargetMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<DpsAssistAction*>(action) && !dynamic_cast<TankAssistAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "fathom-lord karathress") ? 0.0f : 1.0f;
}

float FathomLordKarathressDisableAoeMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsDps(bot))
        return 1.0f;

    auto castSpellAction = dynamic_cast<CastSpellAction*>(action);
    if (!castSpellAction || castSpellAction->getThreatType() != Action::ActionThreatType::Aoe)
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "fathom-lord karathress") ? 0.0f : 1.0f;
}

float FathomLordKarathressWaitForDpsMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsDpsHoldCandidate(bot, action))
        return 1.0f;

    if (PlayerbotAI::IsTank(bot))
        return 1.0f;

    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (!karathress)
        return 1.0f;

    std::optional<uint32> const& waitStart =
        SscState(karathress->GetInstanceId()).karathressDpsWaitTimer;
    if (waitStart && getMSTimeDiff(*waitStart, getMSTime()) >= KARATHRESS_DPS_WAIT_MS)
        return 1.0f;

    return GetDpsHoldValue(bot, action);
}

float FathomLordKarathressMaintainPositionMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "fathom-lord karathress"))
        return 1.0f;

    if (dynamic_cast<FollowAction*>(action) || dynamic_cast<FleeAction*>(action))
        return 0.0f;

    if (dynamic_cast<FathomLordKarathressPositionCaribdisTankHealerAction*>(action) ||
        dynamic_cast<FathomLordKarathressDropToGroundAfterCycloneAction*>(action))
    {
        return 1.0f;
    }

    if (!PlayerbotAI::IsHeal(bot))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "fathom-guard caribdis"))
        return 1.0f;

    return PlayerbotAI::IsAssistHealOfIndex(bot, 0, true) ? 0.0f : 1.0f;
}

// Casts are blocked during the Cyclone and the fall, both for accuracy and so the bot doesn't get
// stuck in midair (point moves fail mid-cast).
float FathomLordKarathressNoCastingWhileLiftedMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<CastSpellAction*>(action))
        return 1.0f;

    if (bot->HasAura(Id(SscSpells::SPELL_CYCLONE)))
        return 0.0f;

    if (bot->movespline->Finalized())
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "fathom-lord karathress"))
        return 1.0f;

    float const floorZ = bot->GetMapHeight(
        bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), true, MAX_FALL_DISTANCE);
    return floorZ > INVALID_HEIGHT && bot->GetPositionZ() - floorZ > CARIBDIS_CYCLONE_DROP_HEIGHT ?
        0.0f : 1.0f;
}

float FathomLordKarathressApproachingCaribdisMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsRangedDps(bot))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) ||
        dynamic_cast<FathomLordKarathressAssignDpsPriorityAction*>(action) ||
        dynamic_cast<FathomLordKarathressDropToGroundAfterCycloneAction*>(action))
    {
        return 1.0f;
    }

    Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
    if (!caribdis)
        return 1.0f;

    if (ShouldAttackSpitfireTotem(bot, GetSpitfireTotem(botAI)) ||
        AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess"))
    {
        return 1.0f;
    }

    return bot->IsWithinLOSInMap(caribdis) ? 1.0f : 0.0f;
}

// Runs to the totem and Caribdis can cause the bot to lose LoS, so assigned bots must be stopped
// from dropping their targets or they will instead switch to a council member in LoS.
float FathomLordKarathressDontDropOutOfSightTargetMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<DropTargetAction*>(action))
        return 1.0f;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return 1.0f;

    if (target->IsAlive() && target->GetEntry() == Id(SscNpcs::NPC_SPITFIRE_TOTEM))
        return 0.0f;

    return target == AI_VALUE2(Unit*, "find target", "fathom-guard caribdis") ? 0.0f : 1.0f;
}

// Morogrim Tidewalker

float MorogrimTidewalkerControlMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<CombatFormationMoveAction*>(action) && !dynamic_cast<FleeAction*>(action) &&
        !dynamic_cast<FollowAction*>(action) && !IsRepositionAction(bot, action))
    {
        return 1.0f;
    }

    if (PlayerbotAI::IsMelee(bot) && dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "morogrim tidewalker") ? 0.0f : 1.0f;
}

float MorogrimTidewalkerStayStackedMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsRanged(bot))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<MorogrimTidewalkerStackRangedBehindBossAction*>(action))
        return 1.0f;

    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!tidewalker || tidewalker->GetHealthPct() > TIDEWALKER_PHASE_2_MOVE_HEALTH_PCT)
        return 1.0f;

    return bot->GetExactDist(GetTidewalkerStackPoint(*bot, *tidewalker)) <=
        TIDEWALKER_RANGED_STACK_RADIUS ? 0.0f : 1.0f;
}

// Lady Vashj <Coilfang Matron>

float LadyVashjSetGroundingTotemMultiplier::GetValueInEncounter(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!dynamic_cast<CastWindfuryTotemAction*>(action) &&
        !dynamic_cast<SetWindfuryTotemAction*>(action) &&
        !dynamic_cast<CastWrathOfAirTotemAction*>(action) &&
        !dynamic_cast<SetWrathOfAirTotemAction*>(action) &&
        !dynamic_cast<CastNatureResistanceTotemAction*>(action) &&
        !dynamic_cast<SetNatureResistanceTotemAction*>(action))
    {
        return 1.0f;
    }

    if (GetVashjGroundingShaman(bot) != bot)
        return 1.0f;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return 1.0f;

    int8 const phase = GetLadyVashjPhase(vashj);
    return phase == 1 || phase == 3 ? 0.0f : 1.0f;
}

float LadyVashjMaintainPhase1RangedSpreadMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsRanged(bot))
        return 1.0f;

    if (!dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<FleeAction*>(action) && !IsRepositionAction(bot, action))
    {
        return 1.0f;
    }

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    return vashj && GetLadyVashjPhase(vashj) == 1 ? 0.0f : 1.0f;
}

float LadyVashjStaticChargeStayAwayFromGroupMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<ReachTargetAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<FollowAction*>(action) && !IsMeleeReachSpell(bot, action))
    {
        return 1.0f;
    }

    if (PlayerbotAI::IsRanged(bot))
        return HasVashjStaticCharge(bot) ? 0.0f : 1.0f;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    return vashj && ShouldAvoidVashjStaticCharge(bot, vashj) ? 0.0f : 1.0f;
}

// Bots won't pick up the Core, so ninja looting is not a concern. This multiplier is instead to
/// keep them from wasting time moving to the corpse to check for loot.
float LadyVashjNoUnauthorizedLootingMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_COMBAT)
        return 1.0f;

    if (!dynamic_cast<LootAction*>(action) && !dynamic_cast<OpenLootAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "lady vashj") ? 0.0f : 1.0f;
}

float LadyVashjCoreHandlersPrioritizePositioningMultiplier::GetValueInEncounter(Action* action)
{
    VashjCorePassingChain const* chain = GetVashjCorePassingChain(bot);
    if (!chain)
        return 1.0f;

    int8 const index = GetVashjCoreCatcherIndex(*chain, bot);
    bool const isOriginBot = chain->originBot == bot->GetGUID();
    if (index < 0 && !isOriginBot)
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) &&
        !IsMeleeReachSpell(bot, action) && !IsRepositionAction(bot, action))
    {
        return 1.0f;
    }

    if (dynamic_cast<AttackAction*>(action) ||
        dynamic_cast<LadyVashjPassTheTaintedCoreAction*>(action) ||
        dynamic_cast<LadyVashjLootTaintedCoreAction*>(action))
    {
        return 1.0f;
    }

    if (Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
        !vashj || GetLadyVashjPhase(vashj) != 2)
    {
        return 1.0f;
    }

    if (HasTaintedCore(bot))
        return 0.0f;

    if (isOriginBot)
    {
        Creature* tainted = GetAssignedTaintedElemental(bot);
        if (tainted && IsTaintedCoreStillToLoot(tainted))
            return 0.0f;
    }

    return index >= 0 && IsVashjCoreCatcherActive(bot, *chain, index) ? 0.0f : 1.0f;
}

float LadyVashjPhase2DisableAutoTargetAndMoveMultiplier::GetValueInEncounter(Action* action)
{
    bool const isAlwaysBlocked =
        dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action) ||
        dynamic_cast<FollowAction*>(action) || dynamic_cast<FleeAction*>(action);

    bool const isReachAction = dynamic_cast<ReachTargetAction*>(action);
    bool const isHealSpell = dynamic_cast<CastHealingSpellAction*>(action);
    bool const isDebuffOnAttacker = dynamic_cast<CastDebuffSpellOnAttackerAction*>(action);
    bool const isCombatFormationAction = dynamic_cast<CombatFormationMoveAction*>(action);
    bool const isDropTarget = dynamic_cast<DropTargetAction*>(action);

    if (!isAlwaysBlocked && !isReachAction && !isHealSpell && !isDebuffOnAttacker &&
        !isCombatFormationAction && !isDropTarget && !IsRepositionAction(bot, action))
    {
        return 1.0f;
    }

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || GetLadyVashjPhase(vashj) != 2)
        return 1.0f;

    if (isAlwaysBlocked)
        return 0.0f;

    // Keep healers in their combat engines.
    if (isDropTarget)
        return PlayerbotAI::IsHeal(bot) ? 0.0f : 1.0f;

    // Don't waste tank and dps mana on healing in their non-combat engines.
    if (isHealSpell)
        return PlayerbotAI::IsHeal(bot) ? 1.0f : 0.0f;

    // Don't put secondary dots on the Enchanted Elementals or untanked Striders/Elites.
    if (isDebuffOnAttacker)
    {
        if (IsEnchantedElemental(AI_VALUE(Unit*, "current target")))
            return 0.0f;

        return PlayerbotAI::IsTank(bot) || !IsAnyVashjAddUntanked(botAI) ? 1.0f : 0.0f;
    }

    // Disable disperse and tank face and, if the target is an Enchanted Elemental, set behind.
    if (isCombatFormationAction)
    {
        return dynamic_cast<SetBehindTargetAction*>(action) &&
            !IsEnchantedElemental(AI_VALUE(Unit*, "current target")) ? 1.0f : 0.0f;
    }

    // Ranged dps with a station slot attack from their assigned positions only, unless pursuing a
    // Tainted Elemental or stepping in to cast range of a Strider.
    if (PlayerbotAI::IsRangedDps(bot))
    {
        if (isReachAction && IsTankedStriderInStepInReach(bot, AI_VALUE(Unit*, "current target")))
            return 1.0f;

        return GetTaintedElementalToKill(bot) ? 1.0f : 0.0f;
    }

    // Healers with a station slot don't move in range attack or heal. Unassigned healers move in
    // range to heal but not to attack.
    if (isReachAction && PlayerbotAI::IsHeal(bot))
    {
        if (GetVashjStationSlot(bot).station >= 0 ||
            !dynamic_cast<ReachPartyMemberToHealAction*>(action))
        {
            return 0.0f;
        }
    }

    return 1.0f;
}

float LadyVashjPhase3DisableAutoTargetAndMoveMultiplier::GetValueInEncounter(Action* action)
{
    bool const isAssistAction =
        dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action);
    bool const isDebuffOnAttacker = dynamic_cast<CastDebuffSpellOnAttackerAction*>(action);
    bool const isHealerSpellReach =
        dynamic_cast<ReachSpellAction*>(action) && PlayerbotAI::IsHeal(bot);

    if (!isAssistAction && !isDebuffOnAttacker && !isHealerSpellReach &&
        !dynamic_cast<AvoidAoeAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<FollowAction*>(action) && !dynamic_cast<FleeAction*>(action))
    {
        return 1.0f;
    }

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || GetLadyVashjPhase(vashj) != 3)
        return 1.0f;

    // Healers move in range to heal but not to attack.
    if (isAssistAction || isHealerSpellReach)
        return 0.0f;

    if (isDebuffOnAttacker)
        return IsEnchantedElemental(AI_VALUE(Unit*, "current target")) ? 0.0f : 1.0f;

    // Getting behind the target, except an Enchanted Elemental, where it is a waste of time
    Unit* target = AI_VALUE(Unit*, "current target");
    if (dynamic_cast<SetBehindTargetAction*>(action))
        return IsEnchantedElemental(target) ? 0.0f : 1.0f;

    // Allow tanks to turn a target away only if it is an Elite (Cleave). This is permitted in
    // phase 3 because the fixed Elite tanking positions from phase 2 are lifted.
    if (dynamic_cast<TankFaceAction*>(action))
        return target && target->GetEntry() == Id(SscNpcs::NPC_COILFANG_ELITE) ? 1.0f : 0.0f;

    return 0.0f;
}

float LadyVashjSaveHandOfFreedomMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_PALADIN)
        return 1.0f;

    if (!dynamic_cast<CastHandOfFreedomOnPartyAction*>(action))
        return 1.0f;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    return vashj && GetLadyVashjPhase(vashj) == 3 ? 0.0f : 1.0f;
}

// Use only the custom melee reach action when near a Toxic Spore pool.
float LadyVashjMeleeControlSporeAvoidanceMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (!PlayerbotAI::IsMelee(bot) || PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (dynamic_cast<LadyVashjMeleeMoveAroundToxicSporesAction*>(action) ||
        dynamic_cast<LadyVashjAssignTargetPriorityAction*>(action) ||
        dynamic_cast<LadyVashjSetGroundingTotemInMainTankGroupAction*>(action))
    {
        return 1.0f;
    }

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || GetLadyVashjPhase(vashj) != 3 || !IsVashjRingMelee(bot, vashj))
        return 1.0f;

    return IsNearToxicSpores(botAI, TOXIC_SPORES_MELEE_CONTROL_RADIUS) ? 0.0f : 1.0f;
}

// Use only the custom ranged dps and healer reach actions when a straight line to the target is
// not entirely clear of Toxic Spore pools.
float LadyVashjRangedDoNotReachThroughSporesMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsCaster(bot))
        return 1.0f;

    bool const isHealerReach = dynamic_cast<ReachPartyMemberToHealAction*>(action);
    bool const isSpellReach = dynamic_cast<ReachSpellAction*>(action);

    if (PlayerbotAI::IsHeal(bot) ? !isHealerReach : !isSpellReach)
        return 1.0f;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || GetLadyVashjPhase(vashj) != 3)
        return 1.0f;

    Unit* target;
    float range;
    return GetVashjReachBlockedBySpores(botAI, target, range) ? 0.0f : 1.0f;
}
