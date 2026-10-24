/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldActions_Mimiron.h"
#include "UldActions_Shared.h"

#include <CombatStrategy.h>
#include <FollowMasterStrategy.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iterator>
#include <limits>

#include "AiObjectContext.h"
#include "DBCEnums.h"
#include "GameObject.h"
#include "Group.h"
#include "LastMovementValue.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "RaidTankDefensive.h"
#include "Spell.h"
#include "Position.h"
#include "UldData.h"
#include "UldEncounter_Mimiron.h"
#include "UldHardMode.h"
#include "UldScripts.h"
#include "UldTargets.h"
#include "EncounterHelpers.h"
#include "RtiValue.h"
#include "ScriptedCreature.h"
#include "ServerFacade.h"
#include "Unit.h"
#include "Vehicle.h"
#include <RtiTargetValue.h>
#include <TankAssistStrategy.h>

using namespace EncounterHelpers;

bool MimironFleeAction::MoveAwayClearOfMines(Unit* from, float distance, MovementPriority priority,
                                             bool fallbackUnfiltered, bool interrupt, float clearRadius,
                                             bool sectorScreen, bool allowFire)
{
    if (!from)
        return false;

    return FleeFan(from->GetPosition(), from, distance, priority, fallbackUnfiltered, interrupt, clearRadius,
                   allowFire, sectorScreen);
}

bool MimironFleeAction::MoveAwayClearOfMines(Position const& from, float distance,
                                             MovementPriority priority, bool fallbackUnfiltered,
                                             bool interrupt, float clearRadius)
{
    return FleeFan(from, nullptr, distance, priority, fallbackUnfiltered, interrupt, clearRadius);
}

bool MimironFleeAction::MoveTowardClearOfMines(Position const& dest, MovementPriority priority,
                                               bool fallbackUnfiltered, bool interrupt, bool allowFire)
{
    float const distance = bot->GetExactDist2d(dest.GetPositionX(), dest.GetPositionY());
    if (distance <= 0.0f)
        return false;

    // Fleeing the mirror walks straight at `dest` on bearing 0, and every swept bearing still ends further
    // from the mirror than the bot started, so the backtrack filter passes them all.
    Position const mirror(2.0f * bot->GetPositionX() - dest.GetPositionX(),
                          2.0f * bot->GetPositionY() - dest.GetPositionY(), bot->GetPositionZ());

    return FleeFan(mirror, nullptr, distance, priority, fallbackUnfiltered, interrupt, 0.0f, allowFire);
}

bool MimironFleeAction::FleeFan(Position const& from, Unit* fallbackFrom, float distance,
                                MovementPriority priority, bool fallbackUnfiltered, bool interrupt,
                                float clearRadius, bool allowFire, bool sectorScreen)
{
    if (distance <= 0.0f)
        return false;

    // A casting bot can't be moved: PointMovementGenerator drops the spline, yet MoveTo reports success and
    // stamps a LastMovement delay that blocks its own retries. Only the hazards that kill interrupt.
    if (interrupt)
        bot->CastStop();

    // Once for the fan, since it doesn't depend on the destination. IsWaitingForLastMove wants a strictly
    // higher priority, so a leg issued at this one blocks every retry until it expires.
    UpdateMovementState();
    if (!IsMovingAllowed() || IsWaitingForLastMove(priority))
        return false;

    float const speed = bot->GetSpeed(MOVE_RUN);
    float const away = from.GetAngle(bot->GetPositionX(), bot->GetPositionY());
    float const started = bot->GetExactDist2d(from.GetPositionX(), from.GetPositionY());

    // Resolved once for the whole fan: reading the window costs a grid scan for the DB Target, and
    // gathering the Firefighter hazards walks a fire field that runs to 50 or 60 nodes.
    Unit* const vx001 = GetUldUnitByEntry(botAI, NPC_VX001);
    MimironBarrageWindow const barrage =
        vx001 ? GetMimironBarrageWindow(bot, vx001) : MimironBarrageWindow();
    MimironRapidBurstWindow const burst =
        vx001 ? GetMimironRapidBurstWindow(botAI, bot, vx001) : MimironRapidBurstWindow();
    MimironFirefighterHazards const hazards = GetMimironFirefighterHazards(botAI);
    MimironBurstSector const sector =
        sectorScreen && vx001 ? GetMimironOtherGroupSector(botAI, bot, vx001) : MimironBurstSector();

    for (float delta = 0.0f; delta <= ULDUAR_MIMIRON_FLEE_FAN_MAX + 0.001f; delta += ULDUAR_MIMIRON_FLEE_FAN_STEP)
    {
        for (float sign : {1.0f, -1.0f})
        {
            if (delta == 0.0f && sign < 0.0f)
                continue;

            float const angle = away + sign * delta;

            // Off straight away a hop r lands sqrt(d^2 + r^2 + 2dr cos delta) from the hazard, not d + r, so
            // solve r per bearing to land on the circle. No root: the bearing never reaches it.
            float hop = distance;
            if (clearRadius > 0.0f)
            {
                float const leg = started * sin(delta);
                float const disc = clearRadius * clearRadius - leg * leg;
                hop = disc > 0.0f ? sqrt(disc) - started * cos(delta) : 0.0f;
                if (hop <= 0.0f)
                    continue;
            }

            float dx = bot->GetPositionX() + cos(angle) * hop;
            float dy = bot->GetPositionY() + sin(angle) * hop;
            float dz = bot->GetPositionZ();
            bool exact = true;
            if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(),
                                                               bot->GetPositionY(),
                                                               bot->GetPositionZ(), dx, dy, dz))
            {
                dx = bot->GetPositionX() + cos(angle) * hop;
                dy = bot->GetPositionY() + sin(angle) * hop;
                dz = bot->GetPositionZ();
                exact = false;
            }

            Position const dest(dx, dy, dz);
            float const travel = speed > 0.0f ? hop / speed : 0.0f;

            // CheckCollisionAndGetValidCoords reports success on an incomplete path, so an off-mesh bearing comes
            // back as the bot's own feet. Judge the step too: the gain test below passes on a millimetre.
            if (dest.GetExactDist2d(bot->GetPositionX(), bot->GetPositionY()) <
                hop * ULDUAR_MIMIRON_FLEE_MIN_PROGRESS_PCT)
                continue;

            // Collision can shorten the step enough to leave the bot no better off than it started.
            if (dest.GetExactDist2d(from.GetPositionX(), from.GetPositionY()) <= started)
                continue;

            if (!IsMimironSpotMineSafe(bot, dest))
                continue;

            // Judged at arrival, not at issue: this leg holds the lock for its whole run, so a spot that's only
            // clear now strands the bot in the beams.
            if (!IsMimironSpotBarrageSafe(vx001, barrage, dest, travel))
                continue;

            // Firefighter only, empty otherwise. Keeps a Shock Blast or barrage dodge out of fresh fire, and a
            // fire dodge out of the next node along.
            if (!allowFire && !IsMimironSpotFireSafe(hazards, dest))
                continue;

            if (!IsMimironSpotBombSafe(hazards, dest))
                continue;

            // Rapid Burst is only 60 degrees wide but 100 yd deep, so any dodge can walk into it.
            if (!IsMimironSpotRapidBurstSafe(vx001, burst, dest))
                continue;

            // Counted with the cone. An escape that crosses VX-001 puts melee in the wedge's line, or
            // ranged in the melee sector, and every burst after that hits both groups.
            if (IsMimironSpotInSector(vx001, sector, dest))
                continue;

            // Including the Shock Blast escape itself: the test is on where the bot ends, not where it starts.
            if (!IsMimironSpotShockSafe(botAI, dest))
                continue;

            if (!IsMimironSpotFireBotSafe(bot, hazards, dest, ULDUAR_MIMIRON_FIREBOT_SIREN_STAND,
                                          ULDUAR_MIMIRON_FIREBOT_SPRAY_STAND_HALF_WIDTH))
                continue;

            if (TryMoveTo(bot->GetMapId(), dx, dy, dz, false, false, true, exact, priority) ==
                MoveOutcome::Issued)
                return true;
        }
    }

    // Every bearing refused. Sweep again without the sector screen before any fallback: the other group's
    // line costs a burst or two, the hazard kills.
    if (sector.valid)
        return FleeFan(from, fallbackFrom, distance, priority, fallbackUnfiltered, interrupt, clearRadius,
                       allowFire, false);

    if (!fallbackUnfiltered)
        return false;

    // Never into the four that kill outright: standing in fire is only ~3.1k a second, and this forced leg
    // would also hold the lock against the Shock Blast escape.
    Position const straightAway(bot->GetPositionX() + cos(away) * distance,
                                bot->GetPositionY() + sin(away) * distance, bot->GetPositionZ());
    float const fallbackTravel = speed > 0.0f ? distance / speed : 0.0f;
    if (!IsMimironSpotShockSafe(botAI, straightAway) ||
        !IsMimironSpotBombSafe(hazards, straightAway) ||
        !IsMimironSpotBarrageSafe(vx001, barrage, straightAway, fallbackTravel) ||
        IsMimironSpotInFireBotSpray(hazards, straightAway, ULDUAR_MIMIRON_FIREBOT_SPRAY_HALF_WIDTH))
        return false;

    if (fallbackFrom)
        return MoveAway(fallbackFrom, distance);

    return MoveTo(bot->GetMapId(), bot->GetPositionX() + cos(away) * distance,
                  bot->GetPositionY() + sin(away) * distance, bot->GetPositionZ(), false, false, true,
                  false, priority);
}

// Last resort once every bearing off the MK II is refused, e.g. where straight away points off the mesh.
// Runs to the bot's slot, screened like any destination, or the room centre when that's inside the circle.
bool MimironShockBlastAction::FleeShockToAnchor()
{
    Position anchor;
    if (!GetMimironSpreadSlot(botAI, bot, anchor) || !IsMimironSpotShockSafe(botAI, anchor))
        anchor = ULDUAR_MIMIRON_ROOM_CENTER;

    return MoveTowardClearOfMines(anchor, MovementPriority::MOVEMENT_FORCED, false, true);
}

bool MimironShockBlastAction::Execute(Event /*event*/)
{
    Unit* leviathanMkII = GetUldUnitByEntry(botAI, NPC_LEVIATHAN_MKII);
    if (!leviathanMkII)
        return false;

    // FORCED: the arc spread issues at COMBAT, and IsWaitingForLastMove drops anything not strictly above
    // the move in flight.
    float const dist = bot->GetExactDist2d(leviathanMkII);
    if (dist < ULDUAR_MIMIRON_SHOCK_BLAST_SAFE_DIST)
    {
        // Fire screened first, then a burning node over the blast: ~3100 a second against 100000.
        float const escape = ULDUAR_MIMIRON_SHOCK_BLAST_SAFE_DIST + ULDUAR_MIMIRON_SHOCK_FLEE_MARGIN;
        float const gap = escape - dist;
        if (!MoveAwayClearOfMines(leviathanMkII, gap, MovementPriority::MOVEMENT_FORCED, false, true, escape) &&
            !MoveAwayClearOfMines(leviathanMkII, gap, MovementPriority::MOVEMENT_FORCED, true, true, escape, false,
                                  true))
            FleeShockToAnchor();

        if (PlayerbotAI::IsMelee(bot))
            botAI->SetNextCheckDelay(100);

        // Still inside the circle: hold the tick even when the lock refused every bearing, or Charge
        // (relevance 40) takes it and runs the bot back under the cast.
        return true;
    }

    // Clear. Melee keep holding the tick: they carry the gap-closers and have nothing to hit at 18 yd anyway.
    // Ranged and healers go back to work rather than lose 4 s in every 30.
    return PlayerbotAI::IsMelee(bot);
}

bool MimironShockBlastAction::isUseful()
{
    MimironShockBlastTrigger mimironShockBlastTrigger(botAI);
    return mimironShockBlastTrigger.IsActive();
}

bool MimironResetEncounterStateAction::Execute(Event /*event*/)
{
    RESET_AI_VALUE(float, "disperse distance");

    // Never claims the tick: clearing state is bookkeeping, and whatever the bot is standing in still
    // needs the nodes below this one to run.
    return false;
}

bool MimironPhase1PositioningAction::Execute(Event /*event*/)
{
    SET_AI_VALUE(float, "disperse distance", GetMimironPhase1DisperseDistance(botAI));

    // Never claims the tick: the engine stops a pass at the first action returning true, and at ACTION_RAID
    // that would be every cast and heal the bot owns.
    return false;
}

bool MimironPhase1PositioningAction::isUseful()
{
    MimironPhase1PositioningTrigger mimironPhase1PositioningTrigger(botAI);
    return mimironPhase1PositioningTrigger.IsActive();
}

bool MimironP3Wx2LaserBarrageAction::isUseful()
{
    MimironP3Wx2LaserBarrageTrigger mimironP3Wx2LaserBarrageTrigger(botAI);
    return mimironP3Wx2LaserBarrageTrigger.IsActive();
}

bool MimironP3Wx2LaserBarrageAction::Execute(Event /*event*/)
{
    Unit* boss = GetUldUnitByEntry(botAI, NPC_VX001);
    if (!boss || !boss->IsAlive())
        return false;

    MimironBarrageWindow const window = GetMimironBarrageWindow(bot, boss);
    if (!window.valid)
        return false;

    float const clearance = ULDUAR_MIMIRON_BARRAGE_HALF_ANGLE + ULDUAR_MIMIRON_BARRAGE_MARGIN;
    float const twoPi = 2.0f * static_cast<float>(M_PI);

    // Clockwise of the cone centreline in [0, 2pi), never folded to (-pi, pi]: the band runs sweep plus two
    // clearances, ~240 degrees, so a signed fold puts part of it past pi and the far side reads safe.
    float const cw = Position::NormalizeOrientation(window.lead - boss->GetAngle(bot));

    // The cone is 50000 yd long, so ranged keep their radius above a floor, and melee orbit inside their reach.
    // GetExactDist2d, not GetDistance2d: the latter subtracts both object sizes, but this point is centre-based.
    bool const melee = PlayerbotAI::IsMelee(bot);
    float const minRing =
        boss->GetCombatReach() +
        (melee ? ULDUAR_MIMIRON_BARRAGE_MELEE_RING_MARGIN : ULDUAR_MIMIRON_BARRAGE_RING_MARGIN);
    float const radius =
        melee ? minRing
              : std::clamp(bot->GetExactDist2d(boss), minRing, ULDUAR_MIMIRON_SPREAD_RADIUS_MAX);

    // Melee only give ground inward: their ring is already the smallest that clears the model, and
    // widening it puts them back outside their own reach test.
    float const shiftMin = melee ? ULDUAR_MIMIRON_BARRAGE_MELEE_RING_MIN : minRing;
    float const shiftMax = melee ? radius : ULDUAR_MIMIRON_SPREAD_RADIUS_MAX;

    // Safe sector: sweep + clearance to 2pi - clearance clockwise. All of it stays clear for the rest of the
    // cast, since the far edge is fixed and the trailing side only opens.
    float const sectorWidth = std::max(twoPi - 2.0f * clearance - window.sweep, 0.0f);

    MimironFirefighterHazards const hazards = GetMimironFirefighterHazards(botAI);
    auto const onOrbit = [&](float heading, float r)
    {
        return Position(boss->GetPositionX() + r * cos(heading), boss->GetPositionY() + r * sin(heading),
                        boss->GetPositionZ());
    };

    // First offset up to `span` round VX-001 from `bearing` (+1 ccw, -1 cw) where some ring radius is clear
    // of fire, the bomb and every rocket. Negative when all of it burns.
    auto const fireFreeOffset = [&](float bearing, float direction, float span)
    {
        for (float offset = 0.0f; offset <= span; offset += ULDUAR_MIMIRON_BARRAGE_REFUGE_PROBE)
        {
            for (float shift : ULDUAR_MIMIRON_BARRAGE_RADIUS_SHIFTS)
            {
                float const candidate = radius + shift;
                if (candidate < shiftMin || candidate > shiftMax)
                    continue;

                Position const spot = onOrbit(bearing + direction * offset, candidate);
                if (IsMimironSpotFireSafe(hazards, spot) && IsMimironSpotBombSafe(hazards, spot) &&
                    IsMimironSpotRocketSafe(botAI, spot))
                    return offset;
            }
        }

        return -1.0f;
    };

    // Walk the ring in bounded steps, `remaining` radians to go, + ccw. One long move draws a chord through
    // VX-001 (creatures aren't in the navmesh) that crosses every bearing the cone covers.
    auto const stepRing = [&](float remaining)
    {
        float const stepped =
            std::copysign(std::min(std::fabs(remaining), ULDUAR_MIMIRON_BARRAGE_STEP), remaining);
        float const heading = Position::NormalizeOrientation(boss->GetAngle(bot) + stepped);

        // Radius is free, so spend it dodging fire, walk and endpoint both. No clean radius still keeps the
        // step: the cone kills in a tick.
        float stepRadius = radius;
        bool settled = false;
        for (float shift : ULDUAR_MIMIRON_BARRAGE_RADIUS_SHIFTS)
        {
            float const candidate = radius + shift;
            if (candidate < shiftMin || candidate > shiftMax)
                continue;

            // A rocket or the bomb kills outright, so a spot in either isn't even a fallback. The step
            // is MOVEMENT_FORCED and locks the rocket dodge out until it lands.
            Position const spot = onOrbit(heading, candidate);
            if (!IsMimironSpotRocketSafe(botAI, spot) || !IsMimironSpotBombSafe(hazards, spot) ||
                !IsMimironSpotFireSafe(hazards, spot))
                continue;

            // First clean endpoint is the fallback; keep looking for one whose walk is clean too.
            if (!settled)
            {
                stepRadius = candidate;
                settled = true;
            }

            if (IsMimironWalkFireSafe(bot, hazards, spot))
            {
                stepRadius = candidate;
                break;
            }
        }

        // A casting bot can't be moved at all, and nothing survives standing in this to finish a cast.
        bot->CastStop();

        Position const step = onOrbit(heading, stepRadius);
        MoveTo(boss->GetMapId(), step.GetPositionX(), step.GetPositionY(), step.GetPositionZ(), false,
               false, false, true, MovementPriority::MOVEMENT_FORCED, true);

        // Fast recheck while relocating: the cone turns ~2.7 degrees per 250 ms damage tick. Only while
        // moving, since a clear bot only gets clearer.
        botAI->SetNextCheckDelay(100);

        // Hold the tick. Spinning Up is only 4s of warning and the cone kills outright, so a relocating
        // bot must not stop to finish a cast.
        return true;
    };

    // Two fringes: ahead of the leading edge, and behind where the trailing edge finishes. Returns false on
    // purpose, so bots that were never in danger keep casting.
    if (cw > window.sweep + clearance && cw < twoPi - clearance)
    {
        // Inside the boundary the beams hit at any bearing. Straight out on the bearing it holds, unscreened:
        // the beams are certain, a marker on the ring isn't, and the flee fan refuses this circle.
        if (bot->GetExactDist2d(boss) < ULDUAR_MIMIRON_BARRAGE_BOUNDARY)
        {
            float const bearing = boss->GetAngle(bot);
            bot->CastStop();
            MoveTo(boss->GetMapId(), boss->GetPositionX() + radius * cos(bearing),
                   boss->GetPositionY() + radius * sin(bearing), boss->GetPositionZ(), false, false, false,
                   true, MovementPriority::MOVEMENT_FORCED, true);
            return true;
        }

        // Clear of the beams but in fire, where the fire dodge is boxed in by band and fire: walk the ring to
        // the nearest clean ground in the sector. None leaves the tick to that dodge.
        if (!IsMimironSpotFireSafe(hazards, bot->GetPosition()))
        {
            // Clockwise raises cw toward the trailing side, counter-clockwise lowers it toward the far edge.
            float const bearing = boss->GetAngle(bot);
            float const cwOffset = fireFreeOffset(bearing, -1.0f, twoPi - clearance - cw);
            float const ccwOffset = fireFreeOffset(bearing, 1.0f, cw - window.sweep - clearance);
            if (cwOffset >= 0.0f && (ccwOffset < 0.0f || cwOffset <= ccwOffset))
                return stepRing(-cwOffset);

            if (ccwOffset >= 0.0f)
                return stepRing(ccwOffset);
        }

        // Clear of the band, so only the radius can be wrong: a melee bot left out wide steps back in on the
        // bearing it holds. Same bearing, same safety.
        if (melee && bot->GetExactDist2d(boss) > radius + 1.0f)
        {
            float const bearing = boss->GetAngle(bot);
            Position const inward(boss->GetPositionX() + radius * cos(bearing),
                                  boss->GetPositionY() + radius * sin(bearing),
                                  boss->GetPositionZ());

            if (IsMimironSpotMineSafe(bot, inward) && IsMimironSpotFireSafe(hazards, inward) &&
                IsMimironSpotBombSafe(hazards, inward) && IsMimironSpotRocketSafe(botAI, inward))
            {
                bot->CastStop();
                MoveTo(boss->GetMapId(), inward.GetPositionX(), inward.GetPositionY(),
                       inward.GetPositionZ(), false, false, false, true,
                       MovementPriority::MOVEMENT_FORCED, true);
                return true;
            }
        }

        return false;
    }

    // Direction by time inside the cone, not distance: the short way round often crosses all 104 degrees.
    // `cw` falls with the sweep: clockwise raises it at turnRate - rate, counter-clockwise drops it at turnRate + rate.
    float const turnRate = radius > 0.0f ? bot->GetSpeed(MOVE_RUN) / radius : 0.0f;
    float const half = ULDUAR_MIMIRON_BARRAGE_HALF_ANGLE;

    // Zero while Spinning Up: the boss aims once and holds for 4 s, so the band is fixed and neither
    // direction gets help from the sweep.
    float const sweepRate = window.untilLive > 0.0f ? 0.0f : window.rate;

    bool goClockwise = false;
    if (cw <= window.sweep + clearance)
    {
        if (cw > half)
        {
            // The beams have not reached this bearing yet. Clockwise keeps it that way and costs
            // nothing; turning back would walk the bot into a cone it is currently in front of.
            goClockwise = true;
        }
        else if (window.untilLive > 0.0f && turnRate > 0.0f &&
                 (cw + clearance) / turnRate <= window.untilLive - ULDUAR_MIMIRON_BARRAGE_IGNITION_BUFFER)
        {
            // Time to clear the trailing side before ignition, and nothing follows a bot behind the beams. The
            // nearer leading edge means outrunning the sweep round the whole band, to a far edge that may burn.
        }
        else
        {
            // Leave by whichever edge is sooner: counter-clockwise rides the sweep out the trailing edge,
            // clockwise pushes out the leading edge against it. Crossover near 30 degrees off the centreline.
            float const outCcw = (cw + half) / (turnRate + sweepRate);
            float const outCw = turnRate > sweepRate
                                    ? (half - cw) / (turnRate - sweepRate)
                                    : std::numeric_limits<float>::max();
            goClockwise = outCw < outCcw;
        }
    }
    // Otherwise the bot sits counter-clockwise of the centreline, where the sweep is already carrying
    // it clear. Counter-clockwise, and it is out in a fraction of a second.

    float const ccwTravel = Position::NormalizeOrientation(cw + clearance);
    float const cwTravel = Position::NormalizeOrientation(window.sweep + clearance - cw);
    float remaining = goClockwise ? -cwTravel : ccwTravel;

    // Fire on the band edge pins a bot between beams and fire, so carry on into the sector to the first
    // clean ground. None anywhere: stop at the edge.
    if (!hazards.flames.empty())
    {
        float const direction = goClockwise ? -1.0f : 1.0f;
        float const beyond = fireFreeOffset(boss->GetAngle(bot) + remaining, direction, sectorWidth);
        if (beyond > 0.0f)
            remaining += direction * beyond;
    }

    // Under a yard of travel left; call it arrived rather than issuing a move nothing can act on.
    if (std::fabs(remaining) * radius < 1.0f)
        return false;

    return stepRing(remaining);
}

bool MimironArcSpreadAction::isUseful()
{
    MimironArcSpreadTrigger mimironArcSpreadTrigger(botAI);
    return mimironArcSpreadTrigger.IsActive();
}

bool MimironArcSpreadAction::Execute(Event /*event*/)
{
    Position slot;
    if (!GetMimironSpreadSlot(botAI, bot, slot))
        return false;

    // The tank's spot is under the mech that laid the mines, so it walks straight there.
    if (IsMimironTankAnchorSlot(botAI, bot))
        return MoveTo(bot->GetMapId(), slot.GetPositionX(), slot.GetPositionY(), slot.GetPositionZ(),
                      false, false, false, true, MovementPriority::MOVEMENT_COMBAT, true);

    // Pathing knows nothing of mines, rocket markers or fire, so every leg is screened here; the next tick
    // replans from wherever the leg ended.
    for (MimironApproach const& approach :
         GetMimironSlotApproaches(botAI, bot, slot, GetMimironFirefighterHazards(botAI)))
    {
        // Only a candidate with no path hands over to the next. Already walking a leg, or held by
        // the movement lock, has to stay put, or the bot swaps waypoints mid leg.
        MoveOutcome const outcome =
            TryMoveTo(bot->GetMapId(), approach.dest.GetPositionX(), approach.dest.GetPositionY(),
                      approach.dest.GetPositionZ(), false, false, false, true,
                      MovementPriority::MOVEMENT_COMBAT, true);
        if (outcome == MoveOutcome::NoPath)
            continue;

        return outcome == MoveOutcome::Issued;
    }

    return false;
}

bool MimironAerialCommandUnitAction::Execute(Event /*event*/)
{
    if (PlayerbotAI::IsMainTank(bot) || PlayerbotAI::IsAssistTankOfIndex(bot, 0))
    {
        Unit* assaultBot = GetUldUnitByEntry(botAI, NPC_ASSAULT_BOT);
        Unit* boss = GetUldUnitByEntry(botAI, NPC_AERIAL_COMMAND_UNIT);

        // For the human raid leader only, nothing reads it back: bots pick their own target through
        // "mimiron set dps priority".
        Unit* focus = assaultBot ? assaultBot : boss;
        if (focus && IsMechanicTrackerBot(bot, ULDUAR_MAP_ID))
            MarkTargetWithSkull(bot, focus);
    }

    return false;
}

bool MimironRocketStrikeAction::isUseful()
{
    MimironRocketStrikeTrigger mimironRocketStrikeTrigger(botAI);
    return mimironRocketStrikeTrigger.IsActive();
}

bool MimironRocketStrikeAction::Execute(Event /*event*/)
{
    Creature* rocketStrikeN = bot->FindNearestCreature(NPC_ROCKET_STRIKE_N, ULDUAR_MIMIRON_ROOM_SEARCH_RANGE);
    if (!rocketStrikeN)
        return false;

    // Per-bearing hop onto the clearance circle, not a flat run. FORCED so the arc spread leg the bot is
    // usually mid-way through can't swallow the dodge.
    float const gap = ULDUAR_MIMIRON_ROCKET_CLEARANCE - bot->GetExactDist2d(rocketStrikeN);
    return MoveAwayClearOfMines(rocketStrikeN, gap, MovementPriority::MOVEMENT_FORCED, true, true,
                                ULDUAR_MIMIRON_ROCKET_CLEARANCE);
}

bool MimironPhase4FocusAction::Execute(Event /*event*/)
{
    if (!IsMimironPhase4(bot))
        return false;

    if (PlayerbotAI::IsMainTank(bot))
    {
        // Always the MK II and never a hold: healing and splash keep building threat on it at the floor, and
        // a tank that stops swinging loses it to a DPS.
        Unit* const focus = GetMimironPhase4TankFocus(botAI, bot);
        if (focus && AI_VALUE(Unit*, "current target") != focus)
            return Attack(focus);

        return false;
    }

    // Hand Pulse re-aims every 1.75 s, so there's nothing to sidestep; only not bunching up behind its
    // target helps.
    if (AI_VALUE(float, "disperse distance") != ULDUAR_MIMIRON_PHASE4_DISPERSE_DISTANCE)
        SET_AI_VALUE(float, "disperse distance", ULDUAR_MIMIRON_PHASE4_DISPERSE_DISTANCE);

    // Never claims the tick: "combat formation move" is what reads the value, and it sits below
    // ACTION_RAID.
    return false;
}

bool MimironProximityMineAction::isUseful()
{
    MimironProximityMineTrigger mimironProximityMineTrigger(botAI);
    return mimironProximityMineTrigger.IsActive();
}

bool MimironProximityMineAction::Execute(Event /*event*/)
{
    Unit* nearest = nullptr;
    float nearestDist = ULDUAR_MIMIRON_MINE_TRIGGER_RADIUS;

    // Mines are non-selectable, so they only show up on the raw nearby-npc list.
    for (auto const& guid : AI_VALUE(GuidVector, "nearest npcs"))
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || unit->GetEntry() != NPC_PROXIMITY_MINE)
            continue;

        float const dist = bot->GetExactDist2d(unit);
        if (dist < nearestDist)
        {
            nearestDist = dist;
            nearest = unit;
        }
    }

    if (!nearest)
        return false;

    // Just past the 3 yd blast, capped so a fresh field means a sidestep, not a run across the room. No
    // unfiltered fallback: one mine for another isn't worth the travel, and one blast is healable.
    float const step = std::min(ULDUAR_MIMIRON_MINE_CLEARANCE + 1.0f - nearestDist,
                                ULDUAR_MIMIRON_MINE_MAX_STEP);
    return MoveAwayClearOfMines(nearest, std::max(step, ULDUAR_MIMIRON_MINE_MIN_STEP),
                                MovementPriority::MOVEMENT_COMBAT, false, false);
}

bool MimironPetControlAction::isUseful()
{
    MimironPetControlTrigger mimironPetControlTrigger(botAI);
    return mimironPetControlTrigger.IsActive();
}

bool MimironPetControlAction::Execute(Event /*event*/)
{
    // PetAttackAction's node is commented out globally, so a pet keeps whatever it last latched onto
    // unless the fight redirects it.
    if (IsMimironPhase4(bot))
    {
        // Pets follow the melee focus and hold when melee hold: a pet leaking damage into the rendezvous is
        // what the floor stops. The ACU is reachable here, its seat offsets are zero.
        if (Unit* focus = GetMimironPhase4Focus(botAI, bot, true))
            CommandPetAttack(botAI, focus);
        else
            StopPet(botAI);

        return false;
    }

    // Phase 3, with a Magnetic Core down: 20 s of a stationary boss taking +50% and nothing new spawning for
    // 25, the one window the phase can be shortened in.
    if (IsMimironAcuGrounded(botAI))
    {
        if (Unit* aerialCommandUnit = GetUldUnitByEntry(botAI, NPC_AERIAL_COMMAND_UNIT))
        {
            CommandPetAttack(botAI, aerialCommandUnit);
            return false;
        }
    }

    // Phase 3 otherwise: a ground pet can't reach the unit hovering 15 yd up. Assault Bot first, the only
    // Magnetic Core source.
    for (uint32 entry : {NPC_ASSAULT_BOT, NPC_JUNK_BOT, NPC_BOMB_BOT})
    {
        Unit* nearest = nullptr;
        float nearestDist = 0.0f;

        for (ObjectGuid const& guid : AI_VALUE(GuidVector, "possible targets no los"))
        {
            Unit* unit = botAI->GetUnit(guid);
            if (!unit || !unit->IsAlive() || unit->GetEntry() != entry)
                continue;

            // Nearest, not first: the pet has to run there, and the raw list is in no useful order.
            float const dist = bot->GetExactDist2d(unit);
            if (!nearest || dist < nearestDist)
            {
                nearest = unit;
                nearestDist = dist;
            }
        }

        if (nearest)
        {
            CommandPetAttack(botAI, nearest);
            return false;
        }
    }

    StopPet(botAI);

    // Always false: this only redirects the pet, and the bot still needs the tick for its own turn.
    return false;
}

bool MimironApproachTargetAction::Execute(Event /*event*/)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || !target->IsAlive())
        return false;

    Position approach;
    if (!GetMimironTargetApproach(botAI, bot, target, GetMimironApproachRange(botAI, bot), approach))
        return false;

    // MOVEMENT_COMBAT, like the formation: every hazard dodge issues FORCED and has to be able to
    // take the tick off this one mid-walk.
    return TryMoveTo(bot->GetMapId(), approach.GetPositionX(), approach.GetPositionY(), approach.GetPositionZ(),
                     false, false, false, true, MovementPriority::MOVEMENT_COMBAT, true) == MoveOutcome::Issued;
}

bool MimironDodgeFlamesAction::isUseful()
{
    MimironDodgeFlamesTrigger mimironDodgeFlamesTrigger(botAI);
    return mimironDodgeFlamesTrigger.IsActive();
}

bool MimironDodgeFlamesAction::Execute(Event /*event*/)
{
    // Flee the burning cluster's centre, not the nearest node, so the bot leaves the cluster instead of
    // stepping into the next node along.
    MimironFirefighterHazards const hazards = GetMimironFirefighterHazards(botAI);

    std::vector<Position> cluster;
    for (Position const& node : hazards.flames)
        if (bot->GetExactDist2d(node.GetPositionX(), node.GetPositionY()) < ULDUAR_MIMIRON_FLAMES_RADIUS)
            cluster.push_back(node);

    if (cluster.empty())
        return false;

    float cx = 0.0f, cy = 0.0f;
    for (Position const& node : cluster)
    {
        cx += node.GetPositionX();
        cy += node.GetPositionY();
    }
    cx /= cluster.size();
    cy /= cluster.size();

    Position const centre(cx, cy, bot->GetPositionZ());
    float spread = 0.0f;
    for (Position const& node : cluster)
        spread = std::max(spread, centre.GetExactDist2d(node.GetPositionX(), node.GetPositionY()));

    // Shortest rung that lands clean, which saves the walk back; the fan screens each against the whole
    // field. Only the last may fall back unscreened.
    for (size_t rung = 0; rung < std::size(ULDUAR_MIMIRON_FLAMES_STEP_LADDER); ++rung)
    {
        float const step = ULDUAR_MIMIRON_FLAMES_STEP_LADDER[rung];
        bool const last = rung + 1 == std::size(ULDUAR_MIMIRON_FLAMES_STEP_LADDER);

        // FORCED: at COMBAT it ties with the formation, and IsWaitingForLastMove wants strictly higher.
        // Capped, since a FORCED leg blocks the Frost Bomb and Rocket Strike dodges while it runs.
        float const hop = std::min(ULDUAR_MIMIRON_FLAMES_RADIUS + spread + step,
                                   ULDUAR_MIMIRON_FLAMES_MAX_HOP);

        if (MoveAwayClearOfMines(centre, hop, MovementPriority::MOVEMENT_FORCED, last, true))
        {
            NoteMimironFireDodge(bot);
            return true;
        }
    }

    return false;
}

bool MimironFireBotAction::isUseful()
{
    MimironFireBotTrigger mimironFireBotTrigger(botAI);
    return mimironFireBotTrigger.IsActive();
}

bool MimironFireBotAction::Execute(Event /*event*/)
{
    MimironFirefighterHazards const hazards = GetMimironFirefighterHazards(botAI);

    // The spray lanes first, since they hurt. Out the near side: running away along a lane stays in
    // it for 15 yd.
    for (Position const& lane : hazards.sprayLanes)
    {
        float const dx = bot->GetPositionX() - lane.GetPositionX();
        float const dy = bot->GetPositionY() - lane.GetPositionY();
        float const facing = lane.GetOrientation();
        float const ahead = dx * std::cos(facing) + dy * std::sin(facing);
        float const side = dy * std::cos(facing) - dx * std::sin(facing);
        if (ahead < 0.0f || ahead >= ULDUAR_MIMIRON_FIREBOT_SPRAY_LENGTH ||
            std::fabs(side) >= ULDUAR_MIMIRON_FIREBOT_SPRAY_HALF_WIDTH)
            continue;

        float const step =
            ULDUAR_MIMIRON_FIREBOT_SPRAY_HALF_WIDTH + ULDUAR_MIMIRON_FIREBOT_SPRAY_SIDESTEP - std::fabs(side);
        float const out = side >= 0.0f ? 1.0f : -1.0f;
        Position const dest(bot->GetPositionX() - out * step * std::sin(facing),
                            bot->GetPositionY() + out * step * std::cos(facing), bot->GetPositionZ());

        // Fire-tolerant: Water Spray is an instant 23-26k against a 22-24k pool, a fire node 3.1k a second.
        return MoveTowardClearOfMines(dest, MovementPriority::MOVEMENT_FORCED, true, true, true);
    }

    // Otherwise the silence. No interrupt: losing a cast to step out of a silence defeats the point.
    Position const* nearest = nullptr;
    float nearestDist = ULDUAR_MIMIRON_FIREBOT_SIREN_CLEARANCE;
    for (Position const& fireBot : hazards.fireBots)
    {
        float const dist = bot->GetExactDist2d(fireBot.GetPositionX(), fireBot.GetPositionY());
        if (dist < nearestDist)
        {
            nearest = &fireBot;
            nearestDist = dist;
        }
    }

    if (!nearest ||
        IsMimironSpotFireBotSafe(bot, hazards, bot->GetPosition(), ULDUAR_MIMIRON_FIREBOT_SIREN_CLEARANCE,
                                 ULDUAR_MIMIRON_FIREBOT_SPRAY_HALF_WIDTH))
        return false;

    // Every bearing lands on the flee circle, past where the formation and the other dodges are
    // allowed to put the bot back, so the fire bot has to walk a few yards before it comes up again.
    return MoveAwayClearOfMines(*nearest, ULDUAR_MIMIRON_FIREBOT_SIREN_FLEE - nearestDist,
                                MovementPriority::MOVEMENT_FORCED, true, false, ULDUAR_MIMIRON_FIREBOT_SIREN_FLEE);
}

bool MimironFrostBombAction::isUseful()
{
    MimironFrostBombTrigger mimironFrostBombTrigger(botAI);
    return mimironFrostBombTrigger.IsActive();
}

bool MimironFrostBombAction::Execute(Event /*event*/)
{
    Creature* frostBomb = bot->FindNearestCreature(NPC_FROST_BOMB, ULDUAR_MIMIRON_FROST_BOMB_RADIUS);
    if (!frostBomb)
        return false;

    // FORCED, or a formation leg holds the lock past the fuse; unfiltered fallback, since 30 yd can leave no
    // clean bearing. Sector screen: straight away from a far-side bomb runs melee through VX-001 into the wedge's line.
    float const gap = ULDUAR_MIMIRON_FROST_BOMB_CLEARANCE - bot->GetExactDist2d(frostBomb);
    return MoveAwayClearOfMines(frostBomb, gap, MovementPriority::MOVEMENT_FORCED, true, true,
                                ULDUAR_MIMIRON_FROST_BOMB_CLEARANCE, true);
}

std::vector<std::pair<uint32, Unit*>> MimironSetDpsPriorityAction::BuildPriorityList()
{
    Unit* leviathanMkII = nullptr;
    Unit* vx001 = nullptr;
    Unit* aerialCommandUnit = nullptr;
    std::vector<Unit*> bombBots;
    std::vector<Unit*> assaultBots;
    std::vector<Unit*> fireBots;
    std::vector<Unit*> junkBots;

    for (ObjectGuid const& guid :
         botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get())
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        switch (unit->GetEntry())
        {
            case NPC_LEVIATHAN_MKII:      leviathanMkII = unit; break;
            case NPC_VX001:               vx001 = unit; break;
            case NPC_AERIAL_COMMAND_UNIT: aerialCommandUnit = unit; break;
            case NPC_BOMB_BOT:            bombBots.push_back(unit); break;
            case NPC_ASSAULT_BOT:         assaultBots.push_back(unit); break;
            case NPC_JUNK_BOT:            junkBots.push_back(unit); break;
            case NPC_EMERGENCY_FIRE_BOT:
                // Left out rather than refused later, or the nearest one being a kept one would hide
                // a cull standing further away.
                if (!IsMimironFireBotProtected(botAI, bot, unit))
                    fireBots.push_back(unit);
                break;
            default: break;
        }
    }

    Unit* currentTarget = AI_VALUE(Unit*, "current target");

    // Bomb Bots can't be outrun (8.0 yd/s against 7.0), so ranged kill them, but only in casting range: a chase
    // deadlocks "reach spell" against the formation. Assault Bot before the other adds, the only Magnetic Core source.
    std::vector<std::pair<uint32, Unit*>> priority;
    if (PlayerbotAI::IsRangedDps(bot))
    {
        std::vector<Unit*> reachable;
        for (Unit* bombBot : bombBots)
            if (bot->GetExactDist2d(bombBot) <= sPlayerbotAIConfig.SpellDistance)
                reachable.push_back(bombBot);

        priority.emplace_back(NPC_BOMB_BOT, SelectByEntry(currentTarget, NPC_BOMB_BOT, reachable));
    }

    // Unkept fire bots, two tiers for ranged: the cleanup sweep up here, as none may reach phase 4, and a
    // handover stray behind the mech. Melee take them after the Assault Bot. Nobody culls in phase 4.
    bool const hardMode = IsMimironHardModeActive(botAI);
    bool const phase4 = IsMimironPhase4(bot);
    bool const cullFireBots = hardMode && !phase4;
    bool const fireBotSweep =
        cullFireBots && aerialCommandUnit &&
        aerialCommandUnit->GetHealthPct() <= ULDUAR_MIMIRON_FIREBOT_CLEANUP_PCT;
    if (fireBotSweep && PlayerbotAI::IsRangedDps(bot))
        priority.emplace_back(NPC_EMERGENCY_FIRE_BOT,
                              SelectByEntry(currentTarget, NPC_EMERGENCY_FIRE_BOT, fireBots));

    // A grounded ACU outranks the adds for melee and ranged DPS: +50% taken and nothing spawns for the
    // window. A Bomb Bot in casting range still comes first.
    if (aerialCommandUnit && (PlayerbotAI::IsMelee(bot) || PlayerbotAI::IsRangedDps(bot)) &&
        IsMimironAcuGrounded(botAI))
        priority.emplace_back(NPC_AERIAL_COMMAND_UNIT, aerialCommandUnit);

    // Melee take the Junk Bot pile once it builds: they stand in it and their cleave covers several. Once
    // on one they finish it, so a dip under the bar leaves no half-dead ones.
    if (PlayerbotAI::IsMelee(bot) && (junkBots.size() >= ULDUAR_MIMIRON_JUNK_BOT_PILE ||
                                (currentTarget && currentTarget->GetEntry() == NPC_JUNK_BOT)))
        priority.emplace_back(NPC_JUNK_BOT, SelectByEntry(currentTarget, NPC_JUNK_BOT, junkBots));

    priority.emplace_back(NPC_ASSAULT_BOT, SelectByEntry(currentTarget, NPC_ASSAULT_BOT, assaultBots));

    if (cullFireBots && !PlayerbotAI::IsRangedDps(bot))
        priority.emplace_back(NPC_EMERGENCY_FIRE_BOT,
                              SelectByEntry(currentTarget, NPC_EMERGENCY_FIRE_BOT, fireBots));

    priority.emplace_back(NPC_JUNK_BOT, SelectByEntry(currentTarget, NPC_JUNK_BOT, junkBots));

    // Phase 4: all three must reach Self Repair inside its 15 s cast, so they come down level. One helper
    // answers for this, the tank node and the pets, so the three agree.
    if (phase4)
    {
        if (Unit* focus = GetMimironPhase4Focus(botAI, bot, PlayerbotAI::IsMelee(bot)))
            priority.emplace_back(focus->GetEntry(), focus);

        return priority;
    }

    // Outside phase 4 exactly one mech is up at a time, so there is nothing to order.
    for (Unit* mech : {leviathanMkII, vx001, aerialCommandUnit})
        if (mech)
            priority.emplace_back(mech->GetEntry(), mech);

    if (cullFireBots && !fireBotSweep && PlayerbotAI::IsRangedDps(bot))
        priority.emplace_back(NPC_EMERGENCY_FIRE_BOT,
                              SelectByEntry(currentTarget, NPC_EMERGENCY_FIRE_BOT, fireBots));

    return priority;
}

Unit* MimironSetDpsPriorityAction::SelectByEntry(Unit* currentTarget, uint32 entry,
                                                std::vector<Unit*> const& candidates) const
{
    Unit* selected = nullptr;
    if (currentTarget && currentTarget->IsAlive() && currentTarget->GetEntry() == entry)
        selected = currentTarget;

    // Adds walk in from pads all round the room, so nearest-to-the-bot beats measuring from an
    // anchor. The margin stops two similar adds trading the bot back and forth every tick.
    constexpr float switchMargin = 10.0f;
    for (Unit* candidate : candidates)
    {
        if (!candidate || candidate == selected)
            continue;

        if (!selected)
        {
            selected = candidate;
            continue;
        }

        if (candidate->GetExactDist2d(bot) + switchMargin < selected->GetExactDist2d(bot))
            selected = candidate;
    }

    return selected;
}

bool MimironSetDpsPriorityAction::IsAllowedTarget(Unit* unit) const
{
    if (!unit || !unit->IsAlive())
        return false;

    switch (unit->GetEntry())
    {
        case NPC_BOMB_BOT:
            // Melee can't reach one without setting it off. Ranged shoot from outside the blast, or inside it
            // when it's chasing them: at 8.0 yd/s against 7.0 they can't leave.
            if (PlayerbotAI::IsMelee(bot))
                return false;

            return unit->GetExactDist2d(bot) >= ULDUAR_MIMIRON_BOMB_BOT_RADIUS ||
                   GetMimironBombBotChasing(botAI, bot) == unit;

        case NPC_AERIAL_COMMAND_UNIT:
        {
            if (!PlayerbotAI::IsMelee(bot))
                return true;

            // Melee only while a core grounds it in phase 3, and in phase 4 when the focus hands it over or a
            // part is self-repairing. Not via MOVEMENTFLAG_HOVER: it survives into phase 4.
            if (!IsMimironPhase4(bot))
                return IsMimironAcuGrounded(botAI);

            return GetUldUnitByEntry(botAI, NPC_LEVIATHAN_MKII) == nullptr ||
                   GetUldUnitByEntry(botAI, NPC_VX001) == nullptr ||
                   GetMimironPhase4Focus(botAI, bot, true) == unit;
        }

        // The kept ones are putting the fire out. Refused here too, so the hold cannot keep a bot
        // on one it was already hitting.
        case NPC_EMERGENCY_FIRE_BOT:
            return !IsMimironFireBotProtected(botAI, bot, unit);

        default:
            return true;
    }
}

Unit* MimironSetDpsPriorityAction::ResolveTarget(Unit* currentTarget, bool allowFallback)
{
    std::vector<std::pair<uint32, Unit*>> const priority = BuildPriorityList();

    Unit* target = nullptr;
    for (auto const& candidate : priority)
    {
        if (IsAllowedTarget(candidate.second))
        {
            target = candidate.second;
            break;
        }
    }

    auto const priorityIndex = [&](Unit* unit) -> size_t
    {
        if (!IsAllowedTarget(unit))
            return priority.size();

        for (size_t index = 0; index < priority.size(); ++index)
        {
            if (priority[index].first == unit->GetEntry())
                return index;
        }

        return priority.size();
    };

    // Hold the current target unless something strictly more urgent is up, or Junk Bot churn keeps
    // resetting swing timers. Allowed only: with nothing allowed both indexes tie, keeping melee on the airborne ACU.
    if (currentTarget && IsAllowedTarget(currentTarget) &&
        priorityIndex(currentTarget) <= priorityIndex(target))
        target = currentTarget;

    if (target)
        return target;

    if (!allowFallback)
        return nullptr;

    // Nothing on the list is allowed, so the generic picker answers.
    return AI_VALUE(Unit*, "dps target");
}

bool MimironSetDpsPriorityAction::Execute(Event /*event*/)
{
    Unit* currentTarget = AI_VALUE(Unit*, "current target");

    // Phase 4 hold: all this bot may hit is at the floor. Holding the tick stops the class strategies
    // re-engaging; dodges above ACTION_RAID still run. Never a healer, it would stop healing.
    if (IsMimironPhase4(bot) && !PlayerbotAI::IsHeal(bot) &&
        !GetMimironPhase4Focus(botAI, bot, PlayerbotAI::IsMelee(bot)))
    {
        bot->AttackStop();
        return true;
    }

    // Phase 3 melee wait while the unit is airborne with no add up, or the generic picker hands them the
    // unit and "reach melee" walks them into the fire under it. Never a healer.
    bool const phase3Wait =
        PlayerbotAI::IsMelee(bot) && !PlayerbotAI::IsHeal(bot) && IsMimironAcuAirborne(botAI, bot);

    Unit* target = ResolveTarget(currentTarget, !phase3Wait);
    if (!target)
    {
        if (!phase3Wait)
            return false;

        bot->AttackStop();
        return true;
    }

    // Returning false once the bot is already on the right target is what lets the lower-priority
    // nodes run at all: the engine ends the tick at the first action that succeeds.
    bool needsAttack = currentTarget != target;
    if (PlayerbotAI::IsMelee(bot))
        needsAttack = needsAttack || !bot->HasUnitState(UNIT_STATE_MELEE_ATTACKING);

    return needsAttack ? Attack(target) : false;
}

bool MimironPlasmaBlastDefensiveAction::isUseful()
{
    MimironPlasmaBlastDefensiveTrigger mimironPlasmaBlastDefensiveTrigger(botAI);
    return mimironPlasmaBlastDefensiveTrigger.IsActive();
}

Unit* MimironPlasmaBlastDefensiveAction::PlasmaVictim()
{
    // The cannon, an MK II passenger, is the caster. Read the victim off the spell: it's fixed at cast
    // start, and the mech may have retargeted since.
    Creature* cannon = bot->FindNearestCreature(NPC_LEVIATHAN_MKII_CANNON, ULDUAR_MIMIRON_ROOM_SEARCH_RANGE);
    if (!cannon)
        return nullptr;

    Spell* spell = GetMimironPlasmaBlastCast(cannon);
    if (!spell)
        return nullptr;

    Unit* victim = spell->m_targets.GetUnitTarget();
    return victim && victim->IsAlive() ? victim : nullptr;
}

bool MimironPlasmaBlastDefensiveAction::TankHasDefensive(Unit* victim)
{
    Player* tank = victim ? victim->ToPlayer() : nullptr;
    if (!tank)
        return false;

    PlayerbotAI* tankAI = GET_PLAYERBOT_AI(tank);
    return tankAI && NextTankDefensive(tankAI, tank) != 0;
}

bool MimironPlasmaBlastDefensiveAction::Execute(Event /*event*/)
{
    Unit* victim = PlasmaVictim();
    if (!victim)
        return false;

    if (victim == bot)
    {
        uint32 const defensive = NextTankDefensive(botAI, bot);
        if (!defensive)
            return false;

        return ClaimMimironPlasmaWindow(bot) && botAI->CastSpell(defensive, bot);
    }

    // Tank spends his own first. Checked, not waited for: both nodes run on the same tick at the same
    // relevance, so a healer claiming on sight would beat him half the time.
    if (TankHasDefensive(victim))
        return false;

    for (char const* external : HEALER_EXTERNALS)
    {
        if (!botAI->CanCastSpell(external, victim))
            continue;

        if (!ClaimMimironPlasmaWindow(bot))
            return false;

        return botAI->CastSpell(external, victim);
    }

    return false;
}

Player* MimironRedirectThreatAction::GetRedirectTank()
{
    // Phase 1 only. Later phases split two mechs across two tanks, and a redirect that names the
    // group main tank by definition feeds the wrong one.
    if (!GetUldUnitByEntry(botAI, NPC_LEVIATHAN_MKII) ||
        GetUldUnitByEntry(botAI, NPC_VX001) ||
        GetUldUnitByEntry(botAI, NPC_AERIAL_COMMAND_UNIT))
        return nullptr;

    Unit* mainTank = AI_VALUE(Unit*, "main tank");
    return mainTank ? mainTank->ToPlayer() : nullptr;
}

Unit* MimironRedirectThreatAction::GetThreatDumpTarget()
{
    return GetUldUnitByEntry(botAI, NPC_LEVIATHAN_MKII);
}

bool MimironSlowBombBotAction::isUseful()
{
    MimironSlowBombBotTrigger mimironSlowBombBotTrigger(botAI);
    return mimironSlowBombBotTrigger.IsActive();
}

bool MimironSlowBombBotAction::Execute(Event event)
{
    std::string const spell = GetMimironBombBotSnare(bot);
    if (spell.empty())
        return false;

    // Returns its own result rather than holding the tick: a bot that does not have the spell, or has
    // it on cooldown, should drop straight through to its rotation and keep shooting.
    return botAI->DoSpecificAction(spell, event, true);
}

bool MimironMagneticCoreAction::isUseful()
{
    MimironMagneticCoreTrigger mimironMagneticCoreTrigger(botAI);
    return mimironMagneticCoreTrigger.IsActive();
}

bool MimironMagneticCoreAction::Execute(Event /*event*/)
{
    Unit* aerialCommandUnit = GetUldUnitByEntry(botAI, NPC_AERIAL_COMMAND_UNIT);
    if (!aerialCommandUnit)
        return false;

    uint32 const held = bot->GetItemCount(ITEM_MIMIRON_MAGNETIC_CORE, false);
    Item* core = bot->GetItemByEntry(ITEM_MIMIRON_MAGNETIC_CORE);

    // 46029 has item_template duration 60, so a banked core expires in the bags; no duration reads 0. With
    // the unit this low the phase ends before another core could be spent.
    uint32 const remaining = core ? core->GetUInt32Value(ITEM_FIELD_DURATION) : 0;
    bool const expiring = remaining && remaining * IN_MILLISECONDS <= ULDUAR_MIMIRON_CORE_HOLD_EXPIRY_MS;
    bool const acuLow = aerialCommandUnit->GetHealthPct() <= ULDUAR_MIMIRON_CORE_HOLD_RELEASE_PCT;

    // One core per landing. A second one while the first is live makes the unit climb with the aura
    // still on and pushes every add timer back another 25 s.
    bool const ready = IsMimironCoreUseReady(botAI, bot);

    // The chain is spent once the bags are empty with nothing on the floor. Not on empty bags alone:
    // a core looted while the last landing runs still follows it down.
    if (!core && ready)
        chaining = false;

    // Wait under the unit, not where the last corpse fell, so a release costs the 2 s climb, not a walk.
    // False once there, so the rest of the tick still runs.
    auto const waitUnderAcu = [&]()
    {
        if (bot->GetExactDist2d(aerialCommandUnit) <= ULDUAR_MIMIRON_CORE_USE_RANGE)
            return false;

        return MoveTo(aerialCommandUnit->GetMapId(), aerialCommandUnit->GetPositionX(),
                      aerialCommandUnit->GetPositionY(), bot->GetPositionZ(), false, false, false,
                      true, MovementPriority::MOVEMENT_COMBAT, true);
    };

    // Collect up to the bank first. Not while the held core is expiring or the phase is ending, or the
    // chase loses the core in hand; nor while a chain has one to spend now.
    if (held < ULDUAR_MIMIRON_CORE_BANK && !expiring && !acuLow && !(chaining && core && ready))
    {
        Creature* corpse = GetMimironCoreCorpse(bot);
        if (!corpse && !core)
            return false;

        if (corpse)
        {
            // Walk to it: the corpse lasts 25 s and lies wherever the raid stopped the Assault Bot.
            if (bot->GetExactDist2d(corpse) > ULDUAR_MIMIRON_CORE_LOOT_RANGE)
                return MoveTo(corpse->GetMapId(), corpse->GetPositionX(), corpse->GetPositionY(),
                              corpse->GetPositionZ(), false, false, false, true,
                              MovementPriority::MOVEMENT_COMBAT, true);

            // Bots can't loot in combat and drop white 46029 as junk anyway, so it's handed over, gated like a
            // player: corpse killed, stood on, bag room. Once per corpse, or every use would refill it.
            if (TakeMimironCore(bot, corpse))
                return true;

            if (!core)
                return false;
        }
    }

    if (!core)
        return false;

    if (!ready)
        return waitUnderAcu();

    // Chain the bank for one unbroken 40 s on the floor. Out early on expiry, a low unit or the next
    // Assault Bot up.
    if (!chaining && held < ULDUAR_MIMIRON_CORE_BANK && !acuLow && !expiring &&
        !bot->FindNearestCreature(NPC_ASSAULT_BOT, ULDUAR_MIMIRON_CORE_SEARCH_RANGE, true))
        return waitUnderAcu();

    // 64444 places its summon by nearest entry, so the core only reaches the ACU from underneath it.
    if (bot->GetExactDist2d(aerialCommandUnit) > ULDUAR_MIMIRON_CORE_USE_RANGE)
        return MoveTo(aerialCommandUnit->GetMapId(), aerialCommandUnit->GetPositionX(),
                      aerialCommandUnit->GetPositionY(), bot->GetPositionZ(), false, false, false,
                      true, MovementPriority::MOVEMENT_COMBAT, true);

    if (bot->CanUseItem(core) != EQUIP_ERR_OK || bot->IsNonMeleeSpellCast(false))
        return false;

    uint32 spellId = 0;
    for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        if (core->GetTemplate()->Spells[i].SpellId > 0)
        {
            spellId = core->GetTemplate()->Spells[i].SpellId;
            break;
        }
    }

    if (!spellId)
        return false;

    uint8 const bagIndex = core->GetBagSlot();
    uint8 const slot = core->GetSlot();
    constexpr uint8 castCount = 0;
    constexpr uint32 glyphIndex = 0;
    constexpr uint8 castFlags = 0;

    WorldPacket packet(CMSG_USE_ITEM);
    packet << bagIndex;
    packet << slot;
    packet << castCount;
    packet << spellId;
    packet << core->GetGUID();
    packet << glyphIndex;
    packet << castFlags;
    packet << (uint32)TARGET_FLAG_NONE;

    bot->GetSession()->HandleUseItemOpcode(packet);
    NoteMimironCoreSpent(bot);
    chaining = true;
    return true;
}

bool MimironFrostResistanceAction::Execute(Event /*event*/)
{
    return botAI->DoSpecificAction("frost resistance aura", Event(), true);
}
