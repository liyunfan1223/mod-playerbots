/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SotaActions.h"

#include <algorithm>
#include <cmath>

#include "BattlegroundSA.h"
#include "DBCStores.h"
#include "GameObject.h"
#include "IVMapMgr.h"
#include "Map.h"
#include "Log.h"
#include "MotionMaster.h"
#include "MoveSpline.h"
#include "MoveSplineInit.h"
#include "PathGenerator.h"
#include "Playerbots.h"
#include "Random.h"
#include "Timer.h"
#include "WorldPacket.h"
#include "WorldSession.h"

namespace
{
constexpr float objectiveTolerance = 4.0f;
constexpr float defenseReleaseDistance = 80.0f;
constexpr uint32 movementRecheckMs = 1000;
constexpr float pathEndpointTolerance = 3.0f;
constexpr uint32 SPELL_SOTA_CAPTURE_BANNER = 21651;
constexpr uint32 deploymentReportIntervalMs = 15000;
Position const GreenDefenseCheckpoint(1245.0f, 70.0f, 44.888f);

bool CheckInfantryPath(Movement::PointsArray const& points)
{
    if (points.size() < 2 || points.size() > MAX_POINT_PATH_LENGTH)
        return false;
    for (std::size_t index = 0; index < points.size(); ++index)
    {
        G3D::Vector3 const& point = points[index];
        if (!std::isfinite(point.x) || !std::isfinite(point.y) || !std::isfinite(point.z))
            return false;
        if (!index)
            continue;
        float const rise = std::fabs(point.z - points[index - 1].z);
        float const distance = std::hypot(point.x - points[index - 1].x, point.y - points[index - 1].y);
        if (rise > 3.0f && rise > distance * 1.2f)
            return false;
    }
    return true;
}

bool BuildInfantryPath(Player* bot, Position const& start, Position const& destination, Movement::PointsArray& points)
{
    PathGenerator path(bot);
    if (!path.CalculatePath(start.GetPositionX(), start.GetPositionY(), start.GetPositionZ(),
                            destination.GetPositionX(), destination.GetPositionY(), destination.GetPositionZ(),
                            false) ||
        path.GetPathType() != PATHFIND_NORMAL || path.GetPath().size() < 2)
        return false;
    points = path.GetPath();
    G3D::Vector3 const& end = points.back();
    return destination.GetExactDist(end.x, end.y, end.z) <= pathEndpointTolerance && CheckInfantryPath(points);
}

bool NeedsFallBack(Player* bot, SotaTactics const& tactics)
{
    return tactics.active && !tactics.attacker && tactics.stage > 0 &&
           bot->GetExactDist2d(tactics.destination) > defenseReleaseDistance;
}

bool AtDestination(Player* bot, SotaTactics const& tactics)
{
    return bot->GetExactDist2d(tactics.destination) <= objectiveTolerance &&
           std::fabs(bot->GetPositionZ() - tactics.destination.GetPositionZ()) <= objectiveTolerance;
}

bool ValidInfantryObjective(PlayerbotAI* botAI, SotaTactics const& tactics)
{
    return IsSotaInfantryReady(botAI, tactics) && tactics.objective != SotaObjective::None;
}

GameObject* GetInteractionTarget(PlayerbotAI* botAI, SotaTactics const& tactics)
{
    Player* bot = botAI->GetBot();
    if (!ValidInfantryObjective(botAI, tactics) || !tactics.attacker ||
        (tactics.objective != SotaObjective::Graveyard && tactics.objective != SotaObjective::Relic))
        return nullptr;

    GameObject* object = bot->GetMap()->GetGameObject(tactics.objectiveGuid);
    if (!object || !object->isSpawned() || object->HasGameObjectFlag(GO_FLAG_NOT_SELECTABLE) ||
        !object->IsAtInteractDistance(bot) || !bot->CanUseBattlegroundObject(object))
        return nullptr;

    return object;
}
}  // namespace

bool IsSotaGroundPathClear(PlayerbotAI* botAI, Unit* mover, Movement::PointsArray const& points)
{
    BattlegroundSA* bg = GetSotaBattleground(botAI ? botAI->GetBot() : nullptr);
    if (!bg || !mover || points.size() < 2)
        return false;
    for (uint32 index = BG_SA_GREEN_GATE; index <= BG_SA_ANCIENT_GATE; ++index)
    {
        if (bg->GetGateState(index) == BG_SA_GATE_DESTROYED || index >= bg->BgObjects.size() ||
            bg->BgObjects[index].IsEmpty())
            continue;
        GameObject* gate = mover->GetMap()->GetGameObject(bg->BgObjects[index]);
        if (!gate || !gate->isSpawned())
            continue;
        GameObjectDisplayInfoEntry const* display =
            sGameObjectDisplayInfoStore.LookupEntry(gate->GetGOInfo()->displayId);
        if (!display)
            return false;
        float scale = gate->GetObjectScale();
        float radius = mover->GetBoundaryRadius();
        float low[3] = {display->minX * scale - radius, display->minY * scale - radius,
                        display->minZ * scale - mover->GetCollisionHeight()};
        float high[3] = {display->maxX * scale + radius, display->maxY * scale + radius, display->maxZ * scale};
        float cosine = std::cos(gate->GetOrientation());
        float sine = std::sin(gate->GetOrientation());
        auto local = [gate, cosine, sine](G3D::Vector3 const& point)
        {
            float x = point.x - gate->GetPositionX();
            float y = point.y - gate->GetPositionY();
            return G3D::Vector3(cosine * x + sine * y, -sine * x + cosine * y, point.z - gate->GetPositionZ());
        };
        for (std::size_t segment = 1; segment < points.size(); ++segment)
        {
            G3D::Vector3 start = local(points[segment - 1]);
            G3D::Vector3 end = local(points[segment]);
            float enter = 0.0f;
            float leave = 1.0f;
            bool intersects = true;
            for (uint8 axis = 0; axis < 3; ++axis)
            {
                float delta = end[axis] - start[axis];
                if (std::fabs(delta) < 0.0001f)
                {
                    if (start[axis] < low[axis] || start[axis] > high[axis])
                    {
                        intersects = false;
                        break;
                    }
                    continue;
                }
                float nearTime = (low[axis] - start[axis]) / delta;
                float farTime = (high[axis] - start[axis]) / delta;
                if (nearTime > farTime)
                    std::swap(nearTime, farTime);
                enter = std::max(enter, nearTime);
                leave = std::min(leave, farTime);
                if (enter > leave)
                {
                    intersects = false;
                    break;
                }
            }
            if (intersects)
                return false;
        }
    }
    return true;
}

bool SotaPrepareAction::isUseful()
{
    SotaTactics tactics = GetSotaTactics(botAI);
    return tactics.enabled && !tactics.active && bot->IsAlive() && bot->isMoving() && !bot->GetVehicle() &&
           !bot->IsBeingTeleported();
}

bool SotaPrepareAction::Execute(Event /*event*/)
{
    if (!isUseful())
        return false;
    bot->StopMoving();
    bot->GetMotionMaster()->Clear();
    AI_VALUE(LastMovement&, "last movement").clear();
    return true;
}

bool SotaMoveAction::isUseful()
{
    SotaTactics tactics = GetSotaTactics(botAI);
    return ValidInfantryObjective(botAI, tactics) && !tactics.deploying && !bot->IsInCombat() &&
           !AtDestination(bot, tactics);
}

bool SotaFallBackAction::isUseful()
{
    SotaTactics tactics = GetSotaTactics(botAI);
    return ValidInfantryObjective(botAI, tactics) && !tactics.deploying && NeedsFallBack(bot, tactics);
}

bool SotaMoveAction::Execute(Event /*event*/)
{
    if (!isUseful() || !botAI->CanMove() || bot->HasUnitFlag(UNIT_FLAG_DISABLE_MOVE) ||
        bot->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_CONTROLLED) != NULL_MOTION_TYPE)
        return false;

    BattlegroundSA* battleground = GetSotaBattleground(bot);
    SotaTactics tactics = GetSotaTactics(botAI);
    if (!battleground || battleground->GetStatus() != STATUS_IN_PROGRESS ||
        (battleground->GetSotaStatus() != BG_SA_ROUND_ONE && battleground->GetSotaStatus() != BG_SA_ROUND_TWO) ||
        tactics.attacker != (bot->GetTeamId() == battleground->GetAttackerTeam()))
        return false;

    Position const destination = GetDestination(tactics);
    MovementPriority priority =
        NeedsFallBack(bot, tactics) ? MovementPriority::MOVEMENT_COMBAT : MovementPriority::MOVEMENT_NORMAL;
    UpdateMovementState();
    if (!IsMovingAllowed() ||
        IsDuplicateMove(destination.GetPositionX(), destination.GetPositionY(), destination.GetPositionZ()) ||
        IsWaitingForLastMove(priority))
        return false;

    Movement::PointsArray points;
    if (!BuildInfantryPath(bot, bot->GetPosition(), destination, points))
    {
        // Approach the outer green ledge around the steep corner below the purple wall.
        if (!tactics.deploying || tactics.gate != BG_SA_GREEN_GATE ||
            tactics.deploymentStart.GetPositionX() <= GreenDefenseCheckpoint.GetPositionX() ||
            !BuildInfantryPath(bot, bot->GetPosition(), GreenDefenseCheckpoint, points))
            return false;
        G3D::Vector3 const& checkpoint = points.back();
        Position const start(checkpoint.x, checkpoint.y, checkpoint.z);
        Movement::PointsArray remaining;
        if (!BuildInfantryPath(bot, start, destination, remaining) ||
            (points.back() - remaining.front()).length() > 0.5f)
            return false;
        points.insert(points.end(), remaining.begin() + 1, remaining.end());
        if (!CheckInfantryPath(points))
            return false;
    }

    if (!IsSotaGroundPathClear(botAI, bot, points))
        return false;
    // Escort regenerates a two-point path; retain the path that was checked above.
    if (points.size() == 2)
        points.insert(points.begin() + 1, (points.front() + points.back()) * 0.5f);
    if (bot->IsSitState())
        bot->SetStandState(UNIT_STAND_STATE_STAND);
    uint32 const previousSpline = bot->movespline->GetId();
    bot->GetMotionMaster()->MoveSplinePath(&points);
    if (bot->movespline->GetId() == previousSpline || bot->movespline->Finalized())
        return false;
    AI_VALUE(LastMovement&, "last movement")
        .Set(bot->GetMapId(), destination.GetPositionX(), destination.GetPositionY(), destination.GetPositionZ(),
             bot->GetOrientation(), movementRecheckMs, priority);
    return true;
}

bool SotaDeployAction::isUseful()
{
    SotaTactics tactics = GetSotaTactics(botAI);
    return IsSotaInfantryReady(botAI, tactics) && !tactics.attacker && tactics.deploying &&
           bot->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_CONTROLLED) == NULL_MOTION_TYPE;
}

bool SotaDeployAction::Execute(Event event)
{
    if (!isUseful() || !botAI->CanMove())
        return false;
    SotaTactics tactics = GetSotaTactics(botAI);
    BattlegroundSA* bg = GetSotaBattleground(bot);
    if (!bg || bg->GetAttackerTeam() == bot->GetTeamId() ||
        (bg->GetSotaStatus() != BG_SA_ROUND_ONE && bg->GetSotaStatus() != BG_SA_ROUND_TWO))
        return false;

    Position const& start = tactics.deploymentStart;
    Position end = tactics.deploymentEnd;
    if (bot->GetExactDist(start) > pathEndpointTolerance)
        return SotaMoveAction::Execute(event);

    auto approachStart = [&](char const* reason, std::size_t segment = 0)
    {
        uint32 const now = getMSTime();
        if (!_lastDeploymentReport || getMSTimeDiff(_lastDeploymentReport, now) >= deploymentReportIntervalMs)
        {
            _lastDeploymentReport = now;
            LOG_DEBUG("playerbots",
                      "SotA deployment blocked: bot={} reason={} lane={} stage={} segment={} "
                      "from=({},{},{}) start=({},{},{}) landing=({},{},{})",
                      bot->GetGUID().ToString(), reason, tactics.lane, tactics.stage, segment, bot->GetPositionX(),
                      bot->GetPositionY(), bot->GetPositionZ(), start.GetPositionX(), start.GetPositionY(),
                      start.GetPositionZ(), end.GetPositionX(), end.GetPositionY(), end.GetPositionZ());
        }
        return bot->GetExactDist(start) > 0.5f ? SotaMoveAction::Execute(event) : false;
    };

    float const startGround = bot->GetMap()->GetHeight(bot->GetPhaseMask(), bot->GetPositionX(), bot->GetPositionY(),
                                                       bot->GetPositionZ() + 1.0f, true, 3.0f);
    if (!std::isfinite(startGround) || std::fabs(bot->GetPositionZ() - startGround) > 1.0f ||
        bot->GetMap()->IsInWater(bot->GetPhaseMask(), bot->GetPositionX(), bot->GetPositionY(), startGround,
                                 bot->GetCollisionHeight()))
        return approachStart("start ground");

    float ground = bot->GetMap()->GetHeight(bot->GetPhaseMask(), end.GetPositionX(), end.GetPositionY(),
                                            end.GetPositionZ() + 3.0f, true, 6.0f);
    if (!std::isfinite(ground) || std::fabs(ground - end.GetPositionZ()) > 2.0f ||
        bot->GetMap()->IsInWater(bot->GetPhaseMask(), end.GetPositionX(), end.GetPositionY(), ground,
                                 bot->GetCollisionHeight()))
        return approachStart("landing ground");
    end.m_positionZ = ground + 0.1f;
    float drop = bot->GetPositionZ() - end.GetPositionZ();
    if (drop < 1.0f || drop > 15.0f || bot->GetExactDist2d(end) > 15.0f)
        return approachStart("jump reach");

    constexpr float jumpApex = 3.0f;
    constexpr uint8 arcSegments = 16;
    Movement::PointsArray arc;
    for (uint8 index = 0; index <= arcSegments; ++index)
    {
        float progress = float(index) / arcSegments;
        arc.emplace_back(bot->GetPositionX() + (end.GetPositionX() - bot->GetPositionX()) * progress,
                         bot->GetPositionY() + (end.GetPositionY() - bot->GetPositionY()) * progress,
                         bot->GetPositionZ() + (end.GetPositionZ() - bot->GetPositionZ()) * progress +
                             4.0f * jumpApex * progress * (1.0f - progress));
    }
    if (!IsSotaGroundPathClear(botAI, bot, arc))
        return approachStart("closed gate");
    float height = std::max(1.0f, bot->GetCollisionHeight() * 0.5f);
    for (std::size_t index = 1; index < arc.size(); ++index)
    {
        G3D::Vector3 const& from = arc[index - 1];
        G3D::Vector3 const& to = arc[index];
        if (!bot->GetMap()->isInLineOfSight(from.x, from.y, from.z + height, to.x, to.y, to.z + height,
                                            bot->GetPhaseMask(), LINEOFSIGHT_ALL_CHECKS,
                                            VMAP::ModelIgnoreFlags::Nothing))
            return approachStart("jump collision", index);
    }

    bot->StopMoving();
    bot->GetMotionMaster()->Clear();
    float verticalSpeed = std::sqrt(float(2.0 * Movement::gravity) * jumpApex);
    bot->GetMotionMaster()->MoveJump(end.GetPositionX(), end.GetPositionY(), end.GetPositionZ(),
                                     bot->GetSpeed(MOVE_RUN), verticalSpeed);
    AI_VALUE(LastMovement&, "last movement")
        .Set(bot->GetMapId(), end.GetPositionX(), end.GetPositionY(), end.GetPositionZ(), bot->GetOrientation(),
             movementRecheckMs, MovementPriority::MOVEMENT_NORMAL);
    return true;
}

bool SotaInteractAction::isUseful() { return GetInteractionTarget(botAI, GetSotaTactics(botAI)) != nullptr; }

bool SotaInteractAction::Execute(Event /*event*/)
{
    BattlegroundSA* battleground = GetSotaBattleground(bot);
    if (!battleground || battleground->GetStatus() != STATUS_IN_PROGRESS ||
        (battleground->GetSotaStatus() != BG_SA_ROUND_ONE && battleground->GetSotaStatus() != BG_SA_ROUND_TWO) ||
        battleground->GetAttackerTeam() != bot->GetTeamId())
        return false;

    SotaTactics const tactics = GetSotaTactics(botAI);
    GameObject* object = GetInteractionTarget(botAI, tactics);
    if (!object)
    {
        _objectGuid.Clear();
        return false;
    }
    if (object->GetGUID() != _objectGuid)
    {
        _objectGuid = object->GetGUID();
        _startedAt = getMSTime();
        _delay = urand(sPlayerbotAIConfig.PvpFlagInteractionDelayMin, sPlayerbotAIConfig.PvpFlagInteractionDelayMax);
    }
    if (getMSTimeDiff(_startedAt, getMSTime()) < _delay)
        return false;

    bot->StopMoving();
    bot->GetMotionMaster()->Clear();
    AI_VALUE(LastMovement&, "last movement").clear();
    if (bot->IsMounted())
        bot->RemoveAurasByType(SPELL_AURA_MOUNTED);
    if (bot->IsInDisallowedMountForm())
        bot->RemoveAurasByType(SPELL_AURA_MOD_SHAPESHIFT);

    if (tactics.objective == SotaObjective::Graveyard)
    {
        if (bot->CastSpell(object, SPELL_SOTA_CAPTURE_BANNER, false) != SPELL_CAST_OK)
            return false;
        botAI->WaitForSpellCast(bot->GetCurrentSpell(CURRENT_GENERIC_SPELL));
    }
    else
    {
        WorldPacket packet(CMSG_GAMEOBJ_USE);
        packet << object->GetGUID();
        bot->GetSession()->HandleGameObjectUseOpcode(packet);
    }
    GetSotaTacticsValue(botAI)->Reset();
    _objectGuid.Clear();
    return true;
}

bool SotaAttackDemolisherAction::isUseful()
{
    SotaTactics const tactics = GetSotaTactics(botAI);
    Unit* target = IsSotaInfantryReady(botAI, tactics) ? GetSotaDefenseTarget(botAI, tactics) : nullptr;
    return target && (AI_VALUE(Unit*, "current target") != target || bot->GetVictim() != target);
}

bool SotaAttackDemolisherAction::Execute(Event /*event*/)
{
    SotaTactics const tactics = GetSotaTactics(botAI);
    Unit* target = IsSotaInfantryReady(botAI, tactics) ? GetSotaDefenseTarget(botAI, tactics) : nullptr;
    if (!target || (AI_VALUE(Unit*, "current target") == target && bot->GetVictim() == target))
        return false;
    return Attack(target);
}
