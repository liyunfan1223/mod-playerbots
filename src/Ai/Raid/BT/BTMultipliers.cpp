/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BTMultipliers.h"
#include "BTActions.h"
#include "BTHelpers.h"
#include "ChooseTargetActions.h"
#include "DruidShapeshiftActions.h"
#include "EncounterHelpers.h"
#include "FollowActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "PriestActions.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "WipeAction.h"
#include <array>
#include <ctime>

using namespace BlackTempleHelpers;
using namespace EncounterHelpers;

// General

// Illidan is handled separately
float BlackTempleDelayDpsCooldownsMultiplier::GetValue(Action* action)
{
    if (!IsDpsCooldownAction(bot, action))
        return 1.0f;

    static constexpr std::array BlackTempleBosses = {
        "gathios the shatterer", "mother shahraz", "essence of suffering", "gurtogg bloodboil",
        "teron gorefiend", "supremus", "high warlord naj'entus" };

    Unit* boss = nullptr;
    for (char const* name : BlackTempleBosses)
    {
        if (Unit* candidate = AI_VALUE2(Unit*, "find target", name))
        {
            boss = candidate;
            break;
        }
    }

    if (boss && boss->GetHealthPct() > 95.0f)
        return 0.0f;

    return 1.0f;
}

// High Warlord Naj'entus

float HighWarlordNajentusDisableCombatFormationMoveMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "high warlord naj'entus"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

// Supremus

float SupremusFocusOnAvoidanceInPhase2Multiplier::GetValue(Action* action)
{
    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    if (!supremus || supremus->GetVictim() != bot ||
        !supremus->HasAura(static_cast<uint32>(BlackTempleSpells::SPELL_SNARE_SELF)))
    {
        return 1.0f;
    }

    if (dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<SupremusKiteBossAction*>(action) &&
        !dynamic_cast<SupremusMoveAwayFromVolcanosAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float SupremusHitboxIsBuggedMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_ROGUE ||
        !AI_VALUE2(Unit*, "find target", "supremus"))
    {
        return 1.0f;
    }

    if (dynamic_cast<CastKillingSpreeAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Teron Gorefiend

float TeronGorefiendControlMovementMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "teron gorefiend"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
    {
        return 0.0f;
    }

    if (dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action))
    {
        return 0.0f;
    }

    if (PlayerbotAI::IsRanged(bot) && dynamic_cast<ReachTargetAction*>(action))
        return 0.0f;

    return 1.0f;
}

float TeronGorefiendMarkedBotOnlyMoveToDieMultiplier::GetValue(Action* action)
{
    Aura* aura = bot->GetAura(
        static_cast<uint32>(BlackTempleSpells::SPELL_SHADOW_OF_DEATH));
    if (!aura || aura->GetDuration() >= 15000)
        return 1.0f;

    if (dynamic_cast<WipeAction*>(action))
        return 1.0f;
    else if (!dynamic_cast<TeronGorefiendMoveToCornerToDieAction*>(action))
        return 0.0f;

    return 1.0f;
}

float TeronGorefiendSpiritsAttackOnlyShadowyConstructsMultiplier::GetValue(Action* action)
{
    if (!bot->HasAura(static_cast<uint32>(BlackTempleSpells::SPELL_SPIRITUAL_VENGEANCE)) ||
        dynamic_cast<WipeAction*>(action))
    {
        return 1.0f;
    }

    if (!dynamic_cast<TeronGorefiendControlAndDestroyShadowyConstructsAction*>(action))
        return 0.0f;

    return 1.0f;
}

float TeronGorefiendDisableAttackingConstructsMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "teron gorefiend"))
        return 1.0f;

    if (bot->GetVictim() != nullptr &&
        dynamic_cast<TankAssistAction*>(action))
    {
        return 0.0f;
    }

    if (!PlayerbotAI::IsRangedDps(bot))
        return 1.0f;

    auto castSpellAction = dynamic_cast<CastSpellAction*>(action);
    if (castSpellAction &&
        castSpellAction->getThreatType() == Action::ActionThreatType::Aoe)
    {
        return 0.0f;
    }

    return 1.0f;
}

// Gurtogg Bloodboil

float GurtoggBloodboilControlMovementMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "gurtogg bloodboil"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
    {
        return 0.0f;
    }

    if (dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action))
    {
        return 0.0f;
    }

    if (bot->HasAura(static_cast<uint32>(BlackTempleSpells::SPELL_PLAYER_FEL_RAGE)) &&
        (dynamic_cast<MovementAction*>(action) &&
         !dynamic_cast<AttackAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

// Reliquary of Souls

float ReliquaryOfSoulsDontWasteHealingMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "essence of suffering"))
        return 1.0f;

    if (dynamic_cast<CastPowerWordShieldOnAlmostFullHealthBelowAction*>(action) ||
        dynamic_cast<CastPowerWordShieldOnNotFullAction*>(action) ||
        dynamic_cast<CastPowerWordShieldAction*>(action) ||
        dynamic_cast<CastPowerWordShieldOnPartyAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<CastTreeFormAction*>(action) ||
        dynamic_cast<CastHealingSpellAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

// Mother Shahraz

float MotherShahrazControlMovementMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "mother shahraz"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
    {
        return 0.0f;
    }

    if (dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float MotherShahrazBotsWithFatalAttractionOnlyRunAwayMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "mother shahraz") ||
        !bot->HasAura(static_cast<uint32>(BlackTempleSpells::SPELL_FATAL_ATTRACTION)))
    {
        return 1.0f;
    }

    if (dynamic_cast<WipeAction*>(action))
        return 1.0f;

    if (!dynamic_cast<MotherShahrazRunAwayToBreakFatalAttractionAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Illidari Council

float IllidariCouncilDisableTankActionsMultiplier::GetValue(Action* action)
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

    if (AI_VALUE2(Unit*, "find target", "gathios the shatterer"))
        return 0.0f;

    return 1.0f;
}

float IllidariCouncilControlMovementMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "high nethermancer zerevor"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action) &&
        !dynamic_cast<TankFaceAction*>(action))
    {
        return 0.0f;
    }

    if (dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action))
    {
        return 0.0f;
    }

    if (PlayerbotAI::IsAssistHealOfIndex(bot, 0, true) &&
        (dynamic_cast<MovementAction*>(action) &&
         !dynamic_cast<IllidariCouncilPositionMageTankHealerAction*>(action)))
    {
        return 0.0f;
    }

    if (!PlayerbotAI::IsAssistTankOfIndex(bot, 0, false) &&
        dynamic_cast<TankFaceAction*>(action))
    {
        return 0.0f;
    }

    if ((PlayerbotAI::IsMainTank(bot) ||
         PlayerbotAI::IsAssistTankOfIndex(bot, 0, false) ||
         PlayerbotAI::IsAssistTankOfIndex(bot, 1, false) ||
         GetZerevorMageTank(bot) == bot) &&
        dynamic_cast<AvoidAoeAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float IllidariCouncilControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER ||
        !AI_VALUE2(Unit*, "find target", "high nethermancer zerevor"))
    {
        return 1.0f;
    }

     if (dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
         return 0.0f;

    return 1.0f;
}

float IllidariCouncilDisableIceBlockMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_MAGE ||
        !AI_VALUE2(Unit*, "find target", "high nethermancer zerevor"))
    {
        return 1.0f;
    }

    if (GetZerevorMageTank(bot) != bot)
        return 1.0f;

    if (dynamic_cast<CastIceBlockAction*>(action))
        return 0.0f;

    return 1.0f;
}

float IllidariCouncilDisableArcaneShotOnZerevorMultiplier::GetValue(Action* action)
{
    Unit* zerevor = AI_VALUE2(Unit*, "find target", "high nethermancer zerevor");
    if (!zerevor)
        return 1.0f;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || target->GetGUID() != zerevor->GetGUID())
        return 1.0f;

    if (dynamic_cast<CastArcaneShotAction*>(action))
        return 0.0f;

    return 1.0f;
}

float IllidariCouncilWaitForDpsMultiplier::GetValue(Action* action)
{
    if (PlayerbotAI::IsMainTank(bot) ||
        PlayerbotAI::IsAssistTankOfIndex(bot, 0, false) ||
        PlayerbotAI::IsAssistTankOfIndex(bot, 1, false) ||
        GetZerevorMageTank(bot) == bot)
    {
        return 1.0f;
    }

    Unit* gathios = AI_VALUE2(Unit*, "find target", "gathios the shatterer");
    if (!gathios)
        return 1.0f;

    if (dynamic_cast<IllidariCouncilMisdirectBossesToTanksAction*>(action))
        return 1.0f;

    const time_t now = std::time(nullptr);
    constexpr uint8 dpsWaitSeconds = 5;

    auto it = councilDpsWaitTimer.find(gathios->GetMap()->GetInstanceId());
    if (it == councilDpsWaitTimer.end() || (now - it->second) >= dpsWaitSeconds)
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action) ||
        (dynamic_cast<CastSpellAction*>(action) &&
         !dynamic_cast<CastHealingSpellAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

// Illidan Stormrage <The Betrayer>

float IllidanStormrageDelayDpsCooldownsMultiplier::GetValue(Action* action)
{
    if (!IsDpsCooldownAction(bot, action))
        return 1.0f;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return 1.0f;

    if (illidan->GetHealthPct() <= 62.0f)
        return 1.0f;

    if (bot->getClass() == CLASS_SHAMAN &&
        (dynamic_cast<CastHeroismAction*>(action) ||
         dynamic_cast<CastBloodlustAction*>(action)))
    {
        return 0.0f;
    }

    if (illidan->GetHealthPct() > 95.0f)
        return 1.0f;

    return 0.0f;
}

float IllidanStormrageControlTankActionsMultiplier::GetValue(Action* action)
{
    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || illidan->GetHealth() == 1)
        return 1.0f;

    if (dynamic_cast<TankFaceAction*>(action))
        return 0.0f;

    if (GetIllidanPhase(illidan) != 2)
        return 1.0f;

    if (PlayerbotAI::IsMainTank(bot))
    {
        if (dynamic_cast<MovementAction*>(action) &&
            !dynamic_cast<IllidanStormragePositionAboveGrateAction*>(action))
        {
            return 0.0f;
        }

        if (dynamic_cast<CastMeleeSpellAction*>(action) ||
            dynamic_cast<CastReachTargetSpellAction*>(action))
        {
            return 0.0f;
        }
    }
    else if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, false) ||
             PlayerbotAI::IsAssistTankOfIndex(bot, 1, false))
    {
        if (dynamic_cast<MovementAction*>(action) &&
            !dynamic_cast<IllidanStormrageAssistTanksHandleFlamesOfAzzinothAction*>(action))
        {
            return 0.0f;
        }

        if (dynamic_cast<CastHealingSpellAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float IllidanStormrageDisableDefaultTargetingMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || illidan->GetHealth() == 1)
        return 1.0f;

    if (dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    int phase = GetIllidanPhase(illidan);

    if (phase == 4 && dynamic_cast<DpsAssistAction*>(action))
        return 0.0f;

    if (PlayerbotAI::IsRangedDps(bot))
    {
        if (phase != 2)
            context->GetValue<bool>("neglect threat")->Set(true);

        if (dynamic_cast<DpsAssistAction*>(action))
            return 0.0f;
    }

    constexpr float searchRadius = 40.0f;
    Unit* shadowDemon = bot->FindNearestCreature(
        static_cast<uint32>(BlackTempleNpcs::NPC_SHADOW_DEMON), searchRadius);
    Unit* shadowfiend = bot->FindNearestCreature(
        static_cast<uint32>(BlackTempleNpcs::NPC_PARASITIC_SHADOWFIEND), searchRadius);

    if (((shadowDemon && bot->GetTarget() == shadowDemon->GetGUID()) ||
         (shadowfiend && bot->GetTarget() == shadowfiend->GetGUID())) &&
        dynamic_cast<CastDebuffSpellOnAttackerAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float IllidanStormrageControlNonTankMovementMultiplier::GetValue(Action* action)
{
    if (PlayerbotAI::IsTank(bot))
        return 1.0f;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || illidan->GetHealth() == 1)
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
    {
        return 0.0f;
    }

    if (dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<FollowAction*>(action))
    {
        return 0.0f;
    }

    int phase = GetIllidanPhase(illidan);

    if (phase == 2 &&
        (dynamic_cast<SetBehindTargetAction*>(action) ||
         dynamic_cast<CastKillingSpreeAction*>(action) ||
         dynamic_cast<ReachTargetAction*>(action) ||
         dynamic_cast<CastReachTargetSpellAction*>(action) ||
         dynamic_cast<AvoidAoeAction*>(action)))
    {
        return 0.0f;
    }

    if (phase == 4 && PlayerbotAI::IsHeal(bot) &&
        dynamic_cast<ReachTargetAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float IllidanStormrageUseEarthbindTotemMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || GetIllidanPhase(illidan) == 2)
        return 1.0f;

    if (dynamic_cast<CastStrengthOfEarthTotemAction*>(action) ||
        dynamic_cast<CastStoneskinTotemAction*>(action) ||
        dynamic_cast<CastStoneclawTotemAction*>(action) ||
        dynamic_cast<CastTremorTotemAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float IllidanStormrageWaitForDpsMultiplier::GetValue(Action* action)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return 1.0f;

    if (dynamic_cast<IllidanStormrageMisdirectToTankAction*>(action))
        return 1.0f;

    const time_t now = std::time(nullptr);
    const uint32 instanceId = illidan->GetMap()->GetInstanceId();

    int phase = GetIllidanPhase(illidan);

    if ((phase == 1 || phase == 3 || phase == 5) &&
        !PlayerbotAI::IsMainTank(bot))
    {
        constexpr uint8 humanoidPhaseDpsWaitSeconds = 3;
        auto it = illidanBossDpsWaitTimer.find(instanceId);

        if ((it == illidanBossDpsWaitTimer.end() ||
             (now - it->second) < humanoidPhaseDpsWaitSeconds) &&
              (dynamic_cast<AttackAction*>(action) ||
               (dynamic_cast<CastSpellAction*>(action) &&
                !dynamic_cast<CastHealingSpellAction*>(action))))
        {
            return 0.0f;
        }
    }

    if (phase == 4 && GetIllidanWarlockTank(bot) != bot)
    {
        constexpr uint8 demonPhaseDpsWaitSeconds = 8;
        auto it = illidanBossDpsWaitTimer.find(instanceId);

        if ((it == illidanBossDpsWaitTimer.end() ||
             (now - it->second) < demonPhaseDpsWaitSeconds) &&
              (dynamic_cast<AttackAction*>(action) ||
               (dynamic_cast<CastSpellAction*>(action) &&
                !dynamic_cast<CastHealingSpellAction*>(action))))
        {
            return 0.0f;
        }
    }

    if (AI_VALUE2(Unit*, "find target", "flame of azzinoth") &&
        !PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) &&
        !PlayerbotAI::IsAssistTankOfIndex(bot, 1, true))
    {
        constexpr uint8 flamePhaseDpsWaitSeconds = 6;
        auto it = illidanFlameDpsWaitTimer.find(instanceId);

        if ((it == illidanFlameDpsWaitTimer.end() ||
             (now - it->second) < flamePhaseDpsWaitSeconds) &&
              (dynamic_cast<AttackAction*>(action) ||
               (dynamic_cast<CastSpellAction*>(action) &&
                !dynamic_cast<CastHealingSpellAction*>(action))))
        {
            return 0.0f;
        }
    }

    return 1.0f;
}
