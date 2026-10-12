/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SSCTriggers.h"
#include "EncounterHelpers.h"
#include "MotionMaster.h"
#include "MoveSpline.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "SSCActions.h"
#include "SSCHelpers.h"

using namespace SscHelpers;
using namespace EncounterHelpers;

// Shared

bool SscNoEncounterInProgressTrigger::IsActive()
{
    return !IsEncounterInProgress(bot, SSC_MAP_ID);
}

bool SscHunterShouldMisdirectTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", _bossName);
    return boss && boss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT;
}

// Trash

bool UnderbogColossusInToxicPoolTrigger::IsActive()
{
    return !IsEncounterInProgress(bot, SSC_MAP_ID) &&
        IsNearToxicPool(botAI, TOXIC_POOL_HAZARD_RADIUS);
}

bool GreyheartTidecallerWaterElementalTotemSpawnedTrigger::IsActive()
{
    if (IsEncounterInProgress(bot, SSC_MAP_ID) || !PlayerbotAI::IsDps(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "greyheart tidecaller"))
        return false;

    return GetWaterElementalTotem(botAI) && !IsSkullOnWaterElementalTotem(botAI);
}

// Hydross the Unstable <Duke of Currents>

bool HydrossTheUnstableShouldBeTankedByFrostTankTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "hydross the unstable") &&
        IsHydrossFrostTank(bot);
}

bool HydrossTheUnstableShouldBeTankedByNatureTankTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "hydross the unstable") &&
        IsHydrossNatureTank(bot);
}

bool HydrossTheUnstableRangedShouldSpreadInFrostPhaseTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross || !IsHydrossInFrostPhase(hydross))
        return false;

    Player* nearestPlayer = GetNearestPlayerInRadius(bot, HYDROSS_FROST_RANGED_SPREAD_DISTANCE);
    return nearestPlayer && !PlayerbotAI::IsTank(nearestPlayer);
}

bool HydrossTheUnstableShouldMisdirectUponPhaseChangeTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    return IsHydrossInFrostPhase(hydross) ? HasNoMarkOfHydross(bot) : HasNoMarkOfCorruption(bot);
}

bool HydrossTheUnstableAggroResetsUponPhaseChangeTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsDps(bot))
        return false;

    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    HydrossDpsHoldWindow const window = GetHydrossDpsHoldWindow(hydross);
    return window == HydrossDpsHoldWindow::BeforePhaseChange ||
        (window == HydrossDpsHoldWindow::AfterPhaseChange && bot->getClass() != CLASS_HUNTER);
}

bool HydrossTheUnstableOffPhaseTankAttackingTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    bool const isOffPhaseTank =
        IsHydrossInFrostPhase(hydross) ? IsHydrossNatureTank(bot) : IsHydrossFrostTank(bot);
    if (!isOffPhaseTank)
        return false;

    return bot->GetVictim() == hydross || AI_VALUE(Unit*, "current target") == hydross;
}

bool HydrossTheUnstableShouldManagePhaseTimersTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, SSC_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "hydross the unstable");
}

// The Lurker Below

bool TheLurkerBelowSpoutIsActiveTrigger::IsActiveInEncounter()
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    return lurker && IsLurkerSpouting(lurker);
}

bool TheLurkerBelowShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker || !IsLurkerSurfacedAndCalm(lurker))
        return false;

    return PlayerbotAI::IsMainTank(bot);
}

bool TheLurkerBelowRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    return lurker && IsLurkerSurfacedAndCalm(lurker);
}

bool TheLurkerBelowGuardiansShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker || lurker->getStandState() != UNIT_STAND_STATE_SUBMERGED)
        return false;

    return GetLurkerGuardianTankIndex(botAI) >= 0;
}

// ReachMeleeAction can't cross the water to an Ambusher or back, so use a direct move to land.
bool TheLurkerBelowMeleeCannotReachTargetTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    if (bot->IsNonMeleeSpellCast(false))
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || bot->IsWithinMeleeRange(target))
        return false;

    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker || IsLurkerSpouting(lurker))
        return false;

    // Stuck: still, with its target out of melee range. Also a bot still running to an add once
    // its target is Lurker; the move to him is forced, so once under way this stops firing.
    if (!bot->isMoving())
        return true;

    return target == lurker &&
        AI_VALUE(LastMovement&, "last movement").priority < MovementPriority::MOVEMENT_FORCED;
}

// Bots can "fall" into the water when crossing between islets.
bool TheLurkerBelowMeleeInWaterTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    if (bot->GetLiquidData().Status == LIQUID_MAP_NO_WATER)
        return false;

    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker || IsLurkerSpouting(lurker))
        return false;

    if (IsDryGround(bot, bot->GetPositionX(), bot->GetPositionY()))
        return false;

    return !PlayerbotAI::IsMainTank(bot);
}

// Leotheras the Blind

bool LeotherasTheBlindRangedShouldSpreadUponPullTrigger::IsActive()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (bot->GetExactDist(LEOTHERAS_SPAWN_POSITION) <= LEOTHERAS_SEARCH_DISTANCE)
        return false;

    Creature* leotheras = GetLeotheras(botAI);
    if (!leotheras || !IsSpellbinderPhase(leotheras))
        return false;

    return GetNearestPlayerInRadius(bot, LEOTHERAS_RANGED_SPREAD_DISTANCE);
}

bool LeotherasTheBlindWarlockShouldTankDemonFormTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "leotheras the blind"))
        return false;

    if (HasInnerDemon(bot) || !GetLeotherasDemonOrShadow(botAI))
        return false;

    return IsLeotherasWarlockTank(bot);
}

bool LeotherasTheBlindTanksShouldAutoAttackDemonFormTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "leotheras the blind"))
        return false;

    if (HasInnerDemon(bot) || !GetLeotherasDemon(botAI))
        return false;

    // If there is no Warlock tank, then traditional tanks will have to tank the demon form.
    return GetLeotherasWarlockTank(bot);
}

bool LeotherasTheBlindRangedShouldKeepDistanceTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras || IsSpellbinderPhase(leotheras))
        return false;

    if (IsLeotherasChannelingWhirlwind(leotheras))
        return false;

    return GetLeotherasHumanoidToAvoid(botAI) || GetChaosBlastTargetToAvoid(botAI);
}

bool LeotherasTheBlindChannelingWhirlwindTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!IsLeotherasChannelingWhirlwind(leotheras))
        return false;

    if (HasInnerDemon(bot))
        return false;

    return bot->GetExactDist2d(leotheras) < LEOTHERAS_WHIRLWIND_SAFE_DISTANCE;
}

bool LeotherasTheBlindTooManyChaosBlastStacksTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsRanged(bot))
        return false;

    if (!HasTooManyChaosBlastStacks(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "leotheras the blind"))
        return false;

    Creature* leotherasDemon = GetLeotherasDemonOrShadow(botAI);
    if (!leotherasDemon || leotherasDemon->GetVictim() == bot)
        return false;

    if (bot->getClass() == CLASS_ROGUE &&
        !bot->HasSpellCooldown(Id(SscSpells::SPELL_CLOAK_OF_SHADOWS)))
    {
        return true;
    }

    return GetDemonTargetToAvoid(bot, leotherasDemon);
}

bool LeotherasTheBlindInnerDemonHasAwakenedTrigger::IsActiveInEncounter()
{
    return HasInnerDemon(bot);
}

bool LeotherasTheBlindInFinalPhaseTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsHeal(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "leotheras the blind"))
        return false;

    if (HasInnerDemon(bot) || !IsLeotherasFinalPhase(botAI))
        return false;

    return !IsLeotherasWarlockTank(bot);
}

bool LeotherasTheBlindShouldSeparateBossFromDemonTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsHeal(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "leotheras the blind"))
        return false;

    if (HasInnerDemon(bot) || !GetShadowTargetToSeparateFrom(botAI))
        return false;

    return !IsLeotherasWarlockTank(bot);
}

bool LeotherasTheBlindHunterShouldMisdirectDemonFormTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "leotheras the blind"))
        return false;

    if (HasInnerDemon(bot))
        return false;

    if (!bot->HasAura(Id(SscSpells::SPELL_MISDIRECTION)) &&
        bot->HasSpellCooldown(Id(SscSpells::SPELL_MISDIRECTION_CAST)))
    {
        return false;
    }

    return GetLeotherasDemonOrShadow(botAI);
}

bool LeotherasTheBlindAggroResetsTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot) || HasInnerDemon(bot))
        return false;

    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    return leotheras && IsLeotherasDpsHoldActive(botAI, leotheras);
}

bool LeotherasTheBlindShouldManageDpsWaitTimersTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, SSC_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "leotheras the blind");
}

// Fathom-Lord Karathress

bool FathomLordKarathressTargetsShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) &&
        AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
}

bool FathomLordKarathressShouldHealCaribdisTankTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsHeal(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "fathom-guard caribdis"))
        return false;

    return PlayerbotAI::IsAssistHealOfIndex(bot, 0, true);
}

bool FathomLordKarathressShouldAssignDpsPriorityTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsHeal(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "fathom-lord karathress"))
        return false;

    if (PlayerbotAI::IsDps(bot))
        return true;

    if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, false))
        return !AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");

    if (PlayerbotAI::IsAssistTankOfIndex(bot, 1, false))
        return !GetSharkkisTankTarget(botAI);

    if (PlayerbotAI::IsAssistTankOfIndex(bot, 2, false))
        return !AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");

    return false;
}

bool FathomLordKarathressShouldManageDpsTimerTrigger::IsActiveInEncounter()
{
    if (SscState(bot->GetInstanceId()).karathressDpsWaitTimer)
        return false;

    return IsMechanicTrackerBot(bot, SSC_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
}

bool FathomLordKarathressRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
    if (!caribdis || bot->GetDistance(caribdis) >= CARIBDIS_CYCLONE_SUMMON_RANGE)
        return false;

    return GetNearestPlayerInRadius(bot, CARIBDIS_RANGED_SPREAD_DISTANCE);
}

bool FathomLordKarathressStuckMidairAfterCycloneTrigger::IsActiveInEncounter()
{
    if (bot->HasAura(Id(SscSpells::SPELL_CYCLONE)))
        return false;

    if (bot->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_CONTROLLED) != EFFECT_MOTION_TYPE &&
        !bot->movespline->Finalized())
    {
        return false;
    }

    if (!AI_VALUE2(Unit*, "find target", "fathom-lord karathress"))
        return false;

    float const floorZ = bot->GetMapHeight(
        bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), true, MAX_FALL_DISTANCE);
    return floorZ > INVALID_HEIGHT && bot->GetPositionZ() - floorZ > CARIBDIS_CYCLONE_DROP_HEIGHT;
}

// Morogrim Tidewalker

bool MorogrimTidewalkerShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "morogrim tidewalker") &&
        PlayerbotAI::IsMainTank(bot);
}

bool MorogrimTidewalkerRangedShouldStackTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!tidewalker || tidewalker->GetHealthPct() > TIDEWALKER_PHASE_2_MOVE_HEALTH_PCT)
        return false;

    return bot->GetExactDist(GetTidewalkerStackPoint(*bot, *tidewalker)) >
        TIDEWALKER_RANGED_STACK_RADIUS;
}

bool MorogrimTidewalkerTooFarFromBossTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    return tidewalker && tidewalker->GetHealthPct() > TIDEWALKER_PHASE_2_MOVE_HEALTH_PCT &&
        bot->GetExactDist(tidewalker) >= TIDEWALKER_MAX_DISTANCE_FROM_BOSS;
}

// Lady Vashj <Coilfang Matron>

bool LadyVashjShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    int8 const phase = GetLadyVashjPhase(vashj);
    return (phase == 1 || phase == 3) && PlayerbotAI::IsMainTank(bot);
}

bool LadyVashjRangedShouldSpreadInPhase1Trigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot) || HasVashjStaticCharge(bot))
        return false;

    Action* spreadAction = context->GetAction("lady vashj phase 1 spread ranged in arc");
    if (!spreadAction || static_cast<LadyVashjPhase1SpreadRangedInArcAction*>(
            spreadAction)->HasReachedRangedPosition())
    {
        return false;
    }

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    return vashj && GetLadyVashjPhase(vashj) == 1;
}

bool LadyVashjStationSlotsNeedHoldersTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, SSC_MAP_ID))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || GetLadyVashjPhase(vashj) != 2)
        return false;

    return HasVashjStationVacancy(bot);
}

bool LadyVashjShouldHoldStationInPhase2Trigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (!GetVashjStationPositionToReturnTo(bot, AI_VALUE(Unit*, "current target")))
        return false;

    if (HasVashjStaticCharge(bot))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    return vashj && GetLadyVashjPhase(vashj) == 2;
}

bool LadyVashjRangedShouldPositionInPhase3Trigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsCaster(bot) || HasVashjStaticCharge(bot))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || GetLadyVashjPhase(vashj) != 3)
        return false;

    if (!IsOnVashjDais(bot->GetPositionX(), bot->GetPositionY(), 0.0f, 0.0f))
        return false;

    return IsVashjPhase3RangedTooClose(bot, vashj);
}

// Attack() rejects targets out of LoS. Thus, bots down the stairs or behind generators in phase 3
// cannot acquire a target, and with follow zeroed, nothing else moves them in phase 3.
bool LadyVashjOutOfSightInPhase3Trigger::IsActiveInEncounter()
{
    if (AI_VALUE(Unit*, "current target") || HasVashjStaticCharge(bot))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    return vashj && GetLadyVashjPhase(vashj) == 3 && !bot->IsWithinLOSInMap(vashj);
}

bool LadyVashjMainTankNeedsGroundingShamanTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, SSC_MAP_ID))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    int8 const phase = GetLadyVashjPhase(vashj);
    return (phase == 1 || phase == 3) && !GetVashjGroundingShaman(bot);
}

bool LadyVashjShamanShouldGroundShockBlastTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_SHAMAN || GetVashjGroundingShaman(bot) != bot)
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    int8 const phase = GetLadyVashjPhase(vashj);
    return phase == 1 || phase == 3;
}

bool LadyVashjStaticChargeOnGroupMemberTrigger::IsActiveInEncounter()
{
    return IsInVashjStaticChargeReach(bot, AI_VALUE2(Unit*, "find target", "lady vashj"));
}

bool LadyVashjShouldAssignTargetPriorityTrigger::IsActiveInEncounter()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    int8 const phase = GetLadyVashjPhase(vashj);
    return phase == 2 || phase == 3;
}

// Intentionally not gated on BotCheatMask as the foundation for the positioning in this strategy
// would not work without the Striders being tankable.
bool LadyVashjTankNeedsFearWardTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot) || bot->HasAura(Id(SscSpells::SPELL_FEAR_WARD)))
        return false;

    return AI_VALUE2(Unit*, "find target", "coilfang strider");
}

bool LadyVashjCoilfangStriderShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* strider = AI_VALUE(Unit*, "current target");
    if (!strider || strider->GetEntry() != Id(SscNpcs::NPC_COILFANG_STRIDER))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    return vashj && ShouldPositionVashjStrider(bot, strider, vashj, GetLadyVashjPhase(vashj));
}

bool LadyVashjCoilfangEliteShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* elite = AI_VALUE(Unit*, "current target");
    if (!elite || elite->GetEntry() != Id(SscNpcs::NPC_COILFANG_ELITE) || elite->GetVictim() != bot)
        return false;

    if (elite->GetExactDist2d(GetVashjEliteTankPosition(*elite)) <= VASHJ_ADD_TANK_ARRIVAL_DISTANCE)
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    return vashj && GetLadyVashjPhase(vashj) == 2;
}

// Idle means not on an Elite, a Strider, or an Enchanted Elemental near Vashj.
bool LadyVashjTankIsIdleAwayFromTheMiddleTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || GetLadyVashjPhase(vashj) != 2 ||
        bot->GetExactDist(vashj) < VASHJ_IDLE_TANK_DISTANCE)
    {
        return false;
    }

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || !target->IsAlive())
        return true;

    switch (target->GetEntry())
    {
        case Id(SscNpcs::NPC_COILFANG_ELITE):
        case Id(SscNpcs::NPC_COILFANG_STRIDER):
            return false;
        case Id(SscNpcs::NPC_ENCHANTED_ELEMENTAL):
            return vashj->GetExactDist2d(target) > VASHJ_ENCHANTED_NEAR_HER_DISTANCE;
        default:
            return true;
    }
}

bool LadyVashjTaintedElementalNeedsLooterTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, SSC_MAP_ID))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || GetLadyVashjPhase(vashj) != 2)
        return false;

    Unit* tainted = AI_VALUE2(Unit*, "find target", "tainted elemental");
    if (!tainted)
        return false;

    std::optional<TaintedCoreLooter> const& assigned =
        SscState(bot->GetInstanceId()).vashjTaintedCoreLooter;
    if (!assigned || assigned->tainted != tainted->GetGUID())
        return true;

    Player* looter = ObjectAccessor::GetPlayer(*bot, assigned->looter);
    return !looter || !looter->IsAlive();
}

bool LadyVashjShouldAttackTaintedElementalTrigger::IsActiveInEncounter()
{
    if (!GetTaintedElementalToKill(bot))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    return vashj && GetLadyVashjPhase(vashj) == 2;
}

bool LadyVashjTaintedCoreLooterTrigger::IsActiveInEncounter()
{
    if (!IsDesignatedCoreLooter(bot))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || GetLadyVashjPhase(vashj) != 2)
        return false;

    Creature* tainted = GetAssignedTaintedElemental(bot);
    if (!IsTaintedCoreStillToLoot(tainted))
        return false;

    return !tainted->IsAlive() || bot->GetDistance(tainted) > VASHJ_CORE_LOOT_RANGE;
}

bool LadyVashjCorePassingChainMemberTrigger::IsActiveInEncounter()
{
    VashjCorePassingChain const* chain = GetVashjCorePassingChain(bot);
    if (!chain || chain->failed)
        return false;

    int8 const index = GetVashjCoreCatcherIndex(*chain, bot);
    if (index < 0 && chain->originBot != bot->GetGUID())
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || GetLadyVashjPhase(vashj) != 2)
        return false;

    if (HasTaintedCore(bot))
        return true;

    if (index < 0)
        return false;

    VashjCoreCatcher const& catcher = chain->catchers[index];
    if (catcher.arrived &&
        bot->GetExactDist2d(catcher.spot) <= GetVashjCoreSpotArrivalDistance(*chain, index))
    {
        return false;
    }

    return IsVashjCoreCatcherActive(bot, *chain, index);
}

bool LadyVashjShouldDestroyTaintedCoreTrigger::IsActiveInEncounter()
{
    if (!HasTaintedCore(bot))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    int8 const phase = GetLadyVashjPhase(vashj);
    if (phase == 3)
        return true;

    if (phase != 2)
        return false;

    VashjCorePassingChain const* chain = GetVashjCorePassingChain(bot);
    if (chain && chain->failed)
        return true;

    Creature* nextTainted = GetAssignedTaintedElemental(bot);
    return nextTainted && !nextTainted->IsAlive() && GetTaintedCoreLootSlot(nextTainted) >= 0;
}

bool LadyVashjPetShouldSwitchTargetTrigger::IsActiveInEncounter()
{
    Guardian* pet = bot->GetGuardianPet();
    if (!pet || !pet->IsAlive() || pet->HasReactState(REACT_PASSIVE))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    int8 const phase = GetLadyVashjPhase(vashj);
    if (phase != 2 && phase != 3)
        return false;

    if (Unit* target = GetVashjPetTarget(botAI, pet, vashj))
        return pet->GetVictim() != target;

    return pet->GetVictim() == vashj;
}

// Bots attacking Sporebats tend to walk up into the air, sometimes onto the ceiling pipes.
bool LadyVashjBotAboveTheGroundTrigger::IsActiveInEncounter()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || GetLadyVashjPhase(vashj) != 3)
        return false;

    float const floorZ = bot->GetMapHeight(
        bot->GetPositionX(), bot->GetPositionY(), VASHJ_PLATFORM_CENTER_POSITION.GetPositionZ());
    return floorZ > INVALID_HEIGHT && bot->GetPositionZ() - floorZ > VASHJ_ABOVE_GROUND_HEIGHT;
}

// This trigger covers everybody in phase 3 except "ring melee" (see below).
bool LadyVashjBotInToxicSporesTrigger::IsActiveInEncounter()
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || GetLadyVashjPhase(vashj) != 3 || IsVashjRingMelee(bot, vashj))
        return false;

    bool const isTanking = vashj->GetVictim() == bot;

    if (!isTanking && bot->isMoving() && CanWalkThroughToxicSpores(bot))
        return false;

    float const radius = isTanking ? TOXIC_SPORES_TANK_AVOID_RADIUS : TOXIC_SPORES_AVOID_RADIUS;
    return IsNearToxicSpores(botAI, radius);
}

// For "ring melee" (melee dps within 10y of a Toxic Spore pool, but only if not already in melee
// range at a position that is >= 7.5y from every pool).
bool LadyVashjMeleeNearToxicSporesTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot) || PlayerbotAI::IsTank(bot))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || GetLadyVashjPhase(vashj) != 3 || !IsVashjRingMelee(bot, vashj) ||
        !IsNearToxicSpores(botAI, TOXIC_SPORES_MELEE_CONTROL_RADIUS))
    {
        return false;
    }

    return !IsInMeleeRangeClearOfSpores(
        bot, AI_VALUE(Unit*, "current target"), GetToxicSporePositions(botAI),
        TOXIC_SPORES_AVOID_RADIUS);
}

bool LadyVashjRangedReachBlockedByToxicSporesTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsCaster(bot))
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj || GetLadyVashjPhase(vashj) != 3)
        return false;

    Unit* target;
    float range;
    return GetVashjReachBlockedBySpores(botAI, target, range);
}

bool LadyVashjEntangleOnMeleeTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_PALADIN)
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    return vashj && GetVashjHandOfFreedomTarget(botAI, vashj);
}

bool LadyVashjStaticChargeOnRogueTrigger::IsActiveInEncounter()
{
    return bot->getClass() == CLASS_ROGUE && HasVashjStaticCharge(bot);
}
