/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldActions_YoggSaron.h"
#include "RaidTankDefensive.h"
#include "UldActions_Shared.h"

#include <CombatStrategy.h>
#include <FollowMasterStrategy.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <tuple>
#include <vector>

#include "AiObjectContext.h"
#include "DBCEnums.h"
#include "GameObject.h"
#include "Group.h"
#include "LastMovementValue.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "Position.h"
#include "UldEncounter_YoggSaron.h"
#include "UldHardMode.h"
#include "UldScripts.h"
#include "EncounterHelpers.h"
#include "RtiValue.h"
#include "ScriptedCreature.h"
#include "ServerFacade.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "Unit.h"
#include "Vehicle.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Item.h"
#include "Opcodes.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include <TankAssistStrategy.h>

using namespace EncounterHelpers;

namespace
{
// The check behind "nearest npcs", with the entry tested first.
struct AnyUnitOfEntriesInRangeCheck
{
    Acore::AnyUnitInObjectRangeCheck inRange;
    uint32 const* entries;
    size_t count;

    bool operator()(Unit* unit)
    {
        return std::find(entries, entries + count, unit->GetEntry()) != entries + count && inRange(unit);
    }
};

// Guardians the raid finishes one at a time. Phase 3: a Shadow Beacon heal outruns split damage.
// Phase 1: two brought down together detonate together.
bool IsYoggSaronFocusedGuardian(Unit* unit)
{
    uint32 const entry = unit->GetEntry();

    return entry == NPC_GUARDIAN_OF_YS || entry == NPC_IMMORTAL_GUARDIAN || entry == NPC_MARKED_IMMORTAL_GUARDIAN;
}

// A cast or channel aimed at this unit, so a drop cuts that and lets a self-heal land.
bool YoggSaronCastingAt(Player* bot, Unit* target)
{
    for (CurrentSpellTypes const type : {CURRENT_GENERIC_SPELL, CURRENT_CHANNELED_SPELL})
        if (Spell* spell = bot->GetCurrentSpell(type))
            if (spell->m_targets.GetUnitTarget() == target)
                return true;

    return false;
}

// Drops a forced walk still booked for somewhere else, since a forced move waits out another forced one's
// hold, up to 5 s. 2D, as the walk stores its end at the ground height it pathed to.
void YoggSaronTakeOverForcedWalk(LastMovement& last, Position const& destination)
{
    if (last.lastMoveShort.GetExactDist2d(destination.GetPositionX(), destination.GetPositionY()) > 1.0f)
        last.clear();
}

// The taunt a tank class points at one Guardian. None of them needs facing.
bool CastYoggSaronTaunt(PlayerbotAI* botAI, Unit* guardian)
{
    switch (botAI->GetBot()->getClass())
    {
        case CLASS_WARRIOR:
            return botAI->CastSpell("taunt", guardian);
        case CLASS_PALADIN:
            return botAI->CastSpell("hand of reckoning", guardian);
        case CLASS_DEATH_KNIGHT:
            return botAI->CastSpell("dark command", guardian);
        case CLASS_DRUID:
            return botAI->CastSpell("growl", guardian);
        default:
            return false;
    }
}

}  // namespace

const Position ULDUAR_YOGG_SARON_BOSS_ROOM_RESTORE_POINT = Position(1928.8923f, -24.871964f, 324.88956f, 6.247805f);

bool YoggSaronGuardianPositioningAction::Execute(Event /*event*/)
{
    if (YoggSaronHandoverState(botAI).clearing)
    {
        // One spot rather than a fan: bots keeping their own bearing spread the front line out of every healer's range.
        Position const& spot = ULDUAR_YOGG_SARON_HANDOVER_MELEE_SPOT;

        return MoveTo(bot->GetMapId(), spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ(),
                      false, false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return MoveTo(bot->GetMapId(), ULDUAR_YOGG_SARON_MIDDLE.GetPositionX(), ULDUAR_YOGG_SARON_MIDDLE.GetPositionY(),
                  ULDUAR_YOGG_SARON_MIDDLE.GetPositionZ(), false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED, true, false);
}

bool YoggSaronSanityAction::Execute(Event /*event*/)
{
    Creature* sanityWell = YoggSaronNearestCreature(botAI, NPC_SANITY_WELL);
    if (!sanityWell)
        return false;

    if (!YoggSaronWalkMakingProgress(botAI, "sanity", sanityWell->GetPosition()))
        return false;

    // Round the body once there is one. Phase 1 has Sara in the middle and clouds on the detour radius.
    Position const destination = YoggSaronNearestCreature(botAI, NPC_YOGG_SARON)
                                     ? YoggSaronBodyRoute(bot, sanityWell->GetPosition())
                                     : sanityWell->GetPosition();

    return MoveTo(bot->GetMapId(), destination.GetPositionX(), destination.GetPositionY(),
                  destination.GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_FORCED, true,
                  false);
}

bool YoggSaronSpacingAction::Execute(Event /*event*/)
{
    HazardSet set;
    if (!Collect(set))
        return false;

    auto const stillClear = [](std::vector<HazardCircle> const& circles,
                               std::function<bool(float, float)> const& clear, Position const& spot)
    {
        for (HazardCircle const& hazard : circles)
            if (hazard.first.GetExactDist2d(spot.GetPositionX(), spot.GetPositionY()) < hazard.second)
                return false;

        return !clear || clear(spot.GetPositionX(), spot.GetPositionY());
    };

    uint32 const now = getMSTime();
    bool const latched = heldSpotMs && getMSTimeDiff(heldSpotMs, now) < ULDUAR_YOGG_SARON_SPACING_HOLD_MS;

    // Claim the tick without touching the motion master while the step is in flight, so combat
    // movement cannot drag the bot back into what it just left and the spline survives.
    if (latched && stillClear(set.hazards, set.clear, heldSpot) && !bot->IsMovementPreventedByCasting() &&
        bot->GetExactDist2d(heldSpot.GetPositionX(), heldSpot.GetPositionY()) > CONTACT_DISTANCE &&
        HeldRouteAcceptable(heldSpot.GetPositionX(), heldSpot.GetPositionY()))
        return true;

    heldSpotMs = 0;

    // Already outside everything, so this decides when a bot moves, not where. Must agree with the trigger:
    // a circle the trigger does not read leaves a band where the bot sits in a hazard and is never asked.
    if (stillClear(set.hazards, set.clear, bot->GetPosition()))
        return false;

    HazardSweepCache sweep;
    Position const middle = Anchor();

    // The cap is the load-bearing half: a bot dodging outward otherwise walks out of spell range and
    // stops contributing for the rest of the phase.
    auto const accept = [this, &middle, &set](float x, float y)
    {
        if (middle.GetExactDist2d(x, y) > MaxFromMiddle())
            return false;

        if (!RouteAcceptable(x, y))
            return false;

        return !set.clear || set.clear(x, y);
    };

    auto const fallbackAccept = [this, &middle, &set](float x, float y)
    {
        if (middle.GetExactDist2d(x, y) > MaxFromMiddle())
            return false;

        if (!RouteAcceptable(x, y))
            return false;

        return !set.fallbackClear || set.fallbackClear(x, y);
    };

    // A spot where the bot can still reach its target, when one exists. Without it the hold lapses, reach melee
    // walks it back into a hazard and this node walks it out again. Dropped, not enforced: a hazard comes first.
    Position safe;
    if (set.preferred)
    {
        auto const preferredAccept = [&accept, &set](float x, float y)
        { return accept(x, y) && set.preferred(x, y); };

        // Anchor on the bot's own bearing, tried before the rings: points 7.8 yd apart at 20 yd miss a 5.6 yd circle.
        // A finer grid clears the passability cache between sweeps. Short of the edge: Sara stands in the middle.
        float const bearing = middle.GetAngle(bot->GetPositionX(), bot->GetPositionY());
        float const inside = ULDUAR_YOGG_SARON_P1_CLOUD_FREE_RADIUS * 0.75f;
        Position const core(middle.GetPositionX() + std::cos(bearing) * inside,
                            middle.GetPositionY() + std::sin(bearing) * inside, bot->GetPositionZ());
        if (preferredAccept(core.GetPositionX(), core.GetPositionY()) &&
            stillClear(set.hazards, set.clear, core))
            safe = core;

        if (safe == Position())
            safe = FindNearestPositionClearOfHazards(bot, set.hazards, SearchRadius(), ULDUAR_YOGG_SARON_SWEEP_STEP,
                                                     ULDUAR_YOGG_SARON_SWEEP_ANGLE_STEP, &middle, preferredAccept,
                                                     &sweep);
    }

    // Every candidate in a ring is the same walk away, so preferNear decides the dodge's character:
    // biased at the middle it sidesteps along the orbit instead of running for the rim.
    if (safe == Position())
        safe = FindNearestPositionClearOfHazards(bot, set.hazards, SearchRadius(), ULDUAR_YOGG_SARON_SWEEP_STEP,
                                                 ULDUAR_YOGG_SARON_SWEEP_ANGLE_STEP, &middle, accept, &sweep);

    // Nothing clears everything at once. Retry on the subset that kills, off the same cache.
    if (safe == Position() && !set.fallback.empty())
        safe = FindNearestPositionClearOfHazards(bot, set.fallback, SearchRadius(), ULDUAR_YOGG_SARON_SWEEP_STEP,
                                                 ULDUAR_YOGG_SARON_SWEEP_ANGLE_STEP, &middle, fallbackAccept, &sweep);

    if (safe == Position())
        return false;

    // MOVEMENT_FORCED: IsWaitingForLastMove yields only to a strictly higher priority, and forced does not beat
    // forced, so a dodge allowed to cut in drops that walk's hold first.
    if (OverridesWalkInFlight())
        AI_VALUE(LastMovement&, "last movement").clear();

    if (!MoveTo(bot->GetMapId(), safe.GetPositionX(), safe.GetPositionY(), safe.GetPositionZ(), false, false, false,
                true, MovementPriority::MOVEMENT_FORCED, true, false))
        return false;

    heldSpot = safe;
    heldSpotMs = now;

    return true;
}

bool YoggSaronPhase1SpacingAction::Collect(HazardSet& set)
{
    clouds.clear();

    std::list<Creature*> found;
    bot->GetCreatureListWithEntryInGrid(found, NPC_OMINOUS_CLOUD,
                                        SearchRadius() + ULDUAR_YOGG_SARON_CLOUD_AVOID_RADIUS);
    for (Creature* cloud : found)
    {
        if (!cloud->IsAlive())
            continue;

        clouds.push_back(cloud->GetPosition());
        set.hazards.emplace_back(cloud->GetPosition(), ULDUAR_YOGG_SARON_CLOUD_AVOID_RADIUS);

        // And where it will be by the time the bot gets there. Without the lead a destination clear of
        // the orbit now is under it on arrival.
        set.hazards.emplace_back(YoggSaronCloudLead(cloud), ULDUAR_YOGG_SARON_CLOUD_AVOID_RADIUS);
    }

    std::vector<Unit*> const novas =
        GetYoggSaronNovaThreats(botAI, SearchRadius() + ULDUAR_YOGG_SARON_SHADOW_NOVA_CLEAR_RADIUS);
    for (Unit* guardian : novas)
        set.fallback.emplace_back(guardian->GetPosition(), ULDUAR_YOGG_SARON_SHADOW_NOVA_CLEAR_RADIUS);

    // Shadow Nova is the one that kills, so it is what the retry keeps when the clouds cannot also be
    // cleared.
    set.hazards.insert(set.hazards.end(), set.fallback.begin(), set.fallback.end());

    // Who a destination preference is for. IsMelee being !IsRanged can't tell a melee healer apart. The tank is in:
    // its control walk aims at the same circle, so the preference agrees with it.
    bool const pileDodger = PlayerbotAI::IsMelee(bot) && !PlayerbotAI::IsHeal(bot);

    // With nothing about to detonate, a melee dodge steps out of swing range and reach melee hauls it back. Require the
    // candidate to stay in reach, only toward a target on the stack: one further out is the tank's to fetch.
    if (novas.empty() && PlayerbotAI::IsMelee(bot))
    {
        Unit* target = AI_VALUE(Unit*, "current target");
        if (target && YoggSaronGuardianOnTheStack(target))
        {
            Position const at = target->GetPosition();
            float const reach = sPlayerbotAIConfig.MeleeDistance;

            set.clear = [at, reach](float x, float y) { return at.GetExactDist2d(x, y) <= reach; };
            set.fallback = set.hazards;
        }

        // Somewhere no orbit reaches, not merely clear of the cloud it is leaving. Preferred rather than required,
        // so it can never hold a bot in a cloud.
        if (pileDodger)
        {
            set.preferred = [](float x, float y)
            {
                return ULDUAR_YOGG_SARON_MIDDLE.GetExactDist2d(x, y) <=
                       ULDUAR_YOGG_SARON_P1_CLOUD_FREE_RADIUS;
            };
        }
    }

    // Fervor holder only, and it has to be clear of the blast. Prefer the station band alone, the one radius only the
    // second orbit reaches: band plus back-line blob leaves the sweep 0 to 2 of ~200 points. Preferred, never required.
    if (!novas.empty() && pileDodger)
    {
        set.preferred = [](float x, float y)
        {
            float const fromMiddle = ULDUAR_YOGG_SARON_MIDDLE.GetExactDist2d(x, y);

            return std::abs(fromMiddle - ULDUAR_YOGG_SARON_P1_RANGED_STATION_RADIUS) <=
                   ULDUAR_YOGG_SARON_P1_RANGED_BAND_TOLERANCE;
        };
    }

    return !set.hazards.empty();
}

bool YoggSaronPhase1SpacingAction::RouteAcceptable(float x, float y) const
{
    return YoggSaronRouteClearOfClouds(bot, clouds, x, y);
}

bool YoggSaronPhase1StationAction::Execute(Event /*event*/)
{
    if (!YoggSaronWalkMakingProgress(botAI, "p1station", ULDUAR_YOGG_SARON_P1_RANGED_SPOT))
        return false;

    return MoveTo(bot->GetMapId(), ULDUAR_YOGG_SARON_P1_RANGED_SPOT.GetPositionX(),
                  ULDUAR_YOGG_SARON_P1_RANGED_SPOT.GetPositionY(),
                  ULDUAR_YOGG_SARON_P1_RANGED_SPOT.GetPositionZ(), false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED, true, false);
}

bool YoggSaronPhase2SpacingAction::Collect(HazardSet& set)
{
    inDeathRay = false;
    rays.clear();

    // The core never despawns the Guardians phase 1 leaves alive and JustDied still fires the nova, so the blast
    // outlives the phase. In the fallback too: the nova is what the retry keeps.
    for (Unit* guardian :
         GetYoggSaronNovaThreats(botAI, SearchRadius() + ULDUAR_YOGG_SARON_SHADOW_NOVA_CLEAR_RADIUS))
    {
        set.hazards.emplace_back(guardian->GetPosition(), ULDUAR_YOGG_SARON_SHADOW_NOVA_CLEAR_RADIUS);
        set.fallback.emplace_back(guardian->GetPosition(), ULDUAR_YOGG_SARON_SHADOW_NOVA_CLEAR_RADIUS);
    }

    std::list<Creature*> found;
    bot->GetCreatureListWithEntryInGrid(found, NPC_DEATH_RAY,
                                        SearchRadius() + ULDUAR_YOGG_SARON_DEATH_RAY_CLEAR_RADIUS);
    for (Creature* ray : found)
    {
        if (!ray->IsAlive())
            continue;

        rays.push_back(ray->GetPosition());
        set.hazards.emplace_back(ray->GetPosition(), ULDUAR_YOGG_SARON_DEATH_RAY_CLEAR_RADIUS);
        inDeathRay = inDeathRay || bot->GetExactDist2d(ray) < ULDUAR_YOGG_SARON_DEATH_RAY_CLEAR_RADIUS;
    }

    // Yogg's body is a hazard from the moment he emerges: 64022 knocks back 14 yd off him every second and nothing
    // can be swept for it. Free for melee: his CombatReach is 30, so melee range on him is about 34 yd.
    set.hazards.emplace_back(ULDUAR_YOGG_SARON_MIDDLE, ULDUAR_YOGG_SARON_BODY_KNOCKBACK_CLEAR_RADIUS);

    std::vector<Position> wedges = GetYoggSaronCrushWedges(botAI, SearchRadius() + ULDUAR_YOGG_SARON_CRUSH_RANGE);
    if (!wedges.empty())
    {
        set.clear = [wedges](float x, float y)
        { return !InYoggSaronCrushWedge(wedges, x, y, ULDUAR_YOGG_SARON_CRUSH_CLEAR_ARC); };

        // The retry keeps the wedge and gives up the rays: Crush kills far more per hit than Death Ray, and dropping
        // the wedge would put bots back in the Crush line.
        set.fallbackClear = set.clear;
    }

    // The retry needs something to sweep against, and the body is the one circle that is always there.
    set.fallback.emplace_back(ULDUAR_YOGG_SARON_MIDDLE, ULDUAR_YOGG_SARON_BODY_KNOCKBACK_CLEAR_RADIUS);

    // Kept by the retry: a Crusher's reach is where its swings kill. Melee only get a stunned Crusher whose stun is
    // about to lift, see GetYoggSaronCrusherReaches.
    for (Position const& reach :
         GetYoggSaronCrusherReaches(botAI, SearchRadius() + ULDUAR_YOGG_SARON_CRUSHER_REACH_CLEAR_RADIUS))
    {
        set.hazards.emplace_back(reach, ULDUAR_YOGG_SARON_CRUSHER_REACH_CLEAR_RADIUS);
        set.fallback.emplace_back(reach, ULDUAR_YOGG_SARON_CRUSHER_REACH_CLEAR_RADIUS);
    }

    // Resolved once here rather than inside the sweep, which asks its filter hundreds of times.
    Unit* target = AI_VALUE(Unit*, "current target");
    if (target && target->IsAlive())
    {
        float const reach =
            PlayerbotAI::IsMelee(bot) ? sPlayerbotAIConfig.MeleeDistance : sPlayerbotAIConfig.SpellDistance;
        float const targetX = target->GetPositionX();
        float const targetY = target->GetPositionY();

        set.preferred = [targetX, targetY, reach](float x, float y)
        { return std::hypot(targetX - x, targetY - y) <= reach; };
    }

    return true;
}

bool YoggSaronPhase2SpacingAction::RouteAcceptable(float x, float y) const
{
    // A spot past a ray means walking through its lane.
    return YoggSaronRouteClearOfBody(bot, x, y) && YoggSaronRouteClearOfDeathRays(bot, rays, x, y);
}

bool YoggSaronPhase2SpacingAction::HeldRouteAcceptable(float x, float y) const
{
    return YoggSaronRouteClearOfDeathRays(bot, rays, x, y);
}

bool YoggSaronIllusionFacingAction::Collect(HazardSet& set)
{
    if (!YoggSaronRoomMiddle(bot, roomMiddle))
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || !target->IsAlive())
        return false;

    std::vector<Position> const skulls = GetYoggSaronSkullsInRange(botAI);
    if (skulls.empty())
        return false;

    // FindNearestPositionClearOfHazards returns nothing for an empty list and the constraint here is angular, so the
    // skulls go in as circles only to give the sweep something to ring outward from.
    for (Position const& skull : skulls)
        set.hazards.emplace_back(skull, ULDUAR_YOGG_SARON_SKULL_CLEAR_RADIUS);

    float const targetX = target->GetPositionX();
    float const targetY = target->GetPositionY();
    float const reach =
        PlayerbotAI::IsMelee(bot) ? sPlayerbotAIConfig.MeleeDistance : sPlayerbotAIConfig.SpellDistance;

    // Staying in reach is required, not a preference: out of reach, reach melee walks the bot back and undoes this.
    // No fallback: every rejected candidate gets the bot gazed anyway, so fighting through it beats a useless walk.
    set.clear = [skulls, targetX, targetY, reach](float x, float y)
    {
        return std::hypot(targetX - x, targetY - y) <= reach &&
               YoggSaronFacingClearOfSkulls(skulls, x, y, targetX, targetY);
    };

    return true;
}

bool YoggSaronBodyDetourAction::Execute(Event /*event*/)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || !target->IsAlive())
        return false;

    Position waypoint;
    if (!YoggSaronBodyDetour(bot, target->GetPosition(), waypoint))
        return false;

    // A blocked arc is worse than a straight walk, because this node claims the tick and reach cannot
    // run while it does. The latch hands the bot back after ULDUAR_YOGG_SARON_WALK_GIVE_UP_MS of it.
    if (!YoggSaronWalkMakingProgress(botAI, "detour", waypoint))
        return false;

    return MoveNear(bot->GetMapId(), waypoint.GetPositionX(), waypoint.GetPositionY(), waypoint.GetPositionZ(),
                    sPlayerbotAIConfig.ContactDistance, MovementPriority::MOVEMENT_FORCED);
}

bool YoggSaronPetGuardAction::Execute(Event /*event*/)
{
    std::vector<Creature*> owned;
    for (Unit* controlled : bot->m_Controlled)
        if (Creature* creature = controlled->ToCreature())
            owned.push_back(creature);

    if (owned.empty())
        return false;

    // A stunned one can't swing, so a pet may stay on it until the stun is about to lift.
    auto const swinging = [this](Unit* unit)
    {
        return unit && unit->IsAlive() && unit->GetEntry() == NPC_CRUSHER_TENTACLE &&
               !YoggSaronCrusherStunHolds(botAI, unit);
    };

    Unit* target = AI_VALUE(Unit*, "current target");
    bool const ownerOnCrusher = swinging(target);
    if (target && (!target->IsAlive() || target->GetEntry() == NPC_CRUSHER_TENTACLE))
        target = nullptr;

    // Phase 3 owners rank every Guardian above the leftover tentacles, so only their pets ever get to
    // one. Proper pets only: a 30 s Treant or Spirit Wolf would spend half its life walking over there.
    Unit* const leftover = bot->GetPet() ? YoggSaronPhase3PetLeftover(botAI) : nullptr;

    for (Creature* pet : owned)
    {
        Unit* const petLeftover = pet->IsPet() ? leftover : nullptr;

        // Around the pet: a ranged owner can stand well outside any Crusher its pet is sitting on.
        std::list<Creature*> crushers;
        pet->GetCreatureListWithEntryInGrid(crushers, NPC_CRUSHER_TENTACLE, ULDUAR_YOGG_SARON_CRUSH_RANGE);

        bool touching = false;
        for (Creature* crusher : crushers)
        {
            // Either half is enough: the victim makes it swing, reach lets it. SetInCombatWithZone puts the raid
            // on its threat list, so a pet that never attacked can still come up as the victim.
            if (swinging(crusher) && (crusher->GetVictim() == pet || crusher->IsWithinMeleeRange(pet)))
            {
                touching = true;
                break;
            }
        }

        Unit* const victim = pet->GetVictim();
        bool const onCrusher = swinging(victim);

        if (!touching && !onCrusher && !ownerOnCrusher)
        {
            Unhush(pet);

            // PetAI never leaves a live victim, so it has to be told, and it stays on it from there.
            if (petLeftover && victim != petLeftover && pet->AI())
            {
                if (CharmInfo* charm = pet->GetCharmInfo())
                    charm->SetIsCommandAttack(true);

                pet->AI()->AttackStart(petLeftover);
            }

            continue;
        }

        // Passive, driven by command. PetAI retargets three ways (OwnerAttacked, KilledUnit, AttackedBy) and all skip
        // a passive pet. Pulling it off once it's in reach is too late.
        Hush(pet);

        // A pet already on another add only needs to stay on it, unless the pet itself is in the reach.
        Unit* elsewhere = nullptr;
        if (petLeftover)
            elsewhere = petLeftover;
        else if (!touching && victim && victim->IsAlive() && victim->GetEntry() != NPC_CRUSHER_TENTACLE)
            elsewhere = victim;
        else
            elsewhere = target ? target : YoggSaronPetFallbackTarget(botAI, pet);

        // Passive pets only attack while this is set (PetAI::CanAttack).
        if (CharmInfo* charm = pet->GetCharmInfo())
            charm->SetIsCommandAttack(elsewhere != nullptr);

        if (touching || onCrusher)
            pet->AttackStop();

        if (elsewhere && elsewhere != pet->GetVictim() && pet->AI())
            pet->AI()->AttackStart(elsewhere);
        else if (!elsewhere && (touching || onCrusher))
            pet->GetMotionMaster()->MoveFollow(bot, PET_FOLLOW_DIST, pet->GetFollowAngle());
    }

    // Never claims the tick. Commanding a pet costs the bot nothing it was going to do with its own
    // body, and the node sits above the dps resolver only so it is asked before the pet swings again.
    return false;
}

void YoggSaronPetGuardAction::Hush(Creature* pet)
{
    // First stance only, or a second hush would save passive as the one to give back.
    hushed.emplace(pet->GetGUID(), pet->GetReactState());
    pet->SetReactState(REACT_PASSIVE);
}

void YoggSaronPetGuardAction::Unhush(Creature* pet)
{
    // Only pets this node silenced, so a stance the player set themselves is never overwritten.
    auto const it = hushed.find(pet->GetGUID());
    if (it == hushed.end())
        return;

    pet->SetReactState(it->second);
    hushed.erase(it);
}

size_t YoggSaronSetDpsPriorityAction::TierOf(Unit* unit, bool brainLevel, bool phaseThreeRanged, bool beaconHealFirst)
{
    constexpr size_t none = std::numeric_limits<size_t>::max();

    if (!unit)
        return none;

    uint32 const entry = unit->GetEntry();

    if (brainLevel)
    {
        // Influence Tentacles gate everything else: while one lives the Brain zeroes damage and kills
        // whoever dealt it, so nothing in the room can be made to count until they are down.
        if (entry == NPC_INFLUENCE_TENTACLE)
            return 0;

        if (std::find(ULDUAR_YOGG_SARON_ILLUSION_MOBS.begin(), ULDUAR_YOGG_SARON_ILLUSION_MOBS.end(), entry) !=
            ULDUAR_YOGG_SARON_ILLUSION_MOBS.end())
            return 1;

        return entry == NPC_BRAIN ? 2 : none;
    }

    // One boss-room ladder for both phases, led by what phase 1 left alive: it keeps casting a 35 yd Dark Volley and
    // regenerates to full at the portal, so unordered it is picked off by nearest for no kill.
    switch (entry)
    {
        case NPC_GUARDIAN_OF_YS:
            return 0;
        // Immortal Guardians exist only in phase 3 and go before most phase 2 leftovers (Crusher excepted, below).
        // Shadow Beacon swaps the entry and pours 750,000 healing into it over 20 s, so focus it. Only with Thorim.
        case NPC_MARKED_IMMORTAL_GUARDIAN:
            return 1;
        case NPC_IMMORTAL_GUARDIAN:
            return 2;
        // Then the Crusher. Diminish Power is a 5 minute channel cutting raid damage 21%, undispellable and unkickable:
        // only a melee hit (~1.5 s) or its death stops it. Phase 3 ranged take a leftover one first.
        case NPC_CRUSHER_TENTACLE:
            return phaseThreeRanged ? 0 : 4;
        // One a mark's heal reaches goes first with the Crusher: it must die between two beacons or it is back to full.
        // Otherwise a Constrictor goes ahead of the Crusher while it holds somebody (99k health, ~2 s of ranged).
        case NPC_CONSTRICTOR_TENTACLE:
        case NPC_CORRUPTOR_TENTACLE:
            if (beaconHealFirst && YoggSaronBeaconHealReaches(unit))
                return 0;

            if (entry == NPC_CORRUPTOR_TENTACLE)
                return 6;

            return YoggSaronConstrictorHolding(unit) ? 3 : 5;
        case NPC_YOGG_SARON:
            return 7;
        default:
            return none;
    }
}

bool YoggSaronSetDpsPriorityAction::IsAllowedTarget(Unit* candidate, BrainApproach& brain) const
{
    if (!candidate || !candidate->IsAlive())
        return false;

    // Only this bot's share of an Icecrown or Stormwind room.
    if (brain.brainLevel && TierOf(candidate, true) <= 1 && !YoggSaronIllusionTentacleAllowed(botAI, candidate))
        return false;

    // Phase 3 splits the raid: ranged take the Guardians and the phase 2 leftovers, melee stay on Yogg and only hit a
    // Guardian near the tank during a gaze or once beacon-healed. Nobody hits one a tank is not holding.
    auto const phaseThreeMelee = [this, &brain]()
    {
        if (!brain.phaseThreeMelee)
            brain.phaseThreeMelee = !brain.brainLevel && PlayerbotAI::IsMelee(bot) && YoggSaronInPhase3(botAI);

        return *brain.phaseThreeMelee;
    };

    bool allowed = true;
    switch (candidate->GetEntry())
    {
        // Only ever reached after phase 1, YoggSaronPhase1Focus picks before that.
        case NPC_GUARDIAN_OF_YS:
            allowed = YoggSaronLeftoverGuardianKillable(botAI, candidate);
            break;
        case NPC_BRAIN:
            if (!brain.approachable)
                brain.approachable = brain.brainLevel && YoggSaronBrainRoomApproachable(botAI);

            allowed = *brain.approachable && !YoggSaronBrainHeld(botAI);
            break;
        // Crush skips its cone test inside 2 yd and re-aims at its victim, so a clear melee bot turns collinear
        // without moving. Healers stay eligible (range). Shattered Illusion stuns it, no swing until near the end.
        case NPC_CRUSHER_TENTACLE:
            allowed = !PlayerbotAI::IsMelee(bot) || YoggSaronCrusherStunHolds(botAI, candidate);
            break;
        // Phase 3 ranged only take one their station reaches, or a Constrictor holding somebody.
        // The pets go for the far ones.
        case NPC_CONSTRICTOR_TENTACLE:
        case NPC_CORRUPTOR_TENTACLE:
            allowed = !phaseThreeMelee() &&
                      (!brain.phaseThreeRanged || YoggSaronConstrictorHolding(candidate) ||
                       YoggSaronTrigger(botAI).PhaseThreeStationReaches(candidate));
            break;
        // Without Thorim a marked one is healed to full whatever the raid does, and it's being dragged
        // clear of the pile, so every hit on it is wasted.
        case NPC_MARKED_IMMORTAL_GUARDIAN:
            if (!YoggThorimKeeperActive(botAI))
            {
                allowed = false;
                break;
            }
            [[fallthrough]];
        // Below 10% it is Weakened, and nothing but Thorim's Titanic Storm can finish one, so holding
        // there is a dead tick for the rest of the fight. Above it, only once a tank leads on threat.
        case NPC_IMMORTAL_GUARDIAN:
        {
            float const health = candidate->GetHealthPct();
            if (!brain.thorim)
                brain.thorim = YoggThorimKeeperActive(botAI);

            auto const gazeTarget = [&]()
            {
                if (!brain.gaze)
                    brain.gaze = YoggSaronGazeActive(botAI);

                return *brain.gaze && health > ULDUAR_YOGG_SARON_GUARDIAN_WEAKENED_PCT &&
                       YoggSaronOutsideGaze(bot, YoggSaronNearestCreature(botAI, NPC_YOGG_SARON), candidate);
            };

            if (*brain.thorim)
                allowed = health > ULDUAR_YOGG_SARON_GUARDIAN_WEAKENED_PCT;
            // Melee only during a gaze or on a beacon-healed one a tank holds. Taking every tanked one over the floor
            // keeps them off Yogg for good, since the pile always has one.
            else if (phaseThreeMelee())
            {
                Unit* victim = candidate->GetVictim();
                bool const healed = candidate->HasAura(SPELL_EMPOWERING_SHADOWS_25) ||
                                    candidate->HasAura(SPELL_EMPOWERING_SHADOWS_10);
                allowed = (healed && health > ULDUAR_YOGG_SARON_PHASE_3_GUARDIAN_FLOOR_PCT && victim &&
                           victim->IsPlayer() && PlayerbotAI::IsTank(victim->ToPlayer())) ||
                          gazeTarget();
            }
            // Down to the floor, the rest goes on Yogg. Under it only while the gaze has the bot off Yogg
            // anyway, and only one it can hit with him out of its front arc. Beacon-healed too: the heal kills tanks.
            else
                allowed = health > ULDUAR_YOGG_SARON_PHASE_3_GUARDIAN_FLOOR_PCT || gazeTarget();

            allowed = allowed &&
                      (!phaseThreeMelee() ||
                       candidate->GetExactDist2d(ULDUAR_YOGG_SARON_PHASE_3_MELEE_SPOT) <=
                           ULDUAR_YOGG_SARON_PHASE_3_MELEE_GUARDIAN_RANGE) &&
                      YoggSaronGuardianThreatAllows(bot, candidate, candidate->GetGUID() == bot->GetTarget());
            break;
        }
        // Shadow Barrier is what phase 2 is read off, and it makes him immune.
        case NPC_YOGG_SARON:
            allowed = !candidate->HasAura(SPELL_SHADOW_BARRIER);
            break;
        default:
            break;
    }

    // Line of sight last, it's the only raycast here.
    return allowed && bot->IsWithinLOSInMap(candidate);
}

Unit* YoggSaronSetDpsPriorityAction::ResolveTarget(Unit* currentTarget)
{
    constexpr size_t none = std::numeric_limits<size_t>::max();
    constexpr float targetSwitchDistance = 10.0f;

    bool const brainLevel = YoggSaronOnBrainLevel(bot);
    bool const phaseThreeRanged =
        !brainLevel && PlayerbotAI::IsRanged(bot) && !PlayerbotAI::IsHeal(bot) && YoggSaronInPhase3(botAI);
    // Marks are only dragged out to the spots without Thorim.
    bool const beaconHealFirst = phaseThreeRanged && !YoggThorimKeeperActive(botAI);

    std::vector<uint32> entries;
    size_t tierCount = 0;
    if (brainLevel)
    {
        entries = ULDUAR_YOGG_SARON_ILLUSION_MOBS;
        entries.push_back(NPC_BRAIN);
        tierCount = 3;
    }
    else
    {
        entries = {NPC_GUARDIAN_OF_YS,     NPC_CRUSHER_TENTACLE,  NPC_CONSTRICTOR_TENTACLE,
                   NPC_CORRUPTOR_TENTACLE, NPC_IMMORTAL_GUARDIAN, NPC_MARKED_IMMORTAL_GUARDIAN,
                   NPC_YOGG_SARON};
        tierCount = 8;
    }

    // The brain level needs the reach: a portal drops the bot 60-72 yd out, past sight distance. Attack() caps at
    // line of sight only. The boss room is ~50 yd across and keeps the cheaper sweep.
    float const range = brainLevel ? ULDUAR_YOGG_SARON_SEARCH_RANGE : sPlayerbotAIConfig.SightDistance;

    // The door as well as the tentacles: the Brain opens it in the branch that fires when the last Influence Tentacle
    // dies, so reading tentacles alone can hit the Brain through a wall not yet opened.
    BrainApproach brain;
    brain.brainLevel = brainLevel;
    brain.phaseThreeRanged = phaseThreeRanged;

    // "nearest npcs" cut down to these entries before its LOS test instead of after. Same units in the
    // same order, without a raycast for every pet and totem in the raid.
    std::vector<Unit*> candidates;
    AnyUnitOfEntriesInRangeCheck check{Acore::AnyUnitInObjectRangeCheck(bot, range), entries.data(), entries.size()};
    Acore::UnitListSearcher<AnyUnitOfEntriesInRangeCheck> searcher(bot, candidates, check);
    Cell::VisitObjects(bot, searcher, range);

    // An illusion room spreads its team over the tentacles instead of ranking them. Execute keeps the
    // healer out of here.
    Position roomMiddle;
    if (brainLevel && YoggSaronRoomMiddle(bot, roomMiddle))
        if (Unit* spread = SpreadTarget(candidates, currentTarget, brain))
            return spread;

    std::vector<size_t> tiers;
    tiers.reserve(candidates.size());
    for (Unit* unit : candidates)
        tiers.push_back(TierOf(unit, brainLevel, phaseThreeRanged, beaconHealFirst));

    // Best tier first, stop at the first one with anything allowed: the rest would be line of sight rays for nothing.
    // Ties go to whichever was found first.
    Unit* target = nullptr;
    size_t desiredTier = none;
    for (size_t tier = 0; tier < tierCount && !target; ++tier)
    {
        for (size_t i = 0; i < candidates.size(); ++i)
        {
            Unit* unit = candidates[i];
            if (tiers[i] != tier || !IsAllowedTarget(unit, brain))
                continue;

            if (!target)
            {
                target = unit;
                continue;
            }

            // Guardians go down lowest first so the raid's damage finishes one instead of spreading
            // over three; everything else is nearest, which is the shortest walk into range.
            bool better;
            if (IsYoggSaronFocusedGuardian(unit))
                better = unit->GetHealth() < target->GetHealth();
            else
                better = unit->GetExactDist2d(bot) < target->GetExactDist2d(bot);

            if (better)
                target = unit;
        }

        if (target)
            desiredTier = tier;
    }

    size_t currentTier = none;
    if (currentTarget && IsAllowedTarget(currentTarget, brain))
        currentTier = TierOf(currentTarget, brainLevel, phaseThreeRanged, beaconHealFirst);

    if (currentTier != none && currentTier <= desiredTier)
    {
        // Never downgrade off something as urgent, and inside a tier switch only for something meaningfully closer,
        // or two tentacles ping-pong the raid. Guardians hold outright: a flipping health order resets every timer.
        bool const comeBack = currentTier == desiredTier && target &&
                              target->GetEntry() == NPC_GUARDIAN_OF_YS &&
                              currentTarget->GetEntry() == NPC_GUARDIAN_OF_YS &&
                              YoggSaronLowGuardianTakesOver(target, currentTarget);
        if (!comeBack &&
            (currentTier < desiredTier || !target || IsYoggSaronFocusedGuardian(currentTarget) ||
             target->GetExactDist2d(bot) + targetSwitchDistance >= currentTarget->GetExactDist2d(bot)))
            target = currentTarget;
    }

    if (target)
        return target;

    // The illusion rooms hold adds the module does not enumerate, so a hard stop would park a bot. The fallback knows
    // nothing about Crush or Guardian threat though: it would put melee on a lone live Crusher.
    Unit* fallback = AI_VALUE(Unit*, "dps target");
    if (fallback && brainLevel && TierOf(fallback, true) <= 1 && !YoggSaronIllusionTentacleAllowed(botAI, fallback))
        return nullptr;

    if (fallback &&
        (fallback->GetEntry() == NPC_CRUSHER_TENTACLE || fallback->GetEntry() == NPC_BRAIN ||
         IsYoggSaronFocusedGuardian(fallback)) &&
        !IsAllowedTarget(fallback, brain))
        return nullptr;

    return fallback;
}

Unit* YoggSaronSetDpsPriorityAction::SpreadTarget(std::vector<Unit*> const& candidates, Unit* currentTarget,
                                                  BrainApproach& brain)
{
    // Who else in this room holds what, humans included: a tentacle a player is on is taken.
    YoggSaronRoom const room = YoggSaronRoomOf(bot);
    std::vector<std::pair<ObjectGuid, ObjectGuid>> held;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member == bot || !member->IsAlive() || PlayerbotAI::IsHeal(member) ||
                YoggSaronRoomOf(member) != room || !member->GetTarget())
                continue;

            held.emplace_back(member->GetGUID(), member->GetTarget());
        }
    }

    auto const heldBy = [&held](Unit* unit)
    {
        auto const same = [unit](auto const& entry) { return entry.second == unit->GetGUID(); };
        return static_cast<size_t>(std::count_if(held.begin(), held.end(), same));
    };

    // Least held first, then nearest, so line of sight is only cast until one passes.
    struct Ranked
    {
        size_t held;
        float distance;
        Unit* unit;
    };

    std::vector<Ranked> ranked;
    for (Unit* unit : candidates)
        if (TierOf(unit, true) <= 1)
            ranked.push_back({heldBy(unit), unit->GetExactDist2d(bot), unit});

    std::sort(ranked.begin(), ranked.end(), [](Ranked const& left, Ranked const& right)
              { return left.held != right.held ? left.held < right.held : left.distance < right.distance; });

    Unit* best = nullptr;
    size_t bestHeld = 0;
    for (Ranked const& entry : ranked)
    {
        if (IsAllowedTarget(entry.unit, brain))
        {
            best = entry.unit;
            bestHeld = entry.held;
            break;
        }
    }

    if (!best)
        return nullptr;

    // Keep a tentacle nobody less held beats, and always keep one no lower GUID is also on, so two bots
    // that pick the same free tentacle in one tick sort themselves out in the next.
    if (currentTarget && TierOf(currentTarget, true) <= 1 && IsAllowedTarget(currentTarget, brain))
    {
        auto const lowerHolder = [this, currentTarget](auto const& entry)
        { return entry.second == currentTarget->GetGUID() && entry.first < bot->GetGUID(); };
        bool const senior = std::none_of(held.begin(), held.end(), lowerHolder);

        if (currentTarget == best || heldBy(currentTarget) <= bestHeld || senior)
            best = currentTarget;
    }

    return best;
}

void YoggSaronSetDpsPriorityAction::DropTarget(Unit* target, bool interruptCasts)
{
    bot->AttackStop();
    if (interruptCasts)
        bot->InterruptNonMeleeSpells(true);
    bot->SetTarget(ObjectGuid::Empty);
    bot->SetSelection(ObjectGuid());
    context->GetValue<Unit*>("current target")->Set(nullptr);

    for (Unit* minion : bot->m_Controlled)
        if (minion && minion->GetVictim() == target)
            minion->AttackStop();
}

bool YoggSaronSetDpsPriorityAction::Execute(Event /*event*/)
{
    Unit* currentTarget = AI_VALUE(Unit*, "current target");

    bool const phaseOne = !YoggSaronOnBrainLevel(bot) && YoggSaronInPhase1(botAI);

    // Phase 1 is one shared target and no "dps target" fallback. A parked Guardian stays parked even when nothing
    // else is killable.
    if (phaseOne && currentTarget && currentTarget->GetEntry() == NPC_GUARDIAN_OF_YS &&
        !YoggSaronPhase1GuardianKillable(botAI, currentTarget))
    {
        DropTarget(currentTarget);
        currentTarget = nullptr;
    }

    // Same for the ones left when the barrier lands. A null resolve keeps the old target, so without the
    // drop the bot would keep swinging at one waiting out its gap.
    bool const leftovers = !phaseOne && !YoggSaronOnBrainLevel(bot);
    if (leftovers && currentTarget && currentTarget->GetEntry() == NPC_GUARDIAN_OF_YS && currentTarget->IsAlive() &&
        !YoggSaronLeftoverGuardianKillable(botAI, currentTarget))
    {
        DropTarget(currentTarget);
        currentTarget = nullptr;
    }

    // The AoE hold only stops new casts: a Blizzard already channeling ticks for up to 8 s. Channel check first,
    // so the sweep only runs for a bot that has one going.
    if (phaseOne || leftovers)
    {
        Spell* channel = bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
        if (channel && channel->GetSpellInfo()->IsAffectingArea() &&
            (phaseOne ? YoggSaronPhase1AoeHold(botAI) : YoggSaronLeftoverAoeHold(botAI)))
            bot->InterruptSpell(CURRENT_CHANNELED_SPELL);
    }

    // Off the tentacles until healed, so no target: the drop is what ends the white swings, and every
    // swing is a hit Grim Reprisal answers. Only a cast at the tentacle is cut, so a self-heal still lands.
    if (YoggSaronReprisalHold(botAI))
    {
        if (currentTarget)
            DropTarget(currentTarget, YoggSaronCastingAt(bot, currentTarget));

        return false;
    }

    // Off a tentacle outside its share, since a null resolve keeps the old target.
    if (currentTarget && currentTarget->IsAlive() && YoggSaronOnBrainLevel(bot) && TierOf(currentTarget, true) <= 1 &&
        !YoggSaronIllusionTentacleAllowed(botAI, currentTarget))
    {
        DropTarget(currentTarget, YoggSaronCastingAt(bot, currentTarget));
        currentTarget = nullptr;
    }

    // Healers take no target in an illusion room or phase 3: a target is what reach spell walks them toward, away from
    // the middle or the tank. With none, nothing turns them back toward Yogg either.
    Position roomMiddle;
    if (PlayerbotAI::IsHeal(bot) && (YoggSaronRoomMiddle(bot, roomMiddle) ||
                                     (!YoggSaronOnBrainLevel(bot) && YoggSaronInPhase3(botAI))))
    {
        if (currentTarget)
            DropTarget(currentTarget, false);

        return false;
    }

    // Held off the Brain: the resolver no longer picks it, but auto attack, a channel and the pets would
    // all carry on.
    if (currentTarget && currentTarget->GetEntry() == NPC_BRAIN && YoggSaronBrainHeld(botAI))
    {
        DropTarget(currentTarget, true);
        currentTarget = nullptr;
    }

    Unit* target = phaseOne ? YoggSaronPhase1Focus(botAI) : ResolveTarget(currentTarget);

    // PetAI never leaves a live victim, so a pet would stay on a Guardian the bot just left at the floor.
    if (target && currentTarget && target != currentTarget && currentTarget->GetEntry() == NPC_IMMORTAL_GUARDIAN)
        for (Unit* minion : bot->m_Controlled)
            if (minion && minion->GetVictim() == currentTarget)
                minion->AttackStop();

    if (!target)
    {
        // Melee still on a Crusher whose stun is about to lift. Holding it would have reach melee walk
        // the bot back into the reach the spacing node is walking it out of.
        if (currentTarget && currentTarget->GetEntry() == NPC_CRUSHER_TENTACLE && PlayerbotAI::IsMelee(bot) &&
            !YoggSaronCrusherStunHolds(botAI, currentTarget))
            DropTarget(currentTarget, false);

        return false;
    }

    bool needsAttack = currentTarget != target;
    if (PlayerbotAI::IsMelee(bot))
        needsAttack = needsAttack || !bot->HasUnitState(UNIT_STATE_MELEE_ATTACKING);

    // Returning false once the bot is on the right target is what lets the lower-priority nodes run:
    // the engine ends the tick at the first action that succeeds.
    return needsAttack ? Attack(target) : false;
}

int32 YoggSaronDarkVolleyInterruptAction::GetInterrupterIndex()
{
    Group* group = bot->GetGroup();
    if (!group)
        return 0;

    int32 index = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !YoggSaronCanInterrupt(member))
            continue;

        if (member == bot)
            return index;

        ++index;
    }

    return 0;
}

bool YoggSaronDarkVolleyInterruptAction::CastClassInterrupt(Unit* target)
{
    if (YoggSaronInterruptClaimedByOther(bot, target))
        return false;

    for (char const* spell : YoggSaronInterruptSpellsFor(bot, target))
    {
        if (YoggSaronInterruptReady(botAI, spell, target) && botAI->CastSpell(spell, target))
        {
            ClaimYoggSaronInterrupt(bot, target);
            return true;
        }
    }

    return false;
}

bool YoggSaronDarkVolleyInterruptAction::Execute(Event /*event*/)
{
    std::vector<Unit*> casters = GetYoggSaronInterruptCasters(botAI);
    if (casters.empty())
        return false;

    // Beacon marks first, they lead the list. The claim keeps a second bot off the same one.
    size_t const marks = static_cast<size_t>(
        std::find_if_not(casters.begin(), casters.end(), YoggSaronMarkCaster) - casters.begin());
    for (size_t i = 0; i < marks; ++i)
    {
        if (!CastClassInterrupt(casters[i]))
            continue;

        return true;
    }

    // Each interrupter starts at its own slot, or the whole raid spends its kicks on one volley. Not for Drain Life:
    // it comes sorted by time left, and a kicked channel drops off the list.
    size_t const rest = casters.size() - marks;
    if (!rest)
        return false;

    size_t const start = YoggSaronInPhase3(botAI) ? 0 : static_cast<size_t>(GetInterrupterIndex()) % rest;
    for (size_t i = 0; i < rest; ++i)
        if (CastClassInterrupt(casters[marks + (start + i) % rest]))
            return true;

    return false;
}

bool YoggSaronDiminishPowerJudgementAction::Execute(Event /*event*/)
{
    std::vector<char const*> const spells = YoggSaronJudgementSpells(bot);

    // No walk and no target swap, only what is already in reach. No stagger either: the Judgement lands
    // inside Spell::cast, so the next paladin's read finds the channel already gone.
    for (Unit* crusher : GetYoggSaronChannellingCrushers(botAI))
    {
        for (char const* spell : spells)
        {
            if (!botAI->CanCastSpell(spell, crusher) || !botAI->CastSpell(spell, crusher))
                continue;

            return true;
        }
    }

    return false;
}

bool YoggSaronMortalStrikeAction::Execute(Event /*event*/)
{
    // No walk and no target swap, only what is already in reach.
    Unit* guardian = YoggSaronMortalStrikeTarget(botAI);
    return guardian && botAI->CanCastSpell("mortal strike", guardian) && botAI->CastSpell("mortal strike", guardian);
}

bool YoggSaronWarlockPetAction::Execute(Event /*event*/)
{
    char const* summon = YoggSaronWarlockSummon(botAI);
    if (!summon)
        return false;

    // Fel Domination first when the warlock has it: 5.5 s off a 10 s summon.
    char const* spell = botAI->CanCastSpell("fel domination", bot) ? "fel domination" : summon;
    return botAI->CastSpell(spell, bot);
}

bool YoggSaronPhase3ControlAction::Execute(Event /*event*/)
{
    bool acted = false;

    TankFaceStrategy tankFaceStrategy(botAI);
    if (botAI->HasStrategy(tankFaceStrategy.getName(), BotState::BOT_STATE_COMBAT))
    {
        botAI->ChangeStrategy(REMOVE_STRATEGY_CHAR + tankFaceStrategy.getName(), BotState::BOT_STATE_COMBAT);
        acted = true;
    }

    TankAssistStrategy tankAssistStrategy(botAI);
    if (!botAI->HasStrategy(tankAssistStrategy.getName(), BotState::BOT_STATE_COMBAT))
    {
        botAI->ChangeStrategy(ADD_STRATEGY_CHAR + tankAssistStrategy.getName(), BotState::BOT_STATE_COMBAT);
        acted = true;
    }

    return acted;
}

bool YoggSaronBrainLinkAction::Execute(Event /*event*/)
{
    Player* partner = YoggSaronBrainLinkTarget(botAI);
    if (!partner)
        return false;

    // Both ends run this, so the gap closes at twice the rate. Aiming at the partner is a chase after somebody
    // moving away at the same speed, which never arrives.
    float x = (bot->GetPositionX() + partner->GetPositionX()) / 2.0f;
    float y = (bot->GetPositionY() + partner->GetPositionY()) / 2.0f;

    // Two bots on opposite sides of Yogg have the body itself as their midpoint, which knocks both of
    // them back once a second. Pushed straight out along the same bearing, so the two still meet.
    float const middleX = ULDUAR_YOGG_SARON_MIDDLE.GetPositionX();
    float const middleY = ULDUAR_YOGG_SARON_MIDDLE.GetPositionY();
    float const fromMiddle = std::hypot(x - middleX, y - middleY);
    if (fromMiddle < ULDUAR_YOGG_SARON_BODY_KNOCKBACK_CLEAR_RADIUS)
    {
        float const heading = fromMiddle > 0.1f ? std::atan2(y - middleY, x - middleX) : bot->GetOrientation();
        x = middleX + std::cos(heading) * ULDUAR_YOGG_SARON_BODY_KNOCKBACK_CLEAR_RADIUS;
        y = middleY + std::sin(heading) * ULDUAR_YOGG_SARON_BODY_KNOCKBACK_CLEAR_RADIUS;
    }

    // Not into a live Crusher's Crush either: a link left over from phase 2 can end there as phase 3 starts.
    // Waiting out there costs a shared hit a second.
    constexpr float crusherSearchRadius = 80.0f;
    std::list<Creature*> crushers;
    bot->GetCreatureListWithEntryInGrid(crushers, NPC_CRUSHER_TENTACLE, crusherSearchRadius);
    for (Creature* crusher : crushers)
    {
        if (!crusher->IsAlive() || YoggSaronCrusherStunHolds(botAI, crusher))
            continue;

        float const gap = std::hypot(x - crusher->GetPositionX(), y - crusher->GetPositionY());
        if (gap >= ULDUAR_YOGG_SARON_CRUSH_RANGE)
            continue;

        float const heading = gap > 0.1f ? std::atan2(y - crusher->GetPositionY(), x - crusher->GetPositionX())
                                         : crusher->GetAngle(bot);
        x = crusher->GetPositionX() + std::cos(heading) * ULDUAR_YOGG_SARON_CRUSH_RANGE;
        y = crusher->GetPositionY() + std::sin(heading) * ULDUAR_YOGG_SARON_CRUSH_RANGE;
    }

    return MoveNear(bot->GetMapId(), x, y, bot->GetPositionZ(), sPlayerbotAIConfig.ContactDistance,
                    MovementPriority::MOVEMENT_FORCED);
}

bool YoggSaronMoveToEnterPortalAction::Execute(Event /*event*/)
{
    Position spot;
    YoggSaronPortalIntent const intent = YoggSaronPortalPlan(botAI, spot);
    if (intent != YOGG_SARON_PORTAL_SPREADING && intent != YOGG_SARON_PORTAL_LATE)
        return false;

    botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("diamond");

    // Progress is measured against the spot, not the waypoint: the detour's bisector moves as the bot
    // walks, and a latch keyed on a moving destination is a new latch every tick.
    if (!YoggSaronWalkMakingProgress(botAI, "enter", spot))
        return false;

    // Straight across the room is straight through the body, which throws the bot back once a second. One waypoint
    // turns the crossing into an arc, and each leg halves the turn the next one has to make.
    Position const destination = YoggSaronBodyRoute(bot, spot);

    return MoveNear(bot->GetMapId(), destination.GetPositionX(), destination.GetPositionY(),
                    destination.GetPositionZ(), sPlayerbotAIConfig.ContactDistance,
                    MovementPriority::MOVEMENT_FORCED);
}

bool YoggSaronFallFromFloorAction::Execute(Event /*event*/)
{
    std::string rtiMark = AI_VALUE(std::string, "rti");
    if (rtiMark == "skull")
    {
        bot->NearTeleportTo(ULDUAR_YOGG_SARON_BOSS_ROOM_RESTORE_POINT.GetPositionX(),
                            ULDUAR_YOGG_SARON_BOSS_ROOM_RESTORE_POINT.GetPositionY(),
                            ULDUAR_YOGG_SARON_BOSS_ROOM_RESTORE_POINT.GetPositionZ(),
                            ULDUAR_YOGG_SARON_BOSS_ROOM_RESTORE_POINT.GetOrientation());
        return true;
    }

    if (rtiMark == "cross")
    {
        bot->NearTeleportTo(ULDUAR_YOGG_SARON_STORMWIND_KEEPER_MIDDLE.GetPositionX(),
                            ULDUAR_YOGG_SARON_STORMWIND_KEEPER_MIDDLE.GetPositionY(),
                            ULDUAR_YOGG_SARON_STORMWIND_KEEPER_MIDDLE.GetPositionZ(), bot->GetOrientation());
        return true;
    }

    if (rtiMark == "circle")
    {
        bot->NearTeleportTo(ULDUAR_YOGG_SARON_ICECROWN_CITADEL_MIDDLE.GetPositionX(),
                            ULDUAR_YOGG_SARON_ICECROWN_CITADEL_MIDDLE.GetPositionY(),
                            ULDUAR_YOGG_SARON_ICECROWN_CITADEL_MIDDLE.GetPositionZ(), bot->GetOrientation());
        return true;
    }

    if (rtiMark == "star")
    {
        bot->NearTeleportTo(ULDUAR_YOGG_SARON_CHAMBER_OF_ASPECTS_MIDDLE.GetPositionX(),
                            ULDUAR_YOGG_SARON_CHAMBER_OF_ASPECTS_MIDDLE.GetPositionY(),
                            ULDUAR_YOGG_SARON_CHAMBER_OF_ASPECTS_MIDDLE.GetPositionZ(), bot->GetOrientation());
        return true;
    }

    return false;
}

bool YoggSaronStopFollowingAction::Execute(Event /*event*/)
{
    FollowMasterStrategy followMasterStrategy(botAI);
    if (!botAI->HasStrategy(followMasterStrategy.getName(), BotState::BOT_STATE_NON_COMBAT))
        return false;

    botAI->ChangeStrategy(REMOVE_STRATEGY_CHAR + followMasterStrategy.getName(), BotState::BOT_STATE_NON_COMBAT);
    return true;
}

bool YoggSaronUsePortalAction::Execute(Event /*event*/)
{
    Creature* assignedPortal =
        bot->FindNearestCreature(NPC_DESCEND_INTO_MADNESS, ULDUAR_YOGG_SARON_PORTAL_CLICK_RADIUS, true);
    if (!assignedPortal)
        return false;

    return assignedPortal->HandleSpellClick(bot);
}

bool YoggSaronIllusionRoomAction::Execute(Event /*event*/)
{
    YoggSaronTrigger yoggSaronTrigger(botAI);

    bool resultSetRtiMark = SetRtiMark(yoggSaronTrigger);
    bool resultGoToBrainRoom = GoToBrainRoom(yoggSaronTrigger);

    return resultSetRtiMark || resultGoToBrainRoom || WalkIntoRoom() || WalkToStage();
}

bool YoggSaronIllusionRoomAction::WalkIntoRoom()
{
    if (YoggSaronRoomStateOf(botAI) != YOGG_SARON_ROOM_STATE_WALKING_IN || PlayerbotAI::IsHeal(bot))
        return false;

    // Walk at the nearest tentacle it may take: the middle is the summon group's centroid, so they ring it and there
    // is never one to fight there. The nearest one is the shortest way into line of sight, all the dps resolver wants.
    Position middle;
    if (!YoggSaronRoomMiddle(bot, middle))
        return false;

    std::vector<Unit*> const mobs = GetYoggSaronRoomIllusionMobs(botAI);
    auto const share = std::find_if(mobs.begin(), mobs.end(),
                                    [this](Unit* mob) { return YoggSaronIllusionTentacleAllowed(botAI, mob); });
    Position const destination = share == mobs.end() ? middle : (*share)->GetPosition();

    if (!YoggSaronWalkMakingProgress(botAI, "illusion", destination))
        return false;

    YoggSaronReleaseStalledWalk(botAI, "illusion", destination);

    return MoveTo(bot->GetMapId(), destination.GetPositionX(), destination.GetPositionY(), destination.GetPositionZ(),
                  false, false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool YoggSaronIllusionRoomAction::WalkToStage()
{
    Position spot;
    if (YoggSaronRoomStateOf(botAI) != YOGG_SARON_ROOM_STATE_STAGED || !YoggSaronIllusionStageSpot(bot, spot) ||
        bot->GetExactDist2d(spot.GetPositionX(), spot.GetPositionY()) <= ULDUAR_YOGG_SARON_STAGE_ARRIVED_RADIUS)
        return false;

    YoggSaronTakeOverForcedWalk(AI_VALUE(LastMovement&, "last movement"), spot);
    YoggSaronReleaseStalledWalk(botAI, "stage", spot);

    return MoveTo(bot->GetMapId(), spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ(), false, false, false,
                  true, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool YoggSaronIllusionRoomAction::SetRtiMark(YoggSaronTrigger yoggSaronTrigger)
{
    if (AI_VALUE(std::string, "rti") == "diamond")
    {
        if (yoggSaronTrigger.IsInStormwindKeeperIllusion())
        {
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("cross");
            return true;
        }
        else if (yoggSaronTrigger.IsInIcecrownKeeperIllusion())
        {
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("circle");
            return true;
        }
        else if (yoggSaronTrigger.IsInChamberOfTheAspectsIllusion())
        {
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("star");
            return true;
        }
    }
    return false;
}

bool YoggSaronIllusionRoomAction::GoToBrainRoom(YoggSaronTrigger yoggSaronTrigger)
{
    if (AI_VALUE(std::string, "rti") == "square" || !yoggSaronTrigger.IsBrainRoomApproachable())
        return false;

    // Square is what the brain spot node walks on. A single walk from here dies on the healer's first
    // heal, and reach spell then stops it at the Stormwind doorway, 55 yd from the Brain.
    botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("square");

    return true;
}

bool YoggSaronIllusionHealerStationAction::Execute(Event /*event*/)
{
    Position station;
    if (!YoggSaronIllusionHealerStation(botAI, station))
        return false;

    YoggSaronReleaseStalledWalk(botAI, "healer", station);

    return MoveTo(bot->GetMapId(), station.GetPositionX(), station.GetPositionY(), station.GetPositionZ(), false, false,
                  false, true, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool YoggSaronReprisalRegroupAction::Execute(Event /*event*/)
{
    Player* healer = YoggSaronRoomHealer(botAI);
    if (!healer)
        return false;

    // Short of the healer on this bot's side, since in range is all a heal needs.
    float const angle = healer->GetAngle(bot);
    Position const spot(healer->GetPositionX() + std::cos(angle) * ULDUAR_YOGG_SARON_REPRISAL_REGROUP_STOP,
                        healer->GetPositionY() + std::sin(angle) * ULDUAR_YOGG_SARON_REPRISAL_REGROUP_STOP,
                        healer->GetPositionZ());

    YoggSaronReleaseStalledWalk(botAI, "regroup", spot);

    return MoveTo(bot->GetMapId(), spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ(), false, false, false,
                  true, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool YoggSaronBrainSpotAction::Execute(Event /*event*/)
{
    Position const spot = YoggSaronBrainSpot(bot);

    // A bot that landed just before the door opened is still booked on the walk to a tentacle across the room.
    YoggSaronTakeOverForcedWalk(AI_VALUE(LastMovement&, "last movement"), spot);
    YoggSaronReleaseStalledWalk(botAI, "brainspot", spot);

    return MoveTo(bot->GetMapId(), spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ(), false, false, false,
                  true, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool YoggSaronNitroBoostsAction::Execute(Event /*event*/)
{
    if (!UseNitroBoosts(bot))
        return false;

    // The walk to the Brain still wants this tick.
    return false;
}

bool YoggSaronBrainDashAction::Execute(Event /*event*/)
{
    if (!botAI->CastSpell("dash", bot))
        return false;

    // The walk to the Brain still wants this tick.
    return false;
}

bool YoggSaronBrainChargeAction::Execute(Event /*event*/)
{
    Unit* brain = AI_VALUE(Unit*, "current target");
    if (!brain || brain->GetEntry() != NPC_BRAIN)
        return false;

    for (char const* spell : {"charge", "intercept"})
    {
        if (!botAI->CanCastSpell(spell, brain) || !botAI->CastSpell(spell, brain))
            continue;

        // The brain spot walk's booking would refuse the same point for 5 s after landing.
        AI_VALUE(LastMovement&, "last movement").clear();

        return true;
    }

    return false;
}

bool YoggSaronMoveToExitPortalAction::Execute(Event /*event*/)
{
    // Three of these are permanently spawned around the brain level, and a window can end with the bot
    // ~120 yd from the nearest one.
    std::list<GameObject*> found;
    bot->GetGameObjectListWithEntryInGrid(found, GO_FLEE_TO_THE_SURFACE_PORTAL, ULDUAR_YOGG_SARON_SEARCH_RANGE);

    std::vector<GameObject*> portals(found.begin(), found.end());
    std::sort(portals.begin(), portals.end(), [this](GameObject* left, GameObject* right)
              { return bot->GetDistance2d(left) < bot->GetDistance2d(right); });

    for (GameObject* portal : portals)
    {
        // Standing down is fatal (Induce Madness lands, and a mind control is a death), so a route going nowhere moves
        // on to the next portal. They sit far apart, so a blocked route to one says nothing about the others.
        if (!YoggSaronWalkMakingProgress(botAI, "exit", portal->GetPosition()))
            continue;

        // The brain spot walk is forced too, and would keep the exit refused until the bot is charmed.
        YoggSaronTakeOverForcedWalk(AI_VALUE(LastMovement&, "last movement"), portal->GetPosition());

        // A spline that died reads as a duplicate of itself until the hold runs out.
        YoggSaronReleaseStalledWalk(botAI, "exit", portal->GetPosition());

        MoveTo(bot->GetMapId(), portal->GetPositionX(), portal->GetPositionY(), portal->GetPositionZ(), false, false,
               false, true, MovementPriority::MOVEMENT_FORCED, true, false);

        // Claim the tick while still walking: everything queued under this node is a walk elsewhere and only has to win
        // one tick to turn the bot round.
        if (bot->GetDistance2d(portal) > INTERACTION_DISTANCE)
            return true;

        portal->Use(bot);

        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("skull");
        return true;
    }

    return false;
}

bool YoggSaronLaughingSkullAction::Execute(Event /*event*/)
{
    // SetFacingTo lays a spline on the spot, which ends any walk under way, like the healer, regroup and walk-in
    // walks below. A walk faces where it goes anyway.
    if (bot->isMoving())
        return false;

    std::vector<Position> const skulls = GetYoggSaronSkullsInRange(botAI);
    if (skulls.empty())
        return false;

    // Judged on every skull in range, not the ones in arc: turning off the in-arc ones brings others round in front,
    // and melee in a reprisal hold would keep flipping headings. Fewest ahead wins, only a strict gain turns.
    float const x = bot->GetPositionX();
    float const y = bot->GetPositionY();
    float const current = bot->GetOrientation();

    uint32 const now = YoggSaronSkullsAhead(skulls, x, y, current);

    // With a target, only inside 40 degrees of it, where set facing and the casts leave the bot alone: turning further
    // gets turned straight back. A skull behind the target is the illusion facing walk's.
    Unit* target = AI_VALUE(Unit*, "current target");
    if (target && target->IsAlive())
    {
        float heading = current;
        uint32 const fewest = YoggSaronAngledSkullHeading(skulls, x, y, bot->GetAngle(target), heading);
        if (fewest < now && YoggSaronHeadingGap(current, heading) > ULDUAR_YOGG_SARON_FACING_TOLERANCE)
            bot->SetFacingTo(heading);

        return false;
    }

    uint32 fewest = now;
    float heading = current;
    for (uint32 i = 0; i < ULDUAR_YOGG_SARON_SKULL_HEADINGS; ++i)
    {
        float const candidate = 2.0f * static_cast<float>(M_PI) * i / ULDUAR_YOGG_SARON_SKULL_HEADINGS;
        uint32 const ahead = YoggSaronSkullsAhead(skulls, x, y, candidate);
        if (ahead < fewest || (ahead == fewest && ahead < now &&
                               YoggSaronHeadingGap(current, candidate) < YoggSaronHeadingGap(current, heading)))
        {
            fewest = ahead;
            heading = candidate;
        }
    }

    if (fewest >= now || YoggSaronHeadingGap(current, heading) <= ULDUAR_YOGG_SARON_FACING_TOLERANCE)
        return false;

    bot->SetFacingTo(heading);

    // The tick is never claimed: walking into the room and leaving it sit below this node and matter more than a
    // heading, and a heading needs no tick of its own once nothing is turning the bot back.
    return false;
}

bool YoggSaronLunaticGazeAction::Execute(Event /*event*/)
{
    Creature* boss = YoggSaronNearestCreature(botAI, NPC_YOGG_SARON);
    if (!boss || !boss->IsAlive())
        return false;

    float const pi = static_cast<float>(M_PI);
    float const towardYogg = bot->GetAngle(boss);
    float const away = Position::NormalizeOrientation(towardYogg + pi);

    // No heal needs facing, so a healer keeps its back to him and keeps casting. The tick stays free: claiming it
    // costs over a third of the raid's healing for every 4 s gaze.
    if (PlayerbotAI::IsHeal(bot))
    {
        if (!bot->isMoving() && YoggSaronHeadingGap(bot->GetOrientation(), away) > ULDUAR_YOGG_SARON_FACING_TOLERANCE)
            bot->SetFacingTo(away);

        return false;
    }

    // A target far enough round from Yogg has a heading that keeps both: set facing leaves the bot alone inside 45
    // degrees (60 for AttackAction, CastSpell), the gaze needs Yogg inside 90. Turn (135 less separation) / 2 off it.
    Unit* target = AI_VALUE(Unit*, "current target");
    if (target && target->IsAlive() && !bot->isMoving())
    {
        float const towardTarget = bot->GetAngle(target);
        float const offset = Position::NormalizeOrientation(towardTarget - towardYogg);
        float const separation = offset > pi ? 2.0f * pi - offset : offset;

        if (separation >= ULDUAR_YOGG_SARON_GAZE_SAFE_SEPARATION)
        {
            float const turn = std::max(0.0f, (0.75f * pi - separation) / 2.0f);
            float const heading = Position::NormalizeOrientation(towardTarget + (offset > pi ? -turn : turn));

            if (YoggSaronHeadingGap(bot->GetOrientation(), heading) > ULDUAR_YOGG_SARON_FACING_TOLERANCE)
                bot->SetFacingTo(heading);

            return false;
        }
    }

    // A beacon dragger is walking a Guardian clear, and a turn on the move ends the walk.
    if (YoggSaronBeaconDragging(bot))
    {
        if (!bot->isMoving() && YoggSaronHeadingGap(bot->GetOrientation(), away) > ULDUAR_YOGG_SARON_FACING_TOLERANCE)
            bot->SetFacingTo(away);

        return false;
    }

    // No taunt needs facing, so the tank turns away and still leaves guardian control its tick. Claiming it
    // leaves a Guardian peeled by heal threat on the healers for the whole 4 s gaze.
    if (PlayerbotAI::IsTank(bot) && YoggSaronGuardianLoose(botAI))
    {
        if (!bot->isMoving() && YoggSaronHeadingGap(bot->GetOrientation(), away) > ULDUAR_YOGG_SARON_FACING_TOLERANCE)
            bot->SetFacingTo(away);

        return false;
    }

    bot->SetFacingTo(away);

    return true;
}

bool YoggSaronBeaconDragAction::Execute(Event /*event*/)
{
    Unit* guardian = YoggSaronBeaconDragGuardian(botAI, true);
    if (!guardian)
        return ReturnHealed();

    if (bot->getClass() == CLASS_HUNTER)
        return HunterDrag(guardian);

    if (guardian->GetVictim() != bot)
    {
        if (!CastYoggSaronTaunt(botAI, guardian))
            return false;

        YoggSaronBeaconTaunted(bot);

        return true;
    }

    Position const& spot = YoggSaronBeaconDragSpot(bot);
    if (bot->GetExactDist2d(spot) > ULDUAR_YOGG_SARON_BEACON_DRAG_ARRIVED_RADIUS &&
        YoggSaronWalkMakingProgress(botAI, "beacondrag", spot))
    {
        Position const destination = YoggSaronBodyRoute(bot, spot);
        MoveTo(bot->GetMapId(), destination.GetPositionX(), destination.GetPositionY(), destination.GetPositionZ(),
               false, false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
        return true;
    }

    if (AI_VALUE(Unit*, "current target") != guardian)
        Attack(guardian);

    return true;
}

bool YoggSaronBeaconDragAction::HunterDrag(Unit* guardian)
{
    // No damage on it until the heal: any threat keeps it on the hunter once the taunt runs out.
    if (bot->GetVictim() == guardian)
    {
        bot->AttackStop();
        bot->InterruptSpell(CURRENT_AUTOREPEAT_SPELL);
    }

    for (Unit* minion : bot->m_Controlled)
        if (minion && minion->GetVictim() == guardian)
            minion->AttackStop();

    // One shot with the mark nearly done. Distracting Shot is a fixed threat and a 6 s taunt, not a match to top
    // threat, so the Guardian goes back once it runs out and the shot only covers the heal if it goes out late.
    Aura const* mark = guardian->GetAura(SPELL_SHADOW_BEACON);
    bool const due = mark && mark->GetDuration() >= 0 &&
                     static_cast<uint32>(mark->GetDuration()) <= ULDUAR_YOGG_SARON_BEACON_SHOT_LEFT_MS;
    if (due && guardian->GetVictim() != bot && !guardian->HasAura(SPELL_DISTRACTING_SHOT, bot->GetGUID()) &&
        bot->IsWithinDistInMap(guardian, ULDUAR_YOGG_SARON_DISTRACTING_SHOT_RANGE) &&
        bot->IsWithinLOSInMap(guardian) && botAI->CastSpell("distracting shot", guardian))
    {
        YoggSaronBeaconTaunted(bot);

        return true;
    }

    // To the hunter spot, or short of it toward the Guardian when the spot is out of the shot's range.
    Position destination = YoggSaronBeaconHunterSpot(bot);
    float const reach = guardian->GetExactDist2d(destination);
    if (reach > ULDUAR_YOGG_SARON_DISTRACTING_SHOT_RANGE && guardian->GetVictim() != bot)
    {
        float const share = (ULDUAR_YOGG_SARON_DISTRACTING_SHOT_RANGE - 2.0f) / reach;
        destination.Relocate(guardian->GetPositionX() + (destination.GetPositionX() - guardian->GetPositionX()) * share,
                             guardian->GetPositionY() + (destination.GetPositionY() - guardian->GetPositionY()) * share,
                             destination.GetPositionZ());
    }

    if (bot->GetExactDist2d(destination) > ULDUAR_YOGG_SARON_BEACON_DRAG_ARRIVED_RADIUS &&
        YoggSaronWalkMakingProgress(botAI, "beacondrag", destination))
    {
        Position const step = YoggSaronBodyRoute(bot, destination);
        MoveTo(bot->GetMapId(), step.GetPositionX(), step.GetPositionY(), step.GetPositionZ(), false, false, false,
               true, MovementPriority::MOVEMENT_FORCED, true, false);
        return true;
    }

    return true;
}

bool YoggSaronBeaconDragAction::ReturnHealed()
{
    Unit* guardian = YoggSaronBeaconReturning(bot);
    if (!guardian)
        return false;

    // A hunter has nothing to hold it with. Once its taunt has run out, a Guardian still on it goes back
    // to whoever is next on its threat, which is the tank that held it before the mark.
    if (bot->getClass() == CLASS_HUNTER)
    {
        if (guardian->HasAura(SPELL_DISTRACTING_SHOT, bot->GetGUID()) || !botAI->CanCastSpell("feign death", bot))
            return false;

        Aura const* heal = guardian->GetAura(SPELL_EMPOWERING_SHADOWS_25);
        if (!heal)
            heal = guardian->GetAura(SPELL_EMPOWERING_SHADOWS_10);
        if (heal &&
            heal->GetMaxDuration() - heal->GetDuration() < static_cast<int32>(ULDUAR_YOGG_SARON_BEACON_FEIGN_DELAY_MS))
            return false;

        return botAI->CastSpell("feign death", bot);
    }

    // Back to the melee spot with it, where the tank's taunt reaches: from the drag spot it can't.
    Position const& spot = ULDUAR_YOGG_SARON_PHASE_3_MELEE_SPOT;
    if (bot->GetExactDist2d(spot) <= ULDUAR_YOGG_SARON_BEACON_DRAG_ARRIVED_RADIUS ||
        !YoggSaronWalkMakingProgress(botAI, "beaconreturn", spot))
        return false;

    Position const step = YoggSaronBodyRoute(bot, spot);

    return MoveTo(bot->GetMapId(), step.GetPositionX(), step.GetPositionY(), step.GetPositionZ(), false, false, false,
                  true, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool YoggSaronBeaconStageAction::Execute(Event /*event*/)
{
    Position const* spot = YoggSaronBeaconStageSpot(bot);
    if (!spot)
        return false;

    bool const away = bot->GetExactDist2d(*spot) > ULDUAR_YOGG_SARON_BEACON_DRAG_ARRIVED_RADIUS;
    if (away && YoggSaronWalkMakingProgress(botAI, "beaconstage", *spot))
    {
        Position const step = YoggSaronBodyRoute(bot, *spot);
        MoveTo(bot->GetMapId(), step.GetPositionX(), step.GetPositionY(), step.GetPositionZ(), false, false, false,
               true, MovementPriority::MOVEMENT_FORCED, true, false);
        return true;
    }

    return true;
}

bool YoggSaronPhase3LustAction::Execute(Event /*event*/)
{
    for (char const* spell : {"heroism", "bloodlust"})
    {
        if (botAI->CanCastSpell(spell, bot) && botAI->CastSpell(spell, bot))
            return true;
    }

    return false;
}

bool YoggSaronPhase3PositioningAction::Execute(Event /*event*/)
{
    // A tank is only ever walked back on the leash: wherever it drifted to, a guardian took it there.
    if (!PlayerbotAI::IsTank(bot))
    {
        YoggSaronTrigger yoggSaronTrigger(botAI);
        Unit* target = AI_VALUE(Unit*, "current target");
        if (target && target->IsAlive() && !yoggSaronTrigger.PhaseThreeStationReaches(target))
            return false;
    }

    // Boots on any long walk: nobody moves much once at the spot, so the 3 min cooldown costs nothing.
    Position const& spot = YoggSaronPhaseThreeSpot(bot);
    if (bot->GetExactDist2d(spot) > ULDUAR_YOGG_SARON_NITRO_MIN_WALK && !bot->HasAura(NITRO_BOOSTS_SPEED_SPELL))
        UseNitroBoosts(bot);

    // Round the body. The brain team surfaces west of Yogg and the melee spot is east: the straight line
    // passes 2 yd from his middle, so the knock back throws melee walking it.
    Position const destination = YoggSaronBodyRoute(bot, spot);

    return MoveTo(bot->GetMapId(), destination.GetPositionX(), destination.GetPositionY(),
                  destination.GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_FORCED, true,
                  false);
}

bool YoggSaronGuardianControlAction::Execute(Event /*event*/)
{
    // Tank only: melee/ranged already focus the skull guardian and the phase-3 positioning stacks them.
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (YoggSaronInPhase1(botAI))
        return ControlPhaseOne();

    return ControlPhaseThree();
}

bool YoggSaronGuardianControlAction::ControlPhaseThree()
{
    Position const& spot = ULDUAR_YOGG_SARON_PHASE_3_MELEE_SPOT;

    GuidVector const targets = GetYoggSaronGuardiansInSight(botAI);
    std::vector<std::pair<float, Unit*>> loose;
    std::vector<Unit*> inReach;
    std::vector<Unit*> atSpot;
    for (ObjectGuid const& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!IsYoggSaronHoldableGuardian(unit))
            continue;

        Player* victim = unit->GetVictim() ? unit->GetVictim()->ToPlayer() : nullptr;
        if (!victim || !PlayerbotAI::IsTank(victim))
            loose.emplace_back(bot->GetDistance(unit), unit);

        if (bot->IsWithinMeleeRange(unit))
            inReach.push_back(unit);
        else if (unit->GetExactDist2d(spot) <= ULDUAR_YOGG_SARON_PHASE_3_MELEE_GUARDIAN_RANGE)
            atSpot.push_back(unit);
    }

    if (Player* holder = YoggSaronTankSwapTake(botAI))
        if (TakeFrom(holder))
            return true;

    // The holder of a swap leaves loose ones to the taker, or its taunts pull the pile straight back, and
    // so does a low tank while the other one is healthy.
    bool const shedding = YoggSaronTankShedding(botAI);

    if (shedding || YoggSaronTankLowHold(bot))
        loose.clear();

    // Loose ones before the leash: a taunt is a cast, not a walk, and the Guardian is already heading for a healer or
    // a ranged. Highest health first: a full or beacon-healed one hits a non-tank for 30-75k, a Weakened one for ~2k.
    std::sort(loose.begin(), loose.end(),
              [](auto const& left, auto const& right)
              {
                  return std::make_tuple(-left.second->GetHealthPct(), left.first) <
                         std::make_tuple(-right.second->GetHealthPct(), right.first);
              });
    for (auto const& [distance, guardian] : loose)
    {
        // Already coming to another tank. Both taunting the same fresh one leaves Hand of Reckoning on cooldown
        // for most swings on the raid.
        auto const& taunts = guardian->GetAuraEffectsByType(SPELL_AURA_MOD_TAUNT);
        if (std::any_of(taunts.begin(), taunts.end(),
                        [this](AuraEffect const* taunt)
                        {
                            Unit* caster = taunt->GetCaster();
                            Player* tank = caster ? caster->ToPlayer() : nullptr;
                            return tank && tank != bot && PlayerbotAI::IsTank(tank);
                        }))
            continue;

        if (Taunt(guardian))
            return true;

        if (Defend(guardian))
            return true;
    }

    // Hold the melee stack so taunted guardians pile onto the melee bots to be cleaved down.
    if (bot->GetDistance(spot) > ULDUAR_YOGG_SARON_PHASE_3_STACK_RADIUS)
    {
        Position const destination = YoggSaronBodyRoute(bot, spot);

        return MoveTo(bot->GetMapId(), destination.GetPositionX(), destination.GetPositionY(),
                      destination.GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_FORCED,
                      true, false);
    }

    // A taunt with no threat behind it lasts 3 s, and Hand of Reckoning is back after 8. Hold the one where the lead
    // is thinnest, in reach before merely near the spot, so holding it is not a walk off the leash.
    Unit* hold = nullptr;
    float holdLead = 0.0f;
    for (Unit* guardian : inReach.empty() ? atSpot : inReach)
    {
        float const lead = YoggSaronTankThreatLead(bot, guardian);
        if (!hold || lead < holdLead)
        {
            hold = guardian;
            holdLead = lead;
        }
    }

    Unit* current = AI_VALUE(Unit*, "current target");
    if (!hold || current == hold)
        return false;

    if (IsYoggSaronHoldableGuardian(current) &&
        current->GetExactDist2d(spot) <= ULDUAR_YOGG_SARON_PHASE_3_MELEE_GUARDIAN_RANGE)
    {
        float const currentLead = YoggSaronTankThreatLead(bot, current);
        if (currentLead <= ULDUAR_YOGG_SARON_TANK_THREAT_SWITCH_LEAD || currentLead <= holdLead)
            return false;
    }

    return Attack(hold);
}

bool YoggSaronGuardianControlAction::ControlPhaseOne()
{
    // Inside the cloud-free circle, not the 5 yd the phase 3 stack uses: a taunted Guardian dies where the tank stands.
    // Not while the handover owns the tank's spot: both walks are forced, each clears the other's hold.
    if (!YoggSaronHandoverState(botAI).clearing &&
        bot->GetDistance2d(ULDUAR_YOGG_SARON_MIDDLE.GetPositionX(), ULDUAR_YOGG_SARON_MIDDLE.GetPositionY()) >
            ULDUAR_YOGG_SARON_P1_CLOUD_FREE_RADIUS)
        return MoveTo(bot->GetMapId(), ULDUAR_YOGG_SARON_MIDDLE.GetPositionX(),
                      ULDUAR_YOGG_SARON_MIDDLE.GetPositionY(), ULDUAR_YOGG_SARON_MIDDLE.GetPositionZ(), false,
                      false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);

    if (Unit* guardian = YoggSaronPhase1TauntTarget(botAI))
        if (Taunt(guardian))
            return true;

    // Nothing to fetch, or the taunt is on cooldown: hold the pile. A taunt that does not go out leaves the bot
    // selected on a stale aim and nothing else replaces it, so the tank never swings.
    Unit* hold = YoggSaronPhase1StackTarget(botAI);
    Unit* current = AI_VALUE(Unit*, "current target");
    if (!hold || current == hold)
        return false;

    return Attack(hold);
}

bool YoggSaronGuardianControlAction::Taunt(Unit* guardian) { return CastYoggSaronTaunt(botAI, guardian); }

bool YoggSaronGuardianControlAction::Defend(Unit* guardian)
{
    if (bot->getClass() != CLASS_PALADIN)
        return false;

    // Only the ally has to be in the spell's 40 yd. The effect casts 31790 triggered on up to 3 of its
    // attackers, and 31790's range is 50,000 yd, so it reaches a Guardian still walking in from the ring.
    Player* victim = guardian->GetVictim() ? guardian->GetVictim()->ToPlayer() : nullptr;
    if (!victim || victim == bot || !victim->IsAlive())
        return false;

    return botAI->CastSpell("righteous defense", victim);
}

bool YoggSaronGuardianControlAction::TakeFrom(Player* holder)
{
    // Up to three of the holder's attackers in one cast.
    if (bot->getClass() == CLASS_PALADIN && botAI->CanCastSpell("righteous defense", holder) &&
        botAI->CastSpell("righteous defense", holder))
    {
        return true;
    }

    Unit* highest = nullptr;
    for (Unit* attacker : holder->getAttackers())
        if (attacker->GetVictim() == holder && IsYoggSaronHoldableGuardian(attacker) &&
            (!highest || attacker->GetHealthPct() > highest->GetHealthPct()))
            highest = attacker;

    return highest && Taunt(highest);
}

bool YoggSaronSanityConservationAction::Execute(Event /*event*/)
{
    Unit* yogg = AI_VALUE2(Unit*, "find target", "yogg-saron");
    if (!yogg || !yogg->IsAlive())
        return false;

    // Pull to the back of Yogg-Saron: the spot behind him, opposite his facing.
    constexpr float behindDistance = 15.0f;
    constexpr float arriveRadius = 5.0f;
    float behindAngle = Position::NormalizeOrientation(yogg->GetOrientation() + M_PI);
    float behindX = yogg->GetPositionX() + behindDistance * cos(behindAngle);
    float behindY = yogg->GetPositionY() + behindDistance * sin(behindAngle);
    float behindZ = yogg->GetPositionZ();

    if (bot->GetDistance2d(behindX, behindY) > arriveRadius)
        return MoveTo(bot->GetMapId(), behindX, behindY, behindZ, false, false, false, true,
                      MovementPriority::MOVEMENT_FORCED, true, false);

    // Face directly away from Yogg: Lunatic Gaze only hits units with him in their front arc.
    float awayAngle = Position::NormalizeOrientation(bot->GetAngle(yogg) + M_PI);
    bot->SetFacingTo(awayAngle);

    // Only heal/DPS a target already in the front hemisphere (away from Yogg), so the bot never
    // turns back toward Yogg and eats a gaze. Otherwise hold, facing away.
    Unit* target = AI_VALUE(Unit*, "current target");
    if (target && target->IsAlive())
    {
        float diff = Position::NormalizeOrientation(bot->GetAngle(target) - awayAngle);
        if (diff > M_PI)
            diff = 2 * M_PI - diff;
        if (diff <= M_PI / 2)
            return false;
    }

    return true;
}

bool YoggSaronSqueezeRescueAction::Execute(Event /*event*/)
{
    Player* victim = YoggSaronSqueezeVictim(botAI);
    if (!victim)
        return false;

    // Hand of Protection's own debuff, matched by id because PlayerbotAI::HasAura compares the DBC
    // string exactly. A second Hand inside two minutes is refused outright, so this is not a nicety.
    constexpr uint32 SPELL_FORBEARANCE = 25771;
    if (victim->HasAura(SPELL_FORBEARANCE))
        return false;

    // 3.3.5a name: SpellIdValue matches the DBC string exactly, and the old spelling resolves to no spell.
    if (!botAI->CanCastSpell("hand of protection", victim))
        return false;

    if (!ClaimYoggSaronSqueezeRescue(botAI, victim))
        return false;

    return botAI->CastSpell("hand of protection", victim);
}

bool YoggSaronSqueezeEscapeAction::Execute(Event /*event*/)
{
    switch (bot->getClass())
    {
        case CLASS_MAGE:
            return botAI->CanCastSpell("ice block", bot) && botAI->CastSpell("ice block", bot);

        case CLASS_PALADIN:
            return botAI->CanCastSpell("divine shield", bot) && botAI->CastSpell("divine shield", bot);

        default:
            return false;
    }
}

bool YoggSaronNovaDefensiveAction::Execute(Event /*event*/)
{
    uint32 const needMs = YoggSaronNovaNeedMs(bot, true);
    if (!needMs)
        return false;

    uint32 const defensive = NextMagicDefensive(botAI, bot, needMs);

    return defensive && botAI->CastSpell(defensive, bot);
}

char const* YoggSaronNovaExternalAction::PickExternal(PlayerbotAI* botAI, Player* victim)
{
    Player* bot = botAI->GetBot();

    for (char const* external : HEALER_EXTERNALS)
    {
        if (!botAI->CanCastSpell(external, victim))
            continue;

        // CanCastSpell counts out of range as castable, so range is checked here or a healer on the far
        // side of the room claims the victim and then casts nothing.
        if (!bot->IsWithinDistInMap(victim, sPlayerbotAIConfig.SpellDistance))
            continue;

        return external;
    }

    return nullptr;
}

bool YoggSaronNovaExternalAction::Execute(Event /*event*/)
{
    Player* victim = YoggSaronNovaVictim(botAI);
    if (!victim)
        return false;

    char const* external = PickExternal(botAI, victim);
    if (!external)
        return false;

    // Claimed only once there is something to cast. A healer that claimed on sight would beat everybody
    // else to the nova and then spend nothing on it.
    if (!ClaimYoggSaronNovaExternal(botAI))
        return false;

    return botAI->CastSpell(external, victim);
}

bool YoggSaronShadowAuraAction::Execute(Event /*event*/)
{
    return botAI->CastSpell("shadow resistance aura", bot);
}

bool YoggSaronRoarAuraAction::Execute(Event /*event*/)
{
    return botAI->CastSpell("concentration aura", bot);
}

bool YoggSaronRoarAuraMasteryAction::Execute(Event /*event*/)
{
    // Claimed only with the aura up and the button ready, so a paladin that can't press never takes the
    // roar off one that can. By the application: a copy left without effects still counts.
    if (!bot->HasAura(SPELL_CONCENTRATION_AURA, bot->GetGUID()))
        return false;

    if (!ClaimYoggSaronRoar(botAI))
        return false;

    // Straight through CastSpell, past the hold that keeps the class nodes off Aura Mastery in phase 3.
    return botAI->CastSpell("aura mastery", bot);
}

bool YoggSaronBeaconDefensiveAction::Execute(Event /*event*/)
{
    uint32 const defensive = NextPhysicalDefensive(botAI, bot, 0);

    return defensive && botAI->CastSpell(defensive, bot);
}

bool YoggSaronBeaconExternalAction::Execute(Event /*event*/)
{
    Player* victim = YoggSaronBeaconExternalVictim(botAI);
    if (!victim)
        return false;

    char const* external = YoggSaronNovaExternalAction::PickExternal(botAI, victim);
    if (!external || !ClaimYoggSaronBeaconExternal(botAI, victim))
        return false;

    return botAI->CastSpell(external, victim);
}

bool YoggSaronTankLayOnHandsAction::Execute(Event /*event*/)
{
    Player* tank = YoggSaronTankLayOnHandsTarget(botAI);
    return tank && ClaimYoggSaronTankLayOnHands(botAI, tank) && botAI->CastSpell("lay on hands", tank);
}

bool YoggSaronHealerCombatEngineAction::Execute(Event /*event*/)
{
    botAI->ChangeEngine(BOT_STATE_COMBAT);

    return true;
}

bool YoggSaronNovaRaidCooldownAction::Execute(Event /*event*/)
{
    char const* cover = YoggSaronNovaRaidCover(botAI);
    if (!cover)
        return false;

    if (!ClaimYoggSaronNovaWindow(bot))
        return false;

    // Through the class node rather than CastSpell, so Divine Sacrifice's cancel action stays its
    // continuer: the split keeps running for 10 s whether the nova has landed or not.
    return botAI->DoSpecificAction(cover);
}
