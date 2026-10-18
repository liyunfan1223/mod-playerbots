/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldTriggers_YoggSaron.h"
#include "UldActions_YoggSaron.h"
#include "RaidTankDefensive.h"

#include <algorithm>
#include <cmath>

#include "GameObject.h"
#include "Group.h"
#include "Object.h"
#include "Pet.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "UldEncounter_YoggSaron.h"
#include "UldScripts.h"
#include "EncounterHelpers.h"
#include "ScriptedCreature.h"
#include "SpellMgr.h"
#include "SharedDefines.h"
#include "PlayerbotAIConfig.h"
#include "Trigger.h"
#include "Vehicle.h"
#include "WorldSession.h"
#include <MovementActions.h>
#include <FollowMasterStrategy.h>
#include <CombatStrategy.h>
#include <TankAssistStrategy.h>

using namespace EncounterHelpers;

// IsBotMainTank goes false for every bot the moment a human holds main tank, which would switch both
// single-actor nodes off. First living bot tank instead.
bool YoggSaronTrigger::IsDesignatedBotTank()
{
    if (PlayerbotAI::IsBotMainTank(bot))
        return true;

    if (!PlayerbotAI::IsTank(bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return true;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !PlayerbotAI::IsTank(member))
            continue;

        if (!member->GetSession() || !member->GetSession()->IsHeadless())
            continue;

        return member == bot;
    }

    return false;
}

bool YoggSaronTrigger::IsPhase2()
{
    return YoggSaronInPhase2(botAI);
}

bool YoggSaronTrigger::IsPhase3()
{
    return YoggSaronInPhase3(botAI);
}

bool YoggSaronTrigger::IsInBrainLevel() { return YoggSaronOnBrainLevel(bot); }

bool YoggSaronTrigger::PhaseThreeStationReaches(Unit* target)
{
    Position const& spot = YoggSaronPhaseThreeSpot(bot);
    float const reach = PlayerbotAI::IsMelee(bot) ? sPlayerbotAIConfig.MeleeDistance : sPlayerbotAIConfig.SpellDistance;

    return spot.GetExactDist2d(target->GetPositionX(), target->GetPositionY()) -
               ULDUAR_YOGG_SARON_PHASE_3_STATION_RADIUS <=
           reach;
}

bool YoggSaronTrigger::IsYoggSaronFight() { return YoggSaronEncounterActive(botAI); }

bool YoggSaronTrigger::IsInIllusionRoom()
{
    YoggSaronRoom const room = YoggSaronRoomOf(bot);

    return room == YOGG_SARON_ROOM_STORMWIND || room == YOGG_SARON_ROOM_ICECROWN ||
           room == YOGG_SARON_ROOM_CHAMBER;
}

bool YoggSaronTrigger::IsInStormwindKeeperIllusion() { return YoggSaronRoomOf(bot) == YOGG_SARON_ROOM_STORMWIND; }

bool YoggSaronTrigger::IsInIcecrownKeeperIllusion() { return YoggSaronRoomOf(bot) == YOGG_SARON_ROOM_ICECROWN; }

bool YoggSaronTrigger::IsInChamberOfTheAspectsIllusion() { return YoggSaronRoomOf(bot) == YOGG_SARON_ROOM_CHAMBER; }

bool YoggSaronTrigger::IsMasterIsInIllusionGroup()
{
    Player* master = botAI->GetMaster();
    return master && !PlayerbotAI::IsTank(master);
}

bool YoggSaronTrigger::IsMasterIsInBrainRoom()
{
    return YoggSaronRoomOf(botAI->GetMaster()) == YOGG_SARON_ROOM_BRAIN;
}

bool YoggSaronTrigger::IsBrainRoomApproachable() { return YoggSaronBrainRoomApproachable(botAI); }

bool YoggSaronGuardianPositioningTrigger::IsActive()
{
    if (!PlayerbotAI::IsTank(bot) && !PlayerbotAI::IsMelee(bot))
        return false;

    // The handover wins over the leash: Sara is already down, so holding a bot on the leash only buys knockbacks. No
    // radius of its own: the anchor is outside the ring, and an arrived bot would hand itself back to the leash.
    if (YoggSaronHandoverState(botAI).clearing)
    {
        returning = false;
        return true;
    }

    float const fromSara =
        bot->GetDistance2d(ULDUAR_YOGG_SARON_MIDDLE.GetPositionX(), ULDUAR_YOGG_SARON_MIDDLE.GetPositionY());

    // A melee bot neither tanking nor healing is held tighter, inside the radius the inner orbit can't be woken from.
    // The wider pair stays the tank's: it is about where the Guardian dies, which follows threat.
    bool const pile = !PlayerbotAI::IsTank(bot) && !PlayerbotAI::IsHeal(bot);
    float const leash = pile ? ULDUAR_YOGG_SARON_P1_PILE_LEASH : ULDUAR_YOGG_SARON_P1_LEASH;
    float const release = pile ? ULDUAR_YOGG_SARON_P1_PILE_RELEASE : ULDUAR_YOGG_SARON_P1_LEASH_RELEASE;

    // The release radius is what makes this hysteresis rather than a boundary: let go the instant the
    // bot crosses the leash, reach melee drags it straight back out and the two trade the tick.
    bool const beyond = fromSara > (returning ? release : leash);

    if (!beyond && fromSara >= ULDUAR_YOGG_SARON_BODY_KNOCKBACK_CLEAR_RADIUS)
    {
        returning = false;
        return false;
    }

    // Phase before the Guardian sweep: melee sit outside this leash for most of phases 2 and 3. The Guardian
    // test stays, it is why nothing walks back before the first spawn.
    bool const active = beyond && fromSara <= ULDUAR_YOGG_SARON_P1_ROOM_RADIUS && YoggSaronInPhase1(botAI) &&
                        bot->FindNearestCreature(NPC_GUARDIAN_OF_YS, sPlayerbotAIConfig.SightDistance, true);

    returning = active;

    return returning;
}

bool YoggSaronSanityTrigger::IsActive()
{
    // All five Sanity Wells stand on the boss platform and nothing restores Sanity underground, so
    // below the floor this node can only ever walk a bot at something it will never reach.
    if (IsInBrainLevel())
        return false;

    Aura* sanityAura = bot->GetAura(SPELL_SANITY);

    if (!sanityAura)
        return false;

    int sanityAuraStacks = sanityAura->GetStackAmount();

    // Full Sanity is false whatever the well lookup below finds, so skip the sweep.
    if (sanityAuraStacks >= 100)
        return false;

    Creature* sanityWell = YoggSaronNearestCreature(botAI, NPC_SANITY_WELL);

    if (!sanityWell)
        return false;

    // A well halves damage done, so a bot goes only with nothing to kill, or when low, and stays until full. Topping
    // outranks the portal walk. Refill is a benched brain team bot (~80 s wave to reach 100), so it always goes.
    char const* reason = nullptr;
    if (bot->GetDistance(sanityWell) < 1.0f && !YoggSaronPortalWalkPending(botAI))
        reason = "topping";
    else if (sanityAuraStacks < static_cast<int>(ULDUAR_YOGG_SARON_SANITY_LOW))
        reason = "low";
    else if (IsPhase2() && sanityAuraStacks < static_cast<int>(ULDUAR_YOGG_SARON_BRAIN_TEAM_MIN_SANITY) &&
             !YoggSaronPortalWalkPending(botAI))
        reason = "refill";
    else if (IsPhase2() && YoggSaronPlatformIdle(botAI) && !YoggSaronPortalWalkPending(botAI))
        reason = "idle";

    if (!reason)
        return false;

    // A well the bot cannot get to would otherwise park it stationary, suppressing everything below this node.
    return YoggSaronWalkMakingProgress(botAI, "sanity", sanityWell->GetPosition());
}

bool YoggSaronPhase1SpacingTrigger::IsActive()
{
    if (!botAI->CanMove() || !YoggSaronInPhase1Room(bot))
        return false;

    // Phase before the hazard sweeps, which would otherwise run all through phases 2 and 3.
    if (!YoggSaronInPhase1(botAI))
        return false;

    // The cloud read is "I am standing in one", since the orbit outruns the step.
    bool hazardNear = bot->FindNearestCreature(NPC_OMINOUS_CLOUD, ULDUAR_YOGG_SARON_CLOUD_AVOID_RADIUS, true);

    char const* reason = hazardNear ? "cloud" : nullptr;

    // Shadow Nova is the other half, and only a Guardian about to detonate counts: running from every
    // one of them scatters the raid to the rim to be picked off one at a time.
    if (!hazardNear && !GetYoggSaronNovaThreats(botAI, ULDUAR_YOGG_SARON_SHADOW_NOVA_TRIGGER_RADIUS).empty())
        reason = bot->HasAura(SPELL_SARAS_FERVOR) ? "fervor" : "nova";

    return reason != nullptr;
}

bool YoggSaronPhase1StationTrigger::IsActive()
{
    if (!botAI->CanMove())
        return false;

    // Melee and tanks have their own station, the leash that holds them on Sara.
    if (!PlayerbotAI::IsRanged(bot) && !PlayerbotAI::IsHeal(bot))
        return false;

    // Distance only, no sweep, so both reads cost nothing before the phase read.
    float const fromSara =
        bot->GetDistance2d(ULDUAR_YOGG_SARON_MIDDLE.GetPositionX(), ULDUAR_YOGG_SARON_MIDDLE.GetPositionY());
    if (fromSara > ULDUAR_YOGG_SARON_P1_ROOM_RADIUS)
        return false;

    float const fromSpot = bot->GetDistance2d(ULDUAR_YOGG_SARON_P1_RANGED_SPOT.GetPositionX(),
                                              ULDUAR_YOGG_SARON_P1_RANGED_SPOT.GetPositionY());

    // The band keeps the first and third orbits off the back line, so the parked latch widens only the stack radius.
    float const stack =
        ULDUAR_YOGG_SARON_P1_RANGED_STACK_RADIUS * (parked ? ULDUAR_YOGG_SARON_P1_PARKED_STACK_SCALE : 1.0f);

    bool const offBand = std::abs(fromSara - ULDUAR_YOGG_SARON_P1_RANGED_STATION_RADIUS) >
                         ULDUAR_YOGG_SARON_P1_RANGED_BAND_TOLERANCE;

    char const* reason = offBand ? "band" : (fromSpot > stack ? "stack" : nullptr);

    parked = !reason;

    return reason && YoggSaronInPhase1(botAI);
}

bool YoggSaronPhase2SpacingTrigger::IsActive()
{
    if (!botAI->CanMove())
        return false;

    // Phase before the hazard sweeps: with no fight it is a boss state read, and this runs for every bot in Ulduar.
    // Phase 3 as well: the body ring outlives phase 2, and no node would walk bots out of it.
    uint32 const phase = YoggSaronPhase(botAI);
    if (phase != 2 && phase != 3)
        return false;

    bool hazardNear = bot->FindNearestCreature(NPC_DEATH_RAY, ULDUAR_YOGG_SARON_DEATH_RAY_TRIGGER_RADIUS, true);

    if (!hazardNear)
    {
        std::vector<Position> wedges = GetYoggSaronCrushWedges(botAI, ULDUAR_YOGG_SARON_CRUSH_RANGE);
        hazardNear = InYoggSaronCrushWedge(wedges, bot->GetPositionX(), bot->GetPositionY(),
                                           ULDUAR_YOGG_SARON_CRUSH_TRIGGER_ARC);
    }

    // No victim exemption here, unlike the wedge. The victim inside the reach is exactly who dies, and
    // stepping out is what stops the swings. Melee only read a stun about to lift.
    if (!hazardNear)
        hazardNear = YoggSaronInCrusherReach(botAI, ULDUAR_YOGG_SARON_CRUSHER_REACH_TRIGGER_RADIUS);

    // The body's ring has to be read here as well as in the action: a circle the action clears but the
    // trigger does not leaves a band where the bot stands in a hazard and the node is never asked.
    if (!hazardNear)
        hazardNear = YoggSaronInBodyKnockback(bot);

    // Same reason for the Guardians phase 1 left alive, which keep their death nova here.
    if (!hazardNear)
        hazardNear = !GetYoggSaronNovaThreats(botAI, ULDUAR_YOGG_SARON_SHADOW_NOVA_TRIGGER_RADIUS).empty();

    return hazardNear;
}

bool YoggSaronSetDpsPriorityTrigger::IsActive()
{
    // Outside phase 2 a tank keeps the generic picker (the resolver would pull it off what it holds; phase 3 guardian
    // control owns it). In phase 2 it walks the tank into a Crusher: GetDistance drops its 8 yd combat reach.
    if (PlayerbotAI::IsTank(bot) && !IsPhase2())
        return false;

    // Fight-wide, and must match the dps target guard rule in the definition, which zeroes the stock assist over this
    // window.
    return IsYoggSaronFight();
}

bool YoggSaronDarkVolleyTrigger::IsActive()
{
    if (!YoggSaronCanInterrupt(bot))
        return false;

    // Answer for this bot, not the raid. The node sits at ACTION_EMERGENCY + 1, so claiming the tick and then finding
    // every caster out of reach wastes the whole tick.
    for (Unit* caster : GetYoggSaronInterruptCasters(botAI))
    {
        if (YoggSaronInterruptClaimedByOther(bot, caster))
            continue;

        for (char const* spell : YoggSaronInterruptSpellsFor(bot, caster))
            if (YoggSaronInterruptReady(botAI, spell, caster))
                return true;
    }

    return false;
}

bool YoggSaronDiminishPowerJudgementTrigger::IsActive()
{
    // Class check first, the grid read only for paladins. No phase read: Crushers only exist in
    // phase 2, on the platform.
    std::vector<char const*> const spells = YoggSaronJudgementSpells(bot);
    if (spells.empty())
        return false;

    // Per bot, same as the dark volley trigger: a paladin out of range must not claim the tick.
    for (Unit* crusher : GetYoggSaronChannellingCrushers(botAI))
        for (char const* spell : spells)
            if (botAI->CanCastSpell(spell, crusher))
                return true;

    return false;
}

bool YoggSaronMortalStrikeTrigger::IsActive()
{
    // Class, cooldown and rage first, the grid read only for a warrior that can swing it. CanCastSpell
    // doesn't check rage.
    if (bot->getClass() != CLASS_WARRIOR)
        return false;

    uint32 const spellId = AI_VALUE2(uint32, "spell id", "mortal strike");
    SpellInfo const* info = spellId ? sSpellMgr->GetSpellInfo(spellId) : nullptr;
    if (!info || bot->HasSpellCooldown(spellId) ||
        info->CalcPowerCost(bot, info->GetSchoolMask()) > static_cast<int32>(bot->GetPower(POWER_RAGE)))
        return false;

    return YoggSaronMortalStrikeTarget(botAI) != nullptr;
}

bool YoggSaronWarlockPetTrigger::IsActive()
{
    // Class and pet first, both free.
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    Pet* pet = bot->GetPet();
    if ((pet && pet->IsAlive()) || YoggSaronOnBrainLevel(bot) || !IsPhase3())
        return false;

    char const* summon = YoggSaronWarlockSummon(botAI);
    return summon && botAI->CanCastSpell(summon, bot);
}

bool YoggSaronMaladyOfTheMindTrigger::IsActive()
{
    if (!IsPhase2())
        return false;

    TooCloseToPlayerWithDebuffTrigger tooCloseToPlayerWithDebuffTrigger(botAI);
    return tooCloseToPlayerWithDebuffTrigger.TooCloseToPlayerWithDebuff(SPELL_MALADY_OF_THE_MIND,
                                                                        ULDUAR_YOGG_SARON_MALADY_SPREAD_RADIUS) &&
           botAI->CanMove();
}

bool YoggSaronPhase3ControlTrigger::IsActive()
{
    if (!IsPhase3() || !IsDesignatedBotTank())
        return false;

    // The strategy swap has to land once even with nothing left to kill, so it gates on its own state
    // rather than on a guardian being up.
    TankFaceStrategy tankFaceStrategy(botAI);
    if (botAI->HasStrategy(tankFaceStrategy.getName(), BotState::BOT_STATE_COMBAT))
        return true;

    TankAssistStrategy tankAssistStrategy(botAI);
    return !botAI->HasStrategy(tankAssistStrategy.getName(), BotState::BOT_STATE_COMBAT);
}

bool YoggSaronBrainLinkTrigger::IsActive()
{
    // Not TooFarFromPlayerWithAura: 63802 sits on one end only, so the pair is read off the cast, which both ends can
    // see. A brain team bot heading for a portal keeps going: the link drops at 10 yd apart vertically.
    return IsPhase2() && YoggSaronBrainLinkTarget(botAI) != nullptr && !YoggSaronPortalWalkPending(botAI);
}

bool YoggSaronMoveToEnterPortalTrigger::IsActive()
{
    // The spread happens before the wave exists, so "diamond" (set by the walk) keeps the node live. The arena test
    // stops it firing underground, where the mark reads diamond until the illusion room node renames it.
    std::string const rti = AI_VALUE(std::string, "rti");
    if (rti != "skull" && rti != "diamond")
        return false;

    if (YoggSaronRoomOf(bot) != YOGG_SARON_ROOM_ARENA)
        return false;

    Position spot;
    YoggSaronPortalIntent const intent = YoggSaronPortalPlan(botAI, spot);

    return intent == YOGG_SARON_PORTAL_SPREADING || intent == YOGG_SARON_PORTAL_LATE;
}

bool YoggSaronFallFromFloorTrigger::IsActive()
{
    if (!IsYoggSaronFight())
        return false;

    std::string rtiMark = AI_VALUE(std::string, "rti");

    if (rtiMark == "skull" && bot->GetPositionZ() < ULDUAR_YOGG_SARON_BOSS_ROOM_AXIS_Z_PATHING_ISSUE_DETECT)
        return true;

    if ((rtiMark == "cross" || rtiMark == "circle" || rtiMark == "star") &&
        bot->GetPositionZ() < ULDUAR_YOGG_SARON_BRAIN_ROOM_AXIS_Z_PATHING_ISSUE_DETECT)
        return true;

    return false;
}

bool YoggSaronStopFollowingTrigger::IsActive()
{
    if (!IsYoggSaronFight() || !IsPhase2())
        return false;

    FollowMasterStrategy followMasterStrategy(botAI);
    return botAI->HasStrategy(followMasterStrategy.getName(), BotState::BOT_STATE_NON_COMBAT);
}

bool YoggSaronUsePortalTrigger::IsActive()
{
    if (!IsPhase2())
        return false;

    if (AI_VALUE(std::string, "rti") != "diamond")
        return false;

    return bot->FindNearestCreature(NPC_DESCEND_INTO_MADNESS, ULDUAR_YOGG_SARON_PORTAL_CLICK_RADIUS, true) !=
           nullptr;
}

bool YoggSaronIllusionRoomTrigger::IsActive()
{
    if (!IsYoggSaronFight() || !IsInIllusionRoom() || AI_VALUE(std::string, "rti") == "square")
        return false;

    if (SetRtiMarkRequired())
        return true;

    if (GoToBrainRoomRequired())
        return true;

    return WalkIntoRoomRequired() || StageRequired();
}

bool YoggSaronIllusionRoomTrigger::WalkIntoRoomRequired()
{
    // Not the healer: it holds no target, so its room state reads walkingin until the door opens, and a
    // forced walk every tick would cost it every heal ranked below this node. It has its own station.
    return YoggSaronRoomStateOf(botAI) == YOGG_SARON_ROOM_STATE_WALKING_IN && !PlayerbotAI::IsHeal(bot);
}

bool YoggSaronIllusionRoomTrigger::StageRequired()
{
    Position spot;
    if (YoggSaronRoomStateOf(botAI) != YOGG_SARON_ROOM_STATE_STAGED || !YoggSaronIllusionStageSpot(bot, spot) ||
        bot->GetExactDist2d(spot.GetPositionX(), spot.GetPositionY()) <= ULDUAR_YOGG_SARON_STAGE_ARRIVED_RADIUS)
        return false;

    return YoggSaronWalkMakingProgress(botAI, "stage", spot);
}

bool YoggSaronIllusionRoomTrigger::GoToBrainRoomRequired()
{
    if (AI_VALUE(std::string, "rti") == "square")
        return false;

    return IsBrainRoomApproachable();
}

bool YoggSaronIllusionRoomTrigger::SetRtiMarkRequired()
{
    return AI_VALUE(std::string, "rti") == "diamond";
}

bool YoggSaronIllusionHealerStationTrigger::IsActive()
{
    if (!PlayerbotAI::IsHeal(bot) || !botAI->CanMove())
        return false;

    // The middle of the room's live tentacles, not the room's own middle: the centroid of the summon group, the one
    // spot with nothing on it. In the Chamber that ring sits 36-44 yd out, past the 38.5 yd heal range.
    Position station;
    if (!YoggSaronIllusionHealerStation(botAI, station))
        return false;

    if (bot->GetExactDist2d(station.GetPositionX(), station.GetPositionY()) <=
        ULDUAR_YOGG_SARON_ILLUSION_HEALER_STATION_RADIUS)
        return false;

    // A cleared room hands the healer to the walk onto the Brain.
    if (!IsYoggSaronFight() || YoggSaronInfluenceTentaclesCleared(botAI))
        return false;

    return YoggSaronWalkMakingProgress(botAI, "healer", station);
}

bool YoggSaronReprisalRegroupTrigger::IsActive()
{
    // The hold is a set lookup, so it goes before the level test.
    if (!YoggSaronReprisalHold(botAI) || !botAI->CanMove() || !IsInBrainLevel())
        return false;

    Player* healer = YoggSaronRoomHealer(botAI);
    return healer && bot->GetExactDist2d(healer) > ULDUAR_YOGG_SARON_REPRISAL_REGROUP_RANGE;
}

bool YoggSaronBrainSpotTrigger::IsActive()
{
    if (!IsInBrainLevel() || !botAI->CanMove())
        return false;

    // Square goes on in the illusion room the tick its door is seen open, and the exit portal hands it
    // back to skull, so it marks exactly the stretch from the open door to the way out.
    if (AI_VALUE(std::string, "rti") != "square")
        return false;

    // The way out wins once Induce Madness is close enough to walk for: both walks are forced and the exit node can't
    // hold every tick of a 20 yd walk, so this one would take the gaps and turn bots round short of a portal.
    if (YoggSaronShouldLeaveBrainLevel(botAI))
        return false;

    Position const spot = YoggSaronBrainSpot(bot);
    if (bot->GetExactDist2d(spot.GetPositionX(), spot.GetPositionY()) <= ULDUAR_YOGG_SARON_BRAIN_SPOT_ARRIVED_RADIUS)
        return false;

    Creature* brain = YoggSaronNearestCreature(botAI, NPC_BRAIN);
    if (!brain || !brain->IsAlive())
        return false;

    return YoggSaronWalkMakingProgress(botAI, "brainspot", spot);
}

bool YoggSaronNitroBoostsTrigger::IsActive()
{
    if (!IsInBrainLevel() || bot->HasAura(NITRO_BOOSTS_SPEED_SPELL) || !GetNitroBoostBoots(bot) ||
        YoggSaronLeavingBrainLevel(bot))
        return false;

    if (YoggSaronRoomOf(bot) != YOGG_SARON_ROOM_CHAMBER ||
        YoggSaronRoomStateOf(botAI) != YOGG_SARON_ROOM_STATE_TO_BRAIN)
        return false;

    Position const spot = YoggSaronBrainSpot(bot);
    if (bot->GetExactDist2d(spot.GetPositionX(), spot.GetPositionY()) <= ULDUAR_YOGG_SARON_NITRO_MIN_WALK)
        return false;

    return !bot->HasSpellCooldown(NITRO_BOOSTS_USE_SPELL);
}

bool YoggSaronBrainDashTrigger::IsActive()
{
    if (bot->getClass() != CLASS_DRUID || !IsInBrainLevel() || bot->GetShapeshiftForm() != FORM_CAT ||
        bot->HasAura(NITRO_BOOSTS_SPEED_SPELL) || YoggSaronLeavingBrainLevel(bot))
        return false;

    // Dash and Nitro Boosts are the same speed aura and only the higher one counts, so a bot with the
    // boots leaves the Chamber walk to them.
    YoggSaronRoom const room = YoggSaronRoomOf(bot);
    if (room == YOGG_SARON_ROOM_CHAMBER && GetNitroBoostBoots(bot))
        return false;

    if (YoggSaronRoomStateOf(botAI) != YOGG_SARON_ROOM_STATE_TO_BRAIN)
        return false;

    Position const spot = YoggSaronBrainSpot(bot);
    if (bot->GetExactDist2d(spot.GetPositionX(), spot.GetPositionY()) <= ULDUAR_YOGG_SARON_NITRO_MIN_WALK ||
        botAI->HasAura("dash", bot))
        return false;

    return botAI->CanCastSpell("dash", bot);
}

bool YoggSaronBrainChargeTrigger::IsActive()
{
    if (bot->getClass() != CLASS_WARRIOR || !IsInBrainLevel() || AI_VALUE(std::string, "rti") != "square" ||
        !botAI->CanMove())
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || !target->IsAlive() || target->GetEntry() != NPC_BRAIN)
        return false;

    // Way in only: the exit walk owns the bot from here, and the hold has it off the Brain anyway.
    if (YoggSaronLeavingBrainLevel(bot) || YoggSaronShouldLeaveBrainLevel(botAI) || YoggSaronBrainHeld(botAI))
        return false;

    // Where the charge lands: the floor ring at both combat reaches from the Brain, on the bot's side.
    Position const spot = YoggSaronBrainSpot(bot);
    float const walkLeft = bot->GetExactDist2d(spot.GetPositionX(), spot.GetPositionY());
    if (walkLeft <= ULDUAR_YOGG_SARON_BRAIN_SPOT_ARRIVED_RADIUS)
        return false;

    float const reach = target->GetCombatReach() + bot->GetCombatReach();
    float const angle = target->GetAngle(bot);
    float const landX = target->GetPositionX() + std::cos(angle) * reach;
    float const landY = target->GetPositionY() + std::sin(angle) * reach;
    float const walkAfter = spot.GetExactDist2d(landX, landY);
    if (walkLeft - walkAfter < ULDUAR_YOGG_SARON_BRAIN_CHARGE_MIN_GAIN)
        return false;

    return botAI->CanCastSpell("charge", target) || botAI->CanCastSpell("intercept", target);
}

bool YoggSaronMoveToExitPortalTrigger::IsActive()
{
    // The Brain sits at z 265 and its room's floor at z 236-244, so a radius to it adds a permanent 25 yd vertical tax.
    // Two of the three portal arrivals land outside 60 yd. The level is the real test.
    if (!IsInBrainLevel() || !IsYoggSaronFight())
        return false;

    // An illusion room has no exit until its door opens: the portals sit in the brain chamber behind doors that open
    // when the room's last Influence Tentacle dies. The brain chamber itself is exempt.
    if (YoggSaronRoomOf(bot) != YOGG_SARON_ROOM_BRAIN && !IsBrainRoomApproachable())
        return false;

    return YoggSaronShouldLeaveBrainLevel(botAI);
}

bool YoggSaronLaughingSkullTrigger::IsActive()
{
    // Level test first: this runs for every bot in Ulduar and the sweep behind it is not free.
    if (!IsInBrainLevel())
        return false;

    return !GetYoggSaronSkullsInArc(botAI).empty();
}

bool YoggSaronIllusionFacingTrigger::IsActive()
{
    // Level test first: this runs for every bot in Ulduar and the sweeps behind it are not free.
    if (!IsInBrainLevel() || !botAI->CanMove())
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || !target->IsAlive())
        return false;

    // What is hitting the bot now, not what is in range: a bot already facing away from every skull is
    // standing somewhere that works, whatever it may have to walk through later.
    return !GetYoggSaronSkullsInArc(botAI).empty();
}

bool YoggSaronPetGuardTrigger::IsActive()
{
    if (bot->m_Controlled.empty() || !IsYoggSaronFight())
        return false;

    // An owner on a Crusher sends its pet there with every harmful cast, from any range.
    Unit* target = AI_VALUE(Unit*, "current target");
    if (target && target->IsAlive() && target->GetEntry() == NPC_CRUSHER_TENTACLE)
        return true;

    // From the pet, not the owner: a ranged owner stands well outside the reach of the Crusher its pet is on. A
    // passive pet may be one the action hushed and has to give back.
    for (Unit* controlled : bot->m_Controlled)
    {
        Unit* victim = controlled->GetVictim();
        if (victim && victim->GetEntry() == NPC_CRUSHER_TENTACLE)
            return true;

        if (Creature* creature = controlled->ToCreature(); creature && creature->HasReactState(REACT_PASSIVE))
            return true;

        if (controlled->FindNearestCreature(NPC_CRUSHER_TENTACLE, ULDUAR_YOGG_SARON_CRUSH_RANGE, true))
            return true;
    }

    // Last, it's the one with a phase read and two sweeps.
    return bot->GetPet() && YoggSaronPhase3PetLeftover(botAI);
}

bool YoggSaronBodyDetourTrigger::IsActive()
{
    if (!botAI->CanMove() || !IsYoggSaronFight())
        return false;

    // Platform only. The illusion rooms are 93 yd underneath and have no body in them, and the brain
    // room middle is 2.3 yd from the platform middle in 2d, so the ring test would fire down there.
    if (bot->GetPositionZ() < ULDUAR_YOGG_SARON_BOSS_ROOM_AXIS_Z_PATHING_ISSUE_DETECT)
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || !target->IsAlive())
        return false;

    // No way round to a target inside the ring, and Yogg himself stands at its middle. The movement guard
    // keeps reach from walking in after one.
    if (target->GetDistance2d(ULDUAR_YOGG_SARON_MIDDLE.GetPositionX(), ULDUAR_YOGG_SARON_MIDDLE.GetPositionY()) <
        ULDUAR_YOGG_SARON_BODY_KNOCKBACK_CLEAR_RADIUS)
        return false;

    // Nothing to route while the bot can already hit from where it stands, which is when reach stands down too. Combat
    // reach counts: a full-health Guardian is several yards across.
    bool const inReach = PlayerbotAI::IsMelee(bot) ? bot->IsWithinMeleeRange(target)
                                             : bot->IsWithinCombatRange(target, sPlayerbotAIConfig.SpellDistance);
    if (inReach || YoggSaronRouteClearOfBody(bot, target->GetPositionX(), target->GetPositionY()))
        return false;

    // Phase 3 as well, where the brain team surfaces on the far side of the body from its Guardians.
    uint32 const phase = YoggSaronPhase(botAI);
    return phase == 2 || phase == 3;
}

bool YoggSaronLunaticGazeTrigger::IsActive()
{
    // 64163 is a 4 s self aura on Yogg, ticking 64164 each second at 130 yd on everyone with him in their front 180
    // degrees. Not a channel (GetCurrentSpell doesn't return it), and Yogg isn't reliably on the bot's threat list.
    Creature* yoggsaron = YoggSaronNearestCreature(botAI, NPC_YOGG_SARON);
    if (!yoggsaron || !yoggsaron->IsAlive())
        return false;

    // Healers keep their back to him for all of phase 3, not just while it ticks: none of their heals
    // need facing, so it costs them nothing and no tick lands while they turn.
    if (PlayerbotAI::IsHeal(bot) && IsPhase3())
        return true;

    return yoggsaron->HasAura(SPELL_LUNATIC_GAZE_YS);
}

bool YoggSaronBeaconDragTrigger::IsActive()
{
    // Class first: the drag pick sweeps from Yogg.
    bool const mayDrag = PlayerbotAI::IsTank(bot) || bot->getClass() == CLASS_HUNTER ||
                         (!PlayerbotAI::IsHeal(bot) &&
                          (bot->getClass() == CLASS_PALADIN || bot->getClass() == CLASS_DEATH_KNIGHT));
    if (!mayDrag || YoggSaronOnBrainLevel(bot))
        return false;

    // A dragger stays on the node after the heal, to walk its Guardian back or feign off it.
    if (YoggSaronBeaconReturning(bot))
        return true;

    return botAI->CanMove() && YoggSaronBeaconDragGuardian(botAI, false) != nullptr;
}

bool YoggSaronBeaconStageTrigger::IsActive()
{
    return YoggSaronBeaconStageSpot(bot) && botAI->CanMove() && !YoggSaronOnBrainLevel(bot);
}

bool YoggSaronPhase3LustTrigger::IsActive()
{
    if (bot->getClass() != CLASS_SHAMAN || bot->HasAura(SPELL_SATED) || bot->HasAura(SPELL_EXHAUSTION))
        return false;

    // The ready test below starts with the same phase test, ahead of anything it writes.
    if (!IsPhase3())
        return false;

    if (!botAI->CanCastSpell("heroism", bot) && !botAI->CanCastSpell("bloodlust", bot))
        return false;

    // Sated stops a second cast on anyone the first reached, not a second shaman spending its own on the
    // rest while the first is still up.
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && (member->HasAura(SPELL_HEROISM_SHAMAN) || member->HasAura(SPELL_BLOODLUST_SHAMAN)))
                return false;
        }
    }

    return YoggSaronPhase3LustReady(botAI);
}

bool YoggSaronPhase3PositioningTrigger::IsActive()
{
    if (!IsYoggSaronFight() || !IsPhase3())
        return false;

    YoggSaronSanityTrigger sanityTrigger(botAI);
    if (sanityTrigger.IsActive())
        return false;

    // Bringing a loose guardian back to the stack is guardian control's job, and that node outranks
    // this one. All the leash adds is a way home once nothing is loose and it returns false.
    if (PlayerbotAI::IsTank(bot))
        return bot->GetDistance(ULDUAR_YOGG_SARON_PHASE_3_MELEE_SPOT) > ULDUAR_YOGG_SARON_PHASE_3_TANK_LEASH;

    Position const& spot = YoggSaronPhaseThreeSpot(bot);

    if (bot->GetDistance2d(spot.GetPositionX(), spot.GetPositionY()) <= ULDUAR_YOGG_SARON_PHASE_3_STATION_RADIUS)
        return false;

    // Somewhere to stand when standing there costs nothing, never a cage: a target the station cannot
    // reach from inside its radius leaves the bot free to go to it.
    Unit* target = AI_VALUE(Unit*, "current target");
    if (target && target->IsAlive() && !PhaseThreeStationReaches(target))
        return false;

    return YoggSaronWalkMakingProgress(botAI, "station", spot);
}

bool YoggSaronGuardianControlTrigger::IsActive()
{
    // Tank first: both branches below are tank-only.
    if (!PlayerbotAI::IsTank(bot))
        return false;

    uint32 const phase = YoggSaronPhase(botAI);

    // Phase 1 wants it harder: a Guardian's death location is the phase. 65719 reaches Sara from 15 yd, so one killed
    // further out is worth nothing, and one killed more than 6.5 yd out novas the back line as well as the melee pile.
    if (phase == 1)
    {
        // The designated bot tank rather than the main tank: IsBotMainTank goes false for every bot the moment a human
        // holds main tank.
        if (!IsDesignatedBotTank() || !YoggSaronInPhase1Room(bot))
            return false;

        return YoggSaronPhase1TauntTarget(botAI) != nullptr;
    }

    // Not gated on Thorim: hard mode means fewer Keepers, so asking for both demands the case where he is least likely
    // to be there. Guardians parked on a tank beat loose ones even where none can die.
    if (phase != 3)
        return false;

    // Fire while a guardian is loose (not Weakened, not held by a tank) or while one at the stack is not the bot's
    // target. Taunting without hitting builds nothing; the action decides whether leaving the held one pays.
    GuidVector const targets = GetYoggSaronGuardiansInSight(botAI);
    if (YoggSaronGuardianLoose(botAI, targets))
        return true;

    Unit* current = AI_VALUE(Unit*, "current target");

    for (ObjectGuid const& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!IsYoggSaronHoldableGuardian(unit))
            continue;

        if (unit != current && unit->GetExactDist2d(ULDUAR_YOGG_SARON_PHASE_3_MELEE_SPOT) <=
                                   ULDUAR_YOGG_SARON_PHASE_3_MELEE_GUARDIAN_RANGE)
            return true;
    }

    return false;
}

bool YoggSaronSanityConservationTrigger::IsActive()
{
    Aura* sanityAura = bot->GetAura(SPELL_SANITY);
    if (!sanityAura || sanityAura->GetStackAmount() > ULDUAR_YOGG_SARON_SANITY_CONSERVE_THRESHOLD)
        return false;

    // The main tank must stay on the adds; never pull it out.
    if (PlayerbotAI::IsBotMainTank(bot))
        return false;

    // A linked bot must stay near its Brain Link partner: retreating would drain more sanity, not less.
    if (bot->HasAura(SPELL_BRAIN_LINK))
        return false;

    // If a Sanity Well is reachable (a Keeper set with Freya), the normal sanity action handles recovery.
    if (YoggSaronNearestCreature(botAI, NPC_SANITY_WELL))
        return false;

    // Boss room only: the spot this retreats to is behind Yogg, who is not down there.
    if (bot->GetPositionZ() < ULDUAR_YOGG_SARON_BOSS_ROOM_AXIS_Z_PATHING_ISSUE_DETECT)
        return false;

    // Let the phase 2 spacing dodge win while a hazard is on top of the bot: a Death Ray still kills at
    // low health. A Death Orb test is useless: it is a marker 27 yd overhead and never reaches anyone.
    if (bot->FindNearestCreature(NPC_DEATH_RAY, ULDUAR_YOGG_SARON_DEATH_RAY_TRIGGER_RADIUS, true))
        return false;

    return true;
}

bool YoggSaronSqueezeRescueTrigger::IsActive()
{
    // Class first: it is free, and this runs for every bot in Ulduar.
    if (bot->getClass() != CLASS_PALADIN || !IsYoggSaronFight())
        return false;

    // A paladin who is held bubbles instead, which is the squeeze escape node below this one. Hand of
    // Protection cannot be cast from inside the tentacle's grip anyway.
    if (bot->HasAura(sSpellMgr->GetSpellIdForDifficulty(SPELL_SQUEEZE, bot)))
        return false;

    return YoggSaronSqueezeVictim(botAI) != nullptr;
}

bool YoggSaronSqueezeEscapeTrigger::IsActive()
{
    if (!IsYoggSaronFight())
        return false;

    // Only these two can shed a periodic damage aura outright; Feign Death and Vanish do not.
    if (bot->getClass() != CLASS_PALADIN && bot->getClass() != CLASS_MAGE)
        return false;

    return bot->HasAura(sSpellMgr->GetSpellIdForDifficulty(SPELL_SQUEEZE, bot));
}

bool YoggSaronNovaDefensiveTrigger::IsActive()
{
    // Cheapest read first: a plain distance, then a 17 yd sweep. A bot spends its own button at the 10% gate only,
    // where the nova lands within 3.7 s; at 20% the raid would empty its cooldowns on the first nova.
    if (!YoggSaronInPhase1Room(bot))
        return false;

    uint32 const needMs = YoggSaronNovaNeedMs(bot, true);

    return needMs && NextMagicDefensive(botAI, bot, needMs);
}

bool YoggSaronNovaExternalTrigger::IsActive()
{
    // Pain Suppression and Guardian Spirit are a healer's, Hand of Sacrifice any paladin's.
    if (!PlayerbotAI::IsHeal(bot) && bot->getClass() != CLASS_PALADIN)
        return false;

    if (!YoggSaronInPhase1Room(bot))
        return false;

    Player* victim = YoggSaronNovaVictim(botAI);

    return victim && YoggSaronNovaExternalAction::PickExternal(botAI, victim);
}

bool YoggSaronRoarAuraTrigger::IsActive()
{
    // Read by the application. With another paladin's Concentration Aura up, ours stays on with no
    // effects, and the name lookup walks effects so it never sees it.
    return YoggSaronRoarAuraHeld(botAI) && !bot->HasAura(SPELL_CONCENTRATION_AURA, bot->GetGUID()) &&
           botAI->CanCastSpell("concentration aura", bot);
}

bool YoggSaronRoarAuraMasteryTrigger::IsActive()
{
    if (bot->getClass() != CLASS_PALADIN || !YoggSaronRoarExpected(botAI))
        return false;

    // Aura Mastery's immunity checks the application too, so an effectless copy still covers the raid.
    return YoggSaronRoarCasting(botAI) && bot->HasAura(SPELL_CONCENTRATION_AURA, bot->GetGUID()) &&
           botAI->CanCastSpell("aura mastery", bot);
}

bool YoggSaronBeaconDefensiveTrigger::IsActive()
{
    // The attacker set before anything else, so a bot nobody is hitting pays nothing.
    if (bot->getAttackers().empty())
        return false;

    // A tank holding the pile only once a failed drag has healed it, at any health: the pile climbs from
    // the floor to 50-77% in 6 s.
    if (PlayerbotAI::IsTank(bot) && !YoggSaronBeaconDragging(bot) && !YoggSaronBeaconReturning(bot))
        return YoggSaronPileHealedOn(bot) >= ULDUAR_YOGG_SARON_BEACON_PILE_RESET_GUARDIANS &&
               NextPhysicalDefensive(botAI, bot, 0) != 0;

    return YoggSaronHealedGuardianOn(botAI, bot) && NextPhysicalDefensive(botAI, bot, 0) != 0;
}

bool YoggSaronBeaconExternalTrigger::IsActive()
{
    // Pain Suppression and Guardian Spirit are a healer's, Hand of Sacrifice any paladin's but a tank's:
    // it moves 30% of the victim's damage onto a tank already under the pile.
    if ((!PlayerbotAI::IsHeal(bot) && bot->getClass() != CLASS_PALADIN) || PlayerbotAI::IsTank(bot))
        return false;

    if (bot->GetPositionZ() < ULDUAR_YOGG_SARON_BOSS_ROOM_AXIS_Z_PATHING_ISSUE_DETECT)
        return false;

    Player* victim = YoggSaronBeaconExternalVictim(botAI);

    return victim && YoggSaronNovaExternalAction::PickExternal(botAI, victim);
}

bool YoggSaronTankLayOnHandsTrigger::IsActive()
{
    return YoggSaronInPhase3(botAI) && YoggSaronTankLayOnHandsTarget(botAI);
}

bool YoggSaronHealerCombatEngineTrigger::IsActive()
{
    return PlayerbotAI::IsHeal(bot) && bot->IsAlive() && bot->IsInCombat() &&
           botAI->GetState() == BOT_STATE_NON_COMBAT && YoggSaronEncounterActive(botAI);
}

bool YoggSaronShadowAuraTrigger::IsActive()
{
    return IsYoggSaronShadowAuraPaladin(bot) &&
           !botAI->HasAura("shadow resistance aura", bot, false, true) &&
           botAI->CanCastSpell("shadow resistance aura", bot);
}

bool YoggSaronNovaRaidCooldownTrigger::IsActive()
{
    if (bot->getClass() != CLASS_PALADIN || !YoggSaronInPhase1Room(bot))
        return false;

    // Cooldown before the raid sweep: both covers run 120 s against a nova every 10, so the count
    // behind this almost never runs.
    if (!YoggSaronNovaRaidCover(botAI))
        return false;

    return YoggSaronNovaHurtCount(botAI) >= ULDUAR_YOGG_SARON_NOVA_RAID_COUNT;
}
