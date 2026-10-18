/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldEncounter_FlameLeviathan.h"

#include "Creature.h"
#include "EncounterHelpers.h"
#include "Group.h"
#include "InstanceScript.h"
#include "Map.h"
#include "ObjectDefines.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RaidInstanceState.h"
#include "Timer.h"
#include "UldEncounterGate.h"
#include "UldHardMode.h"
#include "UldScripts.h"
#include "UldTargets.h"
#include "Unit.h"
#include "Vehicle.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_map>
#include <vector>

using namespace EncounterHelpers;

// The four NPC_FREYA_WARD_TARGET spawn points from boss_flame_leviathan.cpp, in ring order.
std::vector<Position> const ULDUAR_FL_ARENA_CORNERS = {
    Position(159.4f, 64.1f, 409.8f),
    Position(382.9f, 74.0f, 411.6f),
    Position(374.0f, -141.0f, 411.0f),
    Position(157.7f, -140.3f, 409.8f)
};

// The instance script tracks him anywhere in Ulduar. The target scan stops at SightDistance (100 yd), and
// one bot past it reading "no boss" would wipe the shared state for the whole raid.
Unit* FlameLeviathanBoss(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    InstanceScript* instance = bot ? bot->GetInstanceScript() : nullptr;
    if (!instance)
        return botAI ? GetUldUnitByEntry(botAI, NPC_FLAME_LEVIATHAN) : nullptr;

    // The entry as well: index 0 names some other boss in any instance that is not Ulduar.
    Creature* boss = instance->GetCreature(ULD_BOSS_LEVIATHAN);
    return boss && boss->GetEntry() == NPC_FLAME_LEVIATHAN && boss->IsInWorld() && boss->IsAlive() ? boss : nullptr;
}

namespace
{
// Raid-wide answers, folded once per instance per tick rather than once per bot. Sharing them stops two
// vehicles disagreeing about whose channel it is.
struct FlameLeviathanState
{
    // Who has already fired into the running channel. Cleared once he stops channelling; the 10 s gap
    // between channels guarantees a scan sees that.
    ObjectGuid ventClaimedBy{};

    // The vehicle currently wearing Pursued. Battering Ram is a 25 yd sphere centred on it, so this
    // is the thing every other vehicle measures itself against.
    ObjectGuid pursuedVehicle{};

    // The hull he rams while nobody is Pursued: his threat victim's. Empty whenever the aura is on
    // someone.
    ObjectGuid ramTarget{};

    // When the running or last Flame Vents channel began, stamped on the first scan that sees it.
    uint32 ventStartMs = 0;
    bool ventChanneling = false;

    // Vehicles that cannot move. Hodir's Fury carries an undispellable 60 s stun, and a frozen
    // vehicle has to be counted out rather than waited on, or it keeps its roles.
    std::unordered_map<ObjectGuid, bool> frozen{};

    // The siege hull kept on the boss for Flame Vents. Latched: re-electing each scan would drag a posted
    // engine in and back out for one Hodir's Fury.
    ObjectGuid ventReserve{};

    // Siege hulls in guid order, latched the first time anyone asks. Ranking only the live ones would
    // renumber every engine below a loss, so one hull dying would swap all four corners at once.
    std::vector<ObjectGuid> siegeOrder;

    // Hodir's Fury reticles, when each was first seen stunned, and the ones whose strike is still to
    // land. Read off the reticle's stun because nothing else marks the commit.
    std::vector<ObjectGuid> furyReticles;
    std::unordered_map<ObjectGuid, uint32> furyStunnedMs;
    GuidUnorderedSet furyArmed{};

    uint32 scanMs = 0;
};

RaidInstanceState<FlameLeviathanState> flStates;

// Long enough that the scan is cheap, short enough that a 31s Pursued cycle is never missed.
constexpr uint32 ULDUAR_FL_SCAN_INTERVAL_MS = 200;

FlameLeviathanState& FlameLeviathanStateFor(Player* bot) { return flStates.For(bot->GetInstanceId()); }

// Arms a reticle from the tick it is first seen stunned, for the fuse and the strike's flight.
void TickHodirsFury(FlameLeviathanState& state, PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !(FlameLeviathanActiveTowerMask(botAI) & FL_TOWER_FROST))
        return;

    // SummonTowerHelpers places two, and they live for the whole pull.
    if (state.furyReticles.size() < 2)
    {
        for (auto const& guid : botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get())
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->GetEntry() == NPC_FL_HODIRS_FURY_TARGET &&
                std::find(state.furyReticles.begin(), state.furyReticles.end(), guid) == state.furyReticles.end())
                state.furyReticles.push_back(guid);
        }
    }

    Map* map = bot->GetMap();
    uint32 const now = getMSTime();
    for (ObjectGuid const& guid : state.furyReticles)
    {
        Creature* reticle = map ? map->GetCreature(guid) : nullptr;
        if (!reticle || !reticle->IsAlive() || !reticle->HasUnitState(UNIT_STATE_STUNNED))
        {
            state.furyStunnedMs.erase(guid);
            state.furyArmed.erase(guid);
            continue;
        }

        // First seen already stunned counts as a fresh commit: dodging a spent one costs a short drive,
        // standing under a live one costs the hull for a minute.
        auto const [it, fresh] = state.furyStunnedMs.try_emplace(guid, now);
        if (!fresh && getMSTimeDiff(it->second, now) >= ULDUAR_FL_FURY_STUN_MS)
            it->second = now;

        if (getMSTimeDiff(it->second, now) >= ULDUAR_FL_FURY_ARMED_MS)
        {
            state.furyArmed.erase(guid);
            continue;
        }

        state.furyArmed.insert(guid);
    }
}

// The hull a threat victim belongs to. Threat here sits on the vehicle creature, but a turret or a
// rider can hold it too, so walk up the seats.
Unit* FlameLeviathanHullOf(Unit* unit)
{
    for (Unit* each = unit; each; each = each->GetVehicleBase())
    {
        switch (each->GetEntry())
        {
            case NPC_SALVAGED_SIEGE_ENGINE:
            case NPC_SALVAGED_DEMOLISHER:
            case NPC_VEHICLE_CHOPPER:
                return each;
            default:
                break;
        }
    }

    return nullptr;
}

void ElectVentReserve(FlameLeviathanState& state, Player* bot, Unit* boss)
{
    // With the Life tower down nobody posts, and every engine holds station as it always did.
    Group* group = bot->GetGroup();
    if (!group || !boss->HasAura(SPELL_FL_TOWER_OF_LIFE))
    {
        state.ventReserve = ObjectGuid::Empty;
        return;
    }

    Unit* best = nullptr;
    float bestDist = 0.0f;
    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* member = gref->GetSource();
        if (!member || !FlameLeviathanIsDriver(member))
            continue;

        Unit* hull = member->GetVehicleBase();
        if (!hull || hull->GetEntry() != NPC_SALVAGED_SIEGE_ENGINE)
            continue;

        if (!FlameLeviathanCrewUsable(member) || FlameLeviathanIsPursued(member))
            continue;

        if (hull->GetGUID() == state.ventReserve)
            return;

        float const dist = hull->GetExactDist2d(boss);
        if (!best || dist < bestDist || (dist == bestDist && hull->GetGUID() < best->GetGUID()))
        {
            best = hull;
            bestDist = dist;
        }
    }

    state.ventReserve = best ? best->GetGUID() : ObjectGuid::Empty;
}

// Everything that has to be true once per instance per tick rather than once per bot: expire a vent
// claim, notice a Pursued switch, and record which vehicles are frozen.
void TickFlameLeviathan(PlayerbotAI* botAI, Player* bot)
{
    FlameLeviathanState& state = FlameLeviathanStateFor(bot);
    if (state.scanMs && GetMSTimeDiffToNow(state.scanMs) < ULDUAR_FL_SCAN_INTERVAL_MS)
        return;

    state.scanMs = getMSTime();

    Unit* boss = FlameLeviathanBoss(botAI);

    // Off the boss, never off the calling bot: one bot dropping combat is not the pull ending.
    if (!boss || !boss->IsInCombat())
    {
        state.ventClaimedBy = ObjectGuid::Empty;
        state.pursuedVehicle = ObjectGuid::Empty;
        state.ramTarget = ObjectGuid::Empty;
        state.ventReserve = ObjectGuid::Empty;
        state.ventStartMs = 0;
        state.ventChanneling = false;
        state.frozen.clear();
        state.siegeOrder.clear();
        state.furyReticles.clear();
        state.furyStunnedMs.clear();
        state.furyArmed.clear();
        return;
    }

    bool const channeling = FlameLeviathanIsVentChanneling(boss);
    if (!channeling)
        state.ventClaimedBy = ObjectGuid::Empty;
    else if (!state.ventChanneling)
        state.ventStartMs = getMSTime();
    state.ventChanneling = channeling;

    TickHodirsFury(state, botAI, bot);

    // Pursued is read off the vehicles: the aura lands on whatever unit his spell picked, never a gunner's
    // own guid. Keyed on the vehicle, which also folds a crew into one entry.
    ObjectGuid pursued;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* member = gref->GetSource();
            if (!member || !member->IsAlive())
                continue;

            Unit* base = FlameLeviathanRiddenVehicle(member);
            if (!base)
                continue;

            if (!pursued && base->HasAura(SPELL_FL_PURSUED))
                pursued = base->GetGUID();

            state.frozen[base->GetGUID()] = base->HasUnitState(UNIT_STATE_NOT_MOVE);

            // A claim held by an engine that froze mid-channel would block the re-election for the
            // rest of it, and the channel is only ten seconds long. Hand it back instead.
            if (state.ventClaimedBy == member->GetGUID() && !FlameLeviathanCrewUsable(member))
                state.ventClaimedBy = ObjectGuid::Empty;
        }
    }

    state.pursuedVehicle = pursued;

    // With the Pursued hull dead he rams his threat victim until the next Pursued lands.
    ObjectGuid rammed;
    if (!pursued)
        if (Unit* hull = FlameLeviathanHullOf(boss->GetVictim()))
            rammed = hull->GetGUID();
    state.ramTarget = rammed;

    ElectVentReserve(state, bot, boss);
}
}  // namespace

void FlameLeviathanTick(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (bot && bot->GetMapId() == ULDUAR_MAP_ID)
        TickFlameLeviathan(botAI, bot);
}

bool FlameLeviathanEngaged(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (!bot || bot->GetMapId() != ULDUAR_MAP_ID)
        return false;

    Unit* boss = FlameLeviathanBoss(botAI);
    if (!boss || boss->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
        return false;

    // His combat, not the rider's: threat sits on the vehicle, so a bot that drives far enough out drops
    // combat mid-pull, and the mover and its veto would hand the wheel to "follow".
    if (boss->IsInCombat())
        return true;

    // He can be out of combat for a moment while the raid is driving in, so the rider still counts.
    return bot->IsInCombat();
}

Unit* FlameLeviathanRiddenVehicle(Player* bot)
{
    Unit* base = bot ? bot->GetVehicleBase() : nullptr;
    if (!base)
        return nullptr;

    // A gunner rides seat 0 of a turret creature that is itself a passenger of the real vehicle.
    if (Unit* parent = base->GetVehicleBase())
        return parent;

    return base;
}

bool FlameLeviathanIsDriver(Player* bot)
{
    Unit* base = bot ? bot->GetVehicleBase() : nullptr;
    if (!base)
        return false;

    uint32 const entry = base->GetEntry();
    return entry == NPC_SALVAGED_SIEGE_ENGINE || entry == NPC_VEHICLE_CHOPPER ||
           entry == NPC_SALVAGED_DEMOLISHER;
}

bool FlameLeviathanIsPursued(Player* bot)
{
    Unit* base = bot ? bot->GetVehicleBase() : nullptr;
    if (!base)
        return false;

    if (base->HasAura(SPELL_FL_PURSUED))
        return true;

    Unit* parent = base->GetVehicleBase();
    return parent && parent->HasAura(SPELL_FL_PURSUED);
}

bool FlameLeviathanIsVentChanneling(Unit* boss)
{
    if (!boss)
        return false;

    Spell* channel = boss->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
    return channel && channel->m_spellInfo && channel->m_spellInfo->Id == SPELL_FL_FLAME_VENTS;
}

// Not IsWithinCombatRange: that adds both combat reaches, and his is 15, so 25 yd answers yes out to
// 47.7. The cone check adds only the target's GetObjectSize.
static bool FlameLeviathanInConeRange(Unit* caster, Unit* target, float radius)
{
    return caster && target && caster->GetExactDist(target) <= radius + target->GetObjectSize();
}

static bool FlameLeviathanCanElectroshock(Unit* siegeEngine, Unit* boss)
{
    return siegeEngine && boss && !siegeEngine->HasSpellCooldown(SPELL_FL_ELECTROSHOCK) &&
           siegeEngine->GetPower(POWER_ENERGY) >= ULDUAR_FL_ELECTROSHOCK_COST &&
           FlameLeviathanInConeRange(siegeEngine, boss, ULDUAR_FL_ELECTROSHOCK_CONE_RADIUS);
}

bool FlameLeviathanInCone(Unit* vehicleBase, Unit* target, float halfAngle, float radius)
{
    // HasInArc splits what it is handed, so the full cone width goes in.
    return vehicleBase && target && FlameLeviathanInConeRange(vehicleBase, target, radius) &&
           vehicleBase->HasInArc(halfAngle * 2.0f, target);
}

bool FlameLeviathanFaceForCone(Unit* vehicleBase, Unit* target, float halfAngle, float radius)
{
    if (!vehicleBase || !target)
        return false;

    if (!FlameLeviathanInConeRange(vehicleBase, target, radius))
        return false;

    if (vehicleBase->HasInArc(halfAngle * 2.0f, target))
        return true;

    // Outside the cone but inside CAST_ANGLE_IN_FRONT, so CastVehicleSpell would not have turned and
    // the shot would have gone nowhere. Spend the tick turning and let a later one fire.
    vehicleBase->SetFacingToObject(target);
    return false;
}

bool FlameLeviathanIsVentInterrupter(PlayerbotAI* botAI, Player* bot)
{
    Unit* base = bot ? bot->GetVehicleBase() : nullptr;
    if (!base || base->GetEntry() != NPC_SALVAGED_SIEGE_ENGINE)
        return false;

    Unit* boss = FlameLeviathanBoss(botAI);
    if (!FlameLeviathanCanElectroshock(base, boss))
        return false;

    // A stunned engine can't fire, and claiming the channel would waste it. A Pursued one would turn to
    // face him mid-escape.
    if (!FlameLeviathanCrewUsable(bot) || FlameLeviathanIsPursued(bot))
        return false;

    // One shot per channel. Unclaimed, the 20 energy the winner spends promotes the next engine, and the
    // whole line empties into one channel milliseconds apart.
    ObjectGuid const claimed = FlameLeviathanStateFor(bot).ventClaimedBy;
    if (claimed)
        return claimed == bot->GetGUID();

    Group* group = bot->GetGroup();
    if (!group)
        return true;

    uint32 const myEnergy = base->GetPower(POWER_ENERGY);
    ObjectGuid const myGuid = bot->GetGUID();

    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* member = gref->GetSource();
        if (!member || member == bot || !FlameLeviathanCrewUsable(member) || FlameLeviathanIsPursued(member))
            continue;

        Unit* memberBase = member->GetVehicleBase();
        if (!memberBase || memberBase->GetEntry() != NPC_SALVAGED_SIEGE_ENGINE)
            continue;

        if (!FlameLeviathanCanElectroshock(memberBase, boss))
            continue;

        uint32 const energy = memberBase->GetPower(POWER_ENERGY);

        // Highest energy wins, guid breaks ties. Casting spends 20, which drops the caster to the
        // back of its own queue, so the duty rotates across channels with nobody having to be told.
        if (energy > myEnergy || (energy == myEnergy && member->GetGUID() < myGuid))
            return false;
    }

    return true;
}

void FlameLeviathanClaimVentChannel(Player* bot)
{
    if (bot)
        FlameLeviathanStateFor(bot).ventClaimedBy = bot->GetGUID();
}

uint32 FlameLeviathanMsToNextVent(Player* bot, Unit* boss)
{
    if (FlameLeviathanIsVentChanneling(boss))
        return 0;

    uint32 const start = bot ? FlameLeviathanStateFor(bot).ventStartMs : 0;
    if (!start)
        return std::numeric_limits<uint32>::max();

    return ULDUAR_FL_VENT_INTERVAL_MS - getMSTimeDiff(start, getMSTime()) % ULDUAR_FL_VENT_INTERVAL_MS;
}

bool FlameLeviathanVentWindowOpen(Player* bot, Unit* boss)
{
    uint32 const toVent = FlameLeviathanMsToNextVent(bot, boss);
    return toVent == 0 || toVent <= ULDUAR_FL_VENT_RUSH_LEAD_MS;
}

bool FlameLeviathanCrewUsable(Player* member)
{
    if (!member || !member->IsAlive())
        return false;

    // The rider is checked for the stun only: Vehicle::AddPassenger roots every passenger, driver
    // included, so UNIT_STATE_NOT_MOVE on a rider just means seated.
    Unit* base = FlameLeviathanRiddenVehicle(member);
    return base && !base->HasUnitState(UNIT_STATE_NOT_MOVE) && !member->HasUnitState(UNIT_STATE_STUNNED);
}

Unit* FlameLeviathanFrozenVehicle(Player* bot, Unit* from, float minRange, float maxRange)
{
    Group* group = bot ? bot->GetGroup() : nullptr;
    if (!group || !from)
        return nullptr;

    Unit* best = nullptr;
    float bestDist = maxRange;

    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* member = gref->GetSource();
        if (!member || !member->IsAlive())
            continue;

        Unit* base = FlameLeviathanRiddenVehicle(member);
        if (!base || base == from || !base->HasAura(SPELL_FL_HODIRS_FURY_STUN))
            continue;

        float const dist = from->GetExactDist2d(base);
        if (dist < minRange || dist > bestDist)
            continue;

        best = base;
        bestDist = dist;
    }

    return best;
}

// An add on a hull that sits at a post is that post's to kill. A gunner fires from a turret bolted
// onto the hull, so either link counts as riding it.
static bool FlameLeviathanHeldByAnotherPost(Unit* add, Unit* from)
{
    Unit* victim = add->GetVictim();
    if (!victim || victim->GetEntry() != NPC_SALVAGED_SIEGE_ENGINE)
        return false;

    if (from == victim || from->GetVehicleBase() == victim)
        return false;

    bool posted = false;
    for (uint8 i = 0; i < ULDUAR_FL_ARENA_CORNERS.size(); ++i)
        if (victim->GetExactDist2d(FlameLeviathanCornerPostPoint(i)) <= ULDUAR_FL_CORNER_HOLD_RADIUS)
        {
            posted = true;
            break;
        }

    if (!posted)
        return false;

    // The claim holds only while the holder can shoot: under Fire Cannon's 10 yd floor its only answer is
    // Ram, a cone, and a claim it can't back shields the add from everyone.
    if (add->GetExactDist2d(victim) >= ULDUAR_FL_FIRE_CANNON_MIN_RANGE)
        return true;

    return FlameLeviathanInCone(victim, add, ULDUAR_FL_RAM_CONE_HALF_ANGLE, ULDUAR_FL_RAM_CONE_RADIUS);
}

Unit* FlameLeviathanBestAdd(PlayerbotAI* botAI, Unit* from, float minRange, float maxRange, float splash,
                            Position const* around, float aroundRadius)
{
    if (!botAI || !from)
        return nullptr;

    std::vector<Unit*> adds;
    for (auto const& guid : botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets")->Get())
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        uint32 const entry = unit->GetEntry();
        if (entry == NPC_FL_WRITHING_LASHER || entry == NPC_FL_WARD_OF_LIFE)
            adds.push_back(unit);
    }

    Unit* best = nullptr;
    uint32 bestCluster = 0;
    float bestDist = 0.0f;

    for (Unit* add : adds)
    {
        float const dist = from->GetExactDist2d(add);
        if (dist < minRange || dist > maxRange)
            continue;

        if (around && add->GetExactDist2d(*around) > aroundRadius)
            continue;

        // Still counted in other adds' clusters: a held add is still there for the splash to hit.
        if (FlameLeviathanHeldByAnotherPost(add, from))
            continue;

        // Counts itself, which is what makes a lone add score 1 and keeps the comparison honest
        // against a pair. Skipped entirely for a single-target pick, where it would decide nothing.
        uint32 cluster = 0;
        if (splash > 0.0f)
            for (Unit* other : adds)
                if (add->GetExactDist2d(other) <= splash)
                    ++cluster;

        if (!best || cluster > bestCluster || (cluster == bestCluster && dist < bestDist))
        {
            best = add;
            bestCluster = cluster;
            bestDist = dist;
        }
    }

    return best;
}

Unit* FlameLeviathanPursuedVehicle(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot)
        return nullptr;

    ObjectGuid const guid = FlameLeviathanStateFor(bot).pursuedVehicle;
    return guid ? botAI->GetUnit(guid) : nullptr;
}

Unit* FlameLeviathanRamCentre(PlayerbotAI* botAI, Player* bot)
{
    if (Unit* pursued = FlameLeviathanPursuedVehicle(botAI, bot))
        return pursued;

    ObjectGuid const guid = botAI && bot ? FlameLeviathanStateFor(bot).ramTarget : ObjectGuid::Empty;
    return guid ? botAI->GetUnit(guid) : nullptr;
}

bool FlameLeviathanIsRamTarget(Player* bot)
{
    Unit* hull = FlameLeviathanRiddenVehicle(bot);
    ObjectGuid const target = hull ? FlameLeviathanStateFor(bot).ramTarget : ObjectGuid::Empty;
    return target && hull->GetGUID() == target;
}

bool FlameLeviathanInBatteringRamBlast(Unit* vehicleBase, Unit* centre)
{
    if (!vehicleBase || !centre || vehicleBase == centre)
        return false;

    // A sphere round the rammed vehicle, so his facing has nothing to do with it.
    return vehicleBase->GetExactDist2d(centre) <= ULDUAR_FL_BATTERING_RAM_RADIUS + vehicleBase->GetObjectSize();
}

bool FlameLeviathanShouldClearBatteringRam(PlayerbotAI* botAI, Player* bot)
{
    // Standing in his own blast is the rammed vehicle's whole job, and it is already kiting.
    if (!bot || FlameLeviathanIsPursued(bot) || FlameLeviathanIsRamTarget(bot))
        return false;

    // The ridden vehicle, not the seat: a gunner's GetVehicleBase is the bolted-on turret, whose
    // position is the parent's anyway but whose object size is not.
    Unit* vehicleBase = FlameLeviathanRiddenVehicle(bot);
    Unit* centre = FlameLeviathanRamCentre(botAI, bot);
    Unit* boss = FlameLeviathanBoss(botAI);
    if (!vehicleBase || !centre || !boss)
        return false;

    // His own cast test, copied from the script: outside it the blast can't land. A hand-rolled 15 yd
    // would be far too tight, since IsWithinCombatRange adds both reaches.
    if (!boss->IsWithinCombatRange(centre, ULDUAR_FL_BATTERING_RAM_CAST_RANGE))
        return false;

    return FlameLeviathanInBatteringRamBlast(vehicleBase, centre);
}

bool FlameLeviathanIsTarLead(PlayerbotAI* /*botAI*/, Player* bot)
{
    Unit* base = bot ? bot->GetVehicleBase() : nullptr;
    if (!base || base->GetEntry() != NPC_VEHICLE_CHOPPER)
        return false;

    // A pursued chopper kites away with its back turned, dropping tar in his path for free. It doesn't
    // need the lead slot, and taking it would strand the role.
    if (FlameLeviathanIsPursued(bot))
        return false;

    // A frozen chopper can't lead, and winning the election would hold the slot for the full 60 s with
    // nobody else laying tar.
    if (!FlameLeviathanCrewUsable(bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return true;

    ObjectGuid const myGuid = bot->GetGUID();
    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* member = gref->GetSource();
        if (!member || member == bot || !FlameLeviathanCrewUsable(member))
            continue;

        Unit* memberBase = member->GetVehicleBase();
        if (!memberBase || memberBase->GetEntry() != NPC_VEHICLE_CHOPPER)
            continue;

        if (FlameLeviathanIsPursued(member))
            continue;

        if (member->GetGUID() < myGuid)
            return false;
    }

    return true;
}

// Rank of this bot's siege engine in the guid order latched at first ask, or -1. Every hull keeps its
// slot, or a loss or Pursued switch would swap corners. Drivers only, one per hull, so no dedupe.
static int32 FlameLeviathanSiegeRank(Player* bot)
{
    if (!bot || !FlameLeviathanIsDriver(bot))
        return -1;

    Unit* base = bot->GetVehicleBase();
    if (!base || base->GetEntry() != NPC_SALVAGED_SIEGE_ENGINE)
        return -1;

    FlameLeviathanState& state = FlameLeviathanStateFor(bot);
    ObjectGuid const myGuid = base->GetGUID();

    if (state.siegeOrder.empty())
    {
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
            {
                Player* member = gref->GetSource();
                if (!member || !FlameLeviathanIsDriver(member))
                    continue;

                Unit* memberBase = member->GetVehicleBase();
                if (!memberBase || memberBase->GetEntry() != NPC_SALVAGED_SIEGE_ENGINE)
                    continue;

                state.siegeOrder.push_back(memberBase->GetGUID());
            }
        }

        std::sort(state.siegeOrder.begin(), state.siegeOrder.end());
    }

    auto const it = std::find(state.siegeOrder.begin(), state.siegeOrder.end(), myGuid);
    if (it == state.siegeOrder.end())
    {
        // A hull nobody was driving when the order latched. Give it a slot behind the originals
        // rather than refusing, so a bot that took a replacement engine still posts somewhere.
        state.siegeOrder.push_back(myGuid);
        return static_cast<int32>(state.siegeOrder.size()) - 1;
    }

    return static_cast<int32>(std::distance(state.siegeOrder.begin(), it));
}

int8 FlameLeviathanCornerPost(PlayerbotAI* botAI, Player* bot)
{
    Unit* boss = FlameLeviathanBoss(botAI);
    if (!bot || !boss || !boss->HasAura(SPELL_FL_TOWER_OF_LIFE))
        return -1;

    // A pursued engine is kiting and a frozen one cannot drive, so neither drives to its corner.
    if (FlameLeviathanIsPursued(bot) || !FlameLeviathanCrewUsable(bot))
        return -1;

    if (FlameLeviathanIsVentReserve(bot))
        return -1;

    int32 rank = FlameLeviathanSiegeRank(bot);
    if (rank < 0)
        return -1;

    // Rank 0 owns no corner of its own, so it covers the one the reserve left. The reserve moves only
    // when it freezes, dies or is Pursued, so this swaps two engines at most.
    if (rank == 0)
    {
        FlameLeviathanState const& state = FlameLeviathanStateFor(bot);
        auto const it = std::find(state.siegeOrder.begin(), state.siegeOrder.end(), state.ventReserve);
        if (it == state.siegeOrder.end())
            return -1;

        rank = static_cast<int32>(std::distance(state.siegeOrder.begin(), it));
    }

    if (rank <= 0 || rank > static_cast<int32>(ULDUAR_FL_ARENA_CORNERS.size()))
        return -1;

    return static_cast<int8>(rank - 1);
}

int8 FlameLeviathanHullCornerPost(PlayerbotAI* botAI, Player* bot, Unit* hull)
{
    Group* group = bot ? bot->GetGroup() : nullptr;
    if (!group || !hull)
        return -1;

    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* member = gref->GetSource();
        if (member && member->GetVehicleBase() == hull && FlameLeviathanIsDriver(member))
            return FlameLeviathanCornerPost(botAI, member);
    }

    return -1;
}

bool FlameLeviathanIsVentReserve(Player* bot)
{
    Unit* hull = bot && FlameLeviathanIsDriver(bot) ? bot->GetVehicleBase() : nullptr;
    ObjectGuid const reserve = bot ? FlameLeviathanStateFor(bot).ventReserve : ObjectGuid::Empty;
    return hull && reserve && hull->GetGUID() == reserve;
}

Position FlameLeviathanArenaCentre()
{
    float centreX = 0.0f;
    float centreY = 0.0f;
    for (Position const& each : ULDUAR_FL_ARENA_CORNERS)
    {
        centreX += each.GetPositionX();
        centreY += each.GetPositionY();
    }

    float const count = static_cast<float>(ULDUAR_FL_ARENA_CORNERS.size());
    return Position(centreX / count, centreY / count, ULDUAR_FL_ARENA_CORNERS.front().GetPositionZ());
}

Position FlameLeviathanCornerPostPoint(uint8 index)
{
    Position const& corner = ULDUAR_FL_ARENA_CORNERS[index % ULDUAR_FL_ARENA_CORNERS.size()];
    Position const centre = FlameLeviathanArenaCentre();

    // Toward the middle, so the engine ends up between the spawn point and the raid and Ram's
    // knockback drives what it catches back into the corner instead of out at the fleet.
    float dx = centre.GetPositionX() - corner.GetPositionX();
    float dy = centre.GetPositionY() - corner.GetPositionY();
    float const len = std::sqrt(dx * dx + dy * dy);
    if (len > 0.0f)
    {
        dx /= len;
        dy /= len;
    }

    return Position(corner.GetPositionX() + dx * ULDUAR_FL_CORNER_STANDOFF,
                    corner.GetPositionY() + dy * ULDUAR_FL_CORNER_STANDOFF, corner.GetPositionZ());
}

float FlameLeviathanDemolisherStandDist(Unit* boss)
{
    return std::min(ULDUAR_FL_DEMOLISHER_BAND,
                    ULDUAR_FL_HURL_BOULDER_MAX_RANGE - boss->GetCombatReach() - 2.0f * ULDUAR_FL_ARRIVE_TOLERANCE);
}

namespace
{
uint32 FlameLeviathanHazardTower(uint32 entry)
{
    switch (entry)
    {
        case NPC_FL_THORIM_HAMMER_TARGET:
            return FL_TOWER_STORM;
        case NPC_FL_MIMIRONS_INFERNO_TARGET:
        case NPC_FL_MIMIRONS_INFERNO:
            return FL_TOWER_FLAMES;
        case NPC_FL_HODIRS_FURY_TARGET:
            return FL_TOWER_FROST;
        default:
            return 0;
    }
}

bool IsFlameLeviathanTowerHazard(uint32 entry, uint32 towerMask)
{
    // Both Mimiron entries: the target is the head of the trail, and NPC_FL_MIMIRONS_INFERNO is each
    // 9 yd patch it drops behind itself. The patches outlive the head passing by a full 30s.
    return ((towerMask & FL_TOWER_STORM) && entry == NPC_FL_THORIM_HAMMER_TARGET) ||
           ((towerMask & FL_TOWER_FLAMES) &&
            (entry == NPC_FL_MIMIRONS_INFERNO_TARGET || entry == NPC_FL_MIMIRONS_INFERNO)) ||
           ((towerMask & FL_TOWER_FROST) && entry == NPC_FL_HODIRS_FURY_TARGET);
}

// The hazard markers are non-selectable trigger creatures, so they never appear in the attack-target
// lists ("possible targets"). The raw nearby-npc list has every non-player unit regardless of flags.
template <typename Visit>
void ForEachTowerHazard(PlayerbotAI* botAI, uint32 towerMask, Visit visit)
{
    Player* bot = botAI->GetBot();
    for (auto const& guid : botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get())
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || !IsFlameLeviathanTowerHazard(unit->GetEntry(), towerMask))
            continue;

        // Still chasing, so it cannot strike yet, and dodging it only drags it along.
        if (unit->GetEntry() == NPC_FL_HODIRS_FURY_TARGET && !FlameLeviathanFuryArmed(bot, unit))
            continue;

        visit(unit);
    }
}
}  // namespace

float FlameLeviathanHazardReach(Unit* hazard, Unit* vehicle)
{
    switch (hazard->GetEntry())
    {
        case NPC_FL_MIMIRONS_INFERNO:
        case NPC_FL_MIMIRONS_INFERNO_TARGET:
            return ULDUAR_FL_INFERNO_RADIUS + DEFAULT_WORLD_OBJECT_SIZE + (vehicle ? vehicle->GetObjectSize() : 0.0f);
        case NPC_FL_HODIRS_FURY_TARGET:
            return ULDUAR_FL_FURY_RADIUS;
        default:
            return ULDUAR_FL_HAMMER_RADIUS;
    }
}

bool FlameLeviathanFuryArmed(Player* bot, Unit* reticle)
{
    return bot && reticle && FlameLeviathanStateFor(bot).furyArmed.count(reticle->GetGUID());
}

Unit* GetFlameLeviathanNearestTowerHazard(PlayerbotAI* botAI, Unit* from, uint32 towerMask, float margin,
                                          uint32 strictMask)
{
    if (!botAI || !from)
        return nullptr;

    Unit* nearest = nullptr;
    float best = 0.0f;

    ForEachTowerHazard(botAI, towerMask,
                       [&](Unit* hazard)
                       {
                           float const gap = from->GetExactDist2d(hazard) - FlameLeviathanHazardReach(hazard, from);
                           float const pad =
                               (strictMask & FlameLeviathanHazardTower(hazard->GetEntry())) ? 0.0f : margin;
                           if (gap < pad && (!nearest || gap < best))
                           {
                               best = gap;
                               nearest = hazard;
                           }
                       });

    return nearest;
}

void GetFlameLeviathanTowerHazards(PlayerbotAI* botAI, Unit* from, uint32 towerMask, float radius,
                                   std::vector<Unit*>& out)
{
    out.clear();
    if (!botAI || !from)
        return;

    ForEachTowerHazard(botAI, towerMask,
                       [&](Unit* hazard)
                       {
                           if (from->GetExactDist2d(hazard) <= radius)
                               out.push_back(hazard);
                       });
}

bool FlameLeviathanInArena(Position const& pos, float margin)
{
    float minX = ULDUAR_FL_ARENA_CORNERS[0].GetPositionX();
    float maxX = minX;
    float minY = ULDUAR_FL_ARENA_CORNERS[0].GetPositionY();
    float maxY = minY;

    for (Position const& corner : ULDUAR_FL_ARENA_CORNERS)
    {
        minX = std::min(minX, corner.GetPositionX());
        maxX = std::max(maxX, corner.GetPositionX());
        minY = std::min(minY, corner.GetPositionY());
        maxY = std::max(maxY, corner.GetPositionY());
    }

    return pos.GetPositionX() >= minX - margin && pos.GetPositionX() <= maxX + margin &&
           pos.GetPositionY() >= minY - margin && pos.GetPositionY() <= maxY + margin;
}

static Position FlameLeviathanOffsetPoint(Unit* boss, float bearing, float standDist)
{
    float const dist = boss->GetCombatReach() + standDist;
    return Position(boss->GetPositionX() + std::cos(bearing) * dist,
                    boss->GetPositionY() + std::sin(bearing) * dist, boss->GetPositionZ());
}

float FlameLeviathanStationBearingOffset(Player* bot, Unit* vehicleBase, float radius, float* fanHalf)
{
    if (fanHalf)
        *fanHalf = 0.0f;

    Group* group = bot ? bot->GetGroup() : nullptr;
    if (!group || !vehicleBase || radius <= 0.0f)
        return 0.0f;

    uint32 const entry = vehicleBase->GetEntry();
    ObjectGuid const myGuid = vehicleBase->GetGUID();

    // Guid rank among live vehicles of this class, deduped on the hull so a crew of four is one slot. A
    // frozen vehicle keeps its slot, or the whole fan would swing for 60 s.
    std::vector<ObjectGuid> seen;
    int32 index = 0;
    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* member = gref->GetSource();
        if (!member || !member->IsAlive())
            continue;

        Unit* base = FlameLeviathanRiddenVehicle(member);
        if (!base || base->GetEntry() != entry)
            continue;

        ObjectGuid const guid = base->GetGUID();
        if (std::find(seen.begin(), seen.end(), guid) != seen.end())
            continue;

        seen.push_back(guid);
        if (guid < myGuid)
            ++index;
    }

    int32 const count = static_cast<int32>(seen.size());
    if (count < 2)
        return 0.0f;

    // A chord of ULDUAR_FL_STATION_SPACING at this radius, as an angle: what has to hold is a gap on the
    // ground, not an angular sector.
    float spread = 2.0f * std::asin(std::min(1.0f, ULDUAR_FL_STATION_SPACING / (2.0f * radius)));

    // A tight radius wants a wide angle, and five of those would wrap the fan round him into his front.
    // Cap the whole fan instead and accept less spacing when it bites.
    spread = std::min(spread, ULDUAR_FL_STATION_MAX_ARC / static_cast<float>(count - 1));

    if (fanHalf)
        *fanHalf = static_cast<float>(count - 1) * 0.5f * spread;

    return (static_cast<float>(index) - static_cast<float>(count - 1) * 0.5f) * spread;
}

float FlameLeviathanCentreTurn(Unit* boss, float fanHalf)
{
    Position const centre = FlameLeviathanArenaCentre();
    if (!boss || boss->GetExactDist2d(centre) <= ULDUAR_FL_CENTRE_TURN_DEADZONE)
        return 0.0f;

    // NormalizeOrientation lands in [0, 2PI), so fold it into (-PI, PI] for a signed turn.
    float turn = Position::NormalizeOrientation(boss->GetAngle(centre.GetPositionX(), centre.GetPositionY()) -
                                                (boss->GetOrientation() + float(M_PI)));
    if (turn > float(M_PI))
        turn -= 2.0f * float(M_PI);

    float const cap = std::max(0.0f, float(M_PI) / 2.0f - fanHalf);
    return std::clamp(turn, -cap, cap);
}

float FlameLeviathanStationLead(Unit* boss)
{
    if (!boss || !boss->isMoving())
        return 0.0f;

    return std::min(boss->GetSpeed(MOVE_RUN) * ULDUAR_FL_STATION_LEAD_S, ULDUAR_FL_STATION_LEAD_MAX);
}

Position FlameLeviathanRearPoint(Unit* boss, float standDist, float bearingOffset, float lead)
{
    Position point = FlameLeviathanOffsetPoint(boss, boss->GetOrientation() + M_PI + bearingOffset, standDist);
    if (lead <= 0.0f)
        return point;

    // Along his heading, so leading a point behind him moves it closer. The safe way to be wrong: a
    // demolisher loses its barrel past 70 yd and nothing until Hurl Boulder's 10 yd floor.
    point.Relocate(point.GetPositionX() + std::cos(boss->GetOrientation()) * lead,
                   point.GetPositionY() + std::sin(boss->GetOrientation()) * lead, point.GetPositionZ());
    return point;
}

Position FlameLeviathanLeadPoint(Unit* boss, float standDist)
{
    return FlameLeviathanOffsetPoint(boss, boss->GetOrientation(), standDist);
}

float FlameLeviathanTarLeadDistance(Unit* boss, Unit* pursued, float size)
{
    if (!boss)
        return 0.0f;

    // Nobody being chased means no blast to stay out of, so take the whole lead.
    if (!pursued)
        return ULDUAR_FL_TAR_LEAD_DIST;

    // He drives at the Pursued vehicle, so the lead runs toward the Ram sphere's centre: stop short of it.
    // His reach comes off because the offset point adds it back on.
    float const room = boss->GetExactDist2d(pursued) - boss->GetCombatReach() -
                       ULDUAR_FL_BATTERING_RAM_RADIUS - size;

    return room <= 0.0f ? 0.0f : std::min(ULDUAR_FL_TAR_LEAD_DIST, room);
}

std::vector<Position> const& FlameLeviathanKiteRing()
{
    static std::vector<Position> const ring = []
    {
        std::vector<Position> nodes;
        size_t const count = ULDUAR_FL_ARENA_CORNERS.size();

        float centreX = 0.0f;
        float centreY = 0.0f;
        for (Position const& corner : ULDUAR_FL_ARENA_CORNERS)
        {
            centreX += corner.GetPositionX();
            centreY += corner.GetPositionY();
        }
        centreX /= static_cast<float>(count);
        centreY /= static_cast<float>(count);

        // Pull each corner off the wall first, so the chamfer is cut from a point the vehicles can
        // actually path to rather than from the wall itself.
        std::vector<Position> inset;
        inset.reserve(count);
        for (Position const& corner : ULDUAR_FL_ARENA_CORNERS)
        {
            float dx = centreX - corner.GetPositionX();
            float dy = centreY - corner.GetPositionY();
            float const len = std::sqrt(dx * dx + dy * dy);
            if (len > 0.0f)
            {
                dx /= len;
                dy /= len;
            }
            inset.emplace_back(corner.GetPositionX() + dx * ULDUAR_FL_KITE_WALL_INSET,
                               corner.GetPositionY() + dy * ULDUAR_FL_KITE_WALL_INSET,
                               corner.GetPositionZ());
        }

        auto towards = [](Position const& from, Position const& to)
        {
            float dx = to.GetPositionX() - from.GetPositionX();
            float dy = to.GetPositionY() - from.GetPositionY();
            float const len = std::sqrt(dx * dx + dy * dy);
            if (len > 0.0f)
            {
                dx /= len;
                dy /= len;
            }
            return Position(from.GetPositionX() + dx * ULDUAR_FL_KITE_CORNER_CHAMFER,
                            from.GetPositionY() + dy * ULDUAR_FL_KITE_CORNER_CHAMFER,
                            from.GetPositionZ());
        };

        // Two nodes per corner, one on each adjoining edge, so a kiting vehicle rounds the turn
        // instead of driving into the corner while the boss cuts the diagonal.
        for (size_t i = 0; i < count; ++i)
        {
            Position const& prev = inset[(i + count - 1) % count];
            Position const& next = inset[(i + 1) % count];
            nodes.push_back(towards(inset[i], prev));
            nodes.push_back(towards(inset[i], next));
        }

        return nodes;
    }();

    return ring;
}
