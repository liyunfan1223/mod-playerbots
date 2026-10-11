/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ServerFacade.h"
#include "Player.h"
#include "TargetedMovementGenerator.h"

float ServerFacade::GetDistance2d(Unit* unit, WorldObject* wo)
{
    ASSERT_NOTNULL(unit);
    ASSERT_NOTNULL(wo);

    float dist = unit->GetDistance2d(wo);
    return std::round(dist * 10.0f) / 10.0f;
}

float ServerFacade::GetDistance2d(Unit* unit, float x, float y)
{
    float dist = unit->GetDistance2d(x, y);
    return std::round(dist * 10.0f) / 10.0f;
}

bool ServerFacade::IsDistanceLessThan(float dist1, float dist2)
{
    // return dist1 - dist2 < sPlayerbotAIConfig.TargetPosRecalcDistance;
    return dist1 < dist2;
}

bool ServerFacade::IsDistanceGreaterThan(float dist1, float dist2)
{
    // return dist1 - dist2 > sPlayerbotAIConfig.TargetPosRecalcDistance;
    return dist1 > dist2;
}

bool ServerFacade::IsDistanceGreaterOrEqualThan(float dist1, float dist2) { return !IsDistanceLessThan(dist1, dist2); }

bool ServerFacade::IsDistanceLessOrEqualThan(float dist1, float dist2) { return !IsDistanceGreaterThan(dist1, dist2); }

void ServerFacade::SetFacingTo(Player* bot, WorldObject* wo, bool /*force*/)
{
    if (!bot)
        return;

    float angle = bot->GetAngle(wo);

    // if (!force && bot->isMoving())
    //     bot->SetFacingTo(bot->GetAngle(wo));
    // else
    // {
    bot->SetOrientation(angle);

    if (!bot->IsRooted())
        // enforce (bool self) true otherwhise when using real-client with SelfBot wont
        // recieve update; e.g. will not face the target when using (mostly ranged) attack
        bot->SendMovementFlagUpdate(true);
}

Unit* ServerFacade::GetChaseTarget(Unit* target)
{
    MovementGenerator* movementGen = target->GetMotionMaster()->top();
    if (movementGen && movementGen->GetMovementGeneratorType() == CHASE_MOTION_TYPE)
    {
        if (target->IsPlayer())
        {
            return static_cast<ChaseMovementGenerator<Player> const*>(movementGen)->GetTarget();
        }

        return static_cast<ChaseMovementGenerator<Creature> const*>(movementGen)->GetTarget();
    }

    return nullptr;
}

void ServerFacade::SendPacket(Player* player, WorldPacket* packet)
{
    player->GetSession()->SendPacket(packet);
}
