/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SotaTacticsValue.h"

#include "BattlegroundSA.h"
#include "Creature.h"
#include "GameObject.h"
#include "Map.h"
#include "PathGenerator.h"
#include "Playerbots.h"
#include "SotaActions.h"
#include "Vehicle.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace
{
constexpr uint32 SOTA_MAP_ID = 607;
constexpr uint32 MAX_PILOTS = 4;
constexpr float VEHICLE_DEFENSE_RANGE = 60.0f;
constexpr float VEHICLE_DEFENSE_PATH_LENGTH = 120.0f;
constexpr float DEFENSE_RELEASE_DISTANCE = 80.0f;
constexpr float DEFENSE_PATH_ENDPOINT_TOLERANCE = 3.0f;
constexpr float BOARDING_SEARCH_RANGE = 300.0f;
// Demolishers have a 3-yard radius; an 8-yard center would overlap an intact gate's 5.85-yard half depth.
constexpr float SIEGE_STAND_OFF = 15.0f;

struct GateRoute
{
    Position approach;
    Position inside;
};

// The origin of a destructible model is not necessarily its ground level.
std::array<GateRoute, 6> const GateRoutes = {{
    {Position(1416.897f, 102.194f, 28.770f), Position(1401.582f, 119.354f, 31.248f)},
    {Position(1063.447f, -107.828f, 81.331f), Position(1040.461f, -108.610f, 85.306f)},
    {Position(1435.840f, -212.822f, 30.725f), Position(1422.906f, -231.841f, 31.179f)},
    {Position(1234.679f, -208.703f, 55.128f), Position(1214.520f, -219.777f, 56.362f)},
    {Position(1221.550f, 77.109f, 52.760f), Position(1201.801f, 88.899f, 54.649f)},
    {Position(886.555f, -108.200f, 110.144f), Position(863.555f, -108.200f, 110.317f)},
}};

std::array<float, 6> const SiegeApproachHeights = {27.070f, 79.229f, 29.811f, 53.081f, 50.565f, 109.339f};

Position SiegeApproach(uint32 gate)
{
    float const orientation = BG_SA_ObjSpawnlocs[gate][3];
    return Position(BG_SA_ObjSpawnlocs[gate][0] + std::cos(orientation) * SIEGE_STAND_OFF,
                    BG_SA_ObjSpawnlocs[gate][1] + std::sin(orientation) * SIEGE_STAND_OFF, SiegeApproachHeights[gate]);
}

struct WallDrop
{
    Position start;
    Position end;
    uint32 gate;
};

std::array<WallDrop, 2> const MiddleWallDrops = {{
    {Position(1247.196331f, -231.491875f, 66.720078f), Position(1253.418082f, -230.356174f, 56.009695f),
     BG_SA_RED_GATE},
    {Position(1209.248234f, 56.502004f, 64.738564f), Position(1214.400076f, 53.426532f, 53.409196f), BG_SA_PURPLE_GATE},
}};

std::array<WallDrop, 2> const OuterWallDrops = {{
    {Position(1454.032050f, -225.192855f, 41.956253f), Position(1461.838322f, -220.826836f, 30.986086f),
     BG_SA_BLUE_GATE},
    {Position(1397.498073f, 84.882517f, 41.183136f), Position(1401.493077f, 80.405918f, 30.779127f), BG_SA_GREEN_GATE},
}};

std::array<Position, 2> const BeachPositions = {Position(1583.200f, -91.229f, 8.731f),
                                                Position(1593.130f, 46.811f, 7.798f)};

Creature* GetCreature(BattlegroundSA* bg, uint32 index)
{
    if (!bg || !bg->FindBgMap() || index >= bg->BgCreatures.size() || bg->BgCreatures[index].IsEmpty())
        return nullptr;
    return bg->GetBgMap()->GetCreature(bg->BgCreatures[index]);
}

GameObject* GetObject(BattlegroundSA* bg, uint32 index)
{
    if (!bg || !bg->FindBgMap() || index >= bg->BgObjects.size() || bg->BgObjects[index].IsEmpty())
        return nullptr;
    return bg->GetBgMap()->GetGameObject(bg->BgObjects[index]);
}

bool Destroyed(BattlegroundSA* bg, uint32 gate) { return bg->GetGateState(gate) == BG_SA_GATE_DESTROYED; }

uint8 GetStage(BattlegroundSA* bg)
{
    if (Destroyed(bg, BG_SA_ANCIENT_GATE))
        return 4;
    if (Destroyed(bg, BG_SA_YELLOW_GATE))
        return 3;
    if (Destroyed(bg, BG_SA_RED_GATE) || Destroyed(bg, BG_SA_PURPLE_GATE))
        return 2;
    if (Destroyed(bg, BG_SA_BLUE_GATE) || Destroyed(bg, BG_SA_GREEN_GATE))
        return 1;
    return 0;
}

uint32 OuterGate(uint8 lane) { return lane ? BG_SA_GREEN_GATE : BG_SA_BLUE_GATE; }

uint32 MiddleGate(uint8 lane) { return lane ? BG_SA_PURPLE_GATE : BG_SA_RED_GATE; }

uint8 OpenLane(BattlegroundSA* bg, uint8 lane, bool middle)
{
    uint32 const preferred = middle ? MiddleGate(lane) : OuterGate(lane);
    return Destroyed(bg, preferred) ? lane : uint8(1 - lane);
}

Position RouteToObjective(Player* bot, BattlegroundSA* bg, uint8 lane, uint8 stage, SotaTactics& tactics)
{
    std::array<uint32, 3> passedGates = {OuterGate(OpenLane(bg, lane, false)), MiddleGate(OpenLane(bg, lane, true)),
                                         BG_SA_YELLOW_GATE};
    uint8 const gateCount = std::min<uint8>(stage, uint8(passedGates.size()));
    for (uint8 index = 0; index < gateCount; ++index)
    {
        uint32 const gate = passedGates[index];
        if (!Destroyed(bg, gate))
            continue;
        float const differenceX = bot->GetPositionX() - BG_SA_ObjSpawnlocs[gate][0];
        float const differenceY = bot->GetPositionY() - BG_SA_ObjSpawnlocs[gate][1];
        float const orientation = BG_SA_ObjSpawnlocs[gate][3];
        float const outside = differenceX * std::cos(orientation) + differenceY * std::sin(orientation);
        float const distanceSquared = differenceX * differenceX + differenceY * differenceY;
        bool const onHigherTerrace =
            differenceX < 20.0f && bot->GetPositionZ() > GateRoutes[gate].inside.GetPositionZ() + 8.0f;
        if (differenceX < -24.0f || onHigherTerrace || (distanceSquared < 70.0f * 70.0f && outside < -6.0f))
            continue;
        GateRoute const& route = GateRoutes[gate];
        tactics.transiting = true;
        tactics.transitGate = gate;
        if (distanceSquared > 35.0f * 35.0f || (outside > 0.0f && bot->GetExactDist2d(route.approach) > 8.0f))
            return route.approach;
        return route.inside;
    }
    return tactics.destination;
}

bool OutsideWall(Player* bot, WallDrop const& drop)
{
    float const gateX = BG_SA_ObjSpawnlocs[drop.gate][0];
    float const gateY = BG_SA_ObjSpawnlocs[drop.gate][1];
    float const orientation = BG_SA_ObjSpawnlocs[drop.gate][3];
    float const differenceX = bot->GetPositionX() - gateX;
    float const differenceY = bot->GetPositionY() - gateY;
    if (differenceX > 24.0f)
        return true;
    if (differenceX * differenceX + differenceY * differenceY > 70.0f * 70.0f ||
        bot->GetPositionZ() > drop.start.GetPositionZ() - 1.0f)
        return false;
    return differenceX * std::cos(orientation) + differenceY * std::sin(orientation) > 6.0f;
}

void SelectDeployment(Player* bot, BattlegroundSA* bg, SotaTactics& result)
{
    if (result.stage > 1)
        return;
    WallDrop const& middle = MiddleWallDrops[result.lane];
    WallDrop const& outer = OuterWallDrops[result.lane];
    WallDrop const* selected = nullptr;
    if (!Destroyed(bg, middle.gate) && !OutsideWall(bot, middle))
        selected = &middle;
    else if (result.stage == 0 && !Destroyed(bg, outer.gate) && !OutsideWall(bot, outer))
        selected = &outer;
    if (!selected)
        return;
    result.deploying = true;
    result.deploymentStart = selected->start;
    result.deploymentEnd = selected->end;
    result.deploymentWaypoint = selected->start;
}

bool AvailableDemolisher(Creature* creature, Player* player)
{
    if (!creature || creature->GetEntry() != NPC_DEMOLISHER_SA || !creature->IsAlive() || !creature->IsInWorld() ||
        !creature->IsFriendlyTo(player) || creature->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE))
        return false;
    Vehicle* vehicle = creature->GetVehicleKit();
    return vehicle && !vehicle->GetPassenger(0);
}

void AssignVehicles(BattlegroundSA* bg, Player* bot, std::vector<Player*> const& pilots, SotaTactics& result)
{
    std::vector<ObjectGuid> assigned;
    for (Player* pilot : pilots)
    {
        if (!pilot->IsAlive() || pilot->GetVehicle())
            continue;
        Creature* nearest = nullptr;
        float nearestDistance = BOARDING_SEARCH_RANGE;
        for (uint32 index = BG_SA_DEMOLISHER_1; index <= BG_SA_DEMOLISHER_8; ++index)
        {
            Creature* creature = GetCreature(bg, index);
            if (!AvailableDemolisher(creature, pilot) ||
                std::find(assigned.begin(), assigned.end(), creature->GetGUID()) != assigned.end())
                continue;
            float const distance = pilot->GetExactDist(creature);
            if (distance < nearestDistance)
            {
                nearest = creature;
                nearestDistance = distance;
            }
        }
        if (!nearest)
            continue;
        assigned.push_back(nearest->GetGUID());
        if (pilot == bot)
            result.boardingVehicle = nearest->GetGUID();
    }
}

bool IsDefenseReady(PlayerbotAI* botAI, SotaTactics const& tactics)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    return bot && bot->IsInWorld() && bot->IsAlive() && tactics.enabled && tactics.active && !tactics.attacker &&
           !tactics.landingRequired && !tactics.deploying && !bot->IsBeingTeleported() && !bot->GetVehicle() &&
           !bot->GetTransport() && !PlayerbotAI::IsHeal(bot) &&
           (!tactics.stage || bot->GetExactDist2d(tactics.destination) <= DEFENSE_RELEASE_DISTANCE);
}

bool IsEnemyDemolisher(Player* bot, Unit* target)
{
    return target && target->GetEntry() == NPC_DEMOLISHER_SA && target->IsAlive() && target->IsInWorld() &&
           bot->IsInMap(target) && target->InSamePhase(bot) && bot->IsValidAttackTarget(target) &&
           bot->GetExactDist(target) <= VEHICLE_DEFENSE_RANGE && bot->IsWithinLOSInMap(target);
}

bool HasDefensePath(PlayerbotAI* botAI, Creature* target)
{
    Player* bot = botAI->GetBot();
    PathGenerator path(bot);
    path.SetExcludeFlags(NAV_WATER | NAV_MAGMA | NAV_SLIME | NAV_GROUND_STEEP);
    path.SetPathLengthLimit(VEHICLE_DEFENSE_PATH_LENGTH);
    if (!path.CalculatePath(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), false) ||
        path.GetPathType() != PATHFIND_NORMAL)
        return false;

    Movement::PointsArray const& points = path.GetPath();
    if (points.size() < 2 || points.size() > MAX_POINT_PATH_LENGTH ||
        target->GetExactDist(points.back().x, points.back().y, points.back().z) > DEFENSE_PATH_ENDPOINT_TOLERANCE)
        return false;

    float length = 0.0f;
    for (std::size_t index = 0; index < points.size(); ++index)
    {
        G3D::Vector3 const& point = points[index];
        if (!std::isfinite(point.x) || !std::isfinite(point.y) || !std::isfinite(point.z))
            return false;
        if (!index)
            continue;
        G3D::Vector3 const difference = point - points[index - 1];
        float const rise = std::fabs(difference.z);
        if (rise > 3.0f && rise > std::hypot(difference.x, difference.y) * 1.2f)
            return false;
        length += difference.length();
        if (length > VEHICLE_DEFENSE_PATH_LENGTH)
            return false;
    }
    return IsSotaGroundPathClear(botAI, bot, points);
}

void FindEnemyVehicle(BattlegroundSA* bg, PlayerbotAI* botAI, SotaTactics& result)
{
    if (!IsDefenseReady(botAI, result))
        return;
    Player* bot = botAI->GetBot();
    Unit* current = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    std::vector<Creature*> candidates;
    for (uint32 index = BG_SA_DEMOLISHER_1; index <= BG_SA_DEMOLISHER_8; ++index)
    {
        Creature* creature = GetCreature(bg, index);
        if (IsEnemyDemolisher(bot, creature))
            candidates.push_back(creature);
    }
    std::stable_sort(candidates.begin(), candidates.end(),
                     [bot, current](Creature* left, Creature* right)
                     {
                         if ((left == current) != (right == current))
                             return left == current;
                         return bot->GetExactDist(left) < bot->GetExactDist(right);
                     });
    for (Creature* creature : candidates)
    {
        if (!HasDefensePath(botAI, creature))
            continue;
        result.enemyVehicle = creature->GetGUID();
        return;
    }
}
}  // namespace

BattlegroundSA* GetSotaBattleground(Player* bot)
{
    if (!sPlayerbotAIConfig.SotaTacticsEnabled || !bot || !bot->InBattleground() || bot->GetMapId() != SOTA_MAP_ID)
        return nullptr;
    Battleground* bg = bot->GetBattleground();
    if (!bg || (bg->GetBgTypeID() == BATTLEGROUND_RB ? bg->GetBgTypeID(true) : bg->GetBgTypeID()) != BATTLEGROUND_SA)
        return nullptr;
    return dynamic_cast<BattlegroundSA*>(bg);
}

SotaTacticsValue* GetSotaTacticsValue(PlayerbotAI* botAI)
{
    if (!botAI || !GetSotaBattleground(botAI->GetBot()))
        return nullptr;
    return dynamic_cast<SotaTacticsValue*>(botAI->GetAiObjectContext()->GetValue<SotaTactics>("sota tactics"));
}

SotaTactics GetSotaTactics(PlayerbotAI* botAI)
{
    SotaTacticsValue* tactics = GetSotaTacticsValue(botAI);
    return tactics ? tactics->Get() : SotaTactics{};
}

bool IsSotaInfantryReady(PlayerbotAI* botAI, SotaTactics const& tactics)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    return bot && bot->IsInWorld() && bot->IsAlive() && tactics.enabled && tactics.active && !tactics.landingRequired &&
           !bot->IsBeingTeleported() && !bot->GetVehicle() && !bot->GetTransport() &&
           !bot->IsNonMeleeSpellCast(false, false, true);
}

Unit* GetSotaDefenseTarget(PlayerbotAI* botAI, SotaTactics const& tactics)
{
    if (!IsDefenseReady(botAI, tactics) || tactics.enemyVehicle.IsEmpty())
        return nullptr;
    Player* bot = botAI->GetBot();
    BattlegroundSA* bg = GetSotaBattleground(bot);
    if (!bg || bg->GetStatus() != STATUS_IN_PROGRESS || bot->GetBgTeamId() == bg->GetAttackerTeam() ||
        (bg->GetSotaStatus() != BG_SA_ROUND_ONE && bg->GetSotaStatus() != BG_SA_ROUND_TWO))
        return nullptr;
    Unit* target = botAI->GetUnit(tactics.enemyVehicle);
    return IsEnemyDemolisher(bot, target) ? target : nullptr;
}

void SotaTacticsValue::MarkLanded()
{
    _landed = true;
    value.landingRequired = false;
    Reset();
}

SotaTactics SotaTacticsValue::Calculate()
{
    SotaTactics result;
    BattlegroundSA* bg = GetSotaBattleground(bot);
    if (!bg)
    {
        _instanceId = 0;
        _attackerTeam = TEAM_NEUTRAL;
        _lane = 2;
        _landed = false;
        _defenderLaneAssigned = false;
        return result;
    }

    TeamId const attacker = bg->GetAttackerTeam();
    if (_instanceId != bg->GetInstanceID() || _attackerTeam != attacker)
    {
        _instanceId = bg->GetInstanceID();
        _attackerTeam = attacker;
        _lane = 2;
        _landed = false;
        _defenderLaneAssigned = false;
    }
    result.enabled = true;
    result.attacker = bot->GetBgTeamId() == attacker;
    BG_SA_Status const status = bg->GetSotaStatus();
    result.active = bg->GetStatus() == STATUS_IN_PROGRESS && (status == BG_SA_ROUND_ONE || status == BG_SA_ROUND_TWO);

    bool const observeBoat = result.attacker && !_landed;
    if (observeBoat)
    {
        ObjectGuid const transport = bot->GetTransGUID();
        ObjectGuid const rawTransport = bot->m_movementInfo.transport.guid;
        for (uint8 lane = 0; lane < 2; ++lane)
        {
            uint32 const boat = BG_SA_BOAT_ONE + lane;
            if (boat < bg->BgObjects.size() && !bg->BgObjects[boat].IsEmpty() &&
                (transport == bg->BgObjects[boat] || rawTransport == bg->BgObjects[boat]))
                _lane = lane;
        }
        if (_lane > 1 && bot->GetPositionX() > 1800.0f)
            _lane = bot->GetPositionY() >= 0.0f ? 1 : 0;
    }
    if (_lane > 1)
        _lane = uint8(bot->GetGUID().GetRawValue() & 1);
    if (!result.attacker && (!_defenderLaneAssigned || !result.active))
    {
        uint32 rank = 0;
        for (auto const& [guid, participant] : bg->GetPlayers())
            if (participant && participant->IsInWorld() && bot->IsInMap(participant) &&
                participant->GetBgTeamId() == bot->GetBgTeamId() && GET_PLAYERBOT_AI(participant) &&
                guid < bot->GetGUID())
                ++rank;
        _lane = uint8(rank % 2);
        _defenderLaneAssigned = result.active;
    }
    result.lane = _lane;
    result.stage = GetStage(bg);
    result.landingRequired = result.attacker && !_landed;
    if (!result.active)
        return result;

    std::vector<Player*> pilots;
    std::vector<Player*> candidates;
    uint32 teamBotCount = 0;
    for (auto const& [guid, participant] : bg->GetPlayers())
    {
        if (!participant || !participant->IsInWorld() || !bot->IsInMap(participant) ||
            participant->GetBgTeamId() != bot->GetBgTeamId() || !GET_PLAYERBOT_AI(participant))
            continue;
        ++teamBotCount;
        if (!participant->IsAlive() || participant->IsBeingTeleported())
            continue;
        Vehicle* vehicle = participant->GetVehicle();
        Unit* base = vehicle ? vehicle->GetBase() : nullptr;
        if (base && base->GetEntry() == NPC_DEMOLISHER_SA && vehicle->GetPassenger(0) == participant)
            pilots.push_back(participant);
        else if (!vehicle && !PlayerbotAI::IsHeal(participant, true))
            candidates.push_back(participant);
    }

    uint32 const pilotLimit =
        std::max(uint32(pilots.size()), teamBotCount >= 2 ? std::min(MAX_PILOTS, teamBotCount / 2) : 0);
    for (Player* candidate : candidates)
    {
        if (pilots.size() >= pilotLimit)
            break;
        pilots.push_back(candidate);
    }

    if (!result.attacker)
    {
        result.objective = SotaObjective::Defend;
        result.gate = result.stage == 0   ? OuterGate(_lane)
                      : result.stage == 1 ? MiddleGate(_lane)
                      : result.stage == 2 ? BG_SA_YELLOW_GATE
                                          : BG_SA_ANCIENT_GATE;
        result.destination = result.stage == 0 ? BeachPositions[_lane] : GateRoutes[result.gate].approach;
        if (result.stage > 0)
            result.destination = RouteToObjective(bot, bg, _lane, result.stage, result);
        SelectDeployment(bot, bg, result);
        FindEnemyVehicle(bg, botAI, result);
        return result;
    }

    result.pilot = std::find(pilots.begin(), pilots.end(), bot) != pilots.end();

    if (result.stage >= 4)
    {
        result.objective = SotaObjective::Relic;
        if (GameObject* relic = GetObject(bg, BG_SA_TITAN_RELIC))
        {
            result.objectiveGuid = relic->GetGUID();
            result.destination.Relocate(relic->GetPositionX(), relic->GetPositionY(), 111.911f);
            result.destination = RouteToObjective(bot, bg, _lane, 3, result);
        }
        return result;
    }

    uint8 const openLane = result.stage ? OpenLane(bg, _lane, false) : _lane;
    result.gate = result.stage == 0   ? OuterGate(_lane)
                  : result.stage == 1 ? MiddleGate(openLane)
                  : result.stage == 2 ? BG_SA_YELLOW_GATE
                                      : BG_SA_ANCIENT_GATE;
    result.objective = SotaObjective::Gate;
    if (GameObject* gate = GetObject(bg, result.gate))
        result.objectiveGuid = gate->GetGUID();
    result.destination = result.pilot ? SiegeApproach(result.gate) : GateRoutes[result.gate].approach;

    uint32 infantryRank = 0;
    for (auto const& [guid, participant] : bg->GetPlayers())
        if (participant && participant->IsInWorld() && bot->IsInMap(participant) &&
            participant->GetBgTeamId() == bot->GetBgTeamId() && GET_PLAYERBOT_AI(participant) &&
            guid < bot->GetGUID() && std::find(pilots.begin(), pilots.end(), participant) == pilots.end())
            ++infantryRank;
    uint8 transitStage = result.stage;
    if (!result.pilot && infantryRank % 4 == 0 && result.stage >= 1)
    {
        std::array<BG_SA_Graveyards, 3> const graveyards = {
            BG_SA_CENTRAL_CAPTURABLE_GY, _lane ? BG_SA_LEFT_CAPTURABLE_GY : BG_SA_RIGHT_CAPTURABLE_GY,
            _lane ? BG_SA_RIGHT_CAPTURABLE_GY : BG_SA_LEFT_CAPTURABLE_GY};
        for (BG_SA_Graveyards graveyard : graveyards)
        {
            if (graveyard == BG_SA_CENTRAL_CAPTURABLE_GY && result.stage < 2)
                continue;
            uint32 const flagIndex = graveyard == BG_SA_CENTRAL_CAPTURABLE_GY ? BG_SA_CENTRAL_FLAG
                                     : graveyard == BG_SA_LEFT_CAPTURABLE_GY  ? BG_SA_LEFT_FLAG
                                                                              : BG_SA_RIGHT_FLAG;
            GameObject* flag = GetObject(bg, flagIndex);
            if (!flag || bg->GetGraveyardOwner(graveyard) == attacker ||
                flag->HasGameObjectFlag(GO_FLAG_NOT_SELECTABLE | GO_FLAG_INTERACT_COND | GO_FLAG_IN_USE))
                continue;
            result.objective = SotaObjective::Graveyard;
            result.objectiveGuid = flag->GetGUID();
            result.destination.Relocate(flag);
            transitStage = graveyard == BG_SA_CENTRAL_CAPTURABLE_GY ? 2 : 1;
            break;
        }
    }

    result.destination = RouteToObjective(bot, bg, _lane, transitStage, result);
    if (result.pilot)
        AssignVehicles(bg, bot, pilots, result);
    return result;
}
