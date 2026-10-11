/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SSCMultipliers.h"
#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DestroyItemAction.h"
#include "DruidActions.h"
#include "DruidBearActions.h"
#include "DruidCatActions.h"
#include "DruidShapeshiftActions.h"
#include "FollowActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "LootAction.h"
#include "MageActions.h"
#include "PaladinActions.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "SSCActions.h"
#include "SSCHelpers.h"
#include "ShamanActions.h"
#include "WarlockActions.h"
#include "WarriorActions.h"
#include "WipeAction.h"

using namespace SerpentShrineCavernHelpers;

// Trash

float UnderbogColossusEscapeToxicPoolMultiplier::GetValue(Action* action)
{
    if (bot->HasAura(SPELL_TOXIC_POOL) &&
        dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<UnderbogColossusEscapeToxicPoolAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Hydross the Unstable <Duke of Currents>

float HydrossTheUnstableDisableTankActionsMultiplier::GetValue(Action* action)
{
    if (!PlayerbotAI::IsMainTank(bot) && !PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
        return 1.0f;

    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return 1.0f;

    if (dynamic_cast<TankAssistAction*>(action) ||
        dynamic_cast<CombatFormationMoveAction*>(action))
        return 0.0f;

    if ((PlayerbotAI::IsMainTank(bot) && !hydross->HasAura(SPELL_CORRUPTION)) ||
        (PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) && hydross->HasAura(SPELL_CORRUPTION)))
        return 1.0f;

    if (dynamic_cast<CastReachTargetSpellAction*>(action) ||
        dynamic_cast<ReachTargetAction*>(action) ||
        (dynamic_cast<AttackAction*>(action) &&
         !dynamic_cast<HydrossTheUnstablePositionFrostTankAction*>(action) &&
         !dynamic_cast<HydrossTheUnstablePositionNatureTankAction*>(action)))
        return 0.0f;

    return 1.0f;
}

float HydrossTheUnstableWaitForDpsMultiplier::GetValue(Action* action)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return 1.0f;

    Unit* waterElemental = AI_VALUE2(Unit*, "find target", "pure spawn of hydross");
    Unit* natureElemental = AI_VALUE2(Unit*, "find target", "tainted spawn of hydross");
    if (PlayerbotAI::IsAssistTank(bot) && !PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) &&
        (waterElemental || natureElemental))
        return 1.0f;

    if (dynamic_cast<HydrossTheUnstableMisdirectBossToTankAction*>(action))
        return 1.0f;

    const uint32 instanceId = hydross->GetMap()->GetInstanceId();
    const time_t now = std::time(nullptr);
    constexpr uint8 phaseChangeWaitSeconds = 1;
    constexpr uint8 dpsWaitSeconds = 5;

    if (!hydross->HasAura(SPELL_CORRUPTION) && !PlayerbotAI::IsMainTank(bot))
    {
        auto itDps = hydrossFrostDpsWaitTimer.find(instanceId);
        auto itPhase = hydrossChangeToFrostPhaseTimer.find(instanceId);

        bool justChanged = (itDps == hydrossFrostDpsWaitTimer.end() ||
                            (now - itDps->second) < dpsWaitSeconds);
        bool aboutToChange = (itPhase != hydrossChangeToFrostPhaseTimer.end() &&
                              (now - itPhase->second) > phaseChangeWaitSeconds);

        if (!justChanged && !aboutToChange)
            return 1.0f;

        if (dynamic_cast<AttackAction*>(action) ||
            (dynamic_cast<CastSpellAction*>(action) &&
             !dynamic_cast<CastHealingSpellAction*>(action)))
            return 0.0f;
    }

    if (hydross->HasAura(SPELL_CORRUPTION) && !PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
    {
        auto itDps = hydrossNatureDpsWaitTimer.find(instanceId);
        auto itPhase = hydrossChangeToNaturePhaseTimer.find(instanceId);

        bool justChanged = (itDps == hydrossNatureDpsWaitTimer.end() ||
                            (now - itDps->second) < dpsWaitSeconds);
        bool aboutToChange = (itPhase != hydrossChangeToNaturePhaseTimer.end() &&
                              (now - itPhase->second) > phaseChangeWaitSeconds);

        if (!justChanged && !aboutToChange)
            return 1.0f;

        if (dynamic_cast<AttackAction*>(action) ||
            (dynamic_cast<CastSpellAction*>(action) &&
             !dynamic_cast<CastHealingSpellAction*>(action)))
            return 0.0f;
    }

    return 1.0f;
}

float HydrossTheUnstableControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (AI_VALUE2(Unit*, "find target", "hydross the unstable") &&
        dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 0.0f;

    return 1.0f;
}

// The Lurker Below

float TheLurkerBelowStayAwayFromSpoutMultiplier::GetValue(Action* action)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return 1.0f;

    const time_t now = std::time(nullptr);

    auto it = lurkerSpoutTimer.find(lurker->GetMap()->GetInstanceId());
    if (it != lurkerSpoutTimer.end() && it->second > now)
    {
        if (dynamic_cast<CastReachTargetSpellAction*>(action) ||
            dynamic_cast<CastKillingSpreeAction*>(action) ||
            dynamic_cast<CastBlinkBackAction*>(action) ||
            dynamic_cast<CastDisengageAction*>(action))
            return 0.0f;

        if (dynamic_cast<MovementAction*>(action) &&
            !dynamic_cast<AttackAction*>(action) &&
            !dynamic_cast<TheLurkerBelowRunAroundBehindBossAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float TheLurkerBelowMaintainRangedSpreadMultiplier::GetValue(Action* action)
{
    if (!PlayerbotAI::IsRanged(bot))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "the lurker below"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Disable tank assist during Submerge only if there are 3 or more tanks in the raid
float TheLurkerBelowDisableTankAssistMultiplier::GetValue(Action* action)
{
    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (bot->GetVictim() == nullptr)
        return 1.0f;

    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker || lurker->getStandState() != UNIT_STAND_STATE_SUBMERGED)
        return 1.0f;

    Group* group = bot->GetGroup();
    if (!group)
        return 1.0f;

    uint8 tankCount = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;

        if (PlayerbotAI::IsTank(member))
            ++tankCount;
    }

    if (tankCount < 3)
        return 1.0f;

    if (dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Leotheras the Blind

float LeotherasTheBlindAvoidWhirlwindMultiplier::GetValue(Action* action)
{
    if (PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (bot->HasAura(SPELL_INSIDIOUS_WHISPER))
        return 1.0f;

    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras || (!leotheras->HasAura(SPELL_WHIRLWIND) &&
        !leotheras->HasAura(SPELL_WHIRLWIND_CHANNEL)))
        return 1.0f;

    if (dynamic_cast<CastReachTargetSpellAction*>(action))
        return 0.0f;

    if (dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<AttackAction*>(action) &&
        !dynamic_cast<LeotherasTheBlindRunAwayFromWhirlwindAction*>(action))
        return 0.0f;

    return 1.0f;
}

float LeotherasTheBlindDisableTankActionsMultiplier::GetValue(Action* action)
{
    if (!PlayerbotAI::IsTank(bot) || bot->HasAura(SPELL_INSIDIOUS_WHISPER))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "leotheras the blind"))
        return 1.0f;

    if (GetPhase2LeotherasDemon(bot) && dynamic_cast<AttackAction*>(action))
        return 0.0f;

    if (!GetPhase3LeotherasDemon(bot) && dynamic_cast<CastBerserkAction*>(action))
        return 0.0f;

    return 1.0f;
}

float LeotherasTheBlindFocusOnInnerDemonMultiplier::GetValue(Action* action)
{
    if (!bot->HasAura(SPELL_INSIDIOUS_WHISPER))
        return 1.0f;

    if (dynamic_cast<TankAssistAction*>(action) ||
        dynamic_cast<DpsAssistAction*>(action) ||
        dynamic_cast<CastHealingSpellAction*>(action) ||
        dynamic_cast<CastCureSpellAction*>(action) ||
        dynamic_cast<CurePartyMemberAction*>(action) ||
        dynamic_cast<CastBuffSpellAction*>(action) ||
        dynamic_cast<ResurrectPartyMemberAction*>(action) ||
        dynamic_cast<PartyMemberActionNameSupport*>(action) ||
        dynamic_cast<CastBearFormAction*>(action) ||
        dynamic_cast<CastDireBearFormAction*>(action) ||
        dynamic_cast<CastTreeFormAction*>(action))
        return 0.0f;

    return 1.0f;
}

float LeotherasTheBlindMeleeDpsAvoidChaosBlastMultiplier::GetValue(Action* action)
{
    if (PlayerbotAI::IsRanged(bot) || PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!GetPhase2LeotherasDemon(bot))
        return 1.0f;

    Aura* chaosBlast = bot->GetAura(SPELL_CHAOS_BLAST);
    if (!chaosBlast || chaosBlast->GetStackAmount() < 5)
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action) ||
        dynamic_cast<ReachTargetAction*>(action) ||
        dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<CastReachTargetSpellAction*>(action) ||
        dynamic_cast<CastKillingSpreeAction*>(action))
        return 0.0f;

    return 1.0f;
}

float LeotherasTheBlindWaitForDpsMultiplier::GetValue(Action* action)
{
    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras)
        return 1.0f;

    if (bot->HasAura(SPELL_INSIDIOUS_WHISPER))
        return 1.0f;

    if (dynamic_cast<LeotherasTheBlindMisdirectBossToDemonFormTankAction*>(action))
        return 1.0f;

    const uint32 instanceId = leotheras->GetMap()->GetInstanceId();
    const time_t now = std::time(nullptr);

    constexpr uint8 dpsWaitSecondsPhase1 = 5;
    Unit* leotherasHuman = GetLeotherasHuman(bot);
    Unit* leotherasPhase3Demon = GetPhase3LeotherasDemon(bot);
    if (leotherasHuman && !leotherasHuman->HasAura(SPELL_LEOTHERAS_BANISHED) &&
        !leotherasPhase3Demon)
    {
        if (PlayerbotAI::IsTank(bot))
            return 1.0f;

        auto it = leotherasHumanFormDpsWaitTimer.find(instanceId);
        if (it == leotherasHumanFormDpsWaitTimer.end() ||
            (now - it->second) < dpsWaitSecondsPhase1)
        {
            if (dynamic_cast<AttackAction*>(action) ||
                (dynamic_cast<CastSpellAction*>(action) &&
                 !dynamic_cast<CastHealingSpellAction*>(action)))
                return 0.0f;
        }
    }

    constexpr uint8 dpsWaitSecondsPhase2 = 12;
    Unit* leotherasPhase2Demon = GetPhase2LeotherasDemon(bot);
    Player* demonFormTank = GetLeotherasDemonFormTank(bot);
    if (leotherasPhase2Demon)
    {
        if (demonFormTank && demonFormTank == bot)
            return 1.0f;

        if (!demonFormTank && PlayerbotAI::IsTank(bot))
            return 1.0f;

        auto it = leotherasDemonFormDpsWaitTimer.find(instanceId);
        if (it == leotherasDemonFormDpsWaitTimer.end() ||
            (now - it->second) < dpsWaitSecondsPhase2)
        {
            if (dynamic_cast<AttackAction*>(action) ||
                (dynamic_cast<CastSpellAction*>(action) &&
                 !dynamic_cast<CastHealingSpellAction*>(action)))
                return 0.0f;
        }
    }

    constexpr uint8 dpsWaitSecondsPhase3 = 8;
    if (leotherasPhase3Demon)
    {
        if ((demonFormTank && demonFormTank == bot) || PlayerbotAI::IsTank(bot))
            return 1.0f;

        auto it = leotherasFinalPhaseDpsWaitTimer.find(instanceId);
        if (it == leotherasFinalPhaseDpsWaitTimer.end() ||
            (now - it->second) < dpsWaitSecondsPhase3)
        {
            if (dynamic_cast<AttackAction*>(action) ||
                (dynamic_cast<CastSpellAction*>(action) &&
                 !dynamic_cast<CastHealingSpellAction*>(action)))
                return 0.0f;
        }
    }

    return 1.0f;
}

// Don't use Bloodlust/Heroism during the Channeler phase
float LeotherasTheBlindDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras || !leotheras->HasAura(SPELL_LEOTHERAS_BANISHED))
        return 1.0f;

    if (dynamic_cast<CastHeroismAction*>(action) ||
        dynamic_cast<CastBloodlustAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Fathom-Lord Karathress

float FathomLordKarathressDisableTankActionsMultiplier::GetValue(Action* action)
{
    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "fathom-lord karathress"))
        return 1.0f;

    if (bot->GetVictim() != nullptr && dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<AvoidAoeAction*>(action) ||
        dynamic_cast<CastTauntAction*>(action) ||
        dynamic_cast<CastChallengingShoutAction*>(action) ||
        dynamic_cast<CastThunderClapAction*>(action) ||
        dynamic_cast<CastShockwaveAction*>(action) ||
        dynamic_cast<CastCleaveAction*>(action) ||
        dynamic_cast<CastGrowlAction*>(action) ||
        dynamic_cast<CastSwipeBearAction*>(action) ||
        dynamic_cast<CastChallengingRoarAction*>(action) ||
        dynamic_cast<CastHandOfReckoningAction*>(action) ||
        dynamic_cast<CastAvengersShieldAction*>(action) ||
        dynamic_cast<CastConsecrationAction*>(action) ||
        dynamic_cast<CastDarkCommandAction*>(action) ||
        dynamic_cast<CastDeathAndDecayAction*>(action) ||
        dynamic_cast<CastPestilenceAction*>(action) ||
        dynamic_cast<CastBloodBoilAction*>(action))
        return 0.0f;

    return 1.0f;
}

float FathomLordKarathressDisableAoeMultiplier::GetValue(Action* action)
{
    if (!PlayerbotAI::IsDps(bot))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "fathom-lord karathress"))
        return 1.0f;

    auto castSpellAction = dynamic_cast<CastSpellAction*>(action);
    if (castSpellAction && castSpellAction->getThreatType() == Action::ActionThreatType::Aoe)
        return 0.0f;

    return 1.0f;
}

float FathomLordKarathressControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "fathom-lord karathress"))
        return 1.0f;

    if (dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 0.0f;

    return 1.0f;
}

float FathomLordKarathressWaitForDpsMultiplier::GetValue(Action* action)
{
    if (PlayerbotAI::IsTank(bot))
        return 1.0f;

    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (!karathress)
        return 1.0f;

    if (dynamic_cast<FathomLordKarathressMisdirectBossesToTanksAction*>(action))
        return 1.0f;

    const time_t now = std::time(nullptr);
    constexpr uint8 dpsWaitSeconds = 12;

    auto it = karathressDpsWaitTimer.find(karathress->GetMap()->GetInstanceId());
    if (it == karathressDpsWaitTimer.end() || (now - it->second) < dpsWaitSeconds)
    {
        if (dynamic_cast<AttackAction*>(action) ||
            (dynamic_cast<CastSpellAction*>(action) &&
             !dynamic_cast<CastHealingSpellAction*>(action)))
            return 0.0f;
    }

    return 1.0f;
}

float FathomLordKarathressCaribdisTankHealerMaintainPositionMultiplier::GetValue(Action* action)
{
    if (!PlayerbotAI::IsAssistHealOfIndex(bot, 0, true))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "fathom-guard caribdis"))
        return 1.0f;

    if (dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<FollowAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Morogrim Tidewalker

// Use Bloodlust/Heroism after the first Murloc spawn
float MorogrimTidewalkerDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "morogrim tidewalker"))
        return 1.0f;

    if (AI_VALUE2(Unit*, "find target", "tidewalker lurker"))
        return 1.0f;

    if (dynamic_cast<CastHeroismAction*>(action) ||
        dynamic_cast<CastBloodlustAction*>(action))
        return 0.0f;

    return 1.0f;
}

float MorogrimTidewalkerDisableTankActionsMultiplier::GetValue(Action* action)
{
    if (!PlayerbotAI::IsMainTank(bot))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "morogrim tidewalker"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action))
        return 0.0f;

    return 1.0f;
}

float MorogrimTidewalkerMaintainPhase2StackingMultiplier::GetValue(Action* action)
{
    if (!PlayerbotAI::IsRanged(bot))
        return 1.0f;

    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!tidewalker || tidewalker->GetHealthPct() > 25.0f)
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Lady Vashj <Coilfang Matron>

// Wait until phase 3 to use Bloodlust/Heroism
// Don't use other major cooldowns in Phase 1, either
float LadyVashjDelayCooldownsMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "lady vashj"))
        return 1.0f;

    if (bot->getClass() == CLASS_SHAMAN &&
        !IsLadyVashjInPhase3(botAI) &&
        (dynamic_cast<CastBloodlustAction*>(action) ||
         dynamic_cast<CastHeroismAction*>(action)))
        return 0.0f;

    if (!PlayerbotAI::IsDps(bot) || !IsLadyVashjInPhase1(botAI))
        return 1.0f;

    if (dynamic_cast<CastMetamorphosisAction*>(action) ||
        dynamic_cast<CastAdrenalineRushAction*>(action) ||
        dynamic_cast<CastBladeFlurryAction*>(action) ||
        dynamic_cast<CastIcyVeinsAction*>(action) ||
        dynamic_cast<CastColdSnapAction*>(action) ||
        dynamic_cast<CastArcanePowerAction*>(action) ||
        dynamic_cast<CastPresenceOfMindAction*>(action) ||
        dynamic_cast<CastCombustionAction*>(action) ||
        dynamic_cast<CastRapidFireAction*>(action) ||
        dynamic_cast<CastReadinessAction*>(action) ||
        dynamic_cast<CastAvengingWrathAction*>(action) ||
        dynamic_cast<CastElementalMasteryAction*>(action) ||
        dynamic_cast<CastFeralSpiritAction*>(action) ||
        dynamic_cast<CastFireElementalTotemAction*>(action) ||
        dynamic_cast<CastFireElementalTotemMeleeAction*>(action) ||
        dynamic_cast<CastForceOfNatureAction*>(action) ||
        dynamic_cast<CastArmyOfTheDeadAction*>(action) ||
        dynamic_cast<CastSummonGargoyleAction*>(action) ||
        dynamic_cast<CastBerserkingAction*>(action) ||
        dynamic_cast<CastBloodFuryAction*>(action) ||
        dynamic_cast<UseTrinketAction*>(action))
        return 0.0f;

    return 1.0f;
}

float LadyVashjMainTankGroupShamanUseGroundingTotemMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "lady vashj"))
        return 1.0f;

    if (!IsMainTankInSameSubgroup(bot))
        return 1.0f;

    if (dynamic_cast<CastWindfuryTotemAction*>(action) ||
        dynamic_cast<SetWindfuryTotemAction*>(action) ||
        dynamic_cast<CastWrathOfAirTotemAction*>(action) ||
        dynamic_cast<SetWrathOfAirTotemAction*>(action) ||
        dynamic_cast<CastNatureResistanceTotemAction*>(action) ||
        dynamic_cast<SetNatureResistanceTotemAction*>(action))
        return 0.0f;

    return 1.0f;
}

float LadyVashjMaintainPhase1RangedSpreadMultiplier::GetValue(Action* action)
{
    if (!PlayerbotAI::IsRanged(bot))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "lady vashj") ||
        !IsLadyVashjInPhase1(botAI))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action))
        return 0.0f;

    return 1.0f;
}

float LadyVashjStaticChargeStayAwayFromGroupMultiplier::GetValue(Action* action)
{
    if (PlayerbotAI::IsMainTank(bot) || !bot->HasAura(SPELL_STATIC_CHARGE))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "lady vashj"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<ReachTargetAction*>(action) ||
        dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<CastKillingSpreeAction*>(action) ||
        dynamic_cast<CastReachTargetSpellAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Bots should not loot the core with normal looting logic
float LadyVashjDoNotLootTheTaintedCoreMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "lady vashj"))
        return 1.0f;

    if (dynamic_cast<LootAction*>(action))
        return 0.0f;

    return 1.0f;
}

float LadyVashjCorePassersPrioritizePositioningMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "lady vashj") || !IsLadyVashjInPhase2(botAI))
        return 1.0f;

    if (dynamic_cast<WipeAction*>(action) ||
        dynamic_cast<DestroyItemAction*>(action) ||
        dynamic_cast<LadyVashjDestroyTaintedCoreAction*>(action))
        return 1.0f;

    auto coreHandlers = GetCoreHandlers(botAI, bot);

    bool isCoreHandler = false;
    for (int i = 0; i < static_cast<int>(coreHandlers.size()); ++i)
    {
        if (coreHandlers[i] && coreHandlers[i] == bot)
        {
            isCoreHandler = true;
        }
    }
    if (!isCoreHandler)
        return 1.0f;

    auto hasCore = [](Player* player)
    {
        return player && player->HasItemCount(ITEM_TAINTED_CORE, 1, false);
    };

    // If the bot actually has the core, only allow core handling
    if (hasCore(bot) && !dynamic_cast<LadyVashjPassTheTaintedCoreAction*>(action))
        return 0.0f;

    // First and second passers block movement when the looter teleports to the elemental
    Unit* tainted = AI_VALUE2(Unit*, "find target", "tainted elemental");
    if (tainted && coreHandlers[0]->GetExactDist2d(tainted) < 5.0f &&
        (bot == coreHandlers[1] || bot == coreHandlers[2]) &&
        (dynamic_cast<MovementAction*>(action) &&
         !dynamic_cast<LadyVashjPassTheTaintedCoreAction*>(action)))
        return 0.0f;

    // If any prior handler (including self) recently had the core, block other movement
    if (AnyRecentCoreInInventory(botAI, bot) &&
        dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<LadyVashjPassTheTaintedCoreAction*>(action))
        return 0.0f;

    return 1.0f;
}

// All of phases 2 and 3 require a custom movement and targeting system
// So the standard target selection system must be disabled
float LadyVashjDisableAutomaticTargetingAndMovementModifier::GetValue(Action *action)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return 1.0f;

    if (dynamic_cast<AvoidAoeAction*>(action))
        return 0.0f;

    if (IsLadyVashjInPhase2(botAI))
    {
        if (dynamic_cast<DpsAssistAction*>(action) ||
            dynamic_cast<TankAssistAction*>(action) ||
            dynamic_cast<FleeAction*>(action))
            return 0.0f;

        if (bot->GetExactDist2d(vashj) < 60.0f &&
            dynamic_cast<FollowAction*>(action))
            return 0.0f;

        if (!PlayerbotAI::IsHeal(bot) && dynamic_cast<CastHealingSpellAction*>(action))
            return 0.0f;

        Unit* enchanted = AI_VALUE2(Unit*, "find target", "enchanted elemental");
        if (enchanted && AI_VALUE(Unit*, "current target") == enchanted &&
            dynamic_cast<CastDebuffSpellOnAttackerAction*>(action))
            return 0.0f;
    }

    if (IsLadyVashjInPhase3(botAI))
    {
        if (dynamic_cast<DpsAssistAction*>(action) ||
            dynamic_cast<TankAssistAction*>(action))
            return 0.0f;

        Unit* enchanted = AI_VALUE2(Unit*, "find target", "enchanted elemental");
        Unit* strider = AI_VALUE2(Unit*, "find target", "coilfang strider");
        Unit* elite = AI_VALUE2(Unit*, "find target", "coilfang elite");
        if (enchanted || strider || elite)
        {
            if (dynamic_cast<FollowAction*>(action) ||
                dynamic_cast<FleeAction*>(action))
                return 0.0f;

            if (enchanted && AI_VALUE(Unit*, "current target") == enchanted &&
                dynamic_cast<CastDebuffSpellOnAttackerAction*>(action))
                return 0.0f;
        }
        else if (dynamic_cast<CombatFormationMoveAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}
