/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "MovementActions.h"

#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <string>

#include "Event.h"
#include "FleeManager.h"
#include "G3D/Vector3.h"
#include "GameObject.h"
#include "Geometry.h"
#include "LastMovementValue.h"
#include "LootObjectStack.h"
#include "Map.h"
#include "MotionMaster.h"
#include "MoveSplineInitArgs.h"
#include "MovementGenerator.h"
#include "ObjectDefines.h"
#include "ObjectGuid.h"
#include "PathGenerator.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "Position.h"
#include "PositionValue.h"
#include "Random.h"
#include "ServerFacade.h"
#include "SharedDefines.h"
#include "SpellAuraEffects.h"
#include "SpellInfo.h"
#include "Stances.h"
#include "TargetedMovementGenerator.h"
#include "Timer.h"
#include "Transport.h"
#include "Unit.h"
#include "Vehicle.h"
#include "WaypointMovementGenerator.h"
#include "Corpse.h"

MovementAction::MovementAction(PlayerbotAI* botAI, std::string const name) : Action(botAI, name)
{
    bot = botAI->GetBot();
}

void MovementAction::CreateWp(Player* wpOwner, float x, float y, float z, float o, uint32 entry, bool important)
{
    float dist = wpOwner->GetDistance(x, y, z);
    float delay = 1000.0f * dist / wpOwner->GetSpeed(MOVE_RUN) + sPlayerbotAIConfig->reactDelay;

    // if (!important)
    // delay *= 0.25;

    Creature* wpCreature = wpOwner->SummonCreature(entry, x, y, z - 1, o, TEMPSUMMON_TIMED_DESPAWN, delay);
    botAI->GetBot()->AddAura(246, wpCreature);

    if (!important)
        wpCreature->SetObjectScale(0.5f);
}

bool MovementAction::JumpTo(uint32 mapId, float x, float y, float z, MovementPriority priority)
{
    UpdateMovementState();
    if (!IsMovingAllowed(mapId, x, y, z))
    {
        return false;
    }
    if (IsDuplicateMove(mapId, x, y, z))
    {
        return false;
    }
    if (IsWaitingForLastMove(priority))
    {
        return false;
    }
    float botZ = bot->GetPositionZ();
    float speed = bot->GetSpeed(MOVE_RUN);
    MotionMaster& mm = *bot->GetMotionMaster();
    mm.Clear();
    mm.MoveJump(x, y, z, speed, speed, 1);
    AI_VALUE(LastMovement&, "last movement").Set(mapId, x, y, z, bot->GetOrientation(), 1000, priority);
    return true;
}

bool MovementAction::MoveNear(uint32 mapId, float x, float y, float z, float distance, MovementPriority priority)
{
    float angle = GetFollowAngle();
    return MoveTo(mapId, x + cos(angle) * distance, y + sin(angle) * distance, z, false, false, false, false, priority);
}

bool MovementAction::MoveNear(WorldObject* target, float distance, MovementPriority priority)
{
    if (!target)
        return false;

    distance += target->GetCombatReach();

    float x = target->GetPositionX();
    float y = target->GetPositionY();
    float z = target->GetPositionZ();
    float followAngle = GetFollowAngle();

    for (float angle = followAngle; angle <= followAngle + static_cast<float>(2 * M_PI);
         angle += static_cast<float>(M_PI / 4.f))
    {
        float x = target->GetPositionX() + cos(angle) * distance;
        float y = target->GetPositionY() + sin(angle) * distance;
        float z = target->GetPositionZ();

        if (!bot->IsWithinLOS(x, y, z))
            continue;

        bool moved = MoveTo(target->GetMapId(), x, y, z, false, false, false, false, priority);
        if (moved)
            return true;
    }

    // botAI->TellError("All paths not in LOS");
    return false;
}

bool MovementAction::MoveToLOS(WorldObject* target, bool ranged)
{
    if (!target)
        return false;

    // std::ostringstream out; out << "Moving to LOS!";
    // bot->Say(out.str(), LANG_UNIVERSAL);

    float x = target->GetPositionX();
    float y = target->GetPositionY();
    float z = target->GetPositionZ();

    // Use standard PathGenerator to find a route.
    PathGenerator path(bot);
    path.CalculatePath(x, y, z, false);
    PathType type = path.GetPathType();
    if (type != PATHFIND_NORMAL && type != PATHFIND_INCOMPLETE)
        return false;

    if (!ranged)
        return MoveTo((Unit*)target, target->GetCombatReach());

    float dist = FLT_MAX;
    PositionInfo dest;

    if (!path.GetPath().empty())
    {
        for (auto& point : path.GetPath())
        {
            if (botAI->HasStrategy("debug move", BOT_STATE_NON_COMBAT))
                CreateWp(bot, point.x, point.y, point.z, 0.0, 2334);

            float distPoint = target->GetDistance(point.x, point.y, point.z);
            if (distPoint < dist && target->IsWithinLOS(point.x, point.y, point.z + bot->GetCollisionHeight()))
            {
                dist = distPoint;
                dest.Set(point.x, point.y, point.z, target->GetMapId());

                if (ranged)
                    break;
            }
        }
    }

    if (dest.isSet())
        return MoveTo(dest.mapId, dest.x, dest.y, dest.z);
    else
        botAI->TellError("All paths not in LOS");

    return false;
}

bool MovementAction::MoveTo(uint32 mapId, float x, float y, float z, bool idle, bool react, bool normal_only,
                            bool exact_waypoint, MovementPriority priority, bool lessDelay, bool backwards)
{
    UpdateMovementState();
    if (!IsMovingAllowed(mapId, x, y, z))
    {
        return false;
    }
    if (IsDuplicateMove(mapId, x, y, z))
    {
        return false;
    }
    if (IsWaitingForLastMove(priority))
    {
        return false;
    }
    bool generatePath = !bot->IsFlying() && !bot->isSwimming();
    bool disableMoveSplinePath = sPlayerbotAIConfig->disableMoveSplinePath >= 2 ||
                                 (sPlayerbotAIConfig->disableMoveSplinePath == 1 && bot->InBattleground());
    if (Vehicle* vehicle = bot->GetVehicle())
    {
        VehicleSeatEntry const* seat = vehicle->GetSeatForPassenger(bot);
        Unit* vehicleBase = vehicle->GetBase();
        generatePath = vehicleBase->CanFly();
        if (!vehicleBase || !seat || !seat->CanControl())  // is passenger and cant move anyway
            return false;

        float distance = vehicleBase->GetExactDist(x, y, z);  // use vehicle distance, not bot
        if (distance > 0.01f)
        {
            MotionMaster& mm = *vehicleBase->GetMotionMaster();  // need to move vehicle, not bot
            mm.Clear();
            if (!backwards)
            {
                mm.MovePoint(0, x, y, z, generatePath);
            }
            else
            {
                mm.MovePointBackwards(0, x, y, z, generatePath);
            }
            float speed = backwards ? vehicleBase->GetSpeed(MOVE_RUN_BACK) : vehicleBase->GetSpeed(MOVE_RUN);
            float delay = 1000.0f * (distance / speed);
            if (lessDelay)
            {
                delay -= botAI->GetReactDelay();
            }
            delay = std::max(.0f, delay);
            delay = std::min((float)sPlayerbotAIConfig->maxWaitForMove, delay);
            AI_VALUE(LastMovement&, "last movement").Set(mapId, x, y, z, bot->GetOrientation(), delay, priority);
            return true;
        }
    }
    else if (exact_waypoint || disableMoveSplinePath || !generatePath)
    {
        float distance = bot->GetExactDist(x, y, z);
        if (distance > 0.01f)
        {
            if (bot->IsSitState())
                bot->SetStandState(UNIT_STAND_STATE_STAND);

            // if (bot->IsNonMeleeSpellCast(true))
            // {
            //     bot->CastStop();
            //     botAI->InterruptSpell();
            // }
            MotionMaster& mm = *bot->GetMotionMaster();
            mm.Clear();
            if (!backwards)
            {
                mm.MovePoint(0, x, y, z, generatePath);
            }
            else
            {
                mm.MovePointBackwards(0, x, y, z, generatePath);
            }
            float delay = 1000.0f * MoveDelay(distance, backwards);
            if (lessDelay)
            {
                delay -= botAI->GetReactDelay();
            }
            delay = std::max(.0f, delay);
            delay = std::min((float)sPlayerbotAIConfig->maxWaitForMove, delay);
            AI_VALUE(LastMovement&, "last movement").Set(mapId, x, y, z, bot->GetOrientation(), delay, priority);
            return true;
        }
    }
    else
    {
        float modifiedZ;
        Movement::PointsArray path =
            SearchForBestPath(x, y, z, modifiedZ, sPlayerbotAIConfig->maxMovementSearchTime, normal_only);
        if (modifiedZ == INVALID_HEIGHT)
        {
            return false;
        }
        float distance = bot->GetExactDist(x, y, modifiedZ);
        if (distance > 0.01f)
        {
            if (bot->IsSitState())
                bot->SetStandState(UNIT_STAND_STATE_STAND);

            // if (bot->IsNonMeleeSpellCast(true))
            // {
            //     bot->CastStop();
            //     botAI->InterruptSpell();
            // }
            MotionMaster& mm = *bot->GetMotionMaster();
            G3D::Vector3 endP = path.back();
            mm.Clear();
            if (!backwards)
            {
                mm.MovePoint(0, x, y, z, generatePath);
            }
            else
            {
                mm.MovePointBackwards(0, x, y, z, generatePath);
            }
            float delay = 1000.0f * MoveDelay(distance, backwards);
            if (lessDelay)
            {
                delay -= botAI->GetReactDelay();
            }
            delay = std::max(.0f, delay);
            delay = std::min((float)sPlayerbotAIConfig->maxWaitForMove, delay);
            AI_VALUE(LastMovement&, "last movement").Set(mapId, x, y, z, bot->GetOrientation(), delay, priority);
            return true;
        }
    }

    return false;
    //
    // // LOG_DEBUG("playerbots", "IsMovingAllowed {}", IsMovingAllowed());
    // bot->AddUnitMovementFlag()

    // bool isVehicle = false;
    // Unit* mover = bot;
    // if (Vehicle* vehicle = bot->GetVehicle())
    // {
    //     VehicleSeatEntry const* seat = vehicle->GetSeatForPassenger(bot);
    //     LOG_DEBUG("playerbots", "!seat || !seat->CanControl() {}", !seat || !seat->CanControl());
    //     if (!seat || !seat->CanControl())
    //         return false;

    //     isVehicle = true;
    //     mover = vehicle->GetBase();
    // }

    // bool detailedMove = botAI->AllowActivity(DETAILED_MOVE_ACTIVITY);
    // if (!detailedMove)
    // {
    //     time_t now = time(nullptr);
    //     if (AI_VALUE(LastMovement&, "last movement").nextTeleport > now) // We can not teleport yet. Wait.
    //     {
    //         LOG_DEBUG("playerbots", "AI_VALUE(LastMovement&, \"last movement\").nextTeleport > now");
    //         botAI->SetNextCheckDelay((AI_VALUE(LastMovement&, "last movement").nextTeleport - now) * 1000);
    //         return true;
    //     }
    // }

    // float minDist = sPlayerbotAIConfig->targetPosRecalcDistance; //Minium distance a bot should move.
    // float maxDist = sPlayerbotAIConfig->reactDistance;           //Maxium distance a bot can move in one single
    // action. float originalZ = z;                                        // save original destination height to check
    // if bot needs to fly up

    // bool generatePath = !bot->IsFlying() && !bot->HasUnitMovementFlag(MOVEMENTFLAG_SWIMMING) && !bot->IsInWater() &&
    // !bot->IsUnderWater(); if (generatePath)
    // {
    //     z += CONTACT_DISTANCE;
    //     mover->UpdateAllowedPositionZ(x, y, z);
    // }

    // if (!isVehicle && !IsMovingAllowed() && bot->isDead())
    // {
    //     bot->StopMoving();
    //     LOG_DEBUG("playerbots", "!isVehicle && !IsMovingAllowed() && bot->isDead()");
    //     return false;
    // }

    // if (!isVehicle && bot->isMoving() && !IsMovingAllowed())
    // {
    //     if (!bot->HasUnitState(UNIT_STATE_IN_FLIGHT))
    //         bot->StopMoving();
    //     LOG_DEBUG("playerbots", "!isVehicle && bot->isMoving() && !IsMovingAllowed()");
    //     return false;
    // }

    // LastMovement& lastMove = *context->GetValue<LastMovement&>("last movement");

    // WorldPosition startPosition = WorldPosition(bot);             //Current location of the bot
    // WorldPosition endPosition = WorldPosition(mapId, x, y, z, 0); //The requested end location
    // WorldPosition movePosition;                                   //The actual end location

    // float totalDistance = startPosition.distance(endPosition);    //Total distance to where we want to go
    // float maxDistChange = totalDistance * 0.1;                    //Maximum change between previous destination
    // before needing a recalulation

    // if (totalDistance < minDist)
    // {
    //     if (lastMove.lastMoveShort.distance(endPosition) < maxDistChange)
    //         AI_VALUE(LastMovement&, "last movement").clear();

    //     mover->StopMoving();
    //     LOG_DEBUG("playerbots", "totalDistance < minDist");
    //     return false;
    // }

    // TravelPath movePath;

    // if (lastMove.lastMoveShort.distance(endPosition) < maxDistChange &&
    // startPosition.distance(lastMove.lastMoveShort) < maxDist) //The last short movement was to the same place we want
    // to move now.
    //     movePosition = endPosition;
    // else if (!lastMove.lastPath.empty() && lastMove.lastPath.getBack().distance(endPosition) < maxDistChange) //The
    // last long movement was to the same place we want to move now.
    // {
    //     movePath = lastMove.lastPath;
    // }
    // else
    // {
    //     movePosition = endPosition;

    //     if (startPosition.getMapId() != endPosition.getMapId() || totalDistance > maxDist)
    //     {
    //         if (!sTravelNodeMap->getNodes().empty() && !bot->InBattleground())
    //         {
    //             if (sPlayerbotAIConfig->tweakValue)
    //             {
    //                 if (lastMove.future.valid())
    //                 {
    //                     movePath = lastMove.future.get();
    //                 }
    //                 else
    //                 {
    //                     lastMove.future = std::async(&TravelNodeMap::getFullPath, startPosition, endPosition, bot);
    //                     LOG_DEBUG("playerbots", "lastMove.future = std::async(&TravelNodeMap::getFullPath,
    //                     startPosition, endPosition, bot);"); return true;
    //                 }
    //             }
    //             else
    //                 movePath = sTravelNodeMap->getFullPath(startPosition, endPosition, bot);

    //             if (movePath.empty())
    //             {
    //                 //We have no path. Beyond 450yd the standard PathGenerator will probably move the wrong way.
    //                 if (sServerFacade->IsDistanceGreaterThan(totalDistance, maxDist * 3))
    //                 {
    //                     movePath.clear();
    //                     movePath.addPoint(endPosition);
    //                     AI_VALUE(LastMovement&, "last movement").setPath(movePath);

    //                     bot->StopMoving();
    //                     if (botAI->HasStrategy("debug move", BOT_STATE_NON_COMBAT))
    //                         botAI->TellMasterNoFacing("I have no path");
    //                     LOG_DEBUG("playerbots", "sServerFacade->IsDistanceGreaterThan(totalDistance, maxDist * 3)");
    //                     return false;
    //                 }

    //                 movePosition = endPosition;
    //             }
    //         }
    //         else
    //         {
    //             //Use standard PathGenerator to find a route.
    //             movePosition = endPosition;
    //         }
    //     }
    // }

    // if (movePath.empty() && movePosition.distance(startPosition) > maxDist)
    // {
    //     //Use standard PathGenerator to find a route.
    //     PathGenerator path(mover);
    //     path.CalculatePath(movePosition.getX(), movePosition.getY(), movePosition.getZ(), false);
    //     PathType type = path.GetPathType();
    //     Movement::PointsArray const& points = path.GetPath();
    //     movePath.addPath(startPosition.fromPointsArray(points));
    // }

    // if (!movePath.empty())
    // {
    //     if (movePath.makeShortCut(startPosition, maxDist))
    //         if (botAI->HasStrategy("debug move", BOT_STATE_NON_COMBAT))
    //             botAI->TellMasterNoFacing("Found a shortcut.");

    //     if (movePath.empty())
    //     {
    //         AI_VALUE(LastMovement&, "last movement").setPath(movePath);

    //         if (botAI->HasStrategy("debug move", BOT_STATE_NON_COMBAT))
    //             botAI->TellMasterNoFacing("Too far from path. Rebuilding.");
    //         LOG_DEBUG("playerbots", "movePath.empty()");
    //         return true;
    //     }

    //     TravelNodePathType pathType;
    //     uint32 entry;
    //     movePosition = movePath.getNextPoint(startPosition, maxDist, pathType, entry);

    //     if (pathType == TravelNodePathType::portal) // && !botAI->isRealPlayer())
    //     {
    //         //Log bot movement
    //         if (sPlayerbotAIConfig->hasLog("bot_movement.csv"))
    //         {
    //             WorldPosition telePos;
    //             if (entry)
    //             {
    //                 if (AreaTriggerTeleport const* at = sObjectMgr->GetAreaTriggerTeleport(entry))
    //                     telePos = WorldPosition(at->target_mapId, at->target_X, at->target_Y, at->target_Z,
    //                     at->target_Orientation);
    //             }
    //             else
    //                 telePos = movePosition;

    //             std::ostringstream out;
    //             out << sPlayerbotAIConfig->GetTimestampStr() << "+00,";
    //             out << bot->GetName() << ",";
    //             if (telePos && telePos.GetExactDist(movePosition) > 0.001)
    //                 startPosition.printWKT({ startPosition, movePosition, telePos }, out, 1);
    //             else
    //                 startPosition.printWKT({ startPosition, movePosition }, out, 1);

    //             out << std::to_string(bot->getRace()) << ",";
    //             out << std::to_string(bot->getClass()) << ",";
    //             out << bot->GetLevel() << ",";
    //             out << (entry ? -1 : entry);

    //             sPlayerbotAIConfig->log("bot_movement.csv", out.str().c_str());
    //         }

    //         if (entry)
    //         {
    //             AI_VALUE(LastMovement&, "last area trigger").lastAreaTrigger = entry;
    //         }
    //         else {
    //             LOG_DEBUG("playerbots", "!entry");
    //             return bot->TeleportTo(movePosition.getMapId(), movePosition.getX(), movePosition.getY(),
    //             movePosition.getZ(), movePosition.getO(), 0);
    //         }
    //     }

    //     if (pathType == TravelNodePathType::transport && entry)
    //     {
    //         if (!bot->GetTransport())
    //         {
    //             for (auto& transport : movePosition.getTransports(entry))
    //                 if (movePosition.sqDistance2d(WorldPosition((WorldObject*)transport)) < 5 * 5)
    //                     transport->AddPassenger(bot, true);
    //         }
    //         WaitForReach(100.0f);
    //         LOG_DEBUG("playerbots", "pathType == TravelNodePathType::transport && entry");
    //         return true;
    //     }

    //     if (pathType == TravelNodePathType::flightPath && entry)
    //     {
    //         if (TaxiPathEntry const* tEntry = sTaxiPathStore.LookupEntry(entry))
    //         {
    //             Creature* unit = nullptr;

    //             if (!bot->m_taxi.IsTaximaskNodeKnown(tEntry->from))
    //             {
    //                 GuidVector npcs = AI_VALUE(GuidVector, "nearest npcs");
    //                 for (GuidVector::iterator i = npcs.begin(); i != npcs.end(); i++)
    //                 {
    //                     Creature* unit = bot->GetNPCIfCanInteractWith(*i, UNIT_NPC_FLAG_FLIGHTMASTER);
    //                     if (!unit)
    //                         continue;

    //                     bot->GetSession()->SendLearnNewTaxiNode(unit);

    //                     unit->SetFacingTo(unit->GetAngle(bot));
    //                 }
    //             }

    //             uint32 botMoney = bot->GetMoney();
    //             if (botAI->HasCheat(BotCheatMask::gold))
    //             {
    //                 bot->SetMoney(10000000);
    //             }

    //             bool goTaxi = bot->ActivateTaxiPathTo({ tEntry->from, tEntry->to }, unit, 1);

    //             if (botAI->HasCheat(BotCheatMask::gold))
    //             {
    //                 bot->SetMoney(botMoney);
    //             }
    //             LOG_DEBUG("playerbots", "goTaxi");
    //             return goTaxi;
    //         }
    //     }

    //     // if (pathType == TravelNodePathType::teleportSpell && entry)
    //     // {
    //     //     if (entry == 8690)
    //     //     {
    //     //         if (!bot->HasSpellCooldown(8690))
    //     //         {
    //     //             return botAI->DoSpecificAction("hearthstone", Event("move action"));
    //     //         }
    //     //         else
    //     //         {
    //     //             movePath.clear();
    //     //             AI_VALUE(LastMovement&, "last movement").setPath(movePath);
    //     //             LOG_DEBUG("playerbots", "bot->HasSpellCooldown(8690)");
    //     //             return false;
    //     //         }
    //     //     }
    //     // }

    //     //if (!isTransport && bot->GetTransport())
    //     //    bot->GetTransport()->RemovePassenger(bot);
    // }

    // AI_VALUE(LastMovement&, "last movement").setPath(movePath);

    // if (!movePosition || movePosition.getMapId() != bot->GetMapId())
    // {
    //     movePath.clear();
    //     AI_VALUE(LastMovement&, "last movement").setPath(movePath);

    //     if (botAI->HasStrategy("debug move", BOT_STATE_NON_COMBAT))
    //         botAI->TellMasterNoFacing("No point. Rebuilding.");
    //     LOG_DEBUG("playerbots", "!movePosition || movePosition.getMapId() != bot->GetMapId()");
    //     return false;
    // }

    // if (movePosition.distance(startPosition) > maxDist)
    // {
    //     //Use standard pathfinder to find a route.
    //     PathGenerator path(mover);
    //     path.CalculatePath(movePosition.getX(), movePosition.getY(), movePosition.getZ(), false);
    //     PathType type = path.GetPathType();
    //     Movement::PointsArray const& points = path.GetPath();
    //     movePath.addPath(startPosition.fromPointsArray(points));
    //     TravelNodePathType pathType;
    //     uint32 entry;
    //     movePosition = movePath.getNextPoint(startPosition, maxDist, pathType, entry);
    // }

    // if (movePosition == WorldPosition())
    // {
    //     movePath.clear();

    //     AI_VALUE(LastMovement&, "last movement").setPath(movePath);

    //     if (botAI->HasStrategy("debug move", BOT_STATE_NON_COMBAT))
    //         botAI->TellMasterNoFacing("No point. Rebuilding.");

    //     return false;
    // }

    // //Visual waypoints
    // if (botAI->HasStrategy("debug move", BOT_STATE_NON_COMBAT))
    // {
    //     if (!movePath.empty())
    //     {
    //         float cx = x;
    //         float cy = y;
    //         float cz = z;
    //         for (auto i : movePath.getPath())
    //         {
    //             CreateWp(bot, i.point.getX(), i.point.getY(), i.point.getZ(), 0.f, 2334);

    //             cx = i.point.getX();
    //             cy = i.point.getY();
    //             cz = i.point.getZ();
    //         }
    //     }
    //     else
    //         CreateWp(bot, movePosition.getX(), movePosition.getY(), movePosition.getZ(), 0, 2334, true);
    // }

    // //Log bot movement
    // if (sPlayerbotAIConfig->hasLog("bot_movement.csv") && lastMove.lastMoveShort.GetExactDist(movePosition) > 0.001)
    // {
    //     std::ostringstream out;
    //     out << sPlayerbotAIConfig->GetTimestampStr() << "+00,";
    //     out << bot->GetName() << ",";
    //     startPosition.printWKT({ startPosition, movePosition }, out, 1);
    //     out << std::to_string(bot->getRace()) << ",";
    //     out << std::to_string(bot->getClass()) << ",";
    //     out << bot->GetLevel();
    //     out << 0;

    //     sPlayerbotAIConfig->log("bot_movement.csv", out.str().c_str());
    // }
    // // LOG_DEBUG("playerbots", "({}, {}) -> ({}, {})", startPosition.getX(), startPosition.getY(),
    // movePosition.getX(), movePosition.getY()); if (!react)
    //     if (totalDistance > maxDist)
    //         WaitForReach(startPosition.distance(movePosition) - 10.0f);
    //     else
    //         WaitForReach(startPosition.distance(movePosition));

    // if (!isVehicle)
    // {
    //     bot->HandleEmoteCommand(0);
    //     if (bot->IsSitState())
    //         bot->SetStandState(UNIT_STAND_STATE_STAND);

    //     if (bot->IsNonMeleeSpellCast(true))
    //     {
    //         bot->CastStop();
    //         botAI->InterruptSpell();
    //     }
    // }

    //  /* Why do we do this?
    // if (lastMove.lastMoveShort.distance(movePosition) < minDist)
    // {
    //     bot->StopMoving();
    //     bot->GetMotionMaster()->Clear();
    // }
    // */

    // // Clean movement if not already moving the same way.
    // // if (mover->GetMotionMaster()->GetCurrentMovementGeneratorType() != POINT_MOTION_TYPE)
    // // {
    // //     mover->StopMoving();
    // //     mover->GetMotionMaster()->Clear();
    // // }
    // // else
    // // {
    // //     mover->GetMotionMaster()->GetDestination(x, y, z);
    // //     if (movePosition.distance(WorldPosition(movePosition.getMapId(), x, y, z, 0)) > minDist)
    // //     {
    // //         mover->StopMoving();
    // //         mover->GetMotionMaster()->Clear();
    // //     }
    // // }

    // if (totalDistance > maxDist && !detailedMove && !botAI->HasPlayerNearby(&movePosition)) // Why walk if you can
    // fly?
    // {
    //     time_t now = time(nullptr);

    //     AI_VALUE(LastMovement&, "last movement").nextTeleport = now +
    //     (time_t)MoveDelay(startPosition.distance(movePosition)); LOG_DEBUG("playerbots", "totalDistance > maxDist &&
    //     !detailedMove && !botAI->HasPlayerNearby(&movePosition)"); return bot->TeleportTo(movePosition.getMapId(),
    //     movePosition.getX(), movePosition.getY(), movePosition.getZ(), startPosition.getAngleTo(movePosition));
    // }

    // // walk if master walks and is close
    // bool masterWalking = false;
    // if (botAI->GetMaster())
    // {
    //     if (botAI->GetMaster()->m_movementInfo.HasMovementFlag(MOVEMENTFLAG_WALKING) &&
    //     sServerFacade->GetDistance2d(bot, botAI->GetMaster()) < 20.0f)
    //         masterWalking = true;
    // }

    // if (masterWalking)
    //     bot->SetWalk(true);

    // bot->SendMovementFlagUpdate();
    // // LOG_DEBUG("playerbots", "normal move? {} {} {}",
    // !bot->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED) && !bot->HasAuraType(SPELL_AURA_FLY),
    // //     bot->HasUnitFlag(UNIT_FLAG_DISABLE_MOVE), bot->getStandState());
    // if (!bot->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED) && !bot->HasAuraType(SPELL_AURA_FLY))
    // {
    //     bot->SetWalk(masterWalking);
    //     bot->GetMotionMaster()->MovePoint(movePosition.getMapId(), movePosition.getX(), movePosition.getY(),
    //     movePosition.getZ(), generatePath); WaitForReach(startPosition.distance(movePosition));
    //     // LOG_DEBUG("playerbots", "Movepoint to ({}, {})", movePosition.getX(), movePosition.getY());
    // }
    // else
    // {
    //     bool needFly = false;
    //     bool needLand = false;
    //     bool isFly = bot->IsFlying();

    //     if (!isFly && originalZ > bot->GetPositionZ() && (originalZ - bot->GetPositionZ()) > 5.0f)
    //         needFly = true;

    //     if (needFly && !isFly)
    //     {
    //         WorldPacket data(SMSG_SPLINE_MOVE_SET_FLYING, 9);
    //         data << bot->GetPackGUID();
    //         bot->SendMessageToSet(&data, true);

    //         if (!bot->m_movementInfo.HasMovementFlag(MOVEMENTFLAG_FLYING))
    //             bot->m_movementInfo.AddMovementFlag(MOVEMENTFLAG_FLYING);

    //         if (!bot->m_movementInfo.HasMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY))
    //             bot->m_movementInfo.AddMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY);
    //     }

    //     if (isFly)
    //     {
    //         float ground = bot->GetPositionZ();
    //         float height = bot->GetMap()->GetWaterOrGroundLevel(bot->GetPositionX(), bot->GetPositionY(),
    //         bot->GetPositionZ(), ground); if (bot->GetPositionZ() > originalZ && (bot->GetPositionZ() - originalZ
    //         < 5.0f) && (fabs(originalZ - ground) < 5.0f))
    //             needLand = true;

    //         if (needLand)
    //         {
    //             WorldPacket data(SMSG_SPLINE_MOVE_UNSET_FLYING, 9);
    //             data << bot->GetPackGUID();
    //             bot->SendMessageToSet(&data, true);

    //             if (bot->m_movementInfo.HasMovementFlag(MOVEMENTFLAG_FLYING))
    //                 bot->m_movementInfo.RemoveMovementFlag(MOVEMENTFLAG_FLYING);

    //             if (bot->m_movementInfo.HasMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY))
    //                 bot->m_movementInfo.RemoveMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY);
    //         }
    //     }

    //     bot->GetMotionMaster()->MovePoint(movePosition.getMapId(), Position(movePosition.getX(), movePosition.getY(),
    //     movePosition.getZ(), 0.f)); WaitForReach(startPosition.distance(movePosition)); LOG_DEBUG("playerbots",
    //     "Movepoint to ({}, {})", movePosition.getX(), movePosition.getY());
    // }

    // AI_VALUE(LastMovement&, "last movement").setShort(movePosition);

    // if (!idle)
    //     ClearIdleState();

    // LOG_DEBUG("playerbots", "return true in the end");
    // return true;
}

bool MovementAction::MoveTo(WorldObject* target, float distance, MovementPriority priority)
{
    if (!IsMovingAllowed(target))
        return false;

    float bx = bot->GetPositionX();
    float by = bot->GetPositionY();
    float bz = bot->GetPositionZ();

    float tx = target->GetPositionX();
    float ty = target->GetPositionY();
    float tz = target->GetPositionZ();

    float distanceToTarget = bot->GetDistance(target);
    float angle = bot->GetAngle(target);
    float needToGo = distanceToTarget - distance;

    float maxDistance = sPlayerbotAIConfig->spellDistance;
    if (needToGo > 0 && needToGo > maxDistance)
        needToGo = maxDistance;
    else if (needToGo < 0 && needToGo < -maxDistance)
        needToGo = -maxDistance;

    float dx = cos(angle) * needToGo + bx;
    float dy = sin(angle) * needToGo + by;
    float dz;  // = std::max(bz, tz); // calc accurate z position to avoid stuck
    if (distanceToTarget > CONTACT_DISTANCE)
    {
        dz = bz + (tz - bz) * (needToGo / distanceToTarget);
    }
    else
    {
        dz = tz;
    }
    return MoveTo(target->GetMapId(), dx, dy, dz, false, false, false, false, priority);
}

bool MovementAction::ReachCombatTo(Unit* target, float distance)
{
    if (!IsMovingAllowed(target))
        return false;

    float bx = bot->GetPositionX();
    float by = bot->GetPositionY();
    float bz = bot->GetPositionZ();

    float tx = target->GetPositionX();
    float ty = target->GetPositionY();
    float tz = target->GetPositionZ();

    float targetOrientation = target->GetOrientation();

    float deltaAngle = Position::NormalizeOrientation(targetOrientation - target->GetAngle(bot));
    if (deltaAngle > M_PI)
        deltaAngle -= 2.0f * M_PI; // -PI..PI
    // if target is moving forward and moving far away, predict the position
    bool behind = fabs(deltaAngle) > M_PI_2;
    if (target->HasUnitMovementFlag(MOVEMENTFLAG_FORWARD) && behind) {
        float predictDis = std::min(3.0f, target->GetObjectSize() * 2);
        tx += cos(target->GetOrientation()) * predictDis;
        ty += sin(target->GetOrientation()) * predictDis;
        if (!target->GetMap()->CheckCollisionAndGetValidCoords(target, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(),
        tx, ty, tz))
        {
            tx = target->GetPositionX();
            ty = target->GetPositionY();
            tz = target->GetPositionZ();
        }
    }
    float combatDistance = bot->GetCombatReach() + target->GetCombatReach();
    distance += combatDistance;

    if (bot->GetExactDist(tx, ty, tz) <= distance)
        return false;

    PathGenerator path(bot);
    path.CalculatePath(tx, ty, tz, false);
    PathType type = path.GetPathType();
    int typeOk = PATHFIND_NORMAL | PATHFIND_INCOMPLETE | PATHFIND_SHORTCUT;
    if (!(type & typeOk))
        return false;
    float shortenTo = distance;

    // Avoid walking too far when moving towards each other
    float disToGo = bot->GetExactDist(tx, ty, tz) - distance;
    if (disToGo >= 6.0f)
        shortenTo = disToGo / 2 + distance;

    // if (bot->GetExactDist(tx, ty, tz) <= shortenTo)
    //     return false;

    path.ShortenPathUntilDist(G3D::Vector3(tx, ty, tz), shortenTo);
    G3D::Vector3 endPos = path.GetPath().back();
    return MoveTo(target->GetMapId(), endPos.x, endPos.y, endPos.z, false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT, true);
}

float MovementAction::GetFollowAngle()
{
    Player* master = GetMaster();
    Group* group = master ? master->GetGroup() : bot->GetGroup();
    if (!group)
        return 0.0f;

    uint32 index = 1;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        if (ref->GetSource() == master)
            continue;

        if (ref->GetSource() == bot)
            return 2 * M_PI / (group->GetMembersCount() - 1) * index;

        ++index;
    }

    return 0;
}

bool MovementAction::IsMovingAllowed(WorldObject* target)
{
    if (!target)
        return false;

    if (bot->GetMapId() != target->GetMapId())
        return false;

    // float distance = sServerFacade->GetDistance2d(bot, target);
    // if (!bot->InBattleground() && distance > sPlayerbotAIConfig->reactDistance)
    //     return false;

    return IsMovingAllowed();
}

bool MovementAction::IsMovingAllowed(uint32 mapId, float x, float y, float z)
{
    // removed sqrt as means distance limit was effectively 22500 (ReactDistance�)
    // leaving it commented incase we find ReactDistance limit causes problems
    // float distance = sqrt(bot->GetDistance(x, y, z));

    // Remove react distance limit
    // if (!bot->InBattleground())
    //     return false;

    return IsMovingAllowed();
}

bool MovementAction::IsDuplicateMove(uint32 mapId, float x, float y, float z)
{
    LastMovement& lastMove = *context->GetValue<LastMovement&>("last movement");

    // heuristic 5s
    if (lastMove.msTime + sPlayerbotAIConfig->maxWaitForMove < getMSTime() ||
        lastMove.lastMoveShort.GetExactDist(x, y, z) > 0.01f)
        return false;

    return true;
}

bool MovementAction::IsWaitingForLastMove(MovementPriority priority)
{
    LastMovement& lastMove = *context->GetValue<LastMovement&>("last movement");

    if (priority > lastMove.priority)
        return false;

    // heuristic 5s
    if (lastMove.lastdelayTime + lastMove.msTime > getMSTime())
        return true;

    return false;
}

bool MovementAction::IsMovingAllowed()
{
    // do not allow if not vehicle driver
    if (botAI->IsInVehicle() && !botAI->IsInVehicle(true))
        return false;

    if (bot->isFrozen() || bot->IsPolymorphed() || (bot->isDead() && !bot->HasPlayerFlag(PLAYER_FLAGS_GHOST)) ||
        bot->IsBeingTeleported() || bot->HasRootAura() || bot->HasSpiritOfRedemptionAura() ||
        bot->HasConfuseAura() || bot->IsCharmed() || bot->HasStunAura() ||
        bot->IsInFlight() || bot->HasUnitState(UNIT_STATE_LOST_CONTROL))
        return false;

    if (bot->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_CONTROLLED) != NULL_MOTION_TYPE)
    {
        return false;
    }

    // if (bot->HasUnitMovementFlag(MOVEMENTFLAG_FALLING)) {
    //     return false;
    // }
    return bot->GetMotionMaster()->GetCurrentMovementGeneratorType() != FLIGHT_MOTION_TYPE;
}

bool MovementAction::Follow(Unit* target, float distance) { return Follow(target, distance, GetFollowAngle()); }

void MovementAction::UpdateMovementState()
{
    int8 botInLiquidState = bot->GetLiquidData().Status;

    if (botInLiquidState == LIQUID_MAP_IN_WATER || botInLiquidState == LIQUID_MAP_UNDER_WATER)
    {
        bot->SetSwim(true);
    }
    else
    {
        bot->SetSwim(false);
    }

    bool onGround = bot->GetPositionZ() <
                    bot->GetMapWaterOrGroundLevel(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ()) + 1.0f;

    // Keep bot->SendMovementFlagUpdate() withing the if statements to not intefere with bot behavior on ground/(shallow) waters
    if (!bot->HasUnitMovementFlag(MOVEMENTFLAG_FLYING) &&
        bot->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED) && !onGround)
    {
        bot->AddUnitMovementFlag(MOVEMENTFLAG_FLYING);
        bot->SendMovementFlagUpdate();
    }

    else if (bot->HasUnitMovementFlag(MOVEMENTFLAG_FLYING) &&
        (!bot->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED) || onGround))
    {
        bot->RemoveUnitMovementFlag(MOVEMENTFLAG_FLYING);
        bot->SendMovementFlagUpdate();
    }

    // See if the bot is currently slowed, rooted, or otherwise unable to move
    bool isCurrentlyRestricted = bot->isFrozen() || bot->IsPolymorphed() || bot->HasRootAura() || bot->HasStunAura() ||
                                 bot->HasConfuseAura() || bot->HasUnitState(UNIT_STATE_LOST_CONTROL);

    // Detect if movement restrictions have been lifted
    if (wasMovementRestricted && !isCurrentlyRestricted && bot->IsAlive())
    {
        // CC just ended - refresh movement state to ensure animations play correctly
        bot->SendMovementFlagUpdate();
    }

    // Save current state for the next check
    wasMovementRestricted = isCurrentlyRestricted;

    // Temporary speed increase in group
    // if (botAI->HasRealPlayerMaster()) {
    //     bot->SetSpeedRate(MOVE_RUN, 1.1f);
    // } else {
    //     bot->SetSpeedRate(MOVE_RUN, 1.0f);
    // }
    // check if target is not reachable (from Vmangos)
    // if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == CHASE_MOTION_TYPE && bot->CanNotReachTarget() &&
    // !bot->InBattleground())
    // {
    //     if (Unit* pTarget = sServerFacade->GetChaseTarget(bot))
    //     {
    //         if (!bot->IsWithinMeleeRange(pTarget) && pTarget->GetTypeId() == TYPEID_UNIT)
    //         {
    //             float angle = bot->GetAngle(pTarget);
    //             float distance = 5.0f;
    //             float x = bot->GetPositionX() + cos(angle) * distance;
    //             float y = bot->GetPositionY() + sin(angle) * distance;
    //             float z = bot->GetPositionZ();

    //             z += CONTACT_DISTANCE;
    //             bot->UpdateAllowedPositionZ(x, y, z);

    //             bot->StopMoving();
    //             bot->GetMotionMaster()->Clear();
    //             bot->NearTeleportTo(x, y, z, bot->GetOrientation());
    //             //bot->GetMotionMaster()->MovePoint(bot->GetMapId(), x, y, z, FORCED_MOVEMENT_RUN, false);
    //             return;
    //             /*if (pTarget->IsCreature() && !bot->isMoving() && bot->IsWithinDist(pTarget, 10.0f))
    //             {
    //                 // Cheating to prevent getting stuck because of bad mmaps.
    //                 bot->StopMoving();
    //                 bot->GetMotionMaster()->Clear();
    //                 bot->GetMotionMaster()->MovePoint(bot->GetMapId(), pTarget->GetPosition(), FORCED_MOVEMENT_RUN,
    //                 false); return;
    //             }*/
    //         }
    //     }
    // }

    // if ((bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE ||
    //     bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == POINT_MOTION_TYPE) && bot->CanNotReachTarget()
    //     && !bot->InBattleground())
    // {
    //     if (Unit* pTarget = sServerFacade->GetChaseTarget(bot))
    //     {
    //         if (pTarget != botAI->GetGroupMaster())
    //             return;

    //         if (!bot->IsWithinMeleeRange(pTarget))
    //         {
    //             if (!bot->isMoving() && bot->IsWithinDist(pTarget, 10.0f))
    //             {
    //                 // Cheating to prevent getting stuck because of bad mmaps.
    //                 float angle = bot->GetAngle(pTarget);
    //                 float distance = 5.0f;
    //                 float x = bot->GetPositionX() + cos(angle) * distance;
    //                 float y = bot->GetPositionY() + sin(angle) * distance;
    //                 float z = bot->GetPositionZ();

    //                 z += CONTACT_DISTANCE;
    //                 bot->UpdateAllowedPositionZ(x, y, z);

    //                 bot->StopMoving();
    //                 bot->GetMotionMaster()->Clear();
    //                 bot->NearTeleportTo(x, y, z, bot->GetOrientation());
    //                 //bot->GetMotionMaster()->MovePoint(bot->GetMapId(), x, y, z, FORCED_MOVEMENT_RUN, false);
    //                 return;
    //             }
    //         }
    //     }
    // }
}

bool MovementAction::Follow(Unit* target, float distance, float angle)
{
    UpdateMovementState();

    if (!target)
        return false;

    if (!bot->InBattleground() && sServerFacade->IsDistanceLessOrEqualThan(sServerFacade->GetDistance2d(bot, target),
                                                                           sPlayerbotAIConfig->followDistance))
    {
        // botAI->TellError("No need to follow");
        return false;
    }

    /*
    if (!bot->InBattleground()
        && sServerFacade->IsDistanceLessOrEqualThan(sServerFacade->GetDistance2d(bot, target->GetPositionX(),
    target->GetPositionY()), sPlayerbotAIConfig->sightDistance)
        && abs(bot->GetPositionZ() - target->GetPositionZ()) >= sPlayerbotAIConfig->spellDistance &&
    botAI->HasRealPlayerMaster()
        && (target->GetMapId() && bot->GetMapId() != target->GetMapId()))
    {
        bot->StopMoving();
        bot->GetMotionMaster()->Clear();

        float x = bot->GetPositionX();
        float y = bot->GetPositionY();
        float z = target->GetPositionZ();
        if (target->GetMapId() && bot->GetMapId() != target->GetMapId())
        {
            if ((target->GetMap() && target->GetMap()->IsBattlegroundOrArena()) || (bot->GetMap() &&
    bot->GetMap()->IsBattlegroundOrArena())) return false;

            bot->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TELEPORTED | AURA_INTERRUPT_FLAG_CHANGE_MAP);
            bot->TeleportTo(target->GetMapId(), x, y, z, bot->GetOrientation());
        }
        else
        {
            bot->Relocate(x, y, z, bot->GetOrientation());
        }

        AI_VALUE(LastMovement&, "last movement").Set(target);
        ClearIdleState();
        return true;
    }

    if (!IsMovingAllowed(target) && botAI->HasRealPlayerMaster())
    {
        if ((target->GetMap() && target->GetMap()->IsBattlegroundOrArena()) || (bot->GetMap() &&
    bot->GetMap()->IsBattlegroundOrArena())) return false;

        if (bot->isDead() && botAI->GetMaster()->IsAlive())
        {
            bot->ResurrectPlayer(1.0f, false);
            botAI->TellMasterNoFacing("I live, again!");
        }
        else
            botAI->TellError("I am stuck while following");

        bot->CombatStop(true);
        botAI->TellMasterNoFacing("I will there soon.");
        bot->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TELEPORTED | AURA_INTERRUPT_FLAG_CHANGE_MAP);
        bot->TeleportTo(target->GetMapId(), target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(),
    target->GetOrientation()); return false;
    }
    */

    // Move to target corpse if alive.
    if (!target->IsAlive() && bot->IsAlive() && target->GetGUID().IsPlayer())
    {
        Player* pTarget = (Player*)target;

        Corpse* corpse = pTarget->GetCorpse();

        if (corpse)
        {
            WorldPosition botPos(bot);
            WorldPosition cPos(corpse);

            if (botPos.fDist(cPos) > sPlayerbotAIConfig->spellDistance)
                return MoveTo(cPos.getMapId(), cPos.getX(), cPos.getY(), cPos.getZ());
        }
    }

    if (sServerFacade->IsDistanceGreaterOrEqualThan(sServerFacade->GetDistance2d(bot, target),
                                                    sPlayerbotAIConfig->sightDistance))
    {
        if (target->GetGUID().IsPlayer())
        {
            Player* pTarget = (Player*)target;

            PlayerbotAI* targetBotAI = GET_PLAYERBOT_AI(pTarget);
            if (targetBotAI)  // Try to move to where the bot is going if it is closer and in the same direction.
            {
                WorldPosition botPos(bot);
                WorldPosition tarPos(target);
                WorldPosition longMove =
                    targetBotAI->GetAiObjectContext()->GetValue<WorldPosition>("last long move")->Get();

                if (longMove)
                {
                    float lDist = botPos.fDist(longMove);
                    float tDist = botPos.fDist(tarPos);
                    float ang = botPos.getAngleBetween(tarPos, longMove);
                    if ((lDist * 1.5 < tDist && ang < static_cast<float>(M_PI) / 2) ||
                        target->HasUnitState(UNIT_STATE_IN_FLIGHT))
                    {
                        return MoveTo(longMove.getMapId(), longMove.getX(), longMove.getY(), longMove.getZ());
                    }
                }
            }
            else
            {
                if (pTarget->HasUnitState(UNIT_STATE_IN_FLIGHT))  // Move to where the player is flying to.
                {
                    TaxiPathNodeList const& tMap =
                        static_cast<FlightPathMovementGenerator*>(pTarget->GetMotionMaster()->top())->GetPath();
                    if (!tMap.empty())
                    {
                        auto tEnd = tMap.back();
                        if (tEnd)
                            return MoveTo(tEnd->mapid, tEnd->x, tEnd->y, tEnd->z);
                    }
                }
            }
        }

        if (!target->HasUnitState(UNIT_STATE_IN_FLIGHT))
            return MoveTo(target, sPlayerbotAIConfig->followDistance);
    }

    if (sServerFacade->IsDistanceLessOrEqualThan(sServerFacade->GetDistance2d(bot, target),
                                                 sPlayerbotAIConfig->followDistance))
    {
        // botAI->TellError("No need to follow");
        return false;
    }

    if (target->IsFriendlyTo(bot) && bot->IsMounted() && AI_VALUE(GuidVector, "all targets").empty())
        distance += angle;

    if (!bot->InBattleground() && sServerFacade->IsDistanceLessOrEqualThan(sServerFacade->GetDistance2d(bot, target),
                                                                           sPlayerbotAIConfig->followDistance))
    {
        // botAI->TellError("No need to follow");
        return false;
    }

    bot->HandleEmoteCommand(0);

    if (bot->IsSitState())
        bot->SetStandState(UNIT_STAND_STATE_STAND);

    if (bot->IsNonMeleeSpellCast(true))
    {
        bot->CastStop();
        botAI->InterruptSpell();
    }

    // AI_VALUE(LastMovement&, "last movement").Set(target);
    ClearIdleState();

    if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE)
    {
        Unit* currentTarget = sServerFacade->GetChaseTarget(bot);
        if (currentTarget && currentTarget->GetGUID() == target->GetGUID())
            return false;
    }

    if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() != FOLLOW_MOTION_TYPE)
        bot->GetMotionMaster()->Clear();

    bot->GetMotionMaster()->MoveFollow(target, distance, angle);
    return true;
}

bool MovementAction::ChaseTo(WorldObject* obj, float distance, float angle)
{
    if (!IsMovingAllowed())
    {
        return false;
    }

    if (Vehicle* vehicle = bot->GetVehicle())
    {
        VehicleSeatEntry const* seat = vehicle->GetSeatForPassenger(bot);
        if (!seat || !seat->CanControl())
            return false;

        // vehicle->GetMotionMaster()->Clear();
        vehicle->GetBase()->GetMotionMaster()->MoveChase((Unit*)obj, 30.0f);
        return true;
    }

    UpdateMovementState();

    if (!bot->IsStandState())
        bot->SetStandState(UNIT_STAND_STATE_STAND);

    if (bot->IsNonMeleeSpellCast(true))
    {
        bot->CastStop();
        botAI->InterruptSpell();
    }

    // bot->GetMotionMaster()->Clear();
    bot->GetMotionMaster()->MoveChase((Unit*)obj, distance);

    // TODO shouldnt this use "last movement" value?
    WaitForReach(bot->GetExactDist2d(obj) - distance);
    return true;
}

float MovementAction::MoveDelay(float distance, bool backwards)
{
    float speed;
    if (bot->isSwimming())
    {
        speed = backwards ? bot->GetSpeed(MOVE_SWIM_BACK) : bot->GetSpeed(MOVE_SWIM);
    }
    else if (bot->IsFlying())
    {
        speed = backwards ? bot->GetSpeed(MOVE_FLIGHT_BACK) : bot->GetSpeed(MOVE_FLIGHT);
    }
    else
    {
        speed = backwards ? bot->GetSpeed(MOVE_RUN_BACK) :bot->GetSpeed(MOVE_RUN);
    }
    float delay = distance / speed;
    return delay;
}

// TODO should this be removed? (or modified to use "last movement" value?)
void MovementAction::WaitForReach(float distance)
{
    float delay = 1000.0f * MoveDelay(distance);

    if (delay > sPlayerbotAIConfig->maxWaitForMove)
        delay = sPlayerbotAIConfig->maxWaitForMove;

    Unit* target = *botAI->GetAiObjectContext()->GetValue<Unit*>("current target");
    Unit* player = *botAI->GetAiObjectContext()->GetValue<Unit*>("enemy player target");
    if ((player || target) && delay > sPlayerbotAIConfig->globalCoolDown)
        delay = sPlayerbotAIConfig->globalCoolDown;

    if (delay < 0)
        delay = 0;

    botAI->SetNextCheckDelay((uint32)delay);
}

// similiar to botAI->SetNextCheckDelay() but only stops movement
void MovementAction::SetNextMovementDelay(float delayMillis)
{
    AI_VALUE(LastMovement&, "last movement")
        .Set(bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetOrientation(),
             delayMillis,
             MovementPriority::MOVEMENT_FORCED);
}

bool MovementAction::Flee(Unit* target)
{
    Player* master = GetMaster();
    if (!target)
        target = master;

    if (!target)
        return false;

    if (!sPlayerbotAIConfig->fleeingEnabled)
        return false;

    if (!IsMovingAllowed())
    {
        botAI->TellError("I am stuck while fleeing");
        return false;
    }

    bool foundFlee = false;
    time_t lastFlee = AI_VALUE(LastMovement&, "last movement").lastFlee;
    time_t now = time(0);
    uint32 fleeDelay = urand(2, sPlayerbotAIConfig->returnDelay / 1000);

    if (lastFlee)
    {
        if ((now - lastFlee) <= fleeDelay)
        {
            return false;
        }
    }

    HostileReference* ref = target->GetThreatMgr().getCurrentVictim();
    if (ref && ref->getTarget() == bot)  // bot is target - try to flee to tank or master
    {
        if (Group* group = bot->GetGroup())
        {
            Unit* fleeTarget = nullptr;
            float fleeDistance = sPlayerbotAIConfig->sightDistance;

            for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
            {
                Player* player = gref->GetSource();
                if (!player || player == bot || !player->IsAlive())
                    continue;

                if (botAI->IsTank(player))
                {
                    float distanceToTank = sServerFacade->GetDistance2d(bot, player);
                    float distanceToTarget = sServerFacade->GetDistance2d(bot, target);
                    if (distanceToTank < fleeDistance)
                    {
                        fleeTarget = player;
                        fleeDistance = distanceToTank;
                    }
                }
            }

            if (fleeTarget)
                foundFlee = MoveNear(fleeTarget);

            if ((!fleeTarget || !foundFlee) && master)
            {
                foundFlee = MoveNear(master);
            }
        }
    }
    else  // bot is not targeted, try to flee dps/healers
    {
        bool isHealer = botAI->IsHeal(bot);
        bool isDps = !isHealer && !botAI->IsTank(bot);
        bool isTank = botAI->IsTank(bot);
        bool needHealer = !isHealer && AI_VALUE2(uint8, "health", "self target") < 50;
        bool isRanged = botAI->IsRanged(bot);

        Group* group = bot->GetGroup();
        if (group)
        {
            Unit* fleeTarget = nullptr;
            float fleeDistance = botAI->GetRange("shoot") * 1.5f;
            Unit* spareTarget = nullptr;
            float spareDistance = botAI->GetRange("shoot") * 2.0f;
            std::vector<Unit*> possibleTargets;

            for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
            {
                Player* player = gref->GetSource();
                if (!player || player == bot || !player->IsAlive())
                    continue;

                if ((isHealer && botAI->IsHeal(player)) || needHealer)
                {
                    float distanceToHealer = sServerFacade->GetDistance2d(bot, player);
                    float distanceToTarget = sServerFacade->GetDistance2d(player, target);
                    if (distanceToHealer < fleeDistance &&
                        distanceToTarget > (botAI->GetRange("shoot") / 2 + sPlayerbotAIConfig->followDistance) &&
                        (needHealer || player->IsWithinLOSInMap(target)))
                    {
                        fleeTarget = player;
                        fleeDistance = distanceToHealer;
                        possibleTargets.push_back(fleeTarget);
                    }
                }
                else if (isRanged && botAI->IsRanged(player))
                {
                    float distanceToRanged = sServerFacade->GetDistance2d(bot, player);
                    float distanceToTarget = sServerFacade->GetDistance2d(player, target);
                    if (distanceToRanged < fleeDistance &&
                        distanceToTarget > (botAI->GetRange("shoot") / 2 + sPlayerbotAIConfig->followDistance) &&
                        player->IsWithinLOSInMap(target))
                    {
                        fleeTarget = player;
                        fleeDistance = distanceToRanged;
                        possibleTargets.push_back(fleeTarget);
                    }
                }
                // remember any group member in case no one else found
                float distanceToFlee = sServerFacade->GetDistance2d(bot, player);
                float distanceToTarget = sServerFacade->GetDistance2d(player, target);
                if (distanceToFlee < spareDistance &&
                    distanceToTarget > (botAI->GetRange("shoot") / 2 + sPlayerbotAIConfig->followDistance) &&
                    player->IsWithinLOSInMap(target))
                {
                    spareTarget = player;
                    spareDistance = distanceToFlee;
                    possibleTargets.push_back(fleeTarget);
                }
            }

            if (!possibleTargets.empty())
                fleeTarget = possibleTargets[urand(0, possibleTargets.size() - 1)];

            if (!fleeTarget)
                fleeTarget = spareTarget;

            if (fleeTarget)
                foundFlee = MoveNear(fleeTarget);

            if ((!fleeTarget || !foundFlee) && master && master->IsAlive() && master->IsWithinLOSInMap(target))
            {
                float distanceToTarget = sServerFacade->GetDistance2d(master, target);
                if (distanceToTarget > (botAI->GetRange("shoot") / 2 + sPlayerbotAIConfig->followDistance))
                    foundFlee = MoveNear(master);
            }
        }
    }

    if ((foundFlee || lastFlee) && bot->GetGroup())
    {
        if (!lastFlee)
        {
            AI_VALUE(LastMovement&, "last movement").lastFlee = now;
        }
        else
        {
            if ((now - lastFlee) > fleeDelay)
            {
                AI_VALUE(LastMovement&, "last movement").lastFlee = 0;
            }
            else
                return false;
        }
    }

    FleeManager manager(bot, botAI->GetRange("flee"), bot->GetAngle(target) + M_PI);
    if (!manager.isUseful())
        return false;

    float rx, ry, rz;
    if (!manager.CalculateDestination(&rx, &ry, &rz))
    {
        botAI->TellError("Nowhere to flee");
        return false;
    }

    bool result = MoveTo(target->GetMapId(), rx, ry, rz);

    if (result)
        AI_VALUE(LastMovement&, "last movement").lastFlee = time(nullptr);

    return result;
}

void MovementAction::ClearIdleState()
{
    context->GetValue<time_t>("stay time")->Set(0);
    context->GetValue<PositionMap&>("position")->Get()["random"].Reset();
}

bool MovementAction::MoveAway(Unit* target, float distance, bool backwards)
{
    if (!target)
    {
        return false;
    }
    float init_angle = target->GetAngle(bot);
    for (float delta = 0; delta <= M_PI / 2; delta += M_PI / 8)
    {
        float angle = init_angle + delta;
        float dx = bot->GetPositionX() + cos(angle) * distance;
        float dy = bot->GetPositionY() + sin(angle) * distance;
        float dz = bot->GetPositionZ();
        bool exact = true;
        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                            bot->GetPositionZ(), dx, dy, dz))
        {
            // disable prediction if position is invalid
            dx = bot->GetPositionX() + cos(angle) * distance;
            dy = bot->GetPositionY() + sin(angle) * distance;
            dz = bot->GetPositionZ();
            exact = false;
        }
        if (MoveTo(target->GetMapId(), dx, dy, dz, false, false, true, exact, MovementPriority::MOVEMENT_COMBAT, false, backwards))
        {
            return true;
        }
        if (delta == 0)
        {
            continue;
        }
        exact = true;
        angle = init_angle - delta;
        dx = bot->GetPositionX() + cos(angle) * distance;
        dy = bot->GetPositionY() + sin(angle) * distance;
        dz = bot->GetPositionZ();
        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                            bot->GetPositionZ(), dx, dy, dz))
        {
            // disable prediction if position is invalid
            dx = bot->GetPositionX() + cos(angle) * distance;
            dy = bot->GetPositionY() + sin(angle) * distance;
            dz = bot->GetPositionZ();
            exact = false;
        }
        if (MoveTo(target->GetMapId(), dx, dy, dz, false, false, true, exact, MovementPriority::MOVEMENT_COMBAT, false, backwards))
        {
            return true;
        }
    }
    return false;
}

// just calculates average position of group and runs away from that position
bool MovementAction::MoveFromGroup(float distance)
{
    if (Group* group = bot->GetGroup())
    {
        uint32 mapId = bot->GetMapId();
        float closestDist = FLT_MAX;
        float x = 0.0f;
        float y = 0.0f;
        uint32 count = 0;

        for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* player = gref->GetSource();
            if (!player || player == bot || !player->IsAlive() || player->GetMapId() != mapId)
                continue;
            float dist = bot->GetDistance2d(player);
            if (closestDist > dist)
                closestDist = dist;
            x += player->GetPositionX();
            y += player->GetPositionY();
            count++;
        }

        if (count && closestDist < distance)
        {
            x /= count;
            y /= count;
            // x and y are now average position of the group members
            float angle = bot->GetAngle(x, y) + M_PI;
            return Move(angle, distance - closestDist);
        }
    }
    return false;
}

bool MovementAction::Move(float angle, float distance)
{
    float x = bot->GetPositionX() + cos(angle) * distance;
    float y = bot->GetPositionY() + sin(angle) * distance;

    //TODO do we need GetMapWaterOrGroundLevel() if we're using CheckCollisionAndGetValidCoords() ?
    float z = bot->GetMapWaterOrGroundLevel(x, y, bot->GetPositionZ());
    if (z == -100000.0f || z == -200000.0f)
        z = bot->GetPositionZ();
    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                        bot->GetPositionZ(), x, y, z, false))
        return false;

    return MoveTo(bot->GetMapId(), x, y, z);
}

bool MovementAction::MoveInside(uint32 mapId, float x, float y, float z, float distance, MovementPriority priority)
{
    if (bot->GetDistance2d(x, y) <= distance)
    {
        return false;
    }
    return MoveNear(mapId, x, y, z, distance, priority);
}

// float MovementAction::SearchBestGroundZForPath(float x, float y, float z, bool generatePath, float range, bool
// normal_only, float step)
// {
//     if (!generatePath) {
//         return z;
//     }
//     float min_length = 100000.0f;
//     float current_z = INVALID_HEIGHT;
//     float modified_z;
//     float delta;
//     for (delta = 0.0f; delta <= range / 2; delta += step) {
//         modified_z = bot->GetMapWaterOrGroundLevel(x, y, z + delta);
//         PathGenerator gen(bot);
//         gen.CalculatePath(x, y, modified_z);
//         if (gen.GetPathType() == PATHFIND_NORMAL && gen.getPathLength() < min_length) {
//             min_length = gen.getPathLength();
//             current_z = modified_z;
//             if (abs(current_z - z) < 0.5f) {
//                 return current_z;
//             }
//         }
//     }
//     for (delta = -step; delta >= -range / 2; delta -= step) {
//         modified_z = bot->GetMapWaterOrGroundLevel(x, y, z + delta);
//         PathGenerator gen(bot);
//         gen.CalculatePath(x, y, modified_z);
//         if (gen.GetPathType() == PATHFIND_NORMAL && gen.getPathLength() < min_length) {
//             min_length = gen.getPathLength();
//             current_z = modified_z;
//             if (abs(current_z - z) < 0.5f) {
//                 return current_z;
//             }
//         }
//     }
//     for (delta = range / 2 + step; delta <= range; delta += 2) {
//         modified_z = bot->GetMapWaterOrGroundLevel(x, y, z + delta);
//         PathGenerator gen(bot);
//         gen.CalculatePath(x, y, modified_z);
//         if (gen.GetPathType() == PATHFIND_NORMAL && gen.getPathLength() < min_length) {
//             min_length = gen.getPathLength();
//             current_z = modified_z;
//             if (abs(current_z - z) < 0.5f) {
//                 return current_z;
//             }
//         }
//     }
//     if (current_z == INVALID_HEIGHT && normal_only) {
//         return INVALID_HEIGHT;
//     }
//     if (current_z == INVALID_HEIGHT && !normal_only) {
//         return z;
//     }
//     return current_z;
// }

const Movement::PointsArray MovementAction::SearchForBestPath(float x, float y, float z, float& modified_z,
                                                              int maxSearchCount, bool normal_only, float step)
{
    bool found = false;
    modified_z = INVALID_HEIGHT;
    float tempZ = bot->GetMapHeight(x, y, z);
    PathGenerator gen(bot);
    gen.CalculatePath(x, y, tempZ);
    Movement::PointsArray result = gen.GetPath();
    float min_length = gen.getPathLength();
    int typeOk = PATHFIND_NORMAL | PATHFIND_INCOMPLETE;
    if ((gen.GetPathType() & typeOk) && abs(tempZ - z) < 0.5f)
    {
        modified_z = tempZ;
        return result;
    }
    // Start searching
    if (gen.GetPathType() & typeOk)
    {
        modified_z = tempZ;
        found = true;
    }
    int count = 1;
    for (float delta = step; count < maxSearchCount / 2 + 1; count++, delta += step)
    {
        tempZ = bot->GetMapHeight(x, y, z + delta);
        if (tempZ == INVALID_HEIGHT)
        {
            continue;
        }
        PathGenerator gen(bot);
        gen.CalculatePath(x, y, tempZ);
        if ((gen.GetPathType() & typeOk) && gen.getPathLength() < min_length)
        {
            found = true;
            min_length = gen.getPathLength();
            result = gen.GetPath();
            modified_z = tempZ;
        }
    }
    for (float delta = -step; count < maxSearchCount; count++, delta -= step)
    {
        tempZ = bot->GetMapHeight(x, y, z + delta);
        if (tempZ == INVALID_HEIGHT)
        {
            continue;
        }
        PathGenerator gen(bot);
        gen.CalculatePath(x, y, tempZ);
        if ((gen.GetPathType() & typeOk) && gen.getPathLength() < min_length)
        {
            found = true;
            min_length = gen.getPathLength();
            result = gen.GetPath();
            modified_z = tempZ;
        }
    }
    if (!found && normal_only)
    {
        modified_z = INVALID_HEIGHT;
        return Movement::PointsArray{};
    }
    if (!found && !normal_only)
    {
        return result;
    }
    return result;
}

bool FleeAction::Execute(Event event)
{
    return MoveAway(AI_VALUE(Unit*, "current target"), sPlayerbotAIConfig->fleeDistance, true);
}

bool FleeAction::isUseful()
{
    if (bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr)
    {
        return false;
    }
    Unit* target = AI_VALUE(Unit*, "current target");
    if (target && target->IsInWorld() && !bot->IsWithinMeleeRange(target))
        return false;

    return true;
}

bool FleeWithPetAction::Execute(Event event)
{
    if (Pet* pet = bot->GetPet())
    {
        botAI->PetFollow();
    }

    return Flee(AI_VALUE(Unit*, "current target"));
}

bool AvoidAoeAction::isUseful()
{
    if (getMSTime() - moveInterval < lastMoveTimer)
    {
        return false;
    }
    GuidVector traps = AI_VALUE(GuidVector, "nearest trap with damage");
    GuidVector triggers = AI_VALUE(GuidVector, "possible triggers");
    return AI_VALUE(Aura*, "area debuff") || !traps.empty() || !triggers.empty();
}

bool AvoidAoeAction::Execute(Event event)
{
    // Case #1: Aura with dynamic object (e.g. rain of fire)
    if (AvoidAuraWithDynamicObj())
    {
        return true;
    }
    // Case #2: Trap game object with spell (e.g. lava bomb)
    if (AvoidGameObjectWithDamage())
    {
        return true;
    }
    // Case #3: Trigger npc (e.g. Lesser shadow fissure)
    if (AvoidUnitWithDamageAura())
    {
        return true;
    }
    return false;
}

bool AvoidAoeAction::AvoidAuraWithDynamicObj()
{
    Aura* aura = AI_VALUE(Aura*, "area debuff");
    if (!aura || aura->IsRemoved() || aura->IsExpired())
    {
        return false;
    }
    if (!aura->GetOwner() || !aura->GetOwner()->IsInWorld())
    {
        return false;
    }
    // Crash fix: maybe change owner due to check interval
    if (aura->GetType() != DYNOBJ_AURA_TYPE)
    {
        return false;
    }
    const SpellInfo* spellInfo = aura->GetSpellInfo();
    if (!spellInfo)
    {
        return false;
    }
    if (sPlayerbotAIConfig->aoeAvoidSpellWhitelist.find(spellInfo->Id) != sPlayerbotAIConfig->aoeAvoidSpellWhitelist.end())
        return false;

    DynamicObject* dynOwner = aura->GetDynobjOwner();
    if (!dynOwner || !dynOwner->IsInWorld())
    {
        return false;
    }
    float radius = dynOwner->GetRadius();
    if (!radius || radius > sPlayerbotAIConfig->maxAoeAvoidRadius)
        return false;
    if (bot->GetDistance(dynOwner) > radius)
    {
        return false;
    }
    std::ostringstream name;
    name << spellInfo->SpellName[LOCALE_enUS];  // << "] (aura)";
    if (FleePosition(dynOwner->GetPosition(), radius))
    {
        if (sPlayerbotAIConfig->tellWhenAvoidAoe && lastTellTimer < time(NULL) - 10)
        {
            lastTellTimer = time(NULL);
            lastMoveTimer = getMSTime();
            std::ostringstream out;
            out << "I'm avoiding " << name.str() << " (" << spellInfo->Id << ")" << " Radius " << radius << " - [Aura]";
            bot->Say(out.str(), LANG_UNIVERSAL);
        }
        return true;
    }
    return false;
}

bool AvoidAoeAction::AvoidGameObjectWithDamage()
{
    GuidVector traps = AI_VALUE(GuidVector, "nearest trap with damage");
    if (traps.empty())
    {
        return false;
    }
    for (ObjectGuid& guid : traps)
    {
        GameObject* go = botAI->GetGameObject(guid);
        if (!go || !go->IsInWorld())
        {
            continue;
        }
        if (go->GetGoType() != GAMEOBJECT_TYPE_TRAP)
        {
            continue;
        }
        const GameObjectTemplate* goInfo = go->GetGOInfo();
        if (!goInfo)
        {
            continue;
        }
        // 0 trap with no despawn after cast. 1 trap despawns after cast. 2 bomb casts on spawn.
        if (goInfo->trap.type != 0)
            continue;

        uint32 spellId = goInfo->trap.spellId;
        if (!spellId)
        {
            continue;
        }

        if (sPlayerbotAIConfig->aoeAvoidSpellWhitelist.find(spellId) != sPlayerbotAIConfig->aoeAvoidSpellWhitelist.end())
            continue;

        const SpellInfo* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo || spellInfo->IsPositive())
        {
            continue;
        }

        float radius = (float)goInfo->trap.diameter / 2 + go->GetCombatReach();
        if (!radius || radius > sPlayerbotAIConfig->maxAoeAvoidRadius)
            continue;

        if (bot->GetDistance(go) > radius)
        {
            continue;
        }
        std::ostringstream name;
        name << spellInfo->SpellName[LOCALE_enUS];  // << "] (object)";
        if (FleePosition(go->GetPosition(), radius))
        {
            if (sPlayerbotAIConfig->tellWhenAvoidAoe && lastTellTimer < time(NULL) - 10)
            {
                lastTellTimer = time(NULL);
                lastMoveTimer = getMSTime();
                std::ostringstream out;
                out << "I'm avoiding " << name.str() << " (" << spellInfo->Id << ")" << " Radius " << radius << " - [Trap]";
                bot->Say(out.str(), LANG_UNIVERSAL);
            }
            return true;
        }
    }
    return false;
}

bool AvoidAoeAction::AvoidUnitWithDamageAura()
{
    GuidVector triggers = AI_VALUE(GuidVector, "possible triggers");
    if (triggers.empty())
    {
        return false;
    }
    for (ObjectGuid& guid : triggers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsInWorld())
        {
            continue;
        }
        if (!unit->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        {
            return false;
        }
        Unit::AuraEffectList const& aurasPeriodicTriggerSpell =
            unit->GetAuraEffectsByType(SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        Unit::AuraEffectList const& aurasPeriodicTriggerWithValueSpell =
            unit->GetAuraEffectsByType(SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE);
        for (const Unit::AuraEffectList& list : {aurasPeriodicTriggerSpell, aurasPeriodicTriggerWithValueSpell})
        {
            for (auto i = list.begin(); i != list.end(); ++i)
            {
                AuraEffect* aurEff = *i;
                const SpellInfo* spellInfo = aurEff->GetSpellInfo();
                if (!spellInfo)
                    continue;
                const SpellInfo* triggerSpellInfo =
                    sSpellMgr->GetSpellInfo(spellInfo->Effects[aurEff->GetEffIndex()].TriggerSpell);
                if (!triggerSpellInfo)
                    continue;
                if (sPlayerbotAIConfig->aoeAvoidSpellWhitelist.find(triggerSpellInfo->Id) != sPlayerbotAIConfig->aoeAvoidSpellWhitelist.end())
                    return false;
                for (int j = 0; j < MAX_SPELL_EFFECTS; j++)
                {
                    if (triggerSpellInfo->Effects[j].Effect == SPELL_EFFECT_SCHOOL_DAMAGE)
                    {
                        float radius = triggerSpellInfo->Effects[j].CalcRadius();
                        if (bot->GetDistance(unit) > radius)
                        {
                            break;
                        }
                        if (!radius || radius > sPlayerbotAIConfig->maxAoeAvoidRadius)
                            continue;
                        std::ostringstream name;
                        name << triggerSpellInfo->SpellName[LOCALE_enUS];  //<< "] (unit)";
                        if (FleePosition(unit->GetPosition(), radius))
                        {
                            if (sPlayerbotAIConfig->tellWhenAvoidAoe && lastTellTimer < time(NULL) - 10)
                            {
                                lastTellTimer = time(NULL);
                                lastMoveTimer = getMSTime();
                                std::ostringstream out;
                                out << "I'm avoiding " << name.str() << " (" << triggerSpellInfo->Id << ")" << " Radius " << radius << " - [Unit Trigger]";
                                bot->Say(out.str(), LANG_UNIVERSAL);
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

Position MovementAction::BestPositionForMeleeToFlee(Position pos, float radius)
{
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    std::vector<CheckAngle> possibleAngles;
    if (currentTarget)
    {
        // Normally, move to left or right is the best position
        bool isTanking = (!currentTarget->isFrozen() && !currentTarget->HasRootAura()) && (currentTarget->GetVictim() == bot);
        float angle = bot->GetAngle(currentTarget);
        float angleLeft = angle + (float)M_PI / 2;
        float angleRight = angle - (float)M_PI / 2;
        possibleAngles.push_back({angleLeft, false});
        possibleAngles.push_back({angleRight, false});
        possibleAngles.push_back({angle, true});
        if (isTanking)
        {
            possibleAngles.push_back({angle + (float)M_PI, false});
            possibleAngles.push_back({bot->GetAngle(&pos) - (float)M_PI, false});
        }
    }
    else
    {
        float angleTo = bot->GetAngle(&pos) - (float)M_PI;
        possibleAngles.push_back({angleTo, false});
    }
    float farestDis = 0.0f;
    Position bestPos;
    for (CheckAngle& checkAngle : possibleAngles)
    {
        float angle = checkAngle.angle;
        std::list<FleeInfo>& infoList = AI_VALUE(std::list<FleeInfo>&, "recently flee info");
        if (!CheckLastFlee(angle, infoList))
        {
            continue;
        }
        bool strict = checkAngle.strict;
        float fleeDis = std::min(radius + 1.0f, sPlayerbotAIConfig->fleeDistance);
        float dx = bot->GetPositionX() + cos(angle) * fleeDis;
        float dy = bot->GetPositionY() + sin(angle) * fleeDis;
        float dz = bot->GetPositionZ();
        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                            bot->GetPositionZ(), dx, dy, dz))
        {
            continue;
        }
        Position fleePos{dx, dy, dz};
        if (strict && currentTarget &&
            fleePos.GetExactDist(currentTarget) - currentTarget->GetCombatReach() >
                sPlayerbotAIConfig->tooCloseDistance &&
            bot->IsWithinMeleeRange(currentTarget))
        {
            continue;
        }
        if (pos.GetExactDist(fleePos) > farestDis)
        {
            farestDis = pos.GetExactDist(fleePos);
            bestPos = fleePos;
        }
    }
    if (farestDis > 0.0f)
    {
        return bestPos;
    }
    return Position();
}

Position MovementAction::BestPositionForRangedToFlee(Position pos, float radius)
{
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    std::vector<CheckAngle> possibleAngles;
    float angleToTarget = 0.0f;
    float angleFleeFromCenter = bot->GetAngle(&pos) - (float)M_PI;
    if (currentTarget)
    {
        // Normally, move to left or right is the best position
        angleToTarget = bot->GetAngle(currentTarget);
        float angleLeft = angleToTarget + (float)M_PI / 2;
        float angleRight = angleToTarget - (float)M_PI / 2;
        possibleAngles.push_back({angleLeft, false});
        possibleAngles.push_back({angleRight, false});
        possibleAngles.push_back({angleToTarget + (float)M_PI, true});
        possibleAngles.push_back({angleToTarget, true});
        possibleAngles.push_back({angleFleeFromCenter, true});
    }
    else
    {
        possibleAngles.push_back({angleFleeFromCenter, false});
    }
    float farestDis = 0.0f;
    Position bestPos;
    for (CheckAngle& checkAngle : possibleAngles)
    {
        float angle = checkAngle.angle;
        std::list<FleeInfo>& infoList = AI_VALUE(std::list<FleeInfo>&, "recently flee info");
        if (!CheckLastFlee(angle, infoList))
        {
            continue;
        }
        bool strict = checkAngle.strict;
        float fleeDis = std::min(radius + 1.0f, sPlayerbotAIConfig->fleeDistance);
        float dx = bot->GetPositionX() + cos(angle) * fleeDis;
        float dy = bot->GetPositionY() + sin(angle) * fleeDis;
        float dz = bot->GetPositionZ();
        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                            bot->GetPositionZ(), dx, dy, dz))
        {
            continue;
        }
        Position fleePos{dx, dy, dz};
        if (strict && currentTarget &&
            fleePos.GetExactDist(currentTarget) - currentTarget->GetCombatReach() > sPlayerbotAIConfig->spellDistance)
        {
            continue;
        }
        if (strict && currentTarget &&
            fleePos.GetExactDist(currentTarget) - currentTarget->GetCombatReach() <
                (sPlayerbotAIConfig->tooCloseDistance))
        {
            continue;
        }

        if (pos.GetExactDist(fleePos) > farestDis)
        {
            farestDis = pos.GetExactDist(fleePos);
            bestPos = fleePos;
        }
    }
    if (farestDis > 0.0f)
    {
        return bestPos;
    }
    return Position();
}

bool MovementAction::FleePosition(Position pos, float radius, uint32 minInterval)
{
    std::list<FleeInfo>& infoList = AI_VALUE(std::list<FleeInfo>&, "recently flee info");

    if (!infoList.empty() && infoList.back().timestamp + minInterval > getMSTime())
        return false;

    Position bestPos;
    if (botAI->IsMelee(bot))
    {
        bestPos = BestPositionForMeleeToFlee(pos, radius);
    }
    else
    {
        bestPos = BestPositionForRangedToFlee(pos, radius);
    }
    if (bestPos != Position())
    {
        if (MoveTo(bot->GetMapId(), bestPos.GetPositionX(), bestPos.GetPositionY(), bestPos.GetPositionZ(), false,
                   false, true, false, MovementPriority::MOVEMENT_COMBAT))
        {
            uint32 curTS = getMSTime();
            while (!infoList.empty())
            {
                if (infoList.size() > 10 || infoList.front().timestamp + 5000 < curTS)
                {
                    infoList.pop_front();
                }
                else
                {
                    break;
                }
            }
            infoList.push_back({pos, radius, bot->GetAngle(&bestPos), curTS});
            return true;
        }
    }
    return false;
}

bool MovementAction::CheckLastFlee(float curAngle, std::list<FleeInfo>& infoList)
{
    uint32 curTS = getMSTime();
    curAngle = Position::NormalizeOrientation(curAngle);
    while (!infoList.empty())
    {
        if (infoList.size() > 10 || infoList.front().timestamp + 5000 < curTS)
        {
            infoList.pop_front();
        }
        else
        {
            break;
        }
    }
    for (FleeInfo& info : infoList)
    {
        // more than 5 sec
        if (info.timestamp + 5000 < curTS)
        {
            continue;
        }
        float revAngle = Position::NormalizeOrientation(info.angle + M_PI);
        // angle too close
        if (fabs(revAngle - curAngle) < M_PI / 4)
        {
            return false;
        }
    }
    return true;
}

bool CombatFormationMoveAction::isUseful()
{
    if (getMSTime() - moveInterval < lastMoveTimer)
    {
        return false;
    }
    if (bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr)
    {
        return false;
    }
    return true;
}

bool CombatFormationMoveAction::Execute(Event event)
{
    float dis = AI_VALUE(float, "disperse distance");
    if (dis <= 0.0f ||
        (!bot->IsInCombat() && botAI->HasStrategy("stay", BotState::BOT_STATE_NON_COMBAT)) ||
        (bot->IsInCombat() && botAI->HasStrategy("stay", BotState::BOT_STATE_COMBAT)))
        return false;
    Player* playerToLeave = NearestGroupMember(dis);
    if (playerToLeave && bot->GetExactDist(playerToLeave) < dis)
    {
        if (FleePosition(playerToLeave->GetPosition(), dis))
        {
            lastMoveTimer = getMSTime();
        }
    }
    return false;
}

Position CombatFormationMoveAction::AverageGroupPos(float dis, bool ranged, bool self)
{
    float averageX = 0, averageY = 0, averageZ = 0;
    int cnt = 0;
    Group* group = bot->GetGroup();
    if (!group)
    {
        return Position();
    }
    Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
    for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
    {
        Player* member = ObjectAccessor::FindPlayer(itr->guid);
        if (!member)
            continue;

        if (!self && member == bot)
            continue;

        if (ranged && !PlayerbotAI::IsRanged(member))
            continue;

        if (!member->IsAlive() || member->GetMapId() != bot->GetMapId() || member->IsCharmed() ||
            sServerFacade->GetDistance2d(bot, member) > dis)
            continue;

        averageX += member->GetPositionX();
        averageY += member->GetPositionY();
        averageZ += member->GetPositionZ();
    }
    averageX /= cnt;
    averageY /= cnt;
    averageZ /= cnt;
    return Position(averageX, averageY, averageZ);
}

float CombatFormationMoveAction::AverageGroupAngle(Unit* from, bool ranged, bool self)
{
    Group* group = bot->GetGroup();
    if (!from || !group)
    {
        return 0.0f;
    }
    // float average = 0.0f;
    float sumX = 0.0f;
    float sumY = 0.0f;
    int cnt = 0;
    Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
    for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
    {
        Player* member = ObjectAccessor::FindPlayer(itr->guid);
        if (!member)
            continue;

        if (!self && member == bot)
            continue;

        if (ranged && !PlayerbotAI::IsRanged(member))
            continue;

        if (!member->IsAlive() || member->GetMapId() != bot->GetMapId() || member->IsCharmed() ||
            sServerFacade->GetDistance2d(bot, member) > sPlayerbotAIConfig->sightDistance)
            continue;

        cnt++;
        sumX += member->GetPositionX() - from->GetPositionX();
        sumY += member->GetPositionY() - from->GetPositionY();
    }
    if (cnt == 0)
        return 0.0f;

    // unnecessary division
    // sumX /= cnt;
    // sumY /= cnt;

    return atan2(sumY, sumX);
}

Position CombatFormationMoveAction::GetNearestPosition(const std::vector<Position>& positions)
{
    Position result;
    for (const Position& pos : positions)
    {
        if (bot->GetExactDist(pos) < bot->GetExactDist(result))
            result = pos;
    }
    return result;
}

Player* CombatFormationMoveAction::NearestGroupMember(float dis)
{
    float nearestDis = 10000.0f;
    Player* result = nullptr;
    Group* group = bot->GetGroup();
    if (!group)
    {
        return result;
    }
    Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
    for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
    {
        Player* member = ObjectAccessor::FindPlayer(itr->guid);
        if (!member || !member->IsAlive() || member == bot || member->GetMapId() != bot->GetMapId() ||
            member->IsCharmed() || sServerFacade->GetDistance2d(bot, member) > dis)
            continue;
        if (nearestDis > bot->GetExactDist(member))
        {
            result = member;
            nearestDis = bot->GetExactDist(member);
        }
    }
    return result;
}

bool TankFaceAction::Execute(Event event)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    if (!bot->GetGroup())
        return false;

    if (!bot->IsWithinMeleeRange(target) || target->isMoving())
        return false;

    if (!AI_VALUE2(bool, "has aggro", "current target"))
        return false;

    float averageAngle = AverageGroupAngle(target, true);

    if (averageAngle == 0.0f)
        return false;

    float deltaAngle = Position::NormalizeOrientation(averageAngle - target->GetAngle(bot));
    if (deltaAngle > M_PI)
        deltaAngle -= 2.0f * M_PI; // -PI..PI

    float tolerable = M_PI_2;

    if (fabs(deltaAngle) > tolerable)
        return false;

    float goodAngle1 = Position::NormalizeOrientation(averageAngle + M_PI * 3 / 5);
    float goodAngle2 = Position::NormalizeOrientation(averageAngle - M_PI * 3 / 5);

    // if dist < bot->GetMeleeRange(target) / 2, target will move backward
    float dist = std::max(bot->GetExactDist(target), bot->GetMeleeRange(target) / 2) - bot->GetCombatReach() - target->GetCombatReach();
    std::vector<Position> availablePos;
    float x, y, z;
    target->GetNearPoint(bot, x, y, z, 0.0f, dist, goodAngle1);
    if (bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
        x, y, z))
    {
        /// @todo: movement control now is a mess, prepare to rewrite
        std::list<FleeInfo>& infoList = AI_VALUE(std::list<FleeInfo>&, "recently flee info");
        Position pos(x, y, z);
        float angle = bot->GetAngle(&pos);
        if (CheckLastFlee(angle, infoList))
        {
            availablePos.push_back(Position(x, y, z));
        }
    }
    target->GetNearPoint(bot, x, y, z, 0.0f, dist, goodAngle2);
    if (bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
        x, y, z))
    {
        std::list<FleeInfo>& infoList = AI_VALUE(std::list<FleeInfo>&, "recently flee info");
        Position pos(x, y, z);
        float angle = bot->GetAngle(&pos);
        if (CheckLastFlee(angle, infoList))
        {
            availablePos.push_back(Position(x, y, z));
        }
    }
    if (availablePos.empty())
        return false;
    Position nearest = GetNearestPosition(availablePos);
    return MoveTo(bot->GetMapId(), nearest.GetPositionX(), nearest.GetPositionY(), nearest.GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
}

bool RearFlankAction::isUseful()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target) { return false; }

    // Need to double the front angle check to account for mirrored angle.
    bool inFront = target->HasInArc(2.f * minAngle, bot);
    // Rear check does not need to double this angle as the logic is inverted
    // and we are subtracting from 2pi.
    bool inRear = !target->HasInArc((2.f * M_PI) - maxAngle, bot);

    return inFront || inRear;
}

bool RearFlankAction::Execute(Event event)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target) { return false; }

    float angle = frand(minAngle, maxAngle);
    float baseDistance = bot->GetMeleeRange(target) * 0.5f;
    Position leftFlank = target->GetPosition();
    Position rightFlank = target->GetPosition();
    Position* destination = nullptr;
    leftFlank.RelocatePolarOffset(angle, baseDistance + distance);
    rightFlank.RelocatePolarOffset(-angle, baseDistance + distance);

    if (bot->GetExactDist2d(leftFlank) < bot->GetExactDist2d(rightFlank))
    {
        destination = &leftFlank;
    }
    else
    {
        destination = &rightFlank;
    }

    return MoveTo(bot->GetMapId(), destination->GetPositionX(), destination->GetPositionY(), destination->GetPositionZ(),
                  false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
}

bool DisperseSetAction::Execute(Event event)
{
    std::string const text = event.getParam();
    if (text == "disable")
    {
        RESET_AI_VALUE(float, "disperse distance");
        botAI->TellMasterNoFacing("Disable disperse");
        return true;
    }
    if (text == "enable" || text == "reset")
    {
        if (botAI->IsMelee(bot))
        {
            SET_AI_VALUE(float, "disperse distance", DEFAULT_DISPERSE_DISTANCE_MELEE);
        }
        else
        {
            SET_AI_VALUE(float, "disperse distance", DEFAULT_DISPERSE_DISTANCE_RANGED);
        }
        float dis = AI_VALUE(float, "disperse distance");
        std::ostringstream out;
        out << "Enable disperse distance " << std::setprecision(2) << dis;
        botAI->TellMasterNoFacing(out.str());
        return true;
    }
    if (text == "increase")
    {
        float dis = AI_VALUE(float, "disperse distance");
        std::ostringstream out;
        if (dis <= 0.0f)
        {
            out << "Enable disperse first";
            botAI->TellMasterNoFacing(out.str());
            return true;
        }
        dis += 1.0f;
        SET_AI_VALUE(float, "disperse distance", dis);
        out << "Increase disperse distance to " << std::setprecision(2) << dis;
        botAI->TellMasterNoFacing(out.str());
        return true;
    }
    if (text == "decrease")
    {
        float dis = AI_VALUE(float, "disperse distance");
        dis -= 1.0f;
        if (dis <= 0.0f)
        {
            dis += 1.0f;
        }
        SET_AI_VALUE(float, "disperse distance", dis);
        std::ostringstream out;
        out << "Increase disperse distance to " << std::setprecision(2) << dis;
        botAI->TellMasterNoFacing(out.str());
        return true;
    }
    if (text.starts_with("set"))
    {
        float dis = -1.0f;
        ;
        sscanf(text.c_str(), "set %f", &dis);
        std::ostringstream out;
        if (dis < 0 || dis > 100.0f)
        {
            out << "Invalid disperse distance " << std::setprecision(2) << dis;
        }
        else
        {
            SET_AI_VALUE(float, "disperse distance", dis);
            out << "Set disperse distance to " << std::setprecision(2) << dis;
        }
        botAI->TellMasterNoFacing(out.str());
        return true;
    }
    std::ostringstream out;
    out << "Usage: disperse [enable | disable | increase | decrease | set {distance}]";
    float dis = AI_VALUE(float, "disperse distance");
    if (dis > 0.0f)
    {
        out << "(Current disperse distance: " << std::setprecision(2) << dis << ")";
    }
    botAI->TellMasterNoFacing(out.str());
    return true;
}

bool RunAwayAction::Execute(Event event) { return Flee(AI_VALUE(Unit*, "master target")); }

bool MoveToLootAction::Execute(Event event)
{
    LootObject loot = AI_VALUE(LootObject, "loot target");
    if (!loot.IsLootPossible(bot))
        return false;

    return MoveNear(loot.GetWorldObject(bot), sPlayerbotAIConfig->contactDistance);
}

bool MoveOutOfEnemyContactAction::Execute(Event event)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    return MoveTo(target, sPlayerbotAIConfig->contactDistance);
}

bool MoveOutOfEnemyContactAction::isUseful() { return AI_VALUE2(bool, "inside target", "current target"); }

bool SetFacingTargetAction::Execute(Event event)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    if (bot->HasUnitState(UNIT_STATE_IN_FLIGHT))
        return true;

    sServerFacade->SetFacingTo(bot, target);
    botAI->SetNextCheckDelay(sPlayerbotAIConfig->reactDelay);
    return true;
}

bool SetFacingTargetAction::isUseful() { return !AI_VALUE2(bool, "facing", "current target"); }

bool SetFacingTargetAction::isPossible()
{
    if (bot->isFrozen() || bot->IsPolymorphed() || (bot->isDead() && !bot->HasPlayerFlag(PLAYER_FLAGS_GHOST)) ||
        bot->IsBeingTeleported() || bot->HasConfuseAura() || bot->IsCharmed() ||
        bot->HasStunAura() || bot->IsInFlight() ||
        bot->HasUnitState(UNIT_STATE_LOST_CONTROL))
        return false;

    return true;
}

bool SetBehindTargetAction::Execute(Event event)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    if (target->GetVictim() == bot)
        return false;

    if (!bot->IsWithinMeleeRange(target) || target->isMoving())
        return false;

    float deltaAngle = Position::NormalizeOrientation(target->GetOrientation() - target->GetAngle(bot));
    if (deltaAngle > M_PI)
        deltaAngle -= 2.0f * M_PI; // -PI..PI

    float tolerable = M_PI_2;

    if (fabs(deltaAngle) > tolerable)
        return false;

    float goodAngle1 = Position::NormalizeOrientation(target->GetOrientation() + M_PI * 3 / 5);
    float goodAngle2 = Position::NormalizeOrientation(target->GetOrientation() - M_PI * 3 / 5);

    float dist = std::max(bot->GetExactDist(target), bot->GetMeleeRange(target) / 2) - bot->GetCombatReach() - target->GetCombatReach();
    std::vector<Position> availablePos;
    float x, y, z;
    target->GetNearPoint(bot, x, y, z, 0.0f, dist, goodAngle1);
    if (bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
        x, y, z))
    {
        /// @todo: movement control now is a mess, prepare to rewrite
        std::list<FleeInfo>& infoList = AI_VALUE(std::list<FleeInfo>&, "recently flee info");
        Position pos(x, y, z);
        float angle = bot->GetAngle(&pos);
        if (CheckLastFlee(angle, infoList))
        {
            availablePos.push_back(Position(x, y, z));
        }
    }
    target->GetNearPoint(bot, x, y, z, 0.0f, dist, goodAngle2);
    if (bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
        x, y, z))
    {
        std::list<FleeInfo>& infoList = AI_VALUE(std::list<FleeInfo>&, "recently flee info");
        Position pos(x, y, z);
        float angle = bot->GetAngle(&pos);
        if (CheckLastFlee(angle, infoList))
        {
            availablePos.push_back(Position(x, y, z));
        }
    }
    if (availablePos.empty())
        return false;
    Position nearest = GetNearestPosition(availablePos);
    return MoveTo(bot->GetMapId(), nearest.GetPositionX(), nearest.GetPositionY(), nearest.GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
}

bool MoveOutOfCollisionAction::Execute(Event event)
{
    float angle = M_PI * 2000 / frand(1.f, 1000.f);
    float distance = sPlayerbotAIConfig->followDistance;
    return MoveTo(bot->GetMapId(), bot->GetPositionX() + cos(angle) * distance,
                  bot->GetPositionY() + sin(angle) * distance, bot->GetPositionZ());
}

bool MoveOutOfCollisionAction::isUseful()
{
    // do not avoid collision on vehicle
    if (botAI->IsInVehicle())
        return false;

    return AI_VALUE2(bool, "collision", "self target") &&
           botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest friendly players")->Get().size() < 15;
}

bool MoveRandomAction::Execute(Event event)
{
    float distance = sPlayerbotAIConfig->tooCloseDistance + urand(10, 30);

    Map* map = bot->GetMap();
    for (int i = 0; i < 3; ++i)
    {
        float x = bot->GetPositionX();
        float y = bot->GetPositionY();
        float z = bot->GetPositionZ();
        float angle = (float)rand_norm() * static_cast<float>(M_PI);
        x += urand(0, distance) * cos(angle);
        y += urand(0, distance) * sin(angle);
        float ox = x;
        float oy = y;
        float oz = z;
        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                            bot->GetPositionZ(), x, y, z))
            continue;
        if (map->IsInWater(bot->GetPhaseMask(), x, y, z, bot->GetCollisionHeight()))
            continue;

        bool moved = MoveTo(bot->GetMapId(), x, y, z, false, false, false, true);
        if (moved)
            return true;
    }

    return false;
}

bool MoveRandomAction::isUseful() { return !AI_VALUE(GuidPosition, "rpg target"); }

bool MoveInsideAction::Execute(Event event) { return MoveInside(bot->GetMapId(), x, y, bot->GetPositionZ(), distance); }

bool RotateAroundTheCenterPointAction::Execute(Event event)
{
    uint32 next_point = GetCurrWaypoint();
    if (MoveTo(bot->GetMapId(), waypoints[next_point].first, waypoints[next_point].second, bot->GetPositionZ(), false,
               false, false, false, MovementPriority::MOVEMENT_COMBAT))
    {
        call_counters += 1;
        return true;
    }
    return false;
}

bool MoveFromGroupAction::Execute(Event event)
{
    float distance = atoi(event.getParam().c_str());
    if (!distance)
        distance = 20.0f; // flee distance from config is too small for this
    return MoveFromGroup(distance);
}

bool MoveAwayFromCreatureAction::Execute(Event event)
{
    GuidVector targets = AI_VALUE(GuidVector, "nearest npcs");
    Creature* nearestCreature = bot->FindNearestCreature(creatureId, range, alive);

    // Find all creatures with the specified Id
    std::vector<Unit*> creatures;
    for (const auto& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && (alive && unit->IsAlive()) && unit->GetEntry() == creatureId)
        {
            creatures.push_back(unit);
        }
    }

    if (creatures.empty())
    {
        return false;
    }

    // Search for a safe position
    const int directions = 8;
    const float increment = 3.0f;
    float bestX = bot->GetPositionX();
    float bestY = bot->GetPositionY();
    float bestZ = bot->GetPositionZ();
    float maxSafetyScore = -1.0f;

    for (int i = 0; i < directions; ++i)
    {
        float angle = (i * 2 * M_PI) / directions;
        for (float distance = increment; distance <= 30.0f; distance += increment)
        {
            float moveX = bot->GetPositionX() + distance * cos(angle);
            float moveY = bot->GetPositionY() + distance * sin(angle);
            float moveZ = bot->GetPositionZ();

            // Check creature distance constraints
            bool isSafeFromCreatures = true;
            float minCreatureDist = std::numeric_limits<float>::max();
            for (Unit* creature : creatures)
            {
                float dist = creature->GetExactDist2d(moveX, moveY);
                if (dist < range)
                {
                    isSafeFromCreatures = false;
                    break;
                }
                if (dist < minCreatureDist)
                {
                    minCreatureDist = dist;
                }
            }

            if (isSafeFromCreatures && bot->IsWithinLOS(moveX, moveY, moveZ))
            {
                // A simple safety score: the minimum distance to any creature. Higher is better.
                if (minCreatureDist > maxSafetyScore)
                {
                    maxSafetyScore = minCreatureDist;
                    bestX = moveX;
                    bestY = moveY;
                    bestZ = moveZ;
                }
            }
        }
    }

    // Move to the best position found
    if (maxSafetyScore > 0.0f)
    {
        return MoveTo(bot->GetMapId(), bestX, bestY, bestZ, false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

bool MoveAwayFromCreatureAction::isPossible()
{
    return bot->CanFreeMove();
}

bool MoveAwayFromPlayerWithDebuffAction::Execute(Event event)
{
    Player* closestPlayer = nullptr;
    float minDistance = 0.0f;

    Group* group = bot->GetGroup();
    if (!group)
    {
        return false;
    }

    std::vector<Player*> debuffedPlayers;

    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* player = gref->GetSource();
        if (player && player->IsAlive() && player->HasAura(spellId))
        {
            debuffedPlayers.push_back(player);
        }
    }

    if (debuffedPlayers.empty())
    {
        return false;
    }

    // Search for a safe position
    const int directions = 8;
    const float increment = 3.0f;
    float bestX = bot->GetPositionX();
    float bestY = bot->GetPositionY();
    float bestZ = bot->GetPositionZ();
    float maxSafetyScore = -1.0f;

    for (int i = 0; i < directions; ++i)
    {
        float angle = (i * 2 * M_PI) / directions;
        for (float distance = increment; distance <= (range + 5.0f); distance += increment)
        {
            float moveX = bot->GetPositionX() + distance * cos(angle);
            float moveY = bot->GetPositionY() + distance * sin(angle);
            float moveZ = bot->GetPositionZ();

            // Check creature distance constraints
            bool isSafeFromDebuffedPlayer = true;
            float minDebuffedPlayerDistance = std::numeric_limits<float>::max();
            for (Unit* debuffedPlayer : debuffedPlayers)
            {
                float dist = debuffedPlayer->GetExactDist2d(moveX, moveY);
                if (dist < range)
                {
                    isSafeFromDebuffedPlayer = false;
                    break;
                }
                if (dist < minDebuffedPlayerDistance)
                {
                    minDebuffedPlayerDistance = dist;
                }
            }

            if (isSafeFromDebuffedPlayer && bot->IsWithinLOS(moveX, moveY, moveZ))
            {
                // A simple safety score: the minimum distance to any debuffed player. Higher is better.
                if (minDebuffedPlayerDistance > maxSafetyScore)
                {
                    maxSafetyScore = minDebuffedPlayerDistance;
                    bestX = moveX;
                    bestY = moveY;
                    bestZ = moveZ;
                }
            }
        }
    }

    // Move to the best position found
    if (maxSafetyScore > 0.0f)
    {
        return MoveTo(bot->GetMapId(), bestX, bestY, bestZ, false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true);
    }

    return false;
}

bool MoveAwayFromPlayerWithDebuffAction::isPossible()
{
    return bot->CanFreeMove();
}
