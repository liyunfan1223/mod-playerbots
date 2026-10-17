/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MoveToTravelTargetAction.h"
#include "ChooseRpgTargetAction.h"
#include "LootObjectStack.h"
#include "Playerbots.h"

bool MoveToTravelTargetAction::Execute(Event /*event*/)
{
    TravelTarget* target = AI_VALUE(TravelTarget*, "travel target");

    WorldPosition botLocation(bot);
    WorldLocation location = *target->getPosition();

    Group* group = bot->GetGroup();
    if (group && !urand(0, 1) && bot == botAI->GetGroupLeader() && !bot->IsInCombat())
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
            // unused while the angle-based filter below stays commented out
            [[maybe_unused]] WorldPosition targetPos = *target->getPosition();

            float memberDistance = botLocation.distance(memberPos);

            if (memberDistance < 50.0f)
                continue;
            if (memberDistance > sPlayerbotAIConfig.ReactDistance * 20)
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

            target->setExpireIn(target->getTimeLeft() + sPlayerbotAIConfig.MaxWaitForMove);

            botAI->SetNextCheckDelay(sPlayerbotAIConfig.MaxWaitForMove);

            return true;
        }
    }

    float maxDistance = target->getDestination()->getRadiusMin();

    // Spread bots around the target but keep the offset stable per
    // (bot, destination) pair. Previously the angle and radius were
    // re-rolled every time the action re-entered (i.e. every tick the
    // bot wasn't already moving), which made bots oscillate between
    // two random points around the same quest POI instead of
    // committing to one approach.
    uint32 botLow = bot->GetGUID().GetCounter();
    int32 destSeed = static_cast<int32>(location.GetPositionX()) * 73856093 ^
                     static_cast<int32>(location.GetPositionY()) * 19349663;
    uint32 seed = botLow ^ static_cast<uint32>(destSeed);
    float angle = 2.0f * static_cast<float>(M_PI) * static_cast<float>(seed % 1000) / 1000.0f;
    float mod = 0.5f + static_cast<float>((seed / 1000) % 1000) / 2000.0f;  // [0.5, 1.0]

    if (target->getMaxTravelTime() > target->getTimeLeft())  // The bot is late. Speed it up.
    {
        // distance = sPlayerbotAIConfig.FleeDistance;
        // angle = bot->GetAngle(location.GetPositionX(), location.GetPositionY());
        // location = botLocation.getLocation();
    }

    float x = location.GetPositionX();
    float y = location.GetPositionY();
    float z = location.GetPositionZ();
    float mapId = location.GetMapId();

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
