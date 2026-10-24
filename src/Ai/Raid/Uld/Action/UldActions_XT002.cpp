/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldActions_XT002.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "Group.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "EncounterHelpers.h"
#include "UldData.h"
#include "UldEncounter_XT002.h"
#include "UldHardMode.h"
#include "UldScripts.h"
#include "UldTargets.h"
#include "Unit.h"

using namespace EncounterHelpers;

bool XT002MoveClearAction::isPossible() { return bot->CanFreeMove(); }

XT002MoveClearAction::MoveIssue XT002MoveClearAction::IssueMove(float x, float y, float z)
{
    switch (TryMoveTo(bot->GetMapId(), x, y, z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true))
    {
        case MoveOutcome::NoPath:
            return MoveIssue::NoPath;
        case MoveOutcome::NotAllowed:
            return MoveIssue::Refused;
        default:
            // Issued, or already headed there: Duplicate (same destination) or Waiting (an equal or higher
            // priority command in flight). Re-picking on either slides the bot in place.
            return MoveIssue::Taken;
    }
}

bool XT002MoveClearAction::MoveClearOf(std::vector<std::pair<Unit*, float>> const& avoid)
{
    if (avoid.empty())
        return false;

    // Worst shortfall against any unit's own clearance, capped at zero so every spot clearing the whole list
    // ties and the nearest one wins.
    auto score = [&avoid](float x, float y)
    {
        float worst = std::numeric_limits<float>::max();
        for (auto const& entry : avoid)
            worst = std::min(worst, entry.first->GetExactDist2d(x, y) - entry.second);

        return std::min(worst, 0.0f);
    };

    float searchRadius = 0.0f;
    for (auto const& entry : avoid)
        searchRadius = std::max(searchRadius, entry.second);

    constexpr float searchMargin = 5.0f;
    searchRadius += searchMargin;

    int const directions = 8;
    float const increment = 3.0f;

    // A packed 25-man often has no spot that clears everyone, so take any gain over standing still.
    float const standingScore = score(bot->GetPositionX(), bot->GetPositionY());

    struct Candidate
    {
        float x;
        float y;
        float score;
        float distance;
    };

    std::vector<Candidate> candidates;
    for (int i = 0; i < directions; ++i)
    {
        float const angle = (i * 2 * M_PI) / directions;
        for (float distance = increment; distance <= searchRadius; distance += increment)
        {
            float const moveX = bot->GetPositionX() + distance * cos(angle);
            float const moveY = bot->GetPositionY() + distance * sin(angle);

            // No LOS test: the building geometry ends at y -29, so rays from the raid clip its rim and reject spots
            // whose path is clean.
            float const candidate = score(moveX, moveY);
            if (candidate > standingScore)
                candidates.push_back({moveX, moveY, candidate, distance});
        }
    }

    if (candidates.empty())
        return false;

    // Best clearance first, nearest among equals.
    std::sort(candidates.begin(), candidates.end(), [](Candidate const& left, Candidate const& right)
    {
        if (left.score != right.score)
            return left.score > right.score;
        return left.distance < right.distance;
    });

    size_t const attempts = std::min<size_t>(ULDUAR_XT002_MOVE_CANDIDATE_ATTEMPTS, candidates.size());
    for (size_t i = 0; i < attempts; ++i)
    {
        switch (IssueMove(candidates[i].x, candidates[i].y, bot->GetPositionZ()))
        {
            case MoveIssue::Taken:
                return true;
            case MoveIssue::Refused:
                return false;
            case MoveIssue::NoPath:
                break;
        }
    }

    return false;
}

float XT002DebuffCarrierAction::ApproachClearance(float x, float y, std::list<Creature*> const& voidZones) const
{
    float const fromX = bot->GetPositionX();
    float const fromY = bot->GetPositionY();
    float const runX = x - fromX;
    float const runY = y - fromY;
    float const runLengthSq = runX * runX + runY * runY;

    float nearest = std::numeric_limits<float>::max();
    for (Creature* voidZone : voidZones)
    {
        float projection = 0.0f;
        if (runLengthSq > 0.0f)
        {
            projection = ((voidZone->GetPositionX() - fromX) * runX +
                          (voidZone->GetPositionY() - fromY) * runY) / runLengthSq;
            projection = std::max(0.0f, std::min(1.0f, projection));
        }

        nearest = std::min(nearest,
                           voidZone->GetExactDist2d(fromX + runX * projection, fromY + runY * projection));
    }

    return nearest;
}

bool XT002DebuffCarrierAction::StandingInVoidZone(std::list<Creature*> const& voidZones) const
{
    for (Creature* voidZone : voidZones)
        if (voidZone->GetExactDist2d(bot) < ULDUAR_XT002_VOID_ZONE_RADIUS)
            return true;

    return false;
}

bool XT002DebuffCarrierAction::InsideParkingLot() const
{
    // Each grid is axis-aligned, so its extent is a rectangle. The margin is the re-engage band, which
    // is what the carrier already tolerates as "on a cell".
    float const margin = ULDUAR_XT002_BOMB_CELL_REENGAGE;

    for (XT002BombLot const& lot : GetXT002BombLots(bot))
    {
        float const originX = lot.origin.GetPositionX();
        float const originY = lot.origin.GetPositionY();
        float const farY = originY + lot.yDirection * (lot.yCells - 1) * ULDUAR_XT002_BOMB_GRID_STEP;

        float const minX = originX - margin;
        float const maxX = originX + (lot.xCells - 1) * ULDUAR_XT002_BOMB_GRID_STEP + margin;
        float const minY = std::min(originY, farY) - margin;
        float const maxY = std::max(originY, farY) + margin;

        if (bot->GetPositionX() >= minX && bot->GetPositionX() <= maxX && bot->GetPositionY() >= minY &&
            bot->GetPositionY() <= maxY)
            return true;
    }

    return false;
}

float XT002DebuffCarrierAction::TravelReach(uint32 remainingMs) const
{
    if (remainingMs <= ULDUAR_XT002_BOMB_TRAVEL_MARGIN_MS)
        return 0.0f;

    return bot->GetSpeed(MOVE_RUN) * (remainingMs - ULDUAR_XT002_BOMB_TRAVEL_MARGIN_MS) / IN_MILLISECONDS;
}

bool XT002DebuffCarrierAction::StopShortOf(float cellX, float cellY, float reach,
                                           std::list<Creature*> const& voidZones)
{
    float const runX = cellX - bot->GetPositionX();
    float const runY = cellY - bot->GetPositionY();
    float const runLength = std::sqrt(runX * runX + runY * runY);
    if (runLength <= 0.0f || reach <= ULDUAR_XT002_BOMB_CELL_ARRIVED)
        return false;

    float const scale = std::min(reach, runLength) / runLength;
    float const stopX = bot->GetPositionX() + runX * scale;
    float const stopY = bot->GetPositionY() + runY * scale;
    float const stopZ = bot->GetPositionZ();

    // A carrier that can't make its cell dies to Consumption plus the expiry hit, so the stop point needs a
    // cell's clearance too. Refusing hands the tick to the ring search.
    for (Creature* voidZone : voidZones)
        if (voidZone->GetExactDist2d(stopX, stopY) < ULDUAR_XT002_VOID_ZONE_RADIUS)
            return false;

    // No LOS test either: the bearing into the lot is exactly where rays clip the building rim, and IssueMove
    // already rejects a point the pathfinder won't take.
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Worth stopping here only if the expiry burst misses the raid from it. The pull matters more
    // than the damage: 20 yd of yank drops whoever it catches into the puddle that just landed.
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive())
            continue;

        if (member->GetExactDist2d(stopX, stopY) < ULDUAR_XT002_GRAVITY_BOMB_PULL_RADIUS)
            return false;
    }

    return IssueMove(stopX, stopY, stopZ) == MoveIssue::Taken;
}

float XT002DebuffCarrierAction::RaidClearance(float x, float y) const
{
    Group* group = bot->GetGroup();
    if (!group)
        return std::numeric_limits<float>::max();

    float nearest = std::numeric_limits<float>::max();
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() || member->GetMapId() != bot->GetMapId())
            continue;

        nearest = std::min(nearest, member->GetExactDist2d(x, y));
    }

    return nearest;
}

bool XT002DebuffCarrierAction::MoveToSearingLightSpot(bool heartbreak)
{
    // Nothing drops a puddle before XT carries Heartbreak, so normal mode skips the grid scan entirely.
    std::list<Creature*> voidZones;
    if (heartbreak)
        bot->GetCreatureListWithEntryInGrid(voidZones, PB_NPC_XT002_VOID_ZONE, ULDUAR_XT002_VOID_ZONE_SEARCH_RADIUS);

    auto clearOfPuddles = [&voidZones](float x, float y)
    {
        for (Creature* voidZone : voidZones)
            if (voidZone->GetExactDist2d(x, y) < ULDUAR_XT002_VOID_ZONE_RADIUS)
                return false;

        return true;
    };

    float const spotX = ULDUAR_XT002_SEARING_LIGHT_SPOT.GetPositionX();
    float const spotY = ULDUAR_XT002_SEARING_LIGHT_SPOT.GetPositionY();
    float const spotZ = ULDUAR_XT002_SEARING_LIGHT_SPOT.GetPositionZ();

    // Built once for the spot and all eight alternates, not once per candidate.
    std::vector<Position> const formation = GetXT002OtherFormationSlots(bot);

    auto usable = [&clearOfPuddles, &formation](float x, float y)
    {
        if (!clearOfPuddles(x, y))
            return false;

        for (Position const& slot : formation)
            if (slot.GetExactDist2d(x, y) < ULDUAR_XT002_SEARING_LIGHT_SLOT_CLEARANCE)
                return false;

        return true;
    };

    if (bot->GetExactDist(ULDUAR_XT002_SEARING_LIGHT_SPOT) < 1.0f && usable(spotX, spotY))
        return false;

    // The spot itself, then a ring of alternates around it. The headings that point back at the
    // formation are what the clearance test drops.
    struct Candidate
    {
        float x;
        float y;
        float clearance;
    };

    std::vector<Candidate> candidates;
    if (usable(spotX, spotY))
        candidates.push_back({spotX, spotY, RaidClearance(spotX, spotY)});

    int const directions = 8;
    for (int i = 0; i < directions; ++i)
    {
        float const angle = (i * 2 * M_PI) / directions;
        float const ringX = spotX + ULDUAR_XT002_SEARING_LIGHT_DETOUR * cos(angle);
        float const ringY = spotY + ULDUAR_XT002_SEARING_LIGHT_DETOUR * sin(angle);

        if (usable(ringX, ringY))
            candidates.push_back({ringX, ringY, RaidClearance(ringX, ringY)});
    }

    // Roomiest first, never nearest: the nearest alternates lie back towards the raid.
    std::sort(candidates.begin(), candidates.end(), [](Candidate const& left, Candidate const& right)
    {
        return left.clearance > right.clearance;
    });

    size_t const attempts = std::min<size_t>(ULDUAR_XT002_MOVE_CANDIDATE_ATTEMPTS, candidates.size());
    for (size_t i = 0; i < attempts; ++i)
    {
        switch (IssueMove(candidates[i].x, candidates[i].y, spotZ))
        {
            case MoveIssue::Taken:
                return true;
            case MoveIssue::Refused:
                return false;
            case MoveIssue::NoPath:
                break;
        }
    }

    return false;
}

XT002DebuffCarrierAction::ParkResult XT002DebuffCarrierAction::ParkVoidZone(std::list<Creature*> const& voidZones,
                                                                           float reach, float& cellX,
                                                                           float& cellY)
{
    struct Cell
    {
        float x;
        float y;
        float z;
        bool reachable;
        float lane;
        bool roomy;
        float distance;
    };

    std::vector<Cell> cells;
    float nearestDistance = std::numeric_limits<float>::max();

    // Every lot the role owns goes into one pool. Ranged and healers have one on each side of the
    // formation, and the shortest-walk tie-break below is what sends a carrier to the near one.
    for (XT002BombLot const& lot : GetXT002BombLots(bot))
    {
        for (int cell = 0; cell < lot.xCells * lot.yCells; ++cell)
        {
            float const candidateX =
                lot.origin.GetPositionX() + (cell % lot.xCells) * ULDUAR_XT002_BOMB_GRID_STEP;
            float const candidateY =
                lot.origin.GetPositionY() + lot.yDirection * (cell / lot.xCells) * ULDUAR_XT002_BOMB_GRID_STEP;

            // Measured against where the puddles actually are rather than the cells they were aimed at, so
            // a drop that landed off-centre blocks whatever it is really near.
            float nearestVoidZone = std::numeric_limits<float>::max();
            for (Creature* voidZone : voidZones)
                nearestVoidZone = std::min(nearestVoidZone, voidZone->GetExactDist2d(candidateX, candidateY));

            if (nearestVoidZone < ULDUAR_XT002_VOID_ZONE_RADIUS)
                continue;

            float const distance = bot->GetExactDist2d(candidateX, candidateY);

            // Reachable first: a cell the bomb beats the bot to is worthless. Then the walk in, which matters more
            // than room at the cell, since a walk through a puddle ticks Consumption the whole way.
            bool const reachable = distance + ULDUAR_XT002_BOMB_CELL_ARRIVED <= reach;
            float const lane = std::min(ApproachClearance(candidateX, candidateY, voidZones),
                                        ULDUAR_XT002_BOMB_APPROACH_CLEARANCE);
            bool const roomy = nearestVoidZone >= ULDUAR_XT002_BOMB_CELL_PREFERRED_CLEARANCE;

            cells.push_back({candidateX, candidateY, lot.origin.GetPositionZ(), reachable, lane, roomy,
                             distance});
            nearestDistance = std::min(nearestDistance, distance);
        }
    }

    if (cells.empty())
    {
        parked = false;
        return ParkResult::None;
    }

    std::sort(cells.begin(), cells.end(), [](Cell const& left, Cell const& right)
    {
        if (left.reachable != right.reachable)
            return left.reachable;
        if (left.lane != right.lane)
            return left.lane > right.lane;
        if (left.roomy != right.roomy)
            return left.roomy;
        return left.distance < right.distance;
    });

    cellX = cells.front().x;
    cellY = cells.front().y;

    // Arrival is against the nearest free cell, not the best-ranked one; both bands stay under the grid step so
    // only one cell can answer. Never parked inside Consumption, even within re-engage of a cell.
    bool const inVoidZone = StandingInVoidZone(voidZones);

    if (parked && !inVoidZone && nearestDistance <= ULDUAR_XT002_BOMB_CELL_REENGAGE)
    {
        if (bot->isMoving())
            bot->StopMoving();

        return ParkResult::Parked;
    }

    parked = false;

    if (!inVoidZone && nearestDistance <= ULDUAR_XT002_BOMB_CELL_ARRIVED)
    {
        parked = true;
        if (bot->isMoving())
            bot->StopMoving();

        return ParkResult::Parked;
    }

    // Nothing in the lot is close enough to walk to inside the debuff.
    if (!cells.front().reachable)
        return ParkResult::OutOfReach;

    size_t attempts = 0;
    for (Cell const& candidate : cells)
    {
        if (!candidate.reachable || attempts >= ULDUAR_XT002_MOVE_CANDIDATE_ATTEMPTS)
            break;

        ++attempts;
        switch (IssueMove(candidate.x, candidate.y, candidate.z))
        {
            case MoveIssue::Taken:
                return ParkResult::Moving;
            case MoveIssue::Refused:
                return ParkResult::None;
            case MoveIssue::NoPath:
                break;
        }
    }

    // Every reachable cell was refused. The bearing to the best of them still points out of the raid,
    // so hand it to the stop-short fallback rather than to the ring search.
    return ParkResult::OutOfReach;
}

bool XT002DebuffCarrierAction::Execute(Event /*event*/)
{
    bool const hasBomb = bot->HasAura(GetXT002GravityBombSpellId(bot));
    bool const hasSearingLight = bot->HasAura(GetXT002SearingLightSpellId(bot));
    if (!hasBomb && !hasSearingLight)
    {
        parked = false;
        return false;
    }

    // One lookup for the whole carrier tick, since finding XT is a scan.
    Unit* xt002 = GetXT002(botAI);

    // Void Zones only drop once XT carries Heartbreak, so before then a bomb carrier just needs somewhere the
    // splash misses, which keeps melee uptime.
    bool const heartbreak = IsXT002HeartbreakActive(bot, xt002);

    // A Searing Light carrier standing in a lot just had its bomb go off underfoot: it steps off its own puddle
    // there rather than walking the splash back through the raid.
    bool const steppingOffOwnBomb = !hasBomb && heartbreak && InsideParkingLot();

    // One scan for the whole tick: the cell ranking, the stop-short fallback and the ring search all
    // need the same puddles, and each of them standing in Consumption is what kills a carrier.
    std::list<Creature*> voidZones;
    if (heartbreak && xt002)
        xt002->GetCreatureListWithEntryInGrid(voidZones, PB_NPC_XT002_VOID_ZONE,
                                              ULDUAR_XT002_VOID_ZONE_SEARCH_RADIUS);

    if (heartbreak && (hasBomb || steppingOffOwnBomb))
    {
        // Only a bot actually holding the bomb runs against a clock. Stepping off one's own puddle is
        // a single grid step with nothing about to go off, so it is allowed the whole lot.
        float reach = std::numeric_limits<float>::max();
        if (hasBomb)
            if (Aura* bomb = bot->GetAura(GetXT002GravityBombSpellId(bot)))
                reach = TravelReach(std::max(0, bomb->GetDuration()));

        if (xt002)
        {
            float cellX = 0.0f;
            float cellY = 0.0f;
            switch (ParkVoidZone(voidZones, reach, cellX, cellY))
            {
                case ParkResult::Moving:
                    return true;
                // Standing on the cell with movement stopped. Yielding the tick is what lets a healer
                // carrier keep healing and a hunter keep shooting for the rest of the debuff.
                case ParkResult::Parked:
                    return false;
                case ParkResult::OutOfReach:
                    if (StopShortOf(cellX, cellY, reach, voidZones))
                        return true;
                    break;
                case ParkResult::None:
                    break;
            }
        }

        // Without a bomb, reaching here means every cell is taken. Hold position: the splash expires over an empty
        // lot wherever the bot stands in it.
        if (steppingOffOwnBomb)
            return false;
    }
    else
    {
        parked = false;
    }

    if (!hasBomb)
    {
        // Tanks stay put for Searing Light: dragging XT or dropping the Pummeller costs more than the splash.
        if (PlayerbotAI::IsTank(bot))
            return false;

        return MoveToSearingLightSpot(heartbreak);
    }

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<std::pair<Unit*, float>> avoid;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive())
            continue;

        avoid.emplace_back(member, ULDUAR_XT002_DEBUFF_CLEAR_RADIUS);
    }

    // Puddles go in too: this is the last resort for a carrier that can neither reach a cell nor stop short.
    for (Creature* voidZone : voidZones)
        avoid.emplace_back(voidZone, ULDUAR_XT002_VOID_ZONE_RADIUS);

    return MoveClearOf(avoid);
}

bool XT002AvoidHazardAction::Execute(Event /*event*/)
{
    // A carrier's puddles belong to the carrier action, which picks its cell and walks it off its own bomb, and
    // this search would undo both. Boombots still apply to everyone.
    bool const carryingDebuff = bot->HasAura(GetXT002SearingLightSpellId(bot)) ||
                                bot->HasAura(GetXT002GravityBombSpellId(bot));

    std::vector<std::pair<Unit*, float>> hazards;
    GuidVector const& npcs = AI_VALUE(GuidVector, "nearest npcs");
    for (ObjectGuid const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        // Ranged already stand outside the blast, so pulling them out too would only break their casts.
        if (unit->GetEntry() == PB_NPC_XT002_BOOMBOT && PlayerbotAI::IsMelee(bot))
            hazards.emplace_back(unit, ULDUAR_XT002_BOOMBOT_AVOID_RADIUS);
        // Margin over the trigger's radius: FindNearestCreature subtracts both object sizes and MoveClearOf scores
        // centre to centre capped at zero, so in that gap the trigger fires while nothing beats standing still.
        else if (unit->GetEntry() == PB_NPC_XT002_VOID_ZONE && !carryingDebuff)
            hazards.emplace_back(unit, ULDUAR_XT002_VOID_ZONE_RADIUS + ULDUAR_XT002_BOMB_CELL_ARRIVED);
    }

    return MoveClearOf(hazards);
}

bool XT002PummellerTauntAction::Execute(Event /*event*/)
{
    Unit* pummeller = GetXT002EngageableAdd(botAI, bot, PB_NPC_XT002_PUMMELLER, ULDUAR_XT002_TAUNT_RANGE);
    if (!pummeller)
        return false;

    switch (bot->getClass())
    {
        case CLASS_WARRIOR:
            return botAI->CastSpell("taunt", pummeller);
        case CLASS_PALADIN:
            return botAI->CastSpell("hand of reckoning", pummeller);
        case CLASS_DEATH_KNIGHT:
            return botAI->CastSpell("dark command", pummeller);
        case CLASS_DRUID:
            return botAI->CastSpell("growl", pummeller);
        default:
            return false;
    }
}

bool XT002RedirectThreatAction::isUseful()
{
    return bot->getClass() == CLASS_HUNTER || bot->getClass() == CLASS_ROGUE;
}

Player* XT002RedirectThreatAction::GetRedirectTank(Unit* xt002)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    // A Pummeller is the off-tank's job, and it is the add most likely to peel onto a redirecter. One
    // still out at a toy pile is nobody's, so the leash decides whether there is really one in play.
    if (GetXT002EngageableAdd(botAI, bot, PB_NPC_XT002_PUMMELLER, ULDUAR_XT002_ADD_LEASH_RADIUS))
    {
        if (Player* assistTank = GetGroupAssistTank(bot, 0))
            return assistTank;
    }

    // Otherwise feed whoever is actually holding XT, which survives a tank swap or a tank death.
    if (xt002)
    {
        if (Unit* victim = xt002->GetVictim())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (!member || !member->IsAlive() || member == bot)
                    continue;

                if (PlayerbotAI::IsTank(member) && member == victim)
                    return member;
            }
        }
    }

    return GetGroupMainTank(bot);
}

bool XT002RedirectThreatAction::Execute(Event /*event*/)
{
    // One lookup for the whole evaluation, since finding XT is a scan.
    Unit* xt002 = GetXT002(botAI);

    Player* tank = GetRedirectTank(xt002);
    if (!tank || tank == bot)
        return false;

    if (bot->getClass() == CLASS_ROGUE)
    {
        // Tricks redirects everything the rogue does for the next 6s, so there is no dump shot.
        return botAI->CanCastSpell("tricks of the trade", tank) && botAI->CastSpell("tricks of the trade", tank);
    }

    if (botAI->CanCastSpell("misdirection", tank))
        return botAI->CastSpell("misdirection", tank);

    // Misdirection only moves the threat of the next three shots, so spend them on XT, never on whatever the
    // rotation picks or an add the tank doesn't want.
    if (xt002 && !IsXT002Submerged(xt002) && bot->HasAura(SPELL_MISDIRECTION) &&
        botAI->CanCastSpell("steady shot", xt002))
        return botAI->CastSpell("steady shot", xt002);

    return false;
}

bool XT002RaidPositionAction::Execute(Event /*event*/)
{
    if (PlayerbotAI::IsMainTank(bot))
    {
        if (bot->GetExactDist(ULDUAR_XT002_MAINTANK_SPOT) <= ULDUAR_XT002_MAINTANK_SPOT_TOLERANCE)
            return false;

        return MoveTo(bot->GetMapId(), ULDUAR_XT002_MAINTANK_SPOT.GetPositionX(),
                      ULDUAR_XT002_MAINTANK_SPOT.GetPositionY(), ULDUAR_XT002_MAINTANK_SPOT.GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true);
    }

    Position slot;
    if (GetXT002RangedSlot(bot, GetXT002(botAI), slot))
    {
        if (bot->GetExactDist(slot) <= ULDUAR_XT002_RANGED_SPOT_TOLERANCE)
            return false;

        return MoveTo(bot->GetMapId(), slot.GetPositionX(), slot.GetPositionY(), slot.GetPositionZ(), false,
                      false, false, false, MovementPriority::MOVEMENT_COMBAT, true);
    }

    return false;
}

bool XT002SetDpsPriorityAction::IsAllowedTarget(Unit* unit, Unit* xt002) const
{
    // The Heart is hidden, not despawned, when its window shuts, and XT while submerged. The core refuses an
    // untargetable unit, so a bot holding one queues spells that all fail.
    if (!unit || !unit->IsAlive() || unit->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        return false;

    // How far this bot will travel for an add. Tanks are exempt because this runs over their list too,
    // and the off-tank is supposed to walk to the Pummeller.
    float const reach = PlayerbotAI::IsMelee(bot) ? ULDUAR_XT002_MELEE_ENGAGE_RANGE
                                            : ULDUAR_XT002_RANGED_ENGAGE_RANGE;
    bool const inReach = PlayerbotAI::IsTank(bot) || unit->GetExactDist2d(bot) <= reach;

    switch (unit->GetEntry())
    {
        case PB_NPC_XT002_LIFE_SPARK:
            // No leash: it spawns on the Searing Light carrier and chases players. Static Charged reaches 500 yd,
            // so there's nothing to dodge; it just has to die.
            return inReach;

        case NPC_XS013_SCRAPBOT:
        case PB_NPC_XT002_PUMMELLER:
            return inReach && IsXT002AddEngageable(xt002, unit);

        case PB_NPC_XT002_BOOMBOT:
            // Melee must never pick one up, and ranged only from outside the blast: closer than that
            // the avoid action should be moving the bot, not this one holding it in place.
            return !PlayerbotAI::IsMelee(bot) && unit->GetExactDist2d(bot) >= ULDUAR_XT002_BOOMBOT_AVOID_RADIUS &&
                   inReach && IsXT002AddEngageable(xt002, unit);

        case NPC_HEART_OF_DECONSTRUCTOR:
            // Damage only reaches XT while the Heart channels Exposed Heart.
            if (!unit->HasAura(SPELL_XT002_EXPOSED_HEART))
                return false;

            // Only a Life Spark is worth breaking off for: Static Charged is a raid-wide aura that
            // runs until the spark dies, and no amount of distance reduces it.
            if (GetUldUnitByEntry(botAI, PB_NPC_XT002_LIFE_SPARK))
                return false;

            // Config flag, not the Heartbreak aura: this states intent and has to hold before the Heart breaks.
            if (IsXT002HardModeActive(botAI))
                return true;

            return unit->GetHealthPct() > ULDUAR_XT002_HEART_SAFE_HP_PCT;

        case NPC_XT002:
            return !IsXT002Submerged(xt002);

        default:
            return true;
    }
}

Unit* XT002SetDpsPriorityAction::SelectByEntry(Unit* currentTarget, uint32 entry,
                                               std::vector<Unit*> const& candidates) const
{
    Unit* selected = nullptr;
    if (currentTarget && currentTarget->IsAlive() && currentTarget->GetEntry() == entry)
        selected = currentTarget;

    // Adds come from toy piles on both flanks, so nearest-to-the-bot beats measuring from a raid
    // anchor. The margin stops two similar adds from trading the bot back and forth every tick.
    float const switchMargin = 10.0f;
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

std::vector<std::pair<uint32, Unit*>> XT002SetDpsPriorityAction::BuildPriorityList(Unit*& xt002)
{
    Unit* boss = nullptr;
    Unit* heart = nullptr;
    std::vector<Unit*> lifeSparks;
    std::vector<Unit*> scrapbots;
    std::vector<Unit*> boombots;
    std::vector<Unit*> pummellers;
    xt002 = nullptr;

    // One scan for the whole action: the same pass hands back XT for the gates, so they need none.
    GuidVector const& npcs = AI_VALUE(GuidVector, "nearest npcs");
    for (ObjectGuid const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        switch (unit->GetEntry())
        {
            case NPC_XT002:
                boss = unit;
                // First match, which is what GetXT002 returns; `boss` keeps the last one.
                if (!xt002)
                    xt002 = unit;
                break;
            case NPC_HEART_OF_DECONSTRUCTOR:
                heart = unit;
                break;
            case PB_NPC_XT002_LIFE_SPARK:
                lifeSparks.push_back(unit);
                break;
            case NPC_XS013_SCRAPBOT:
                scrapbots.push_back(unit);
                break;
            case PB_NPC_XT002_BOOMBOT:
                boombots.push_back(unit);
                break;
            case PB_NPC_XT002_PUMMELLER:
                pummellers.push_back(unit);
                break;
            default:
                break;
        }
    }

    Unit* currentTarget = AI_VALUE(Unit*, "current target");

    // A tank is only ever handed the boss. The generic smart tank targeting ranks every add the tank
    // has no aggro on above XT, so a tank left on it walks into the add pile and the boss follows.
    if (PlayerbotAI::IsTank(bot))
    {
        std::vector<std::pair<uint32, Unit*>> tankPriority;

        // Taunt reaches 30 yd and the toy piles are 80 yd out, so the add comes to the tank on XT. Only a tank with
        // someone else on the boss chases it.
        if (IsXT002PummellerTank(bot) && PlayerbotAI::GetGroupTankNum(bot) > 1)
            tankPriority.emplace_back(PB_NPC_XT002_PUMMELLER,
                                      SelectByEntry(currentTarget, PB_NPC_XT002_PUMMELLER, pummellers));

        // Hard mode: XT is submerged the whole Heart window, so tanks join the burn. Normal mode keeps them off,
        // since one stray tank hit flips the raid into hard mode.
        if (IsXT002HardModeActive(botAI))
            tankPriority.emplace_back(NPC_HEART_OF_DECONSTRUCTOR, heart);

        tankPriority.emplace_back(NPC_XT002, boss);
        return tankPriority;
    }

    // Sparks keep Static Charged on the raid and Scrapbots heal XT; Boombots only cost damage and a Pummeller
    // can be tanked. Hard mode puts the Heart first, since the whole mode is decided in its 30 s window.
    bool const hardMode = IsXT002HardModeActive(botAI);

    std::vector<std::pair<uint32, Unit*>> priority;
    if (hardMode)
        priority.emplace_back(NPC_HEART_OF_DECONSTRUCTOR, heart);

    priority.emplace_back(PB_NPC_XT002_LIFE_SPARK,
                          SelectByEntry(currentTarget, PB_NPC_XT002_LIFE_SPARK, lifeSparks));
    priority.emplace_back(NPC_XS013_SCRAPBOT, SelectByEntry(currentTarget, NPC_XS013_SCRAPBOT, scrapbots));
    if (!PlayerbotAI::IsMelee(bot))
        priority.emplace_back(PB_NPC_XT002_BOOMBOT,
                              SelectByEntry(currentTarget, PB_NPC_XT002_BOOMBOT, boombots));
    priority.emplace_back(PB_NPC_XT002_PUMMELLER,
                          SelectByEntry(currentTarget, PB_NPC_XT002_PUMMELLER, pummellers));
    if (!hardMode)
        priority.emplace_back(NPC_HEART_OF_DECONSTRUCTOR, heart);

    priority.emplace_back(NPC_XT002, boss);

    return priority;
}

Unit* XT002SetDpsPriorityAction::ResolveTarget(Unit* currentTarget)
{
    Unit* xt002 = nullptr;
    std::vector<std::pair<uint32, Unit*>> const priority = BuildPriorityList(xt002);

    Unit* target = nullptr;
    for (auto const& candidate : priority)
    {
        if (IsAllowedTarget(candidate.second, xt002))
        {
            target = candidate.second;
            break;
        }
    }

    auto const priorityIndex = [&](Unit* unit) -> size_t
    {
        if (!IsAllowedTarget(unit, xt002))
            return priority.size();

        for (size_t index = 0; index < priority.size(); ++index)
        {
            if (priority[index].first == unit->GetEntry())
                return index;
        }

        return priority.size();
    };

    // Hold the current target unless something strictly more urgent is up, so Scrapbot churn can't reset swing
    // and cast timers. Never hold one the gates just rejected.
    size_t const currentIndex = priorityIndex(currentTarget);
    if (currentIndex < priority.size() && currentIndex <= priorityIndex(target))
        target = currentTarget;

    // No "dps target" fallback: it has no leash and hands back the add at a toy pile the gates rejected. An
    // empty list means XT is submerged, so falling through to the anchor is right.
    return target;
}

bool XT002SetDpsPriorityAction::Execute(Event /*event*/)
{
    Unit* currentTarget = AI_VALUE(Unit*, "current target");

    // Healers get a target too: without one a bot never enters combat and loses every combat-engine heal.
    // XT002TargetGuardMultiplier keeps them from walking at an add.
    Unit* target = ResolveTarget(currentTarget);
    if (!target)
        return false;

    // Returning false once the bot is already on the right target is what lets the lower-priority
    // nodes run at all: the engine ends the tick at the first action that succeeds.
    bool needsAttack = currentTarget != target;
    if (PlayerbotAI::IsMelee(bot))
        needsAttack = needsAttack || !bot->HasUnitState(UNIT_STATE_MELEE_ATTACKING);

    return needsAttack ? Attack(target) : false;
}
