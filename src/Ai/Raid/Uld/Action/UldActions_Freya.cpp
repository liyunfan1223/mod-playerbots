/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldActions_Freya.h"
#include "UldActions_Shared.h"

#include <CombatStrategy.h>
#include <FollowMasterStrategy.h>

#include <cmath>

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
#include "Spell.h"
#include "UldData.h"
#include "UldEncounter_Freya.h"
#include "UldScripts.h"
#include "EncounterHelpers.h"
#include "ScriptedCreature.h"
#include "ServerFacade.h"
#include "Unit.h"
#include "Vehicle.h"
#include <TankAssistStrategy.h>

using namespace EncounterHelpers;

namespace
{
// PointMovementGenerator won't launch a spline while the bot is casting, yet MoveTo still reports
// success, so a channel pins the bot in the blast. Only interrupts when movement is really blocked.
void FreyaClearCastBlockingMove(Player* bot)
{
    if (bot->IsMovementPreventedByCasting())
        bot->InterruptNonMeleeSpells(true);
}

// Rooted or stunned: the walk never starts but the latch would claim it did, and the interrupt would
// waste the one cast left. Falls through to the rotation and the node that kills the root.
bool FreyaBotCannotWalk(Player* bot)
{
    return bot->HasUnitState(UNIT_STATE_NOT_MOVE);
}

// Every other living raid member near the bot. Nature's Fury has no ground marker to route around: the
// raid itself is the hazard, and the carrier is the only one who can move away from it.
std::vector<Position> GetFreyaAlliesNear(PlayerbotAI* botAI, float radius)
{
    Player* bot = botAI->GetBot();
    Group* group = bot->GetGroup();
    if (!group)
        return {};

    std::vector<Position> allies;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() || member->GetMapId() != bot->GetMapId())
            continue;

        if (bot->GetExactDist2d(member) < radius)
            allies.push_back(member->GetPosition());
    }

    return allies;
}

// Last resort once every ring sweep has failed: walk straight out, away from the covering bombs'
// centre. Crossing another bomb on the way beats standing inside four.
Position StepOffNatureBombs(Player* bot, std::vector<Position> const& bombs)
{
    float sumX = 0.0f;
    float sumY = 0.0f;
    uint32 covering = 0;
    for (Position const& bomb : bombs)
    {
        if (bomb.GetExactDist2d(bot->GetPositionX(), bot->GetPositionY()) >= ULDUAR_FREYA_NATURE_BOMB_AVOID_RADIUS)
            continue;

        sumX += bomb.GetPositionX();
        sumY += bomb.GetPositionY();
        ++covering;
    }

    if (!covering)
        return Position();

    float const centreX = sumX / covering;
    float const centreY = sumY / covering;

    // Standing dead on the centre leaves no bearing to run on, and one direction is as good as another.
    float const bearing = bot->GetExactDist2d(centreX, centreY) > CONTACT_DISTANCE
                              ? std::atan2(bot->GetPositionY() - centreY, bot->GetPositionX() - centreX)
                              : bot->GetOrientation();

    float x = bot->GetPositionX() + std::cos(bearing) * ULDUAR_FREYA_NATURE_BOMB_CLEAR_RADIUS;
    float y = bot->GetPositionY() + std::sin(bearing) * ULDUAR_FREYA_NATURE_BOMB_CLEAR_RADIUS;
    float z = bot->GetPositionZ();
    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                        bot->GetPositionZ(), x, y, z))
        return Position();

    return Position(x, y, z, 0.0f);
}
}  // namespace

bool FreyaMoveAwayNatureBombAction::isUseful()
{
    FreyaNearNatureBombTrigger trigger(botAI);
    if (!trigger.IsActive())
        return false;

    // The trigger covers the whole search radius so a bot that stepped clear can still hold its spot.
    // Narrow it back to the blast plus the latch window, so everyone else falls through without a verdict.
    if (bot->FindNearestGameObject(GOBJECT_NATURE_BOMB, ULDUAR_FREYA_NATURE_BOMB_AVOID_RADIUS))
        return true;

    return bombSpotMs && getMSTimeDiff(bombSpotMs, getMSTime()) < ULDUAR_FREYA_NATURE_BOMB_LATCH_MS;
}

bool FreyaMoveAwayNatureBombAction::Execute(Event /*event*/)
{
    if (FreyaBotCannotWalk(bot))
    {
        bombSpotMs = 0;
        return false;
    }

    // Not FleePosition: it caps travel at FleeDistance (5 yd), short of a 10 yd blast, and reads one
    // hazard only. A volley drops a bomb on every player, so the melee stack sits under several.
    std::vector<Position> bombs = GetFreyaNatureBombPositions(bot, ULDUAR_FREYA_HAZARD_SEARCH_RADIUS);
    if (bombs.empty())
    {
        bombSpotMs = 0;
        return false;
    }

    // Beams and low lashers too: both escapes share a relevance, so a bot reading only its own hazard
    // flips between them forever.
    std::vector<HazardCircle> const hazards =
        BuildFreyaEscapeHazards(bombs, GetFreyaSunBeamPositions(botAI, ULDUAR_FREYA_HAZARD_SEARCH_RADIUS),
                                GetFreyaEscapeLasherPositions(botAI, ULDUAR_FREYA_HAZARD_SEARCH_RADIUS));

    auto const stillClear = [&hazards](Position const& spot)
    {
        for (HazardCircle const& hazard : hazards)
            if (hazard.first.GetExactDist2d(spot.GetPositionX(), spot.GetPositionY()) < hazard.second)
                return false;

        return true;
    };

    auto const covered = [&bombs](Position const& spot)
    {
        for (Position const& bomb : bombs)
            if (bomb.GetExactDist2d(spot.GetPositionX(), spot.GetPositionY()) < ULDUAR_FREYA_NATURE_BOMB_AVOID_RADIUS)
                return true;

        return false;
    };

    uint32 const now = getMSTime();
    bool const latched = bombSpotMs && getMSTimeDiff(bombSpotMs, now) < ULDUAR_FREYA_NATURE_BOMB_LATCH_MS;

    // Escape in flight: claim the tick without touching the motion master, so combat movement can't pull
    // the bot back onto the bomb. A cast-pinned bot isn't in flight, so it falls through to the interrupt.
    if (latched && stillClear(bombSpot) && !bot->IsMovementPreventedByCasting() &&
        bot->GetExactDist2d(bombSpot.GetPositionX(), bombSpot.GetPositionY()) > CONTACT_DISTANCE)
        return true;

    if (!covered(bot->GetPosition()))
    {
        // Arrived, and the bomb it fled still has fuse left. Melee hold here, since they're out of range
        // either way. Ranged and healers don't: returning true would silence them for the whole fuse.
        if (latched && PlayerbotAI::IsMelee(bot) && stillClear(bombSpot) && covered(bombOrigin))
            return true;

        bombSpotMs = 0;
        return false;
    }

    bombSpotMs = 0;

    // Every tier below walks the same ring of candidates with a looser clearance, so the collision
    // check only ever has to answer for each of them once.
    HazardSweepCache sweep;

    Position safe = FindNearestPositionClearOfHazards(bot, hazards, ULDUAR_FREYA_HAZARD_SEARCH_RADIUS, &sweep);

    // A volley drops a bomb on every player at once, so nothing may clear all of them by the full
    // margin. Somebody else's hazard beats your own, and barely outside beats standing in one.
    if (safe == Position())
        safe = FindNearestPositionClearOfHazards(bot, bombs, ULDUAR_FREYA_NATURE_BOMB_CLEAR_RADIUS,
                                                 ULDUAR_FREYA_HAZARD_SEARCH_RADIUS, &sweep);

    if (safe == Position())
        safe = FindNearestPositionClearOfHazards(bot, bombs, ULDUAR_FREYA_NATURE_BOMB_AVOID_RADIUS + 1.0f,
                                                 ULDUAR_FREYA_HAZARD_SEARCH_RADIUS, &sweep);

    // Then the bare blast radius, then straight out: giving up leaves the bot in the circle until it blows.
    if (safe == Position())
        safe = FindNearestPositionClearOfHazards(bot, bombs, ULDUAR_FREYA_NATURE_BOMB_BLAST_RADIUS,
                                                 ULDUAR_FREYA_HAZARD_SEARCH_RADIUS, &sweep);

    if (safe == Position())
        safe = StepOffNatureBombs(bot, bombs);

    if (safe == Position())
        return false;

    FreyaClearCastBlockingMove(bot);

    if (!MoveTo(bot->GetMapId(), safe.GetPositionX(), safe.GetPositionY(), safe.GetPositionZ(), false, false, false,
                true, MovementPriority::MOVEMENT_FORCED, true, false))
        return false;

    bombOrigin = bot->GetPosition();
    bombSpot = safe;
    bombSpotMs = now;

    return true;
}

bool FreyaTankNatureBombAction::isUseful()
{
    FreyaTankNatureBombTrigger trigger(botAI);
    return trigger.IsActive();
}

bool FreyaTankNatureBombAction::Execute(Event /*event*/)
{
    if (FreyaBotCannotWalk(bot))
    {
        kiteSpotMs = 0;
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "freya");
    if (!boss || !boss->IsAlive())
    {
        kiteSpotMs = 0;
        return false;
    }

    std::vector<Position> bombs = GetFreyaNatureBombPositions(bot, ULDUAR_FREYA_HAZARD_SEARCH_RADIUS);
    if (bombs.empty())
    {
        kiteSpotMs = 0;
        return false;
    }

    auto const stillClear = [&bombs](Position const& spot)
    {
        for (Position const& bomb : bombs)
            if (bomb.GetExactDist2d(spot.GetPositionX(), spot.GetPositionY()) < ULDUAR_FREYA_NATURE_BOMB_CLEAR_RADIUS)
                return false;

        return true;
    };

    uint32 const now = getMSTime();

    if (kiteSpotMs && getMSTimeDiff(kiteSpotMs, now) < ULDUAR_FREYA_TANK_BOMB_LATCH_MS && stillClear(kiteSpot) &&
        !bot->IsMovementPreventedByCasting() &&
        bot->GetExactDist2d(kiteSpot.GetPositionX(), kiteSpot.GetPositionY()) > CONTACT_DISTANCE)
        return true;

    kiteSpotMs = 0;

    // Aim past Freya, away from the back line. preferNear only reorders equally short walks.
    // Past the leash aim at the anchor instead, or that bias compounds volley after volley.
    Position preferred = ULDUAR_FREYA_TANK_ANCHOR;
    if (boss->GetExactDist2d(&ULDUAR_FREYA_TANK_ANCHOR) <= ULDUAR_FREYA_TANK_LEASH)
    {
        if (Player* anchor = GetFreyaRangedCampAnchor(botAI))
        {
            float const dx = boss->GetPositionX() - anchor->GetPositionX();
            float const dy = boss->GetPositionY() - anchor->GetPositionY();
            float const length = std::sqrt(dx * dx + dy * dy);
            if (length > 0.0f)
                preferred = Position(boss->GetPositionX() + dx / length * ULDUAR_FREYA_HAZARD_SEARCH_RADIUS,
                                     boss->GetPositionY() + dy / length * ULDUAR_FREYA_HAZARD_SEARCH_RADIUS,
                                     boss->GetPositionZ(), 0.0f);
        }
    }

    Position const* preferNear = &preferred;

    std::vector<EncounterHelpers::HazardCircle> hazards =
        BuildFreyaEscapeHazards(bombs, GetFreyaSunBeamPositions(botAI, ULDUAR_FREYA_HAZARD_SEARCH_RADIUS));

    HazardSweepCache sweep;

    Position safe = FindNearestPositionClearOfHazards(bot, hazards, ULDUAR_FREYA_HAZARD_SEARCH_RADIUS,
                                                      ULDUAR_FREYA_SWEEP_STEP, ULDUAR_FREYA_SWEEP_ANGLE_STEP,
                                                      preferNear, {}, &sweep);

    if (safe == Position())
        safe = FindNearestPositionClearOfHazards(bot, bombs, ULDUAR_FREYA_NATURE_BOMB_CLEAR_RADIUS,
                                                 ULDUAR_FREYA_HAZARD_SEARCH_RADIUS, &sweep);

    if (safe == Position())
        safe = FindNearestPositionClearOfHazards(bot, bombs, ULDUAR_FREYA_NATURE_BOMB_AVOID_RADIUS + 1.0f,
                                                 ULDUAR_FREYA_HAZARD_SEARCH_RADIUS, &sweep);

    if (safe == Position())
        return false;

    FreyaClearCastBlockingMove(bot);

    if (!MoveTo(bot->GetMapId(), safe.GetPositionX(), safe.GetPositionY(), safe.GetPositionZ(), false, false, false,
                true, MovementPriority::MOVEMENT_FORCED, true, false))
        return false;

    kiteSpot = safe;
    kiteSpotMs = now;

    return true;
}

bool FreyaSetDpsPriorityAction::isUseful()
{
    FreyaSetDpsPriorityTrigger trigger(botAI);
    return trigger.IsActive();
}

bool FreyaSetDpsPriorityAction::Execute(Event /*event*/)
{
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    Unit* target = ResolveFreyaDpsTarget(currentTarget);
    if (!target)
        return false;

    // Every tick, not only on a switch: the early return below is the common case, and a pet left on
    // a dead or floored add would never catch up. CommandPetAttack no-ops when it is already there.
    CommandPetAttack(botAI, target);

    bool needsAttack = currentTarget != target;
    if (PlayerbotAI::IsMelee(bot))
        needsAttack = needsAttack || !bot->HasUnitState(UNIT_STATE_MELEE_ATTACKING);

    // Returning false once the bot is on the right target is what lets the lower-priority nodes run:
    // the engine ends the tick at the first action that succeeds.
    return needsAttack ? Attack(target) : false;
}

bool FreyaSetDpsPriorityAction::TakesEonarsGift(FreyaWaveState const& state)
{
    if (!state.eonarsGift || !state.eonarsGift->IsAlive())
    {
        giftGuid.Clear();
        return false;
    }

    bool packUp = false;
    for (Unit* lasher : state.detonatingLashers)
        packUp = packUp || (lasher && lasher->IsAlive());

    // Nothing competes for the ranged, so the Gift takes all of them.
    if (!packUp)
        return true;

    uint32 const now = getMSTime();
    if (giftGuid != state.eonarsGift->GetGUID())
    {
        giftGuid = state.eonarsGift->GetGUID();
        giftSeenMs = now;
    }

    // A head start, not a hand-off: whatever the share has managed by now, the rest of the ranged join
    // it with time left on the 12s.
    if (getMSTimeDiff(giftSeenMs, now) >= ULDUAR_FREYA_GIFT_SHARE_MS)
        return true;

    return GetFreyaRangedDpsRank(botAI) < ULDUAR_FREYA_GIFT_SHARE;
}

Unit* FreyaSetDpsPriorityAction::ShedSnaplasherThreat(FreyaWaveState const& state, Unit* assigned)
{
    if (!state.snaplasher)
    {
        snaplasherGuid.Clear();
        snaplasherShed = false;
        return assigned;
    }

    uint32 const now = getMSTime();
    if (snaplasherGuid != state.snaplasher->GetGUID())
    {
        snaplasherGuid = state.snaplasher->GetGUID();
        snaplasherSeenMs = now;
        snaplasherShed = false;
    }

    if (assigned != state.snaplasher)
        return assigned;

    Unit* sibling = nullptr;
    for (Unit* member : {state.stormLasher, state.waterSpirit})
    {
        if (!member || !member->IsAlive() || FreyaTrioSyncSuppress(state, member))
            continue;

        if (!sibling || member->GetHealth() > sibling->GetHealth())
            sibling = member;
    }

    // Nowhere to move the damage, and the Snaplasher has to die inside the same window as the other two.
    if (!sibling)
        return assigned;

    snaplasherShed = FreyaShedsSnaplasherThreat(botAI, state.snaplasher, getMSTimeDiff(snaplasherSeenMs, now),
                                                snaplasherShed);

    return snaplasherShed ? sibling : assigned;
}

Unit* FreyaSetDpsPriorityAction::ResolveFreyaDpsTarget(Unit* currentTarget)
{
    FreyaWaveState state;
    GatherFreyaWaveState(botAI, state);

    // Eonar's Gift heals Freya for 30-60% if it lives 12s. Ranged burn it from where they stand, so
    // melee never eat the travel time both ways, unless there is no ranged DPS left to do it.
    bool const takesGift =
        (PlayerbotAI::IsRangedDps(bot) || !FreyaHasLivingRangedDps(botAI)) && TakesEonarsGift(state);

    // Once the trio is low, leaving it costs the whole wave: a member abandoned above the floor
    // revives 11s later, and the 60s until the next wave spawns is gone.
    bool const trioLocked = state.TrioLocked();

    std::vector<Unit*> priority;

    if (takesGift && !state.TrioReleased())
        priority.push_back(state.eonarsGift);

    if (!trioLocked)
        priority.push_back(state.conservator);

    // One slot for all three members; which one this bot takes is the greedy split, not entry order.
    Unit* const trioMember = ShedSnaplasherThreat(state, GetFreyaTrioAssignment(botAI, state));
    priority.push_back(trioMember);

    if (trioLocked)
    {
        priority.push_back(state.conservator);
    }
    else
    {
        bool const isRanged = PlayerbotAI::IsRangedDps(bot);

        // Healthy pack: aim at its middle, since AoeTrigger counts 8 yd around the current target. At
        // the finish the lowest-health focus takes over, so the detonations stagger instead of chaining.
        Unit* lasher = nullptr;
        if (isRanged)
        {
            lasher = GetFreyaLasherPackFocus(state);
            if (lasher && IsFreyaLasherPackFinishing(state, lasher->GetPosition()))
                lasher = GetFreyaRangedLasherFocus(state);
        }

        // A focus out of reach would walk the bot into the 15 yd blast, so shoot what's already in reach.
        // The two converge as lashers close on players.
        float const reach = isRanged ? sPlayerbotAIConfig.SpellDistance : ULDUAR_FREYA_MELEE_LASHER_RANGE;
        if (lasher && bot->GetExactDist2d(lasher) > reach)
            lasher = nullptr;

        if (!lasher)
            lasher = GetFreyaLocalLasherTarget(botAI, state, currentTarget, reach);

        priority.push_back(lasher);
    }

    priority.push_back(AI_VALUE2(Unit*, "find target", "freya"));

    Unit* target = nullptr;
    for (Unit* candidate : priority)
    {
        if (candidate && candidate->IsAlive())
        {
            target = candidate;
            break;
        }
    }

    // Keep the current target unless something strictly higher is up, so add churn can't reset swing timers.
    // Not the trio slot: the split is already stable, and stickiness would freeze each bot's first pick.
    auto const priorityIndex = [&priority](Unit* unit) -> size_t
    {
        if (!unit || !unit->IsAlive())
            return priority.size();

        for (size_t i = 0; i < priority.size(); ++i)
        {
            if (priority[i] == unit)
                return i;
        }

        return priority.size();
    };

    if (currentTarget && currentTarget != trioMember && target != trioMember &&
        priorityIndex(currentTarget) <= priorityIndex(target))
        target = currentTarget;

    return target ? target : AI_VALUE(Unit*, "dps target");
}

bool FreyaTankAddsAction::isUseful()
{
    FreyaTankAddsTrigger trigger(botAI);
    return trigger.IsActive();
}

bool FreyaTankAddsAction::Execute(Event /*event*/)
{
    FreyaWaveState state;
    GatherFreyaWaveState(botAI, state);

    Unit* currentTarget = AI_VALUE(Unit*, "current target");

    Unit* target = GetFreyaTankTarget(botAI, state, currentTarget);
    if (!target)
        return false;

    // Taunt only what the encounter owns: Freya may be on a human tank, taunting Storm Lasher or Water
    // Spirit means owning Tidal Wave, and a lasher wipes the taunt on its next 10s threat reset.
    bool const owned = target == state.snaplasher || target == state.conservator;
    if (owned && target->GetVictim() != bot && CastClassTaunt(botAI, target))
        return true;

    if (currentTarget != target)
        return Attack(target);

    return target == state.conservator ? ParkConservator(target) : false;
}

bool FreyaTankAddsAction::ParkConservator(Unit* conservator)
{
    Unit* spore = botAI->GetUnit(parkedSpore);
    if (!spore || !spore->IsAlive())
    {
        spore = GetFreyaConservatorSpore(conservator);
        parkedSpore = spore ? spore->GetGUID() : ObjectGuid::Empty;
    }

    if (!spore)
        return false;

    // The Conservator stops in melee range of the tank, so standing on the spore puts it inside the
    // 6 yd aura. A yard of slack so it isn't nudged back and forth.
    if (conservator->GetExactDist2d(spore) <= ULDUAR_FREYA_SPORE_RADIUS - 1.0f)
        return false;

    return MoveTo(bot->GetMapId(), spore->GetPositionX(), spore->GetPositionY(), spore->GetPositionZ(), false, false,
                  false, true, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool FreyaMoveToHealingSporeAction::isUseful()
{
    FreyaMoveToHealingSporeTrigger freyaMoveToHealingSporeTrigger(botAI);
    return freyaMoveToHealingSporeTrigger.IsActive();
}

bool FreyaMoveToHealingSporeAction::Execute(Event /*event*/)
{
    Unit* target = GetFreyaTargetSpore(botAI);
    if (!target)
        return false;

    float const distance = bot->GetExactDist2d(target);
    if (distance <= ULDUAR_FREYA_SPORE_STAND_RANGE)
        return false;

    // Stop at the aura's near edge: the centre is inside the spore's collision, IsDuplicateMove rejects
    // the repeat, and the tick falls through to the DPS chase, which walks the bot back out.
    float const ratio = ULDUAR_FREYA_SPORE_STAND_RANGE / distance;
    float const x = target->GetPositionX() + (bot->GetPositionX() - target->GetPositionX()) * ratio;
    float const y = target->GetPositionY() + (bot->GetPositionY() - target->GetPositionY()) * ratio;

    return MoveTo(target->GetMapId(), x, y, target->GetPositionZ(), false, false, false, true,
                  MovementPriority::MOVEMENT_COMBAT);
}

bool FreyaRedirectThreatAction::isUseful()
{
    return bot->getClass() == CLASS_HUNTER || bot->getClass() == CLASS_ROGUE;
}

Player* FreyaRedirectThreatAction::GetRedirectTank(Unit*& dumpTarget)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    // One scan walk for all three, since this runs every tick for every hunter and rogue. Lashers are
    // left out: they wipe threat every 10s, so no redirect helps.
    Unit* snaplasher = nullptr;
    Unit* conservator = nullptr;
    Unit* freya = nullptr;
    for (ObjectGuid const& guid : GetFreyaScan(botAI).PossibleTargets())
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        switch (unit->GetEntry())
        {
            case NPC_SNAPLASHER:
                if (!snaplasher)
                    snaplasher = unit;
                break;
            case NPC_ANCIENT_CONSERVATOR:
                if (!conservator)
                    conservator = unit;
                break;
            case NPC_FREYA:
                if (!freya)
                    freya = unit;
                break;
            default:
                break;
        }
    }

    if (snaplasher || conservator)
    {
        if (Player* assistTank = GetGroupAssistTank(bot, 0))
        {
            // The Snaplasher first, as on the add tank's own ladder: it is the one that kills whoever takes it.
            dumpTarget = snaplasher ? snaplasher : conservator;
            return assistTank;
        }
    }

    dumpTarget = freya;

    // Otherwise feed whoever is actually holding Freya, which survives a swap or a tank death.
    if (freya)
    {
        if (Unit* victim = freya->GetVictim())
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

bool FreyaRedirectThreatAction::Execute(Event /*event*/)
{
    Unit* dumpTarget = nullptr;
    Player* tank = GetRedirectTank(dumpTarget);
    if (!tank || tank == bot)
        return false;

    if (bot->getClass() == CLASS_ROGUE)
    {
        // Tricks redirects everything the rogue does for the next 6s, so there is no dump shot.
        return botAI->CanCastSpell("tricks of the trade", tank) && botAI->CastSpell("tricks of the trade", tank);
    }

    if (botAI->CanCastSpell("misdirection", tank))
        return botAI->CastSpell("misdirection", tank);

    // Misdirection only moves the next three shots' threat, so spend them on what that tank holds: shot
    // into Freya, they put the add tank on her table and nothing on the Snaplasher's.
    if (!dumpTarget || !bot->HasAura(SPELL_MISDIRECTION))
        return false;

    // Cast from here, so the trio sync multiplier never sees it and the floor has to be asked directly.
    FreyaWaveState state;
    GatherFreyaWaveState(botAI, state);
    if (FreyaTrioSyncSuppress(state, dumpTarget))
        return false;

    return botAI->CanCastSpell("steady shot", dumpTarget) && botAI->CastSpell("steady shot", dumpTarget);
}

bool FreyaBreakIronRootsAction::isUseful()
{
    FreyaBreakIronRootsTrigger freyaBreakIronRootsTrigger(botAI);
    return freyaBreakIronRootsTrigger.IsActive();
}

bool FreyaBreakIronRootsAction::Execute(Event /*event*/)
{
    // The root creature is summoned on the trapped bot; killing it removes the root DoT.
    Creature* root = bot->FindNearestCreature(NPC_FREYA_STRENGTHENED_IRON_ROOTS, ULDUAR_FREYA_IRON_ROOTS_SEARCH_RADIUS);
    if (!root)
        root = bot->FindNearestCreature(NPC_FREYA_IRON_ROOTS, ULDUAR_FREYA_IRON_ROOTS_SEARCH_RADIUS);

    if (!root || !root->IsAlive())
        return false;

    return Attack(root);
}

bool FreyaDodgeUnstableSunBeamAction::isUseful()
{
    FreyaDodgeUnstableSunBeamTrigger freyaDodgeUnstableSunBeamTrigger(botAI);
    return freyaDodgeUnstableSunBeamTrigger.IsActive();
}

bool FreyaDodgeUnstableSunBeamAction::Execute(Event /*event*/)
{
    if (FreyaBotCannotWalk(bot))
    {
        dodgeSpotMs = 0;
        return false;
    }

    // Route around every beam in range, or the bot sidesteps one into another. FleePosition can't: it
    // caps travel at FleeDistance (5 yd) and reads one hazard only.
    std::vector<Position> beams = GetFreyaSunBeamPositions(botAI, ULDUAR_FREYA_HAZARD_SEARCH_RADIUS);

    if (beams.empty())
    {
        dodgeSpotMs = 0;
        return false;
    }

    auto const stillClear = [&beams](Position const& spot)
    {
        for (Position const& beam : beams)
            if (beam.GetExactDist2d(spot.GetPositionX(), spot.GetPositionY()) < ULDUAR_FREYA_SUN_BEAM_CLEARANCE)
                return false;

        return true;
    };

    uint32 const now = getMSTime();

    // Dodge in flight: claim the tick without touching the motion master, so no lower node re-aims the
    // bot. A cast-pinned bot isn't in flight, so it falls through to the interrupt.
    if (dodgeSpotMs && getMSTimeDiff(dodgeSpotMs, now) < ULDUAR_FREYA_SUN_BEAM_LATCH_MS && stillClear(dodgeSpot) &&
        !bot->IsMovementPreventedByCasting() &&
        bot->GetExactDist2d(dodgeSpot.GetPositionX(), dodgeSpot.GetPositionY()) > CONTACT_DISTANCE)
        return true;

    dodgeSpotMs = 0;

    HazardSweepCache sweep;

    // Bombs and low lashers too: both escapes share a relevance, so a bot reading only its own hazard
    // flips between them forever.
    Position safe = FindNearestPositionClearOfHazards(
        bot,
        BuildFreyaEscapeHazards(GetFreyaNatureBombPositions(bot, ULDUAR_FREYA_HAZARD_SEARCH_RADIUS), beams,
                                GetFreyaEscapeLasherPositions(botAI, ULDUAR_FREYA_HAZARD_SEARCH_RADIUS)),
        ULDUAR_FREYA_HAZARD_SEARCH_RADIUS, &sweep);

    if (safe == Position())
        safe = FindNearestPositionClearOfHazards(bot, beams, ULDUAR_FREYA_SUN_BEAM_CLEARANCE,
                                                 ULDUAR_FREYA_HAZARD_SEARCH_RADIUS, &sweep);

    // Overlapping beams can leave nowhere that clears all of them by the full margin. Barely outside
    // beats standing in one, so fall back to the radius itself before giving up.
    if (safe == Position())
        safe = FindNearestPositionClearOfHazards(bot, beams, ULDUAR_FREYA_UNSTABLE_SUN_BEAM_RADIUS + 1.0f,
                                                 ULDUAR_FREYA_HAZARD_SEARCH_RADIUS, &sweep);

    if (safe == Position())
        return false;

    FreyaClearCastBlockingMove(bot);

    if (!MoveTo(bot->GetMapId(), safe.GetPositionX(), safe.GetPositionY(), safe.GetPositionZ(), false, false, false,
                true, MovementPriority::MOVEMENT_FORCED, true, false))
        return false;

    dodgeSpot = safe;
    dodgeSpotMs = now;

    return true;
}

bool FreyaNaturesFuryBailAction::isUseful()
{
    FreyaNaturesFuryBailTrigger trigger(botAI);
    return trigger.IsActive();
}

bool FreyaNaturesFuryBailAction::Execute(Event /*event*/)
{
    if (FreyaBotCannotWalk(bot))
    {
        bailSpotMs = 0;
        return false;
    }

    auto const stillClear = [this](Position const& spot)
    { return CountFreyaRaidNear(botAI, spot, ULDUAR_FREYA_NATURES_FURY_RADIUS, bot) == 0; };

    uint32 const now = getMSTime();

    // Claim the tick without touching the motion master while the walk is in flight, the same way the
    // beam dodge does: a re-issued MoveTo kills the spline that is carrying the mark out of the raid.
    if (bailSpotMs && getMSTimeDiff(bailSpotMs, now) < ULDUAR_FREYA_NATURES_FURY_LATCH_MS && stillClear(bailSpot) &&
        !bot->IsMovementPreventedByCasting() &&
        bot->GetExactDist2d(bailSpot.GetPositionX(), bailSpot.GetPositionY()) > CONTACT_DISTANCE)
        return true;

    bailSpotMs = 0;

    Position safe;
    if (Unit* shelter = GetFreyaNaturesFuryShelter(botAI))
    {
        // A spore beats open floor, as Pheromones is the only answer to the Grip. Stop at the aura's near
        // edge: the centre is inside the spore's collision and MoveTo would re-issue it until the mark ends.
        float const distance = bot->GetExactDist2d(shelter);
        if (distance > ULDUAR_FREYA_SPORE_STAND_RANGE)
        {
            float const ratio = ULDUAR_FREYA_SPORE_STAND_RANGE / distance;
            safe = Position(shelter->GetPositionX() + (bot->GetPositionX() - shelter->GetPositionX()) * ratio,
                            shelter->GetPositionY() + (bot->GetPositionY() - shelter->GetPositionY()) * ratio,
                            shelter->GetPositionZ(), 0.0f);
        }
    }

    // No free spore, or none up at all: open floor and ten seconds of being pacified is still cheaper
    // than five volleys into the raid.
    if (safe == Position())
        safe = FindNearestPositionClearOfHazards(bot, GetFreyaAlliesNear(botAI, ULDUAR_FREYA_HAZARD_SEARCH_RADIUS),
                                                 ULDUAR_FREYA_NATURES_FURY_CLEAR, ULDUAR_FREYA_HAZARD_SEARCH_RADIUS);

    if (safe == Position())
        return false;

    FreyaClearCastBlockingMove(bot);

    if (!MoveTo(bot->GetMapId(), safe.GetPositionX(), safe.GetPositionY(), safe.GetPositionZ(), false, false, false,
                true, MovementPriority::MOVEMENT_FORCED, true, false))
        return false;

    bailSpot = safe;
    bailSpotMs = now;

    return true;
}

bool FreyaStepOutOfSunbeamAction::isUseful()
{
    FreyaStepOutOfSunbeamTrigger trigger(botAI);
    return trigger.IsActive();
}

bool FreyaStepOutOfSunbeamAction::Execute(Event /*event*/)
{
    if (FreyaBotCannotWalk(bot))
    {
        stepSpotMs = 0;
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "freya");
    Unit* target = boss ? GetFreyaSunbeamTarget(boss) : nullptr;
    if (!target)
    {
        stepSpotMs = 0;
        return false;
    }

    Position const beam = target->GetPosition();

    auto const stillClear = [&beam](Position const& spot)
    { return beam.GetExactDist2d(spot.GetPositionX(), spot.GetPositionY()) >= ULDUAR_FREYA_SUNBEAM_CLEAR_RADIUS; };

    uint32 const now = getMSTime();

    if (stepSpotMs && getMSTimeDiff(stepSpotMs, now) < ULDUAR_FREYA_SUNBEAM_LATCH_MS && stillClear(stepSpot) &&
        !bot->IsMovementPreventedByCasting() &&
        bot->GetExactDist2d(stepSpot.GetPositionX(), stepSpot.GetPositionY()) > CONTACT_DISTANCE)
        return true;

    stepSpotMs = 0;

    // Bombs, beams and low lashers in the same sweep: the escapes share a relevance, so a bot reading one
    // hazard walks between them until something kills it.
    std::vector<EncounterHelpers::HazardCircle> hazards =
        GetFreyaEscapeHazards(botAI, ULDUAR_FREYA_HAZARD_SEARCH_RADIUS);
    hazards.emplace_back(beam, ULDUAR_FREYA_SUNBEAM_CLEAR_RADIUS);

    HazardSweepCache sweep;

    Position safe = FindNearestPositionClearOfHazards(bot, hazards, ULDUAR_FREYA_HAZARD_SEARCH_RADIUS, &sweep);

    // Drop the bombs before the blast when nothing clears both. A bomb is a six second fuse on a spot
    // the bot can still walk off, where a lasher under 15% goes off on its own schedule.
    if (safe == Position())
    {
        std::vector<EncounterHelpers::HazardCircle> blastAndBeam = BuildFreyaEscapeHazards(
            {}, {}, GetFreyaEscapeLasherPositions(botAI, ULDUAR_FREYA_HAZARD_SEARCH_RADIUS));
        blastAndBeam.emplace_back(beam, ULDUAR_FREYA_SUNBEAM_CLEAR_RADIUS);

        safe = FindNearestPositionClearOfHazards(bot, blastAndBeam, ULDUAR_FREYA_HAZARD_SEARCH_RADIUS, &sweep);
    }

    // Barely outside the beam beats standing in it.
    if (safe == Position())
        safe = FindNearestPositionClearOfHazards(bot, std::vector<Position>{beam}, ULDUAR_FREYA_SUNBEAM_CLEAR_RADIUS,
                                                 ULDUAR_FREYA_HAZARD_SEARCH_RADIUS, &sweep);

    if (safe == Position())
        return false;

    FreyaClearCastBlockingMove(bot);

    if (!MoveTo(bot->GetMapId(), safe.GetPositionX(), safe.GetPositionY(), safe.GetPositionZ(), false, false, false,
                true, MovementPriority::MOVEMENT_FORCED, true, false))
        return false;

    stepSpot = safe;
    stepSpotMs = now;

    return true;
}

bool FreyaTankHoldFreyaAction::isUseful()
{
    FreyaTankHoldFreyaTrigger trigger(botAI);
    return trigger.IsActive();
}

bool FreyaTankHoldFreyaAction::Execute(Event /*event*/)
{
    uint32 const now = getMSTime();

    if (holdMs && getMSTimeDiff(holdMs, now) < ULDUAR_FREYA_TANK_HOLD_LATCH_MS &&
        !bot->IsMovementPreventedByCasting() && bot->GetExactDist2d(&ULDUAR_FREYA_TANK_ANCHOR) > CONTACT_DISTANCE)
        return true;

    holdMs = 0;

    // Onto the anchor itself: Freya stops in melee range of the tank, so she settles a boss length short
    // anyway. MOVEMENT_COMBAT, so a bomb volley still outranks the walk home.
    if (!MoveTo(bot->GetMapId(), ULDUAR_FREYA_TANK_ANCHOR.GetPositionX(), ULDUAR_FREYA_TANK_ANCHOR.GetPositionY(),
                ULDUAR_FREYA_TANK_ANCHOR.GetPositionZ(), false, false, false, true,
                MovementPriority::MOVEMENT_COMBAT))
        return false;

    holdMs = now;

    return true;
}

bool FreyaLasherAboutToBlowAction::isUseful()
{
    FreyaLasherAboutToBlowTrigger trigger(botAI);
    return trigger.IsActive();
}

bool FreyaLasherAboutToBlowAction::Execute(Event /*event*/)
{
    FreyaWaveState state;
    GatherFreyaWaveState(botAI, state);

    return StepOutOfBlasts(
        GetFreyaLowLasherPositions(botAI, state, ULDUAR_FREYA_LASHER_BAIL_PCT, ULDUAR_FREYA_HAZARD_SEARCH_RADIUS),
        ULDUAR_FREYA_LASHER_PACK_CLEAR);
}

bool FreyaLasherBailAction::StepOutOfBlasts(std::vector<Position> const& blasts, float clearance)
{
    if (FreyaBotCannotWalk(bot) || blasts.empty())
    {
        bailSpotMs = 0;
        return false;
    }

    // What an exit has to keep, whichever sweep found it.
    float const edge = ULDUAR_FREYA_DETONATE_RADIUS + 1.0f;

    auto const stillClear = [&blasts, edge](Position const& spot)
    {
        for (Position const& blast : blasts)
            if (blast.GetExactDist2d(spot.GetPositionX(), spot.GetPositionY()) < edge)
                return false;

        return true;
    };

    uint32 const now = getMSTime();

    // Walk in flight: claim the tick without touching the motion master, so no lower node re-aims the
    // bot onto the pile. A cast-pinned bot isn't in flight, so it falls through to the interrupt.
    if (bailSpotMs && getMSTimeDiff(bailSpotMs, now) < ULDUAR_FREYA_LASHER_BAIL_LATCH_MS && stillClear(bailSpot) &&
        !bot->IsMovementPreventedByCasting() &&
        bot->GetExactDist2d(bailSpot.GetPositionX(), bailSpot.GetPositionY()) > CONTACT_DISTANCE)
        return true;

    bailSpotMs = 0;

    HazardSweepCache sweep;

    Position safe =
        FindNearestPositionClearOfHazards(bot, blasts, clearance, ULDUAR_FREYA_HAZARD_SEARCH_RADIUS, &sweep);

    // At the finish half the pile is under the bail line at once, and nowhere in the room clears all of
    // them by the full margin. Barely outside the blast beats standing in the middle of it.
    if (safe == Position() && clearance > edge)
        safe = FindNearestPositionClearOfHazards(bot, blasts, edge, ULDUAR_FREYA_HAZARD_SEARCH_RADIUS, &sweep);

    if (safe == Position())
        return false;

    FreyaClearCastBlockingMove(bot);

    if (!MoveTo(bot->GetMapId(), safe.GetPositionX(), safe.GetPositionY(), safe.GetPositionZ(), false, false, false,
                true, MovementPriority::MOVEMENT_FORCED, true, false))
        return false;

    bailSpot = safe;
    bailSpotMs = now;

    return true;
}

bool FreyaRangedCampAction::isUseful()
{
    FreyaRangedCampTrigger trigger(botAI);
    return trigger.IsActive();
}

bool FreyaRangedCampAction::Execute(Event /*event*/)
{
    FreyaWaveState state;
    GatherFreyaWaveState(botAI, state);

    // The trigger only lets a bot through here with a low lasher in reach when it stands in the blast. Each
    // bot takes its own nearest exit: the ring below is a spacing short of the camp, which can be inside one.
    std::vector<Position> const blasts =
        GetFreyaLowLasherPositions(botAI, state, ULDUAR_FREYA_LASHER_BAIL_PCT, ULDUAR_FREYA_HAZARD_SEARCH_RADIUS);
    if (!blasts.empty())
        return StepOutOfBlasts(blasts, ULDUAR_FREYA_LASHER_CAMP_STANDOFF);

    Position const camp = GetFreyaLasherCampSpot(botAI);
    if (camp == Position())
        return false;

    // Stop a spacing short of the camp on the bearing the bot came in on, so the back line lands as a
    // ring instead of on one square, with no shared state.
    float const distance = bot->GetExactDist2d(camp.GetPositionX(), camp.GetPositionY());
    float const bearing =
        distance > CONTACT_DISTANCE ? camp.GetAngle(bot->GetPositionX(), bot->GetPositionY()) : bot->GetOrientation();
    float const x = camp.GetPositionX() + std::cos(bearing) * ULDUAR_FREYA_RANGED_CAMP_SPACING;
    float const y = camp.GetPositionY() + std::sin(bearing) * ULDUAR_FREYA_RANGED_CAMP_SPACING;

    // No arrival latch: the trigger's tolerance stops the churn, and a latch would swallow the re-aim
    // when the anchor moves. MOVEMENT_COMBAT, so the bomb and beam escapes still outrank it.
    return MoveTo(bot->GetMapId(), x, y, camp.GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT);
}

bool FreyaFrostNovaLashersAction::isUseful()
{
    FreyaFrostNovaLashersTrigger trigger(botAI);
    return trigger.IsActive();
}

bool FreyaFrostNovaLashersAction::Execute(Event /*event*/)
{
    // On self, not through "frost nova": that gates on the current target being within 10 yd, and a
    // ranged mage's target is usually across the room. The nova is a sphere on the caster.
    return botAI->CastSpell("frost nova", bot);
}

bool FreyaTrapLashersAction::isUseful()
{
    FreyaTrapLashersTrigger trigger(botAI);
    return trigger.IsActive();
}

bool FreyaTrapLashersAction::Execute(Event /*event*/)
{
    // No walk to a post. The trap drops at the hunter's feet, which is exactly where the lasher that
    // picked this bot is heading, so it arms in the one place something is guaranteed to walk over.
    return botAI->CastSpell("frost trap", bot);
}

bool FreyaSummonArmyAction::isUseful()
{
    FreyaSummonArmyTrigger trigger(botAI);
    return trigger.IsActive();
}

bool FreyaSummonArmyAction::Execute(Event /*event*/)
{
    // By name, not through the class node: IsBurstCooldownAction matches on the action name, and the
    // Ulduar burst gate holds those until Attuned to Nature is gone.
    return botAI->CastSpell("army of the dead", bot);
}

bool FreyaGroundTremorHoldCastAction::isUseful()
{
    FreyaGroundTremorHoldCastTrigger trigger(botAI);
    return trigger.IsActive();
}

bool FreyaGroundTremorHoldCastAction::Execute(Event /*event*/)
{
    Unit* boss = GetFreyaBossByEntry(botAI);
    if (!boss)
        return false;

    Spell* tremor = boss->GetCurrentSpell(CURRENT_GENERIC_SPELL);

    // The cast gate already stops long casts from starting, so a channel already running is all that's
    // left. m_timer holds what's left in either slot, a channel's included, so one compare covers both.
    Spell* own = bot->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!own)
        own = bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL);

    if (!tremor || !own)
        return false;

    // A cast that still lands first is worth finishing. Only the ones the tremor would cut are thrown
    // away, and those are the expensive ones anyway: being cut is what applies the 10s school lockout.
    if (own->GetCastTimeRemaining() < tremor->GetCastTimeRemaining())
        return false;

    bot->CastStop();

    return true;
}
