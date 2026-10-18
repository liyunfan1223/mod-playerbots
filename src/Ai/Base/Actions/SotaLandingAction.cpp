/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include <algorithm>
#include <cmath>
#include <iterator>

#include "BattlegroundSA.h"
#include "GameObject.h"
#include "Map.h"
#include "PlayerbotAI.h"
#include "SotaActions.h"
#include "Transport.h"

namespace
{
struct SotaLandingLane
{
    float shoreX;
    float shoreY;
    float shoreZ;
    float shipStartX;
    float shipStartY;
    float shipEndX;
    float shipEndY;
};

// Shore points are existing SotA bomb spawns; ship endpoints follow TransportAnimation.dbc.
constexpr SotaLandingLane landingLanes[] = {{1583.2f, -91.2291f, 8.49227f, 2682.936f, -830.368f, 1602.8976f, -82.1057f},
                                            {1593.13f, 46.8106f, 7.54073f, 2577.003f, 980.261f, 1619.8796f, 31.4473f}};
constexpr float recoveryCorridorWidth = 100.0f;
constexpr float recoveryMinimumZ = -25.0f;
constexpr float recoveryMaximumZ = 40.0f;
constexpr float inlandBoundaryX = 1550.0f;
constexpr float beachBoundaryX = 1700.0f;
constexpr float groundSearchOffset = 3.0f;
constexpr float groundSearchDistance = 10.0f;
constexpr float maximumGroundDifference = 4.0f;
constexpr float landingGroundOffset = 0.2f;
constexpr float inlandOrientation = 3.1415927f;

bool IsInShipCorridor(Player const* bot, SotaLandingLane const& lane)
{
    float x = bot->GetPositionX();
    float y = bot->GetPositionY();
    float z = bot->GetPositionZ();
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z) || z < recoveryMinimumZ || z > recoveryMaximumZ)
        return false;

    float deltaX = lane.shipEndX - lane.shipStartX;
    float deltaY = lane.shipEndY - lane.shipStartY;
    float progress = std::clamp(
        ((x - lane.shipStartX) * deltaX + (y - lane.shipStartY) * deltaY) / (deltaX * deltaX + deltaY * deltaY), 0.0f,
        1.0f);
    float distanceX = x - (lane.shipStartX + progress * deltaX);
    float distanceY = y - (lane.shipStartY + progress * deltaY);
    return distanceX * distanceX + distanceY * distanceY <= recoveryCorridorWidth * recoveryCorridorWidth;
}
}  // namespace

bool SotaLandingAction::isUseful()
{
    SotaTactics tactics = GetSotaTactics(botAI);
    return tactics.enabled && tactics.active && tactics.attacker && tactics.landingRequired && bot->IsAlive() &&
           bot->IsInWorld() && !bot->GetVehicle() && !bot->IsBeingTeleported();
}

bool SotaLandingAction::Execute(Event /*event*/)
{
    if (!isUseful() || bot->IsNonMeleeSpellCast(false, false, true))
        return false;

    BattlegroundSA* battleground = GetSotaBattleground(bot);
    SotaTacticsValue* tacticsValue = GetSotaTacticsValue(botAI);
    SotaTactics tactics = GetSotaTactics(botAI);
    Map* map = bot->GetMap();
    if (!battleground || !tacticsValue || !map || tactics.lane >= std::size(landingLanes))
        return false;

    BG_SA_Status status = battleground->GetSotaStatus();
    if ((status != BG_SA_ROUND_ONE && status != BG_SA_ROUND_TWO) || battleground->GetAttackerTeam() != bot->GetTeamId())
        return false;

    GameObject* boat = battleground->GetBGObject(tactics.lane == 0 ? BG_SA_BOAT_ONE : BG_SA_BOAT_TWO);
    bool onBoat =
        boat && (bot->GetTransGUID() == boat->GetGUID() || bot->m_movementInfo.transport.guid == boat->GetGUID());
    if (boat && !onBoat)
    {
        Transport* transport = map->GetTransportForPos(bot->GetPhaseMask(), bot->GetPositionX(), bot->GetPositionY(),
                                                       bot->GetPositionZ(), bot);
        onBoat = transport && transport->GetGUID() == boat->GetGUID();
    }

    bool alreadyOnShore = !onBoat && bot->GetPositionX() < inlandBoundaryX;
    if (!onBoat && !alreadyOnShore && bot->GetPositionX() < beachBoundaryX &&
        !map->IsInWater(bot->GetPhaseMask(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
                        bot->GetCollisionHeight()))
    {
        float ground = map->GetHeight(bot->GetPhaseMask(), bot->GetPositionX(), bot->GetPositionY(),
                                      bot->GetPositionZ() + groundSearchOffset, true, groundSearchDistance);
        alreadyOnShore = std::isfinite(ground) && std::fabs(ground - bot->GetPositionZ()) <= maximumGroundDifference;
    }

    if (alreadyOnShore)
    {
        tacticsValue->MarkLanded();
        return true;
    }

    if (bot->IsInCombat())
        return false;

    SotaLandingLane const& lane = landingLanes[tactics.lane];
    if (!onBoat && !IsInShipCorridor(bot, lane))
        return false;

    float ground = map->GetHeight(bot->GetPhaseMask(), lane.shoreX, lane.shoreY, lane.shoreZ + groundSearchOffset, true,
                                  groundSearchDistance);
    if (!std::isfinite(ground) || std::fabs(ground - lane.shoreZ) > maximumGroundDifference ||
        map->IsInWater(bot->GetPhaseMask(), lane.shoreX, lane.shoreY, ground, bot->GetCollisionHeight()))
        return false;

    if (!bot->TeleportTo(bot->GetMapId(), lane.shoreX, lane.shoreY, ground + landingGroundOffset, inlandOrientation))
        return false;

    tacticsValue->MarkLanded();
    return true;
}
