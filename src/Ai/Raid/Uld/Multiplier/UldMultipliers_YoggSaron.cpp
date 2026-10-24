/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldMultipliers_YoggSaron.h"

#include "AttackAction.h"
#include "BurstCooldowns.h"
#include "ChooseTargetActions.h"
#include "EncounterHelpers.h"
#include "FollowActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "MovementActions.h"
#include "PaladinActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "PriestActions.h"
#include "RaidSplash.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "SharedDefines.h"
#include "Spell.h"
#include "SpellMgr.h"
#include "Timer.h"
#include "UldActions.h"
#include "UldEncounter_YoggSaron.h"
#include "UldHardMode.h"
#include "UldScripts.h"
#include "UldTriggers.h"
#include "VehicleActions.h"

#include <algorithm>
#include <cmath>
#include <list>
#include <set>
#include <string>

using namespace EncounterHelpers;

namespace
{
// SetBehindTargetAction's two spots, either side of the target's facing.
constexpr float SET_BEHIND_SPOT_ANGLE = 3.0f * static_cast<float>(M_PI) / 5.0f;
}  // namespace

float YoggSaronMovementGuardMultiplier::FleeGuard()
{
    // Ranged and healers only: the station owns their feet; stepping off it hands the second orbit a stacked back line.
    // Melee and the tank keep it, since the leash already owns them.
    if (!PlayerbotAI::IsRanged(bot) && !PlayerbotAI::IsHeal(bot))
        return 1.0f;

    return YoggSaronInPhase1(botAI) ? 0.0f : 1.0f;
}

bool YoggSaronMovementGuardMultiplier::ReachIsWrong(Action* action)
{
    // The spell reach too, for pile fighters: taunt and judgement are 30 yd casts, so chasing a Guardian
    // with either walks the bot off the middle. The back line keeps its own, the 21.5 yd station holds range.
    std::string const name = action->getName();
    bool const spellReach =
        name == "reach spell" && !PlayerbotAI::IsRanged(bot) && !PlayerbotAI::IsHeal(bot);

    // Only these two pay the phase read, so every other reach keeps the cheap ring arithmetic below
    // ahead of it.
    if ((name != "reach melee" && !spellReach) || !YoggSaronInPhase1(botAI))
        return false;

    // Walking at a Guardian that is not on the stack leaves the cloud-free circle. The tank taunts it in instead,
    // but only if a tank is alive, or the melee half of the raid has nothing to close on.
    Unit* target = AI_VALUE(Unit*, "current target");

    return target && !YoggSaronGuardianOnTheStack(target) && YoggSaronBotTankAlive(botAI);
}

bool YoggSaronMovementGuardMultiplier::CrusherReachIsWrong()
{
    // Crush is a sure proc on the tentacle's swing and an idle Crusher cannot cast it, so walking in is the cost.
    // The resolver bars them, but a raid mark or tank picker can still hand one over. Stunned, it can't act.
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");

    return target && target->IsAlive() && target->GetEntry() == NPC_CRUSHER_TENTACLE &&
           !YoggSaronCrusherStunHolds(botAI, target);
}

float YoggSaronMovementGuardMultiplier::SetBehindGuard()
{
    // Arithmetic first. Below the platform there is no body.
    if (bot->GetPositionZ() < ULDUAR_YOGG_SARON_BOSS_ROOM_AXIS_Z_PATHING_ISSUE_DETECT)
        return 1.0f;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return 1.0f;

    // Both SetBehindTargetAction::Execute spots (108 degrees either side of the facing) often land inside the ring:
    // Guardians are tanked 18.5 yd out and are yards across. No frontal attack, so skipping it costs parries only.
    float const distance =
        std::max(bot->GetExactDist(target), bot->GetMeleeRange(target) / 2.0f) - bot->GetCombatReach();
    Position const& middle = ULDUAR_YOGG_SARON_MIDDLE;

    bool intoRing = false;
    for (float const side : {1.0f, -1.0f})
    {
        float const angle = Position::NormalizeOrientation(target->GetOrientation() + side * SET_BEHIND_SPOT_ANGLE);
        float const x = target->GetPositionX() + std::cos(angle) * distance;
        float const y = target->GetPositionY() + std::sin(angle) * distance;

        intoRing = intoRing || middle.GetExactDist2d(x, y) < ULDUAR_YOGG_SARON_BODY_KNOCKBACK_CLEAR_RADIUS ||
                   !YoggSaronRouteClearOfBody(bot, x, y);
    }

    if (!intoRing)
        return 1.0f;

    uint32 const phase = YoggSaronPhase(botAI);
    return phase == 2 || phase == 3 ? 0.0f : 1.0f;
}

float YoggSaronMovementGuardMultiplier::GetValue(Action* action)
{
    if (!action)
        return 1.0f;

    if (dynamic_cast<FleeAction*>(action))
        return FleeGuard();

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return SetBehindGuard();

    if (!dynamic_cast<ReachTargetAction*>(action))
        return 1.0f;

    // The heal reach is what keeps a healer in range of somebody no station can see, and it never walks
    // anyone into the body. Leave it alone.
    if (action->getName() == "reach party member to heal")
        return 1.0f;

    // Both windows sit near the platform middle, so the arithmetic goes first.
    // The brain room middle is 2.3 yd from the platform middle in 2d: the ring test would veto reach on the Brain.
    if (bot->GetPositionZ() < ULDUAR_YOGG_SARON_BOSS_ROOM_AXIS_Z_PATHING_ISSUE_DETECT)
        return 1.0f;

    if (ReachIsWrong(action))
        return 0.0f;

    if (CrusherReachIsWrong())
        return 0.0f;

    // Above the radius test: the handover anchor is outside the ring, so an arrived bot would get reach back and leave.
    // Spans the phase boundary: the window is phase 1, the hold that outlives it is phase 2.
    if (YoggSaronHandoverState(botAI).clearing)
        return 0.0f;

    float const fromMiddle =
        bot->GetDistance2d(ULDUAR_YOGG_SARON_MIDDLE.GetPositionX(), ULDUAR_YOGG_SARON_MIDDLE.GetPositionY());
    if (fromMiddle >= ULDUAR_YOGG_SARON_BODY_KNOCKBACK_CLEAR_RADIUS)
        return 1.0f;

    uint32 const phase = YoggSaronPhase(botAI);

    // Exactly the radius the phase 2 spacing trigger fires at, so reach stands down only while that node owns the bot.
    // A wider band leaves a ring where neither node moves anybody. The spacing node runs in phase 3 too.
    if ((phase == 2 || phase == 3) && fromMiddle < ULDUAR_YOGG_SARON_BODY_KNOCKBACK_RADIUS)
        return 0.0f;

    return 1.0f;
}

float YoggSaronPhase1AoeHoldMultiplier::GetValue(Action* action)
{
    if (!action || action->getThreatType() != Action::ActionThreatType::Aoe)
        return 1.0f;

    if (dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    // Brain level first: no grid sweep for the bots under the floor.
    if (YoggSaronOnBrainLevel(bot))
        return 1.0f;

    // Past phase 1 only the Guardians left over from it matter, and the sweep finds none once they're dead.
    bool const hold = YoggSaronInPhase1(botAI) ? YoggSaronPhase1AoeHold(botAI) : YoggSaronLeftoverAoeHold(botAI);
    return hold ? 0.0f : 1.0f;
}

float YoggSaronIllusionMultiTargetHoldMultiplier::GetValue(Action* action)
{
    if (!action || dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    // Under the floor first, so the bots on the platform pay one compare.
    if (bot->GetPositionZ() >= ULDUAR_YOGG_SARON_BOSS_ROOM_AXIS_Z_PATHING_ISSUE_DETECT)
        return 1.0f;

    YoggSaronRoom const room = YoggSaronRoomOf(bot);
    if (room != YOGG_SARON_ROOM_STORMWIND && room != YOGG_SARON_ROOM_ICECROWN && room != YOGG_SARON_ROOM_CHAMBER)
        return 1.0f;

    if (action->getThreatType() == Action::ActionThreatType::Aoe || MULTI_TARGET_ACTIONS.count(action->getName()))
        return 0.0f;

    return 1.0f;
}

float YoggSaronExitHoldCastMultiplier::GetValue(Action* action)
{
    CastSpellAction* spellAction = dynamic_cast<CastSpellAction*>(action);
    if (!spellAction || bot->GetMapId() != ULDUAR_MAP_ID || !YoggSaronOnBrainLevel(bot) ||
        !YoggSaronLeavingBrainLevel(bot))
        return 1.0f;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(AI_VALUE2(uint32, "spell id", spellAction->getSpell()));
    if (!spellInfo)
        return 1.0f;

    // Bladestorm and Killing Spree fail the portal's untriggered cast for as long as they run.
    bool const locks = spellInfo->HasAura(SPELL_AURA_ALLOW_ONLY_ABILITY);

    return spellInfo->CalcCastTime() || spellInfo->IsChanneled() || locks ? 0.0f : 1.0f;
}

float YoggSaronHeldTargetCastMultiplier::GetValue(Action* action)
{
    CastSpellAction* spellAction = dynamic_cast<CastSpellAction*>(action);
    if (!spellAction || bot->GetMapId() != ULDUAR_MAP_ID)
        return 1.0f;

    // Both reads are map lookups, no sweep. Immortal Guardians only ever stand on the platform.
    bool const brainDots = YoggSaronOnBrainLevel(bot) && YoggSaronBrainDotHeld(botAI);
    bool const hunterHold = !brainDots && YoggSaronBeaconHunterHold(botAI);
    bool const platform =
        !brainDots && bot->GetPositionZ() >= ULDUAR_YOGG_SARON_BOSS_ROOM_AXIS_Z_PATHING_ISSUE_DETECT;
    if (!brainDots && !hunterHold && !platform)
        return 1.0f;

    Unit* target = action->GetTarget();
    if (!target)
        return 1.0f;

    if (hunterHold && target->GetEntry() == NPC_MARKED_IMMORTAL_GUARDIAN)
        return 0.0f;

    if (!brainDots)
        return GuardianThreatHeld(spellAction, target) ? 0.0f : 1.0f;

    if (target->GetEntry() != NPC_BRAIN)
        return 1.0f;

    if (YoggSaronBrainHeld(botAI))
        return 0.0f;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(AI_VALUE2(uint32, "spell id", spellAction->getSpell()));
    if (!spellInfo)
        return 1.0f;

    return spellInfo->IsChanneled() || spellInfo->HasAura(SPELL_AURA_PERIODIC_DAMAGE) ||
                   spellInfo->HasAura(SPELL_AURA_PERIODIC_LEECH)
               ? 0.0f
               : 1.0f;
}

bool YoggSaronHeldTargetCastMultiplier::GuardianThreatHeld(CastSpellAction* spellAction, Unit* target)
{
    uint32 const entry = target->GetEntry();
    if ((entry != NPC_IMMORTAL_GUARDIAN && entry != NPC_MARKED_IMMORTAL_GUARDIAN) || PlayerbotAI::IsTank(bot))
        return false;

    // Kicks still go, a Drain Life left running costs more than the threat.
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(AI_VALUE2(uint32, "spell id", spellAction->getSpell()));
    if (spellInfo && spellInfo->HasEffect(SPELL_EFFECT_INTERRUPT_CAST))
        return false;

    return !YoggSaronGuardianThreatAllows(bot, target, target->GetGUID() == bot->GetTarget());
}

float YoggSaronPhase3AreaMultiplier::GetValue(Action* action)
{
    // Only spells that hit each target for less than the single-target spell they replace. Divine Storm,
    // Whirlwind, Chain Lightning, Starfall, Multi-Shot etc. hit the main target in full and stay out.
    static std::set<std::string> const areaActions = {
        "blizzard", "flamestrike", "blast wave", "dragon's breath", "cone of cold", "hurricane", "typhoon",
        "volley", "mind sear", "rain of fire", "seed of corruption", "seed of corruption on attacker",
        "fan of knives", "swipe (cat)", "death and decay", "blood boil", "cleave", "fire nova", "magma totem"};
    if (!action || !areaActions.count(action->getName()))
        return 1.0f;

    if (bot->GetMapId() != ULDUAR_MAP_ID ||
        bot->GetPositionZ() < ULDUAR_YOGG_SARON_BOSS_ROOM_AXIS_Z_PATHING_ISSUE_DETECT || PlayerbotAI::IsTank(bot))
        return 1.0f;

    Unit* target = action->GetTarget();
    if (!target || target == bot)
        target = AI_VALUE(Unit*, "current target");

    if (!target || !YoggSaronInPhase3(botAI))
        return 1.0f;

    // The area trigger's own test: distance minus the unit's size. Yogg's reach is 30, so he counts from
    // 38 yd off his middle, and the sweep has to reach that far.
    std::list<Creature*> nearby;
    target->GetCreatureListWithEntryInGrid(nearby,
                                           {NPC_YOGG_SARON, NPC_IMMORTAL_GUARDIAN, NPC_MARKED_IMMORTAL_GUARDIAN,
                                            NPC_CRUSHER_TENTACLE, NPC_CORRUPTOR_TENTACLE, NPC_CONSTRICTOR_TENTACLE},
                                           ULDUAR_YOGG_SARON_PHASE_3_AREA_RADIUS + ULDUAR_YOGG_SARON_COMBAT_REACH);

    // Same lines the dps resolver keeps: with Thorim down to Weakened, without him to the floor and
    // never a marked one.
    bool const thorim = YoggThorimKeeperActive(botAI);
    float const floorPct =
        thorim ? ULDUAR_YOGG_SARON_GUARDIAN_WEAKENED_PCT : ULDUAR_YOGG_SARON_PHASE_3_GUARDIAN_FLOOR_PCT;

    // Mind Sear never hits its own target.
    bool const sear = action->getName() == "mind sear";

    uint32 worth = 0;
    for (Creature* unit : nearby)
    {
        if (!unit->IsAlive() || (sear && unit == target) ||
            unit->GetDistance(target->GetPosition()) > ULDUAR_YOGG_SARON_PHASE_3_AREA_RADIUS)
            continue;

        uint32 const entry = unit->GetEntry();
        if (entry == NPC_IMMORTAL_GUARDIAN || entry == NPC_MARKED_IMMORTAL_GUARDIAN)
        {
            // One no tank holds yet turns on whoever hits it, and a dragged one is on its dragger.
            if (!YoggSaronGuardianThreatAllows(bot, unit, false))
                return 0.0f;

            if ((entry == NPC_MARKED_IMMORTAL_GUARDIAN && !thorim) || unit->GetHealthPct() <= floorPct)
                continue;
        }

        ++worth;
    }

    return worth >= ULDUAR_YOGG_SARON_PHASE_3_AREA_MIN_WORTH ? 1.0f : 0.0f;
}

float YoggSaronPhase1WalkGuardMultiplier::GetValue(Action* action)
{
    if (!action)
        return 1.0f;

    std::string const name = action->getName();
    bool const positioning = name == "yogg-saron guardian positioning action";
    bool const station = name == "yogg-saron phase 1 station action";
    bool const reachMelee = name == "reach melee";
    if (!positioning && !station && !reachMelee && name != "reach spell" && name != "set behind" &&
        name != "yogg-saron body detour action" && name != "combat formation move")
        return 1.0f;

    // Plain distance and height before any sweep: this weighs every reach in Ulduar, and the brain room
    // sits under the middle of the platform.
    if (!YoggSaronInPhase1Room(bot) || bot->GetPositionZ() < ULDUAR_YOGG_SARON_BOSS_ROOM_AXIS_Z_PATHING_ISSUE_DETECT)
        return 1.0f;

    // After phase 1 no station or leash keeps a step-out and the body detour hauls it back: hold on Fervor's radius.
    // Phase 3 too. The sweep misses Immortal Guardians (36064), so a low one that cannot die pins nobody.
    if (!YoggSaronInPhase1(botAI))
        return GetYoggSaronNovaThreats(botAI, ULDUAR_YOGG_SARON_FERVOR_HOLD_RADIUS).empty() ? 1.0f
                                                                                            : 0.0f;

    bool const fervor = bot->HasAura(SPELL_SARAS_FERVOR) &&
                        !GetYoggSaronNovaThreats(botAI, ULDUAR_YOGG_SARON_FERVOR_HOLD_RADIUS).empty();
    if (!fervor && !positioning && !station && !reachMelee)
        return 1.0f;

    if (!fervor && !WalkCrossesCloud(station, reachMelee))
        return 1.0f;

    // The positioning node's ring walk at the handover heads out, away from both.
    return positioning && YoggSaronHandoverState(botAI).clearing ? 1.0f : 0.0f;
}

bool YoggSaronPhase1WalkGuardMultiplier::WalkCrossesCloud(bool station, bool reachMelee)
{
    if (station)
    {
        // The second orbit passing over the stack is the station's own cost, and one more bot joining
        // it summons nothing extra.
        Position const& spot = ULDUAR_YOGG_SARON_P1_RANGED_SPOT;
        return YoggSaronWalkCrossesCloud(
            bot, spot, &spot, ULDUAR_YOGG_SARON_CLOUD_AVOID_RADIUS + ULDUAR_YOGG_SARON_P1_RANGED_STACK_RADIUS);
    }

    // Never inside the leash: the inner orbit reaches most of it, so a step on the stack would be held
    // every time that cloud came round.
    Position const& middle = ULDUAR_YOGG_SARON_MIDDLE;
    if (bot->GetDistance2d(middle.GetPositionX(), middle.GetPositionY()) <= ULDUAR_YOGG_SARON_P1_LEASH)
        return false;

    if (!reachMelee)
        return YoggSaronWalkCrossesCloud(bot, middle);

    Unit* target = AI_VALUE(Unit*, "current target");
    return target && YoggSaronWalkCrossesCloud(bot, target->GetPosition());
}

bool YoggSaronAntiFearTotemGuardMultiplier::FearWindowActive() { return YoggSaronFearWindowActive(botAI); }
