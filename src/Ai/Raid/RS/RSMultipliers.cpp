/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RSMultipliers.h"
#include "AttackAction.h"
#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidActions.h"
#include "DruidBearActions.h"
#include "DruidCatActions.h"
#include "FollowActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "ICCActions.h"
#include "MageActions.h"
#include "MovementActions.h"
#include "NearestNpcsValue.h"
#include "PaladinActions.h"
#include "PetsAction.h"
#include "Playerbots.h"
#include "PriestActions.h"
#include "RSActions.h"
#include "RSScripts.h"
#include "RSTriggers.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "Timer.h"
#include "UseMeetingStoneAction.h"
#include "WarlockActions.h"
#include "WarriorActions.h"

namespace
{
    bool RsIsAoeDamageAction(Action* action)
    {
        return dynamic_cast<DpsAoeAction*>(action) || dynamic_cast<CastHurricaneAction*>(action) ||
               dynamic_cast<CastVolleyAction*>(action) || dynamic_cast<CastBlizzardAction*>(action) ||
               dynamic_cast<CastStarfallAction*>(action) || dynamic_cast<FanOfKnivesAction*>(action) ||
               dynamic_cast<CastWhirlwindAction*>(action) || dynamic_cast<CastMindSearAction*>(action) ||
               dynamic_cast<CastArmyOfTheDeadAction*>(action);
    }
}

float RsSavianaBeaconMultiplier::GetValue(Action* action)
{
    if (!bot->HasAura(SPELL_FLAME_BEACON))
        return 1.0f;

    if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<RsSavianaConflagrationAction*>(action))
        return 0.0f;

    return 1.0f;
}

float RsBaltharusBrandSafeMultiplier::GetValue(Action* action)
{
    if (PlayerbotAI::IsTank(bot) || !bot->HasAura(SPELL_ENERVATING_BRAND))
        return 1.0f;

    Group* group = bot->GetGroup();
    if (!group)
        return 1.0f;

    if (RsIsAoeDamageAction(action))
        return 0.0f;

    for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (member && member != bot && member->IsAlive() &&
            member->GetExactDist2d(bot) < RS_BALTHARUS_BRAND_RANGE)
            return 1.0f;
    }

    if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<RsBaltharusBrandAction*>(action))
        return 0.0f;

    return 1.0f;
}

float RsSavianaMeleeSpreadMultiplier::GetValue(Action* action)
{
    if (PlayerbotAI::IsTank(bot) || (!PlayerbotAI::IsMelee(bot) && !PlayerbotAI::IsHeal(bot)))
        return 1.0f;

    if (bot->HasAura(SPELL_FLAME_BEACON))
        return 1.0f;

    Unit* boss = AI_VALUE2(Unit*, "find target", "saviana ragefire");
    if (!boss || !boss->IsLevitating())
        return 1.0f;

    if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<RsSavianaMeleeSpreadAction*>(action))
        return 0.0f;

    return 1.0f;
}

float RsZarithrianAddsMultiplier::GetValue(Action* action)
{
    if (PlayerbotAI::IsTank(bot) || PlayerbotAI::IsHeal(bot))
        return 1.0f;

    Unit* boss = AI_VALUE2(Unit*, "find target", "general zarithrian");
    if (!boss)
        return 1.0f;

    if (!RsFindTarget(botAI, [](Unit* unit) { return unit->GetEntry() == NPC_ONYX_FLAMECALLER; }))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FollowAction*>(action))
        return 0.0f;

    return 1.0f;
}

float RsZarithrianTankSwapMultiplier::GetValue(Action* action)
{
    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    Unit* boss = AI_VALUE2(Unit*, "find target", "general zarithrian");
    if (!boss)
        return 1.0f;

    Aura* aura = botAI->GetAura("cleave armor", bot, false, true);
    if (!aura || aura->GetStackAmount() < RS_ZARITHRIAN_CLEAVE_SWAP_STACKS)
        return 1.0f;

    if (dynamic_cast<CastTauntAction*>(action) || dynamic_cast<CastDarkCommandAction*>(action) ||
        dynamic_cast<CastHandOfReckoningAction*>(action) || dynamic_cast<CastGrowlAction*>(action))
        return 0.0f;

    if (dynamic_cast<MovementAction*>(action) || dynamic_cast<RsZarithrianTankAction*>(action))
        return 1.0f;

    return 0.0f;
}

float RsHalionCombustionMultiplier::GetValue(Action* action)
{

    if (RsHalionEngaged(botAI) && RsHalionCombustionNotClear(bot))
    {
        if (dynamic_cast<CurePartyMemberAction*>(action) || dynamic_cast<CastDispelMagicAction*>(action) ||
            dynamic_cast<CastCleanseMagicAction*>(action) || dynamic_cast<CastCleanseSpiritAction*>(action))
            return 0.0f;
    }

    if (PlayerbotAI::IsTank(bot) && !RsHalionAssistTankAsMelee(botAI))
        return 1.0f;

    if (!RsHalionHasCombustion(bot))
        return 1.0f;

    if (RsIsAoeDamageAction(action))
        return 0.0f;

    if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<RsHalionCombustionAction*>(action))
        return 0.0f;

    return 1.0f;
}

float RsHalionMeteorMultiplier::GetValue(Action* action)
{
    if (PlayerbotAI::IsTank(bot) && !RsHalionAssistTankAsMelee(botAI))
        return 1.0f;

    if (RsHalionInTwilight(bot))
        return 1.0f;

    Unit* physBoss = RsHalionAnyPhysicalBoss(botAI);
    if (!physBoss || !physBoss->IsInCombat())
        return 1.0f;

    if (RsHalionHasCombustion(bot) || RsHalionIsCombustionDispeller(botAI) || RsHalionCombustionReturning(bot))
        return 1.0f;

    if (!RsHalionMeteorShouldRally(bot))
        return 1.0f;

    if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<RsHalionMeteorAction*>(action) &&
        !dynamic_cast<RsHalionEnterPortalAction*>(action))
        return 0.0f;

    return 1.0f;
}

float RsHalionMeleeFlankMultiplier::GetValue(Action* action)
{
    if (!RsHalionEngaged(botAI))
        return 1.0f;

    if (dynamic_cast<CastChargeAction*>(action) || dynamic_cast<CastFeralChargeBearAction*>(action) ||
        dynamic_cast<CastFeralChargeCatAction*>(action))
        return 0.0f;

    if (dynamic_cast<AvoidAoeAction*>(action) &&
        (RsHalionEnteringTwilight(botAI, bot) || RsHalionPortalHeldForAdds(botAI)))
        return 1.0f;

    if (PlayerbotAI::IsTank(bot) && dynamic_cast<AvoidAoeAction*>(action))
        return 0.0f;

    if (dynamic_cast<CastDisengageAction*>(action) || dynamic_cast<TankAssistAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action))
        return 0.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FollowAction*>(action))
        return 0.0f;

    if (RsHalionHasCombustion(bot) || RsHalionIsCombustionDispeller(botAI) || RsHalionCombustionReturning(bot))
        return 1.0f;

    Player* const bossTank = RsHalionBossTank(botAI);

    if (Unit* boss = RsHalionPhase1Boss(botAI))
        if (boss->HealthAbovePct(99) && bossTank != bot && dynamic_cast<ReachMeleeAction*>(action))
            return 0.0f;

    Unit* const physBoss = RsHalionAnyPhysicalBoss(botAI);

    if (physBoss && bossTank == bot && dynamic_cast<ReachMeleeAction*>(action))
        return 0.0f;

    if (physBoss)
        if ((!PlayerbotAI::IsTank(bot) || RsHalionAssistTankAsMelee(botAI)) && PlayerbotAI::IsMelee(bot) &&
            bot->GetExactDist2d(physBoss->GetPositionX(), physBoss->GetPositionY()) <= RS_HALION_LINE_MELEE_MAX &&
            dynamic_cast<ReachMeleeAction*>(action))
            return 0.0f;

    return 1.0f;
}

float RsHalionHpBalanceMultiplier::GetValue(Action* action)
{
    if (PlayerbotAI::IsTank(bot) || PlayerbotAI::IsHeal(bot))
        return 1.0f;

    if (!RsHalionRealmThrottled(botAI, bot))
        return 1.0f;

    if (!RsHalionInTwilight(bot) && RsHalionAnyAddAlive(botAI))
        return 1.0f;

    if (!RsHalionLeadingTooMuch(botAI, bot) && !RsHalionInThrottledHalf(bot))
        return 1.0f;

    if (dynamic_cast<RsHalionAvoidConesAction*>(action) ||
        dynamic_cast<RsHalionMeteorAction*>(action) ||
        dynamic_cast<RsHalionCombustionAction*>(action) ||
        dynamic_cast<RsHalionConsumptionAction*>(action) ||
        dynamic_cast<RsHalionCutterAction*>(action) ||
        dynamic_cast<RsHalionP2AvoidConesAction*>(action) ||
        dynamic_cast<RsHalionP2TankPositionAction*>(action) ||
        dynamic_cast<RsHalionEnterPortalAction*>(action) ||
        dynamic_cast<RsHalionHealConsumptionAction*>(action))
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action))
        return 0.0f;

    if (dynamic_cast<PetAttackAction*>(action))
        return 0.0f;

    CastSpellAction* cast = dynamic_cast<CastSpellAction*>(action);
    if (cast && cast->GetTargetName() == "current target")
        return 0.0f;

    return 1.0f;
}

float RsHalionRealmIsolationMultiplier::GetValue(Action* action)
{
    if (!RsHalionEngaged(botAI))
        return 1.0f;

    Unit* target = action->GetTarget();
    if (!target || target == bot)
        return 1.0f;

    Player* member = target->ToPlayer();
    if (!member || !member->IsAlive())
        return 1.0f;

    if (RsHalionInTwilight(member) != RsHalionInTwilight(bot))
        return 0.0f;

    return 1.0f;
}

float RsTrashAddsMultiplier::GetValue(Action* action)
{
    if (PlayerbotAI::IsTank(bot) || PlayerbotAI::IsHeal(bot))
        return 1.0f;

    if (!RsTrashActive(botAI, bot))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FollowAction*>(action))
        return 0.0f;

    return 1.0f;
}

float RsHalionP2Multiplier::GetValue(Action* action)
{
    if (!RsHalionInTwilight(bot))
        return 1.0f;

    if (dynamic_cast<CastChargeAction*>(action) || dynamic_cast<CastFeralChargeBearAction*>(action) ||
        dynamic_cast<CastFeralChargeCatAction*>(action))
        return 0.0f;

    if (dynamic_cast<CastCreateSoulstoneAction*>(action) || dynamic_cast<UseSoulstoneHealerAction*>(action)
        || dynamic_cast<UseSoulstoneMasterAction*>(action) || dynamic_cast<UseSoulstoneSelfAction*>(action)
        || dynamic_cast<UseSoulstoneTankAction*>(action))
        return 0.0f;

    if (!RsHalionTwilightBoss(botAI))
    {
        if (bot->HasAura(SPELL_MARK_OF_CONSUMPTION) || bot->HasAura(SPELL_SOUL_CONSUMPTION))
            return 1.0f;

        if (!RsHalionEnteringTwilight(botAI, bot) &&
            !dynamic_cast<RsHalionP2AvoidConesAction*>(action) &&
            !dynamic_cast<RsHalionCutterAction*>(action) &&
            (dynamic_cast<MovementAction*>(action) || dynamic_cast<AttackAction*>(action)))
            return 0.0f;

        return 1.0f;
    }

    if (RsHalionConsumptionNotClear(botAI))
    {
        if (dynamic_cast<CurePartyMemberAction*>(action) || dynamic_cast<CastDispelMagicAction*>(action) ||
            dynamic_cast<CastCleanseMagicAction*>(action) || dynamic_cast<CastCleanseSpiritAction*>(action))
            return 0.0f;
    }

    if (bot->HasAura(SPELL_MARK_OF_CONSUMPTION) || bot->HasAura(SPELL_SOUL_CONSUMPTION))
    {
        if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<RsHalionConsumptionAction*>(action))
            return 0.0f;
        return 1.0f;
    }

    Player* const twilightTank = RsHalionTwilightTank(botAI);

    if (twilightTank != bot && RsHalionCutterShouldMove(bot->GetInstanceId()))
    {
        if ((bot->getClass() == CLASS_ROGUE || bot->getClass() == CLASS_WARRIOR) &&
            !dynamic_cast<MeleeAction*>(action) &&
            !dynamic_cast<RsHalionCutterAction*>(action) &&
            !dynamic_cast<RsHalionEnterPortalAction*>(action) &&
            !dynamic_cast<RsHalionP2AvoidConesAction*>(action))
            return 0.0f;

        if (RsHalionCutterBeamDanger(botAI, bot) &&
            !dynamic_cast<RsHalionCutterAction*>(action) &&
            !dynamic_cast<RsHalionEnterPortalAction*>(action) &&
            !dynamic_cast<RsHalionP2AvoidConesAction*>(action))
            return 0.0f;
    }

    if (twilightTank != bot && dynamic_cast<ReachMeleeAction*>(action))
        return 0.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FollowAction*>(action))
        return 0.0f;

    return 1.0f;
}
