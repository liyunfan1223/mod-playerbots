/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include <algorithm>
#include <cmath>
#include <iterator>

#include "BattlegroundSA.h"
#include "Creature.h"
#include "GameObject.h"
#include "Log.h"
#include "MotionMaster.h"
#include "MoveSpline.h"
#include "PathGenerator.h"
#include "Playerbots.h"
#include "PositionValue.h"
#include "SotaActions.h"
#include "Spell.h"
#include "SpellMgr.h"
#include "Timer.h"
#include "Vehicle.h"
#include "WorldPacket.h"
#include "WorldSession.h"

namespace
{
constexpr uint32 SPELL_SOTA_HURL_BOULDER = 52338;
constexpr uint32 SPELL_SOTA_RAM = 60206;
constexpr float SOTA_FIRING_DISTANCE = 25.0f;
constexpr float SOTA_ARRIVAL_DISTANCE = 2.5f;
constexpr uint32 SOTA_PATH_REPORT_INTERVAL = 15000;
// Approach the gate ramps around steep mesh corners when the direct path is rejected.
Position const BlueSiegeCheckpoint(1460.0f, -220.0f, 31.078f);
Position const RedSiegeCheckpoint(1275.0f, -215.0f, 35.200f);
Position const YellowSiegeCheckpoint(1177.0f, 55.0f, 63.485f);

Position const* GetSiegeCheckpoint(uint32 gate)
{
    switch (gate)
    {
        case BG_SA_BLUE_GATE:
            return &BlueSiegeCheckpoint;
        case BG_SA_RED_GATE:
            return &RedSiegeCheckpoint;
        case BG_SA_YELLOW_GATE:
            return &YellowSiegeCheckpoint;
        default:
            return nullptr;
    }
}

struct SotaPathFailure
{
    char const* reason = "movement state";
    PathType pathType = PATHFIND_BLANK;
    std::size_t points = 0;
    std::size_t segment = 0;
    G3D::Vector3 from = G3D::Vector3::zero();
    G3D::Vector3 to = G3D::Vector3::zero();
    float rise = 0.0f;
    float horizontal = 0.0f;
};

bool IsActiveSotaAttacker(PlayerbotAI* botAI, SotaTactics const& tactics)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot)
        return false;
    BattlegroundSA* bg = GetSotaBattleground(bot);
    if (!tactics.enabled || !tactics.active || !tactics.attacker || !bot->IsAlive() || !bot->IsInWorld() ||
        bot->IsBeingTeleported() || !bg || bg->GetStatus() != STATUS_IN_PROGRESS)
        return false;

    BG_SA_Status const status = bg->GetSotaStatus();
    return bg->GetAttackerTeam() == bot->GetBgTeamId() && (status == BG_SA_ROUND_ONE || status == BG_SA_ROUND_TWO);
}

Creature* GetSotaDriver(PlayerbotAI* botAI, SotaTactics const& tactics)
{
    if (!IsActiveSotaAttacker(botAI, tactics) || !tactics.pilot || tactics.landingRequired)
        return nullptr;

    Player* bot = botAI->GetBot();
    Creature* vehicle = bot->GetVehicleCreatureBase();
    if (!vehicle || vehicle->GetEntry() != NPC_DEMOLISHER_SA || !vehicle->IsAlive() || !bot->IsInMap(vehicle) ||
        !vehicle->InSamePhase(bot) || !vehicle->IsFriendlyTo(bot) || !botAI->IsInVehicle(true))
        return nullptr;

    return vehicle;
}

GameObject* GetSotaGate(PlayerbotAI* botAI, SotaTactics const& tactics)
{
    if (tactics.objective != SotaObjective::Gate || tactics.gate > BG_SA_ANCIENT_GATE)
        return nullptr;

    BattlegroundSA* bg = GetSotaBattleground(botAI->GetBot());
    if (!bg || !bg->FindBgMap() || tactics.gate >= bg->BgObjects.size() || bg->BgObjects[tactics.gate].IsEmpty() ||
        bg->GetGateState(tactics.gate) == BG_SA_GATE_DESTROYED)
        return nullptr;

    // The core unlocks each inner tier only after a gate in the previous tier falls.
    if ((tactics.gate == BG_SA_RED_GATE || tactics.gate == BG_SA_PURPLE_GATE) &&
        bg->GetGateState(BG_SA_GREEN_GATE) != BG_SA_GATE_DESTROYED &&
        bg->GetGateState(BG_SA_BLUE_GATE) != BG_SA_GATE_DESTROYED)
        return nullptr;
    if (tactics.gate == BG_SA_YELLOW_GATE && bg->GetGateState(BG_SA_RED_GATE) != BG_SA_GATE_DESTROYED &&
        bg->GetGateState(BG_SA_PURPLE_GATE) != BG_SA_GATE_DESTROYED)
        return nullptr;
    if (tactics.gate == BG_SA_ANCIENT_GATE && bg->GetGateState(BG_SA_YELLOW_GATE) != BG_SA_GATE_DESTROYED)
        return nullptr;

    GameObject* gate = bg->GetBGObject(tactics.gate);
    if (!gate || gate->GetGUID() != tactics.objectiveGuid || !gate->IsInWorld() || !gate->isSpawned() ||
        !botAI->GetBot()->IsInMap(gate) || !gate->InSamePhase(botAI->GetBot()) || !gate->IsDestructibleBuilding() ||
        !gate->GetGOValue()->Building.Health)
        return nullptr;

    return gate;
}

bool CheckSotaGroundPoints(Movement::PointsArray const& points, SotaPathFailure& failure)
{
    failure.reason = "route length";
    failure.points = points.size();
    if (points.size() < 2 || points.size() > MAX_POINT_PATH_LENGTH)
        return false;

    for (std::size_t index = 0; index < points.size(); ++index)
    {
        G3D::Vector3 const& point = points[index];
        failure.reason = "invalid point";
        failure.segment = index;
        if (!std::isfinite(point.x) || !std::isfinite(point.y) || !std::isfinite(point.z))
            return false;
        if (!index)
            continue;
        G3D::Vector3 const& previous = points[index - 1];
        float const dx = point.x - previous.x;
        float const dy = point.y - previous.y;
        float const rise = std::fabs(point.z - previous.z);
        float const horizontal = std::sqrt(dx * dx + dy * dy);
        failure.from = previous;
        failure.to = point;
        failure.rise = rise;
        failure.horizontal = horizontal;
        failure.reason = "steep segment";
        if ((horizontal < 0.2f && rise > 2.75f) || (rise > 4.0f && rise > horizontal * 0.9f))
            return false;
    }
    return true;
}

bool BuildSotaGroundPath(Unit* mover, Position const& start, Position const& destination, Movement::PointsArray& points,
                         SotaPathFailure& failure)
{
    failure = {"invalid destination"};
    if (!std::isfinite(destination.GetPositionX()) || !std::isfinite(destination.GetPositionY()) ||
        !std::isfinite(destination.GetPositionZ()) || mover->IsFlying() || mover->isSwimming() || mover->GetTransport())
        return false;

    PathGenerator path(mover);
    path.SetExcludeFlags(NAV_WATER | NAV_MAGMA | NAV_SLIME | NAV_GROUND_STEEP);
    // The optional climb check rejects valid ground corridors before height normalization.
    bool const calculated =
        path.CalculatePath(start.GetPositionX(), start.GetPositionY(), start.GetPositionZ(), destination.GetPositionX(),
                           destination.GetPositionY(), destination.GetPositionZ(), false);
    failure = {"navigation", path.GetPathType(), path.GetPath().size()};
    if (!calculated || path.GetPathType() != PATHFIND_NORMAL)
        return false;

    points = path.GetPath();
    failure.reason = "endpoint";
    if (points.size() < 2 || destination.GetExactDist(points.back().x, points.back().y, points.back().z) > 3.0f)
        return false;
    if (!CheckSotaGroundPoints(points, failure))
        return false;

    // Two-point escort paths regenerate navigation instead of retaining the checked segment.
    if (points.size() == 2)
        points.insert(points.begin() + 1, (points.front() + points.back()) * 0.5f);
    return CheckSotaGroundPoints(points, failure);
}

bool StartSotaGroundMovement(PlayerbotAI* botAI, Unit* mover, Position const& destination, uint32& lastReport,
                             Position const* routeCheckpoint = nullptr)
{
    if (!mover)
        return false;
    SotaPathFailure failure;
    char const* pathLeg = "direct";
    auto reportFailure = [&]()
    {
        uint32 const now = getMSTime();
        if (!lastReport || getMSTimeDiff(lastReport, now) >= SOTA_PATH_REPORT_INTERVAL)
        {
            lastReport = now;
            LOG_DEBUG("playerbots",
                      "SotA movement blocked: bot={} mover={} reason={} leg={} path={} points={} segment={} "
                      "from=({},{},{}) destination=({},{},{}) edge=({},{},{})->({},{},{}) rise={} horizontal={}",
                      botAI->GetBot()->GetGUID().ToString(), mover->GetGUID().ToString(), failure.reason, pathLeg,
                      uint32(failure.pathType), failure.points, failure.segment, mover->GetPositionX(),
                      mover->GetPositionY(), mover->GetPositionZ(), destination.GetPositionX(),
                      destination.GetPositionY(), destination.GetPositionZ(), failure.from.x, failure.from.y,
                      failure.from.z, failure.to.x, failure.to.y, failure.to.z, failure.rise, failure.horizontal);
        }
        return false;
    };
    if (mover->HasUnitState(UNIT_STATE_NOT_MOVE) || mover->HasUnitFlag(UNIT_FLAG_DISABLE_MOVE) ||
        mover->IsNonMeleeSpellCast(false, false, true) ||
        mover->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_CONTROLLED) != NULL_MOTION_TYPE)
        return reportFailure();

    Movement::PointsArray points;
    if (!BuildSotaGroundPath(mover, mover->GetPosition(), destination, points, failure))
    {
        if (!routeCheckpoint || mover->GetExactDist2d(*routeCheckpoint) <= SOTA_ARRIVAL_DISTANCE)
            return reportFailure();
        pathLeg = "checkpoint";
        if (!BuildSotaGroundPath(mover, mover->GetPosition(), *routeCheckpoint, points, failure))
            return reportFailure();
        G3D::Vector3 const& checkpoint = points.back();
        Position const start(checkpoint.x, checkpoint.y, checkpoint.z);
        Movement::PointsArray remaining;
        pathLeg = "goal";
        if (!BuildSotaGroundPath(mover, start, destination, remaining, failure))
            return reportFailure();
        failure.reason = "checkpoint floor";
        if ((points.back() - remaining.front()).length() > 0.5f)
            return reportFailure();
        points.insert(points.end(), remaining.begin() + 1, remaining.end());
        pathLeg = "combined";
        if (!CheckSotaGroundPoints(points, failure))
            return reportFailure();
    }
    failure.reason = "closed gate";
    if (!IsSotaGroundPathClear(botAI, mover, points))
        return reportFailure();

    float distance = 0.0f;
    for (std::size_t index = 1; index < points.size(); ++index)
        distance += (points[index] - points[index - 1]).length();

    mover->GetMotionMaster()->Clear();
    uint32 const previousSpline = mover->movespline->GetId();
    mover->GetMotionMaster()->MoveSplinePath(&points);
    failure.reason = "spline launch";
    if (mover->movespline->GetId() == previousSpline || mover->movespline->Finalized())
        return reportFailure();
    float const delay = std::min(float(sPlayerbotAIConfig.MaxWaitForMove),
                                 IN_MILLISECONDS * distance / std::max(0.1f, mover->GetSpeed(MOVE_RUN)));
    botAI->GetAiObjectContext()
        ->GetValue<LastMovement&>("last movement")
        ->Get()
        .Set(mover->GetMapId(), destination.GetPositionX(), destination.GetPositionY(), destination.GetPositionZ(),
             mover->GetOrientation(), delay, MovementPriority::MOVEMENT_NORMAL);
    return true;
}

Creature* GetSotaBoardingVehicle(PlayerbotAI* botAI, SotaTactics const& tactics)
{
    if (!tactics.pilot || !tactics.attacker || !tactics.boardingVehicle)
        return nullptr;

    Unit* unit = botAI->GetUnit(tactics.boardingVehicle);
    Creature* vehicle = unit ? unit->ToCreature() : nullptr;
    Player* bot = botAI->GetBot();
    if (!vehicle || vehicle->GetEntry() != NPC_DEMOLISHER_SA || !vehicle->IsAlive() || !bot->IsInMap(vehicle) ||
        !vehicle->InSamePhase(bot) || !vehicle->IsFriendlyTo(bot) ||
        vehicle->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE) ||
        !vehicle->HasNpcFlag(UNIT_NPC_FLAG_SPELLCLICK))
        return nullptr;

    Vehicle* kit = vehicle->GetVehicleKit();
    return kit && kit->HasEmptySeat(0) ? vehicle : nullptr;
}

bool CanUseSotaWeapon(PlayerbotAI* botAI, Creature* vehicle, GameObject* gate, uint32 spellId)
{
    if (!vehicle || !gate || !botAI->IsInVehicle(false, true) || vehicle->isMoving() ||
        vehicle->HasUnitState(UNIT_STATE_LOST_CONTROL) || vehicle->IsNonMeleeSpellCast(false, false, true) ||
        vehicle->HasSpellCooldown(spellId) ||
        std::find(std::begin(vehicle->m_spells), std::end(vehicle->m_spells), spellId) == std::end(vehicle->m_spells))
        return false;

    SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
    if (!info || info->IsPassive())
        return false;

    bool inRange = false;
    if (spellId == SPELL_SOTA_HURL_BOULDER)
        inRange = vehicle->GetExactDist2d(gate) > 0.1f && vehicle->GetExactDist2d(gate) <= SOTA_FIRING_DISTANCE;
    else
        for (SpellEffectInfo const& effect : info->GetEffects())
            if (effect.Effect == SPELL_EFFECT_GAMEOBJECT_DAMAGE &&
                gate->IsInRange3d(vehicle->GetPositionX(), vehicle->GetPositionY(), vehicle->GetPositionZ(),
                                  effect.CalcRadius(vehicle)))
                inRange = true;

    // These artillery spells ignore linear LOS in DBC; their normal trajectory still collides with obstacles.
    return inRange && (info->HasAttribute(SPELL_ATTR2_IGNORE_LINE_OF_SIGHT) || vehicle->IsWithinLOSInMap(gate));
}
}  // namespace

bool SotaEnterVehicleAction::isUseful()
{
    SotaTactics const tactics = GetSotaTactics(botAI);
    return IsSotaInfantryReady(botAI, tactics) && IsActiveSotaAttacker(botAI, tactics) &&
           tactics.objective == SotaObjective::Gate && tactics.pilot && tactics.boardingVehicle;
}

bool SotaEnterVehicleAction::Execute(Event /*event*/)
{
    if (!isUseful())
        return false;

    SotaTactics const tactics = GetSotaTactics(botAI);
    Creature* vehicle = GetSotaBoardingVehicle(botAI, tactics);
    if (!vehicle || !IsMovingAllowed() || !bot->GetSession())
        return false;

    if (bot->GetExactDist(vehicle) > INTERACTION_DISTANCE)
    {
        Position const destination = vehicle->GetPosition();
        if (IsDuplicateMove(destination.GetPositionX(), destination.GetPositionY(), destination.GetPositionZ()) ||
            IsWaitingForLastMove(MovementPriority::MOVEMENT_NORMAL))
            return false;
        return StartSotaGroundMovement(botAI, bot, destination, _lastPathReport);
    }
    if (!bot->IsWithinLOSInMap(vehicle))
        return false;

    if (bot->isMoving())
        bot->StopMoving();
    if (bot->IsMounted())
    {
        WorldPacket packet;
        bot->GetSession()->HandleCancelMountAuraOpcode(packet);
        return true;
    }

    vehicle->HandleSpellClick(bot, 0);
    if (!bot->IsOnVehicle(vehicle))
        return false;

    AI_VALUE(LastMovement&, "last movement").clear();
    AI_VALUE(PositionMap&, "position")["bg siege"].Reset();
    return true;
}

bool SotaDriveVehicleAction::isUseful()
{
    SotaTactics const tactics = GetSotaTactics(botAI);
    Creature* vehicle = GetSotaDriver(botAI, tactics);
    return vehicle && tactics.objective == SotaObjective::Gate && !vehicle->IsNonMeleeSpellCast(false, false, true) &&
           vehicle->GetExactDist(tactics.destination) > SOTA_ARRIVAL_DISTANCE;
}

bool SotaDriveVehicleAction::Execute(Event /*event*/)
{
    SotaTactics const tactics = GetSotaTactics(botAI);
    Creature* vehicle = GetSotaDriver(botAI, tactics);
    GameObject* gate = GetSotaGate(botAI, tactics);
    if (!vehicle || !gate || !IsMovingAllowed() ||
        vehicle->GetExactDist(tactics.destination) <= SOTA_ARRIVAL_DISTANCE ||
        IsDuplicateMove(tactics.destination.GetPositionX(), tactics.destination.GetPositionY(),
                        tactics.destination.GetPositionZ()) ||
        IsWaitingForLastMove(MovementPriority::MOVEMENT_NORMAL))
        return false;

    Position const* checkpoint = GetSiegeCheckpoint(tactics.transiting ? tactics.transitGate : tactics.gate);
    return StartSotaGroundMovement(botAI, vehicle, tactics.destination, _lastPathReport, checkpoint);
}

bool SotaLeaveVehicleAction::isUseful()
{
    SotaTactics const tactics = GetSotaTactics(botAI);
    Creature* vehicle = bot->GetVehicleCreatureBase();
    return tactics.enabled && bot->IsAlive() && GetSotaBattleground(bot) && vehicle &&
           vehicle->GetEntry() == NPC_DEMOLISHER_SA &&
           (!vehicle->IsAlive() || !botAI->IsInVehicle(true) || !IsActiveSotaAttacker(botAI, tactics) ||
            !tactics.pilot || tactics.landingRequired || tactics.objective != SotaObjective::Gate);
}

bool SotaLeaveVehicleAction::Execute(Event /*event*/)
{
    if (!isUseful() || !bot->GetSession())
        return false;

    Vehicle* vehicle = bot->GetVehicle();
    VehicleSeatEntry const* seat = vehicle ? vehicle->GetSeatForPassenger(bot) : nullptr;
    if (!seat || !seat->CanEnterOrExit())
        return false;

    if (seat->CanControl())
    {
        Unit* base = vehicle->GetBase();
        if (base)
        {
            base->GetMotionMaster()->Clear();
            base->StopMoving();
        }
    }

    WorldPacket packet;
    bot->GetSession()->HandleRequestVehicleExit(packet);
    if (bot->GetVehicle())
        return false;

    AI_VALUE(LastMovement&, "last movement").clear();
    AI_VALUE(PositionMap&, "position")["bg siege"].Reset();
    return true;
}

bool SotaFireVehicleAction::isUseful()
{
    SotaTactics const tactics = GetSotaTactics(botAI);
    Creature* vehicle = GetSotaDriver(botAI, tactics);
    return vehicle && tactics.objective == SotaObjective::Gate && !vehicle->isMoving() &&
           !vehicle->IsNonMeleeSpellCast(false, false, true) && botAI->IsInVehicle(false, true);
}

bool SotaFireVehicleAction::isPossible()
{
    if (!isUseful())
        return false;

    SotaTactics const tactics = GetSotaTactics(botAI);
    Creature* vehicle = GetSotaDriver(botAI, tactics);
    GameObject* gate = GetSotaGate(botAI, tactics);
    return CanUseSotaWeapon(botAI, vehicle, gate, SPELL_SOTA_HURL_BOULDER) ||
           CanUseSotaWeapon(botAI, vehicle, gate, SPELL_SOTA_RAM);
}

bool SotaFireVehicleAction::Execute(Event /*event*/)
{
    SotaTactics const tactics = GetSotaTactics(botAI);
    Creature* vehicle = GetSotaDriver(botAI, tactics);
    GameObject* gate = GetSotaGate(botAI, tactics);
    for (uint32 spellId : {SPELL_SOTA_HURL_BOULDER, SPELL_SOTA_RAM})
    {
        if (!CanUseSotaWeapon(botAI, vehicle, gate, spellId))
            continue;

        SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
        SpellCastTargets targets;
        targets.SetGOTarget(gate);
        targets.SetSrc(*vehicle);
        if (spellId == SPELL_SOTA_HURL_BOULDER)
        {
            targets.SetDst(*gate);
            targets.SetSpeed(30.0f);
            float const sightline =
                std::atan2(gate->GetPositionZ() - vehicle->GetPositionZ(), vehicle->GetExactDist2d(gate));
            targets.SetElevation(sightline + (float(M_PI / 2.0) - sightline) * 0.25f);
        }

        vehicle->SetFacingTo(vehicle->GetAngle(gate));
        if (vehicle->CastSpell(targets, info, nullptr, TRIGGERED_NONE) == SPELL_CAST_OK)
            return true;
    }
    return false;
}
