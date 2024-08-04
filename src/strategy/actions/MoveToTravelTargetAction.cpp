/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "MoveToTravelTargetAction.h"

#include "ChooseRpgTargetAction.h"
#include "LootObjectStack.h"
#include "PathGenerator.h"
#include "Playerbots.h"

bool MoveToTravelTargetAction::Execute(Event event)
{
    TravelTarget* target = AI_VALUE(TravelTarget*, "travel target");

    WorldPosition botLocation(bot);
    WorldLocation location = *target->getPosition();

    Group* group = bot->GetGroup();
    if (group && !urand(0, 1) && bot == botAI->GetGroupMaster())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member == bot)
                continue;

            if (!member->IsAlive())
                continue;

            if (!member->isMoving())
                continue;

            PlayerbotAI* memberBotAI = GET_PLAYERBOT_AI(member);
            if (memberBotAI && !memberBotAI->HasStrategy("follow", BOT_STATE_NON_COMBAT))
                continue;

            WorldPosition memberPos(member);
            WorldPosition targetPos = *target->getPosition();

            float memberDistance = botLocation.distance(memberPos);

            if (memberDistance < 50.0f)
                continue;
            if (memberDistance > sPlayerbotAIConfig->reactDistance * 20)
                continue;

            // float memberAngle = botLocation.getAngleBetween(targetPos, memberPos);

            // if (botLocation.getMapId() == targetPos.getMapId() && botLocation.getMapId() == memberPos.getMapId() &&
            // memberAngle < static_cast<float>(M_PI) / 2) //We are heading that direction anyway.
            //     continue;

            if (!urand(0, 5))
            {
                std::ostringstream out;
                if (botAI->GetMaster() && !bot->GetGroup()->IsMember(botAI->GetMaster()->GetGUID()))
                    out << "Waiting a bit for ";
                else
                    out << "Please hurry up ";

                out << member->GetName();

                botAI->TellMasterNoFacing(out);
            }

            target->setExpireIn(target->getTimeLeft() + sPlayerbotAIConfig->maxWaitForMove);

            botAI->SetNextCheckDelay(sPlayerbotAIConfig->maxWaitForMove);

            return true;
        }
    }

    float maxDistance = target->getDestination()->getRadiusMin();

    // Evenly distribute around the target.
    float angle = 2 * M_PI * urand(0, 100) / 100.0;

    if (target->getMaxTravelTime() > target->getTimeLeft())  // The bot is late. Speed it up.
    {
        // distance = sPlayerbotAIConfig->fleeDistance;
        // angle = bot->GetAngle(location.GetPositionX(), location.GetPositionY());
        // location = botLocation.getLocation();
    }

    float x = location.GetPositionX();
    float y = location.GetPositionY();
    float z = location.GetPositionZ();
    float mapId = location.GetMapId();

    // Move between 0.5 and 1.0 times the maxDistance.
    float mod = frand(50.f, 100.f) / 100.0f;

    x += cos(angle) * maxDistance * mod;
    y += sin(angle) * maxDistance * mod;

    bool canMove = false;

    if (bot->IsWithinLOS(x, y, z))
        canMove = MoveNear(mapId, x, y, z, 0);
    else
        canMove = MoveTo(mapId, x, y, z, false, false);

    if (!canMove && !target->isForced())
    {
        target->incRetry(true);

        if (target->isMaxRetry(true))
            target->setStatus(TRAVEL_STATUS_COOLDOWN);
    }
    else
        target->setRetry(true);

    return canMove;
}

bool MoveToTravelTargetAction::isUseful()
{
    if (!botAI->AllowActivity(TRAVEL_ACTIVITY))
        return false;

    if (!context->GetValue<TravelTarget*>("travel target")->Get()->isTraveling())
        return false;

    if (bot->HasUnitState(UNIT_STATE_IN_FLIGHT))
        return false;

    if (bot->IsFlying())
        return false;

    if (bot->isMoving())
        return false;

    if (!AI_VALUE(bool, "can move around"))
        return false;

    LootObject loot = AI_VALUE(LootObject, "loot target");
    if (loot.IsLootPossible(bot))
        return false;

    if (!ChooseRpgTargetAction::isFollowValid(bot,
                                              *context->GetValue<TravelTarget*>("travel target")->Get()->getPosition()))
        return false;

    return true;
}
