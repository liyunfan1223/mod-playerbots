#include "RaidNaxxMultipliers.h"

#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidActions.h"
#include "DruidBearActions.h"
#include "FollowActions.h"
#include "GenericActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "MovementActions.h"
#include "PaladinActions.h"
#include "PriestActions.h"
#include "RaidNaxxActions.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ScriptedCreature.h"
#include "ShamanActions.h"
#include "UseMeetingStoneAction.h"
#include "WarriorActions.h"

float GrobbulusMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grobbulus");
    if (!boss)
    {
        return 1.0f;
    }
    if (dynamic_cast<AvoidAoeAction*>(action) || dynamic_cast<CombatFormationMoveAction*>(action))
    {
        return 0.0f;
    }
    return 1.0f;
}

float HeiganDanceMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "heigan the unclean");
    if (!boss)
    {
        return 1.0f;
    }

    auto* boss_ai = dynamic_cast<Heigan::boss_heigan::boss_heiganAI*>(boss->GetAI());
    EventMap* eventMap = &boss_ai->events;
    uint32 curr_phase = boss_ai->currentPhase;
    uint32 curr_dance = eventMap->GetNextEventTime(4);
    uint32 curr_timer = eventMap->GetTimer();
    uint32 curr_erupt = eventMap->GetNextEventTime(3);
    if (dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action) )
    {
        return 0.0f;
    }
    if (curr_phase != 1 && (int32)curr_dance - curr_timer >= 3000)
    {
        return 1.0f;
    }
    if (dynamic_cast<HeiganDanceAction*>(action) || dynamic_cast<CurePartyMemberAction*>(action))
    {
        return 1.0f;
    }
    if (dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<CastMeleeSpellAction*>(action))
    {
        CastSpellAction* spellAction = dynamic_cast<CastSpellAction*>(action);
        uint32 spellId = AI_VALUE2(uint32, "spell id", spellAction->getSpell());
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
        {
            return 0.0f;
        }
        uint32 castTime = spellInfo->CalcCastTime();
        if (castTime == 0 && !spellInfo->IsChanneled())
        {
            return 1.0f;
        }
    }
    return 0.0f;
}

float LoathebGenericMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "loatheb");
    if (!boss)
    {
        return 1.0f;
    }
    context->GetValue<bool>("neglect threat")->Set(true);
    if (botAI->GetState() == BOT_STATE_COMBAT &&
        (dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action) ||
         dynamic_cast<CastDebuffSpellOnAttackerAction*>(action) || dynamic_cast<FleeAction*>(action) ||
         dynamic_cast<CombatFormationMoveAction*>(action)))
    {
        return 0.0f;
    }
    if (!dynamic_cast<CastHealingSpellAction*>(action))
    {
        return 1.0f;
    }
    Aura* aura = botAI->GetAura("necrotic aura", bot);
    if (!aura || aura->GetDuration() <= 1500)
    {
        return 1.0f;
    }
    return 0.0f;
}

float ThaddiusGenericMultiplier::GetValue(Action* action)
{
    if (!helper.UpdateBossAI())
    {
        return 1.0f;
    }
    if (dynamic_cast<CombatFormationMoveAction*>(action))
        return 0.0f;
    // pet phase
    if (helper.IsPhasePet() &&
        (dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action) ||
         dynamic_cast<CastDebuffSpellOnAttackerAction*>(action) ||
         dynamic_cast<ReachPartyMemberToHealAction*>(action) || dynamic_cast<BuffOnMainTankAction*>(action)))
    {
        return 0.0f;
    }
    // die at the same time
    Unit* target = AI_VALUE(Unit*, "current target");
    Unit* feugen = AI_VALUE2(Unit*, "find target", "feugen");
    Unit* stalagg = AI_VALUE2(Unit*, "find target", "stalagg");
    if (helper.IsPhasePet() && target && feugen && stalagg && target->GetHealthPct() <= 40 &&
        (feugen->GetHealthPct() >= target->GetHealthPct() + 3 || stalagg->GetHealthPct() >= target->GetHealthPct() + 3))
    {
        if (dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<CastHealingSpellAction*>(action))
        {
            return 0.0f;
        }
    }
    // magnetic pull
    // uint32 curr_timer = eventMap->GetTimer();
    // // if (curr_phase == 2 && bot->GetPositionZ() > 312.5f && dynamic_cast<MovementAction*>(action)) {
    // if (curr_phase == 2 && (curr_timer % 20000 >= 18000 || curr_timer % 20000 <= 2000) &&
    // dynamic_cast<MovementAction*>(action)) {
    // 	// MotionMaster *mm = bot->GetMotionMaster();
    // 	// mm->Clear();
    // 	return 0.0f;
    // }
    // thaddius phase
    // if (curr_phase == 8 && dynamic_cast<FleeAction*>(action)) {
    // 		return 0.0f;
    // }
    return 1.0f;
}

float SapphironGenericMultiplier::GetValue(Action* action)
{
    if (!helper.UpdateBossAI())
    {
        return 1.0f;
    }
    if (dynamic_cast<FollowAction*>(action) || dynamic_cast<CastDeathGripAction*>(action) ||
        dynamic_cast<CombatFormationMoveAction*>(action))
    {
        return 0.0f;
    }
    return 1.0f;
}

float InstructorRazuviousGenericMultiplier::GetValue(Action* action)
{
    if (!helper.UpdateBossAI())
    {
        return 1.0f;
    }
    context->GetValue<bool>("neglect threat")->Set(true);
    if (botAI->GetState() == BOT_STATE_COMBAT &&
        (dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action) ||
         dynamic_cast<CastTauntAction*>(action) || dynamic_cast<CastDarkCommandAction*>(action) ||
         dynamic_cast<CastHandOfReckoningAction*>(action) || dynamic_cast<CastGrowlAction*>(action)))
    {
        return 0.0f;
    }
    return 1.0f;
}

float KelthuzadGenericMultiplier::GetValue(Action* action)
{
    if (!helper.UpdateBossAI())
    {
        return 1.0f;
    }
    if ((dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action) ||
         dynamic_cast<CastDebuffSpellOnAttackerAction*>(action) || dynamic_cast<FollowAction*>(action) ||
         dynamic_cast<FleeAction*>(action)))
    {
        return 0.0f;
    }
    if (helper.IsPhaseOne())
    {
        if (dynamic_cast<CastTotemAction*>(action) || dynamic_cast<CastShadowfiendAction*>(action) ||
            dynamic_cast<CastRaiseDeadAction*>(action) || dynamic_cast<CastFeignDeathAction*>(action) ||
            dynamic_cast<CastInvisibilityAction*>(action) || dynamic_cast<CastVanishAction*>(action) ||
            dynamic_cast<PetAttackAction*>(action))
        {
            return 0.0f;
        }
    }
    if (helper.IsPhaseTwo())
    {
        if (dynamic_cast<CastBlizzardAction*>(action) || dynamic_cast<CastFrostNovaAction*>(action))
        {
            return 0.0f;
        }
    }
    return 1.0f;
}

float AnubrekhanGenericMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "anub'rekhan");
    if (!boss)
    {
        return 1.0f;
    }
    if (
        // (dynamic_cast<DpsAssistAction*>(action) ||
        //  dynamic_cast<DpsAssistAction*>(action) ||
        //  dynamic_cast<TankAssistAction*>(action) ||
        dynamic_cast<FollowAction*>(action))
    {
        return 0.0f;
    }
    // BossAI* boss_ai = dynamic_cast<BossAI*>(boss->GetAI());
    // EventMap* eventMap = boss_ai->GetEvents();
    // uint32 curr_phase = eventMap->GetPhaseMask();
    if (botAI->HasAura("locust swarm", boss))
    {
        if (dynamic_cast<FleeAction*>(action))
        {
            return 0.0f;
        }
    }
    return 1.0f;
}

float FourhorsemanGenericMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sir zeliek");
    if (!boss)
    {
        return 1.0f;
    }
    context->GetValue<bool>("neglect threat")->Set(true);
    if ((dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action)))
    {
        return 0.0f;
    }
    return 1.0f;
}

// float GothikGenericMultiplier::GetValue(Action* action)
// {
// 	Unit* boss = AI_VALUE2(Unit*, "find target", "gothik the harvester");
// 	if (!boss) {
//         return 1.0f;
//     }
// 	BossAI* boss_ai = dynamic_cast<BossAI*>(boss->GetAI());
// 	EventMap* eventMap = boss_botAI->GetEvents();
//     uint32 curr_phase = eventMap->GetPhaseMask();
// 	if (curr_phase == 1 && (dynamic_cast<FollowAction*>(action))) {
// 		return 0.0f;
// 	}
// 	if (curr_phase == 1 && (dynamic_cast<AttackAction*>(action))) {
// 		Unit* target = action->GetTarget();
// 		if (target == boss) {
// 			return 0.0f;
// 		}
// 	}
// 	return 1.0f;
// }

float GluthGenericMultiplier::GetValue(Action* action)
{
    if (!helper.UpdateBossAI())
    {
        return 1.0f;
    }
    if ((dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action) ||
         dynamic_cast<FleeAction*>(action) || dynamic_cast<CastDebuffSpellOnAttackerAction*>(action) ||
         dynamic_cast<CastStarfallAction*>(action)))
    {
        return 0.0f;
    }

    if (botAI->IsMainTank(bot))
    {
        Aura* aura = botAI->GetAura("mortal wound", bot, false, true);
        if (aura && aura->GetStackAmount() >= 5)
        {
            if (dynamic_cast<CastTauntAction*>(action) || dynamic_cast<CastDarkCommandAction*>(action) ||
                dynamic_cast<CastHandOfReckoningAction*>(action) || dynamic_cast<CastGrowlAction*>(action))
            {
                return 0.0f;
            }
        }
    }
    if (dynamic_cast<PetAttackAction*>(action))
    {
        Unit* target = AI_VALUE(Unit*, "current target");
        if (target && target->GetEntry() == Gluth::NPC_ZOMBIE_CHOW)
        {
            return 0.0f;
        }
    }
    return 1.0f;
}
