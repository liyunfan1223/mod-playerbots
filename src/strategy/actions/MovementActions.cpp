/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "MovementActions.h"
#include "MotionMaster.h"
#include "MovementGenerator.h"
#include "ObjectDefines.h"
#include "ObjectGuid.h"
#include "PathGenerator.h"
#include "PlayerbotAIConfig.h"
#include "Random.h"
#include "SharedDefines.h"
#include "TargetedMovementGenerator.h"
#include "Event.h"
#include "LastMovementValue.h"
#include "PositionValue.h"
#include "Stances.h"
#include "FleeManager.h"
#include "LootObjectStack.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "Transport.h"
#include "Unit.h"
#include "Vehicle.h"
#include "WaypointMovementGenerator.h"

MovementAction::MovementAction(PlayerbotAI* botAI, std::string const name) : Action(botAI, name)
{
    bot = botAI->GetBot();
}

void MovementAction::CreateWp(Player* wpOwner, float x, float y, float z, float o, uint32 entry, bool important)
{
    float dist = wpOwner->GetDistance(x, y, z);
    float delay = 1000.0f * dist / wpOwner->GetSpeed(MOVE_RUN) + sPlayerbotAIConfig->reactDelay;

    //if (!important)
        //delay *= 0.25;

    Creature* wpCreature = wpOwner->SummonCreature(entry, x, y, z - 1, o, TEMPSUMMON_TIMED_DESPAWN, delay);
    botAI->GetBot()->AddAura(246, wpCreature);

    if (!important)
        wpCreature->SetObjectScale(0.5f);
}

bool MovementAction::MoveNear(uint32 mapId, float x, float y, float z, float distance)
{
    float angle = GetFollowAngle();
    return MoveTo(mapId, x + cos(angle) * distance, y + sin(angle) * distance, z);
}

bool MovementAction::MoveNear(WorldObject* target, float distance)
{
    if (!target)
        return false;

    distance += target->GetCombatReach();

    float x = target->GetPositionX();
    float y = target->GetPositionY();
    float z = target->GetPositionZ();
    float followAngle = GetFollowAngle();

    for (float angle = followAngle; angle <= followAngle + static_cast<float>(2 * M_PI); angle += static_cast<float>(M_PI / 4.f))
    {
        float x = target->GetPositionX() + cos(angle) * distance;
        float y = target->GetPositionY() + sin(angle) * distance;
        float z = target->GetPositionZ();

        if (!bot->IsWithinLOS(x, y, z))
            continue;

        bool moved = MoveTo(target->GetMapId(), x, y, z);
        if (moved)
            return true;
    }

    //botAI->TellError("All paths not in LOS");
    return false;
}

bool MovementAction::MoveToLOS(WorldObject* target, bool ranged)
{
    if (!target)
        return false;

    //std::ostringstream out; out << "Moving to LOS!";
    //bot->Say(out.str(), LANG_UNIVERSAL);

    float x = target->GetPositionX();
    float y = target->GetPositionY();
    float z = target->GetPositionZ();

    //Use standard PathGenerator to find a route.
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

            float distPoint = sqrt(target->GetDistance(point.x, point.y, point.z));
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

bool MovementAction::MoveTo(uint32 mapId, float x, float y, float z, bool idle, bool react, bool normal_only)
{
    UpdateMovementState();
    if (!IsMovingAllowed(mapId, x, y, z)) {
        return false;
    }
    // if (bot->Unit::IsFalling()) {
    //     bot->Say("I'm falling!, flag:" + std::to_string(bot->m_movementInfo.GetMovementFlags()), LANG_UNIVERSAL);
    //     return false;
    // }
    // if (bot->m_movementInfo.HasMovementFlag(MOVEMENTFLAG_SWIMMING)) {
    //     bot->Say("I'm swimming", LANG_UNIVERSAL);
    // }
    // if (bot->Unit::IsFalling()) {
    //     bot->Say("I'm falling", LANG_UNIVERSAL);
    // }
    float distance = bot->GetDistance(x, y, z);
    if (distance > sPlayerbotAIConfig->contactDistance)
    {
        WaitForReach(distance);

        if (bot->IsSitState())
            bot->SetStandState(UNIT_STAND_STATE_STAND);

        if (bot->IsNonMeleeSpellCast(true))
        {
            bot->CastStop();
            botAI->InterruptSpell();
        }
        bool generatePath = !bot->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED) &&
                !bot->IsFlying() && !bot->HasUnitMovementFlag(MOVEMENTFLAG_SWIMMING) && !bot->IsInWater();
        MotionMaster &mm = *bot->GetMotionMaster();
        
        mm.Clear();
        float modifiedZ = SearchBestGroundZForPath(x, y, z, generatePath, normal_only);
        if (modifiedZ == INVALID_HEIGHT) {
            return false;
        }
        mm.MovePoint(mapId, x, y, modifiedZ, generatePath);
        AI_VALUE(LastMovement&, "last movement").Set(mapId, x, y, z, bot->GetOrientation());
        return true;
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
    // float maxDist = sPlayerbotAIConfig->reactDistance;           //Maxium distance a bot can move in one single action.
    // float originalZ = z;                                        // save original destination height to check if bot needs to fly up

    // bool generatePath = !bot->IsFlying() && !bot->HasUnitMovementFlag(MOVEMENTFLAG_SWIMMING) && !bot->IsInWater() && !bot->IsUnderWater();
    // if (generatePath)
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
    // float maxDistChange = totalDistance * 0.1;                    //Maximum change between previous destination before needing a recalulation

    // if (totalDistance < minDist)
    // {
    //     if (lastMove.lastMoveShort.distance(endPosition) < maxDistChange)
    //         AI_VALUE(LastMovement&, "last movement").clear();

    //     mover->StopMoving();
    //     LOG_DEBUG("playerbots", "totalDistance < minDist");
    //     return false;
    // }

    // TravelPath movePath;

    // if (lastMove.lastMoveShort.distance(endPosition) < maxDistChange && startPosition.distance(lastMove.lastMoveShort) < maxDist) //The last short movement was to the same place we want to move now.
    //     movePosition = endPosition;
    // else if (!lastMove.lastPath.empty() && lastMove.lastPath.getBack().distance(endPosition) < maxDistChange) //The last long movement was to the same place we want to move now.
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
    //                     LOG_DEBUG("playerbots", "lastMove.future = std::async(&TravelNodeMap::getFullPath, startPosition, endPosition, bot);");
    //                     return true;
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
    //                     telePos = WorldPosition(at->target_mapId, at->target_X, at->target_Y, at->target_Z, at->target_Orientation);
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
    //             out << bot->getLevel() << ",";
    //             out << (entry ? -1 : entry);

    //             sPlayerbotAIConfig->log("bot_movement.csv", out.str().c_str());
    //         }

    //         if (entry)
    //         {
    //             AI_VALUE(LastMovement&, "last area trigger").lastAreaTrigger = entry;
    //         }
    //         else {
    //             LOG_DEBUG("playerbots", "!entry");
    //             return bot->TeleportTo(movePosition.getMapId(), movePosition.getX(), movePosition.getY(), movePosition.getZ(), movePosition.getO(), 0);
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
    //     out << bot->getLevel();
    //     out << 0;

    //     sPlayerbotAIConfig->log("bot_movement.csv", out.str().c_str());
    // }
    // // LOG_DEBUG("playerbots", "({}, {}) -> ({}, {})", startPosition.getX(), startPosition.getY(), movePosition.getX(), movePosition.getY());
    // if (!react)
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

    // if (totalDistance > maxDist && !detailedMove && !botAI->HasPlayerNearby(&movePosition)) // Why walk if you can fly?
    // {
    //     time_t now = time(nullptr);

    //     AI_VALUE(LastMovement&, "last movement").nextTeleport = now + (time_t)MoveDelay(startPosition.distance(movePosition));
    //     LOG_DEBUG("playerbots", "totalDistance > maxDist && !detailedMove && !botAI->HasPlayerNearby(&movePosition)");
    //     return bot->TeleportTo(movePosition.getMapId(), movePosition.getX(), movePosition.getY(), movePosition.getZ(), startPosition.getAngleTo(movePosition));
    // }

    // // walk if master walks and is close
    // bool masterWalking = false;
    // if (botAI->GetMaster())
    // {
    //     if (botAI->GetMaster()->m_movementInfo.HasMovementFlag(MOVEMENTFLAG_WALKING) && sServerFacade->GetDistance2d(bot, botAI->GetMaster()) < 20.0f)
    //         masterWalking = true;
    // }

    // if (masterWalking)
    //     bot->SetWalk(true);

    // bot->SendMovementFlagUpdate();
    // // LOG_DEBUG("playerbots", "normal move? {} {} {}", !bot->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED) && !bot->HasAuraType(SPELL_AURA_FLY), 
    // //     bot->HasUnitFlag(UNIT_FLAG_DISABLE_MOVE), bot->getStandState());
    // if (!bot->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED) && !bot->HasAuraType(SPELL_AURA_FLY))
    // {
    //     bot->SetWalk(masterWalking);
    //     bot->GetMotionMaster()->MovePoint(movePosition.getMapId(), movePosition.getX(), movePosition.getY(), movePosition.getZ(), generatePath);
    //     WaitForReach(startPosition.distance(movePosition));
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
    //         float height = bot->GetMap()->GetWaterOrGroundLevel(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), ground);
    //         if (bot->GetPositionZ() > originalZ && (bot->GetPositionZ() - originalZ < 5.0f) && (fabs(originalZ - ground) < 5.0f))
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

    //     bot->GetMotionMaster()->MovePoint(movePosition.getMapId(), Position(movePosition.getX(), movePosition.getY(), movePosition.getZ(), 0.f));
    //     WaitForReach(startPosition.distance(movePosition));
    //     LOG_DEBUG("playerbots", "Movepoint to ({}, {})", movePosition.getX(), movePosition.getY());
    // }

    // AI_VALUE(LastMovement&, "last movement").setShort(movePosition);

    // if (!idle)
    //     ClearIdleState();

    // LOG_DEBUG("playerbots", "return true in the end");
    // return true;
}

bool MovementAction::MoveTo(Unit* target, float distance)
{
    if (!IsMovingAllowed(target))
        return false;

    float bx = bot->GetPositionX();
    float by = bot->GetPositionY();
    float bz = bot->GetPositionZ();

    float tx = target->GetPositionX();
    float ty = target->GetPositionY();
    float tz = target->GetPositionZ();

    float distanceToTarget = bot->GetDistance2d(target);
    float angle = bot->GetAngle(target);
    float needToGo = distanceToTarget - distance;

    float maxDistance = sPlayerbotAIConfig->spellDistance;
    if (needToGo > 0 && needToGo > maxDistance)
        needToGo = maxDistance;
    else if (needToGo < 0 && needToGo < -maxDistance)
        needToGo = -maxDistance;

    float dx = cos(angle) * needToGo + bx;
    float dy = sin(angle) * needToGo + by;
    float dz; // = std::max(bz, tz); // calc accurate z position to avoid stuck
    if (distanceToTarget > CONTACT_DISTANCE) {
        dz = bz + (tz - bz) * (needToGo / distanceToTarget);
    } else {
        dz = tz;
    }
    return MoveTo(target->GetMapId(), dx, dy, dz);
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
            return 2 * M_PI / (group->GetMembersCount() -1) * index;

        ++index;
    }

    return 0;
}

bool MovementAction::IsMovingAllowed(Unit* target)
{
    if (!target)
        return false;

    if (bot->GetMapId() != target->GetMapId())
        return false;

   float distance = sServerFacade->GetDistance2d(bot, target);
    if (!bot->InBattleground() && distance > sPlayerbotAIConfig->reactDistance)
        return false;

    return IsMovingAllowed();
}

bool MovementAction::IsMovingAllowed(uint32 mapId, float x, float y, float z)
{
    float distance = sqrt(bot->GetDistance(x, y, z));
    if (!bot->InBattleground() && distance > sPlayerbotAIConfig->reactDistance)
        return false;

    return IsMovingAllowed();
}

bool MovementAction::IsMovingAllowed()
{
    // do not allow if not vehicle driver
    if (botAI->IsInVehicle() && !botAI->IsInVehicle(true))
        return false;

    if (bot->isFrozen() || bot->IsPolymorphed() || (bot->isDead() && !bot->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST)) || bot->IsBeingTeleported() ||
        bot->isInRoots() || bot->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION) || bot->HasAuraType(SPELL_AURA_MOD_CONFUSE) || bot->IsCharmed() ||
        bot->HasAuraType(SPELL_AURA_MOD_STUN) || bot->HasUnitState(UNIT_STATE_IN_FLIGHT) ||
        bot->HasUnitState(UNIT_STATE_LOST_CONTROL))
        return false;

    if (bot->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_CONTROLLED) != NULL_MOTION_TYPE) {
        return false;
    }

    // if (bot->HasUnitMovementFlag(MOVEMENTFLAG_FALLING)) {
    //     return false;
    // }
    return bot->GetMotionMaster()->GetCurrentMovementGeneratorType() != FLIGHT_MOTION_TYPE;
}

bool MovementAction::Follow(Unit* target, float distance)
{
    return Follow(target, distance, GetFollowAngle());
}

void MovementAction::UpdateMovementState()
{
    if (bot->Unit::IsInWater() || bot->Unit::IsUnderWater())
    {
        bot->SetSwim(true);
    }
    else
    {
        bot->SetSwim(false);
    }

    if (bot->IsFlying())
        bot->UpdateSpeed(MOVE_FLIGHT, true);
    // Temporary speed increase in group
    //if (botAI->HasRealPlayerMaster())
        //bot->SetSpeedRate(MOVE_RUN, 1.1f);

    // check if target is not reachable (from Vmangos)
    // if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == CHASE_MOTION_TYPE && bot->CanNotReachTarget() && !bot->InBattleground())
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
    //                 bot->GetMotionMaster()->MovePoint(bot->GetMapId(), pTarget->GetPosition(), FORCED_MOVEMENT_RUN, false);
    //                 return;
    //             }*/
    //         }
    //     }
    // }

    // if ((bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE ||
    //     bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == POINT_MOTION_TYPE) && bot->CanNotReachTarget() && !bot->InBattleground())
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

    if (!bot->InBattleground() && sServerFacade->IsDistanceLessOrEqualThan(sServerFacade->GetDistance2d(bot, target), sPlayerbotAIConfig->followDistance))
    {
        //botAI->TellError("No need to follow");
        return false;
    }

    /*
    if (!bot->InBattleground()
        && sServerFacade->IsDistanceLessOrEqualThan(sServerFacade->GetDistance2d(bot, target->GetPositionX(), target->GetPositionY()), sPlayerbotAIConfig->sightDistance)
        && abs(bot->GetPositionZ() - target->GetPositionZ()) >= sPlayerbotAIConfig->spellDistance && botAI->HasRealPlayerMaster()
        && (target->GetMapId() && bot->GetMapId() != target->GetMapId()))
    {
        bot->StopMoving();
        bot->GetMotionMaster()->Clear();

        float x = bot->GetPositionX();
        float y = bot->GetPositionY();
        float z = target->GetPositionZ();
        if (target->GetMapId() && bot->GetMapId() != target->GetMapId())
        {
            if ((target->GetMap() && target->GetMap()->IsBattlegroundOrArena()) || (bot->GetMap() && bot->GetMap()->IsBattlegroundOrArena()))
                return false;

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
        if ((target->GetMap() && target->GetMap()->IsBattlegroundOrArena()) || (bot->GetMap() && bot->GetMap()->IsBattlegroundOrArena()))
            return false;

        if (bot->isDead() && botAI->GetMaster()->IsAlive())
        {
            bot->ResurrectPlayer(1.0f, false);
            botAI->TellMasterNoFacing("I live, again!");
        }
        else
            botAI->TellError("I am stuck while following");

        bot->CombatStop(true);
        botAI->TellMasterNoFacing("I will there soon.");
		bot->TeleportTo(target->GetMapId(), target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), target->GetOrientation());
        return false;
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

            if(botPos.fDist(cPos) > sPlayerbotAIConfig->spellDistance)
                return MoveTo(cPos.getMapId(),cPos.getX(),cPos.getY(), cPos.getZ());
        }
    }

    if (sServerFacade->IsDistanceGreaterOrEqualThan(sServerFacade->GetDistance2d(bot, target), sPlayerbotAIConfig->sightDistance))
    {
        if (target->GetGUID().IsPlayer())
        {
            Player* pTarget = (Player*)target;

            PlayerbotAI* targetBotAI = GET_PLAYERBOT_AI(pTarget);
            if (targetBotAI) // Try to move to where the bot is going if it is closer and in the same direction.
            {
                WorldPosition botPos(bot);
                WorldPosition tarPos(target);
                WorldPosition longMove = targetBotAI->GetAiObjectContext()->GetValue<WorldPosition>("last long move")->Get();

                if (longMove)
                {
                    float lDist = botPos.fDist(longMove);
                    float tDist = botPos.fDist(tarPos);
                    float ang = botPos.getAngleBetween(tarPos, longMove);
                    if ((lDist * 1.5 < tDist && ang < static_cast<float>(M_PI) / 2) || target->HasUnitState(UNIT_STATE_IN_FLIGHT))
                    {
                        return MoveTo(longMove.getMapId(), longMove.getX(), longMove.getY(), longMove.getZ());
                    }
                }
            }
            else
            {
                if (pTarget->HasUnitState(UNIT_STATE_IN_FLIGHT)) //Move to where the player is flying to.
                {
                    TaxiPathNodeList const& tMap = static_cast<FlightPathMovementGenerator*>(pTarget->GetMotionMaster()->top())->GetPath();
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

    if (sServerFacade->IsDistanceLessOrEqualThan(sServerFacade->GetDistance2d(bot, target), sPlayerbotAIConfig->followDistance))
    {
        //botAI->TellError("No need to follow");
        return false;
    }

    if (target->IsFriendlyTo(bot) && bot->IsMounted() && AI_VALUE(GuidVector, "all targets").empty())
        distance += angle;

    if (!bot->InBattleground() && sServerFacade->IsDistanceLessOrEqualThan(sServerFacade->GetDistance2d(bot, target), sPlayerbotAIConfig->followDistance))
    {
        //botAI->TellError("No need to follow");
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

    AI_VALUE(LastMovement&, "last movement").Set(target);
    ClearIdleState();

    if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE)
    {
        Unit *currentTarget = sServerFacade->GetChaseTarget(bot);
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

        //vehicle->GetMotionMaster()->Clear();
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
    bot->GetMotionMaster()->MoveChase((Unit*) obj, distance);
    WaitForReach(bot->GetExactDist2d(obj) - distance);
    return true;
}

float MovementAction::MoveDelay(float distance)
{
    return distance / bot->GetSpeed(MOVE_RUN);
}

void MovementAction::WaitForReach(float distance)
{
    float delay = 1000.0f * MoveDelay(distance) + sPlayerbotAIConfig->reactDelay;
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

bool MovementAction::Flee(Unit *target)
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
    if (ref && ref->getTarget() == bot) // bot is target - try to flee to tank or master
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
                        fleeTarget   = player;
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
    else // bot is not targeted, try to flee dps/healers
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

            for (GroupReference *gref = group->GetFirstMember(); gref; gref = gref->next())
            {
                Player* player = gref->GetSource();
                if (!player || player == bot || !player->IsAlive())
                    continue;

                if ((isHealer && botAI->IsHeal(player)) || needHealer)
                {
                    float distanceToHealer = sServerFacade->GetDistance2d(bot, player);
                    float distanceToTarget = sServerFacade->GetDistance2d(player, target);
                    if (distanceToHealer < fleeDistance && distanceToTarget > (botAI->GetRange("shoot") / 2 + sPlayerbotAIConfig->followDistance) && (needHealer || player->IsWithinLOSInMap(target)))
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
                    if (distanceToRanged < fleeDistance && distanceToTarget > (botAI->GetRange("shoot") / 2 + sPlayerbotAIConfig->followDistance) && player->IsWithinLOSInMap(target))
                    {
                        fleeTarget = player;
                        fleeDistance = distanceToRanged;
                        possibleTargets.push_back(fleeTarget);
                    }
                }
                // remember any group member in case no one else found
                float distanceToFlee = sServerFacade->GetDistance2d(bot, player);
                float distanceToTarget = sServerFacade->GetDistance2d(player, target);
                if (distanceToFlee < spareDistance && distanceToTarget >(botAI->GetRange("shoot") / 2 + sPlayerbotAIConfig->followDistance) && player->IsWithinLOSInMap(target))
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

bool MovementAction::MoveAway(Unit* target)
{
    if (!target) {
        return false;
    }
    float init_angle = target->GetAngle(bot);
    for (float delta = 0; delta <= M_PI / 2; delta += M_PI / 8) {
        float angle = init_angle + delta;
        float dx = bot->GetPositionX() + cos(angle) * sPlayerbotAIConfig->fleeDistance;
        float dy = bot->GetPositionY() + sin(angle) * sPlayerbotAIConfig->fleeDistance;
        float dz = bot->GetPositionZ();
        if (MoveTo(target->GetMapId(), dx, dy, dz, false, false, true)) {
            return true;
        }
        if (delta == 0) {
            continue;
        }
        angle = init_angle - delta;
        dx = bot->GetPositionX() + cos(angle) * sPlayerbotAIConfig->fleeDistance;
        dy = bot->GetPositionY() + sin(angle) * sPlayerbotAIConfig->fleeDistance;
        dz = bot->GetPositionZ();
        if (MoveTo(target->GetMapId(), dx, dy, dz, false, false, true)) {
            return true;
        }
    }
    return false;
}

bool MovementAction::MoveInside(uint32 mapId, float x, float y, float z, float distance)
{
    if (bot->GetDistance2d(x, y) <= distance) {
        return false;
    }
    return MoveNear(mapId, x, y, z, distance);
}

float MovementAction::SearchBestGroundZForPath(float x, float y, float z, bool generatePath, float range, bool normal_only)
{
    if (!generatePath) {
        return z;
    }
    float modified_z;
    float delta;
    for (delta = 0.0f; delta <= range / 2; delta++) {
        modified_z = bot->GetMapWaterOrGroundLevel(x, y, z + delta);
        PathGenerator gen(bot);
        gen.CalculatePath(x, y, modified_z);
        if (gen.GetPathType() == PATHFIND_NORMAL) {
            return modified_z;
        }
    }
    for (delta = -1.0f; delta >= -range / 2; delta--) {
        modified_z = bot->GetMapWaterOrGroundLevel(x, y, z + delta);
        PathGenerator gen(bot);
        gen.CalculatePath(x, y, modified_z);
        if (gen.GetPathType() == PATHFIND_NORMAL) {
            return modified_z;
        }
    }
    for (delta = range / 2 + 1.0f; delta <= range; delta++) {
        modified_z = bot->GetMapWaterOrGroundLevel(x, y, z + delta);
        PathGenerator gen(bot);
        gen.CalculatePath(x, y, modified_z);
        if (gen.GetPathType() == PATHFIND_NORMAL) {
            return modified_z;
        }
    }
    if (normal_only) {
        return INVALID_HEIGHT;
    }
    return z;
}
    

bool FleeAction::Execute(Event event)
{
    // return Flee(AI_VALUE(Unit*, "current target"));
    return MoveAway(AI_VALUE(Unit*, "current target"));
}

bool FleeAction::isUseful()
{
    if (bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr) {
        return false;
    }
    return true;
}

bool FleeWithPetAction::Execute(Event event)
{
    if (Pet* pet = bot->GetPet())
    {
        if (CreatureAI* creatureAI = ((Creature*)pet)->AI())
        {
            pet->SetReactState(REACT_PASSIVE);
            pet->GetCharmInfo()->SetIsCommandFollow(true);
            pet->GetCharmInfo()->IsReturning();
            pet->GetMotionMaster()->MoveFollow(bot, PET_FOLLOW_DIST, pet->GetFollowAngle());
        }
    }

    return Flee(AI_VALUE(Unit*, "current target"));
}

bool RunAwayAction::Execute(Event event)
{
    return Flee(AI_VALUE(Unit*, "master target"));
}

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

bool MoveOutOfEnemyContactAction::isUseful()
{
    return AI_VALUE2(bool, "inside target", "current target");
}

bool SetFacingTargetAction::Execute(Event event)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    if (bot->HasUnitState(UNIT_STATE_IN_FLIGHT))
        return true;

    sServerFacade->SetFacingTo(bot, target);
    botAI->SetNextCheckDelay(sPlayerbotAIConfig->globalCoolDown);
    return true;
}

bool SetFacingTargetAction::isUseful()
{
    return !AI_VALUE2(bool, "facing", "current target");
}

bool SetFacingTargetAction::isPossible()
{
    if (bot->isFrozen() || bot->IsPolymorphed() ||
        (bot->isDead() && !bot->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST)) ||
        bot->IsBeingTeleported() ||
        bot->HasAuraType(SPELL_AURA_MOD_CONFUSE) || bot->IsCharmed() ||
        bot->HasAuraType(SPELL_AURA_MOD_STUN) || bot->HasUnitState(UNIT_STATE_IN_FLIGHT) ||
        bot->HasUnitState(UNIT_STATE_LOST_CONTROL))
        return false;

    return true;
}

bool SetBehindTargetAction::Execute(Event event)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    float angle = GetFollowAngle() / 3 + target->GetOrientation() + M_PI;

    //return ChaseTo(target, 0.f, angle);

    float distance = sPlayerbotAIConfig->contactDistance;
    float x = target->GetPositionX() + cos(angle) * distance;
    float y = target->GetPositionY() + sin(angle) * distance;
    float z = target->GetPositionZ();
    bot->UpdateGroundPositionZ(x, y, z);

    return MoveTo(bot->GetMapId(), x, y, z);
}

bool SetBehindTargetAction::isUseful()
{
    return !AI_VALUE2(bool, "behind", "current target");
}

bool SetBehindTargetAction::isPossible()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    return target && !(target->GetVictim() && target->GetVictim()->GetGUID() == bot->GetGUID());
}

bool MoveOutOfCollisionAction::Execute(Event event)
{
    float angle = M_PI * 2000 / frand(1.f, 1000.f);
    float distance = sPlayerbotAIConfig->followDistance;
    return MoveTo(bot->GetMapId(), bot->GetPositionX() + cos(angle) * distance, bot->GetPositionY() + sin(angle) * distance, bot->GetPositionZ());
}

bool MoveOutOfCollisionAction::isUseful()
{
    // do not avoid collision on vehicle
    if (botAI->IsInVehicle())
        return false;

    return AI_VALUE2(bool, "collision", "self target") && botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest friendly players")->Get().size() < 15;
}

bool MoveRandomAction::Execute(Event event)
{
    float distance = sPlayerbotAIConfig->tooCloseDistance + sPlayerbotAIConfig->grindDistance * urand(3, 10) / 10.0f;

    Map* map = bot->GetMap();
    for (int i = 0; i < 10; ++i)
    {
        float x = bot->GetPositionX();
        float y = bot->GetPositionY();
        float z = bot->GetPositionZ();
        float angle = (float)rand_norm() * static_cast<float>(M_PI);
        x += urand(0, distance) * cos(angle);
        y += urand(0, distance) * sin(angle);
        bot->UpdateGroundPositionZ(x, y, z);

        if (map->IsInWater(bot->GetPhaseMask(), x, y, z, bot->GetCollisionHeight()))
            continue;

        bool moved = MoveTo(bot->GetMapId(), x, y, z, false, false, true);
        if (moved)
            return true;
    }

    return false;
}

bool MoveRandomAction::isUseful()
{
    return !AI_VALUE(GuidPosition, "rpg target");
}

bool MoveInsideAction::Execute(Event event)
{
    return MoveInside(bot->GetMapId(), x, y, bot->GetPositionZ(), distance);
}

bool RotateAroundTheCenterPointAction::Execute(Event event)
{
    uint32 next_point = GetCurrWaypoint();
    if (MoveTo(bot->GetMapId(), waypoints[next_point].first, waypoints[next_point].second, bot->GetPositionZ())) {
        call_counters += 1;
        return true;
    }
    return false;
}