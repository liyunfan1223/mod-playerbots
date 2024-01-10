/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "MoveToRpgTargetAction.h"
#include "ChatHelper.h"
#include "ChooseRpgTargetAction.h"
#include "Event.h"
#include "LastMovementValue.h"
#include "Playerbots.h"

bool MoveToRpgTargetAction::Execute(Event event)
{
    GuidPosition guidP = AI_VALUE(GuidPosition, "rpg target");
    Unit* unit = botAI->GetUnit(guidP);
    if (unit && !unit->IsInWorld()) {
        return false;
    }
    GameObject* go = botAI->GetGameObject(guidP);
    if (go && !go->IsInWorld()) {
        return false;
    }
    Player* player = guidP.GetPlayer();

    WorldObject* wo = nullptr;
    if (unit)
        wo = unit;
    else if (go)
        wo = go;
    else
        return false;

    if (botAI->HasStrategy("debug rpg", BOT_STATE_NON_COMBAT) && guidP.GetWorldObject())
    {
        std::ostringstream out;
        out << "Heading to: ";
        out << chat->FormatWorldobject(guidP.GetWorldObject());
        botAI->TellMasterNoFacing(out);
    }

    if (guidP.IsPlayer())
    {
        Player* player = guidP.GetPlayer();

        if (player && GET_PLAYERBOT_AI(player))
        {
            GuidPosition guidPP = PAI_VALUE(GuidPosition, "rpg target");

            if (guidPP.IsPlayer())
            {
                AI_VALUE(GuidSet&,"ignore rpg target").insert(AI_VALUE(GuidPosition, "rpg target"));

                RESET_AI_VALUE(GuidPosition, "rpg target");
                return false;
            }
        }
    }

    if ((unit && unit->isMoving() && !urand(0, 20)) || !ChooseRpgTargetAction::isFollowValid(bot, wo) || guidP.distance(bot) > sPlayerbotAIConfig->reactDistance * 2 || !urand(0, 50))
    {
        AI_VALUE(GuidSet&, "ignore rpg target").insert(AI_VALUE(GuidPosition, "rpg target"));
        RESET_AI_VALUE(GuidPosition, "rpg target");
        return false;
    }

    float x = wo->GetPositionX();
    float y = wo->GetPositionY();
    float z = wo->GetPositionZ();
    float mapId = wo->GetMapId();

    if (sPlayerbotAIConfig->randombotsWalkingRPG)
        if (!bot->IsOutdoors())
            bot->m_movementInfo.AddMovementFlag(MOVEMENTFLAG_WALKING);

    float angle = 0.f;
    float distance = 1.0f;
    if (bot->IsWithinLOS(x, y, z))
    {
        if (!unit || !unit->isMoving())
            angle = wo->GetAngle(bot) + (M_PI * irand(-25, 25) / 100.0); //Closest 45 degrees towards the target
        else
            angle = wo->GetOrientation() + (M_PI * irand(-25, 25) / 100.0); //45 degrees infront of target (leading it's movement)

        distance = frand(0.5f, 1.f);
    }
    else
        angle = 2 * M_PI * urand(0, 100) / 100.0; //A circle around the target.

    x += cos(angle) * INTERACTION_DISTANCE * distance;
    y += sin(angle) * INTERACTION_DISTANCE * distance;

    //WaitForReach(distance);

    bool couldMove = false;

    if (bot->IsWithinLOS(x, y, z))
        couldMove = MoveNear(mapId, x, y, z, 0);
    else
        couldMove = MoveTo(mapId, x, y, z);

    if (!couldMove && WorldPosition(mapId, x, y, z).distance(bot) > INTERACTION_DISTANCE)
    {
        AI_VALUE(GuidSet&, "ignore rpg target").insert(AI_VALUE(GuidPosition, "rpg target"));
        RESET_AI_VALUE(GuidPosition, "rpg target");
    }

    return couldMove;
}

bool MoveToRpgTargetAction::isUseful()
{
    GuidPosition guidP = AI_VALUE(GuidPosition, "rpg target");
    if (!guidP)
        return false;

    WorldObject* wo = guidP.GetWorldObject();
    if (!wo)
        return false;

    TravelTarget* travelTarget = AI_VALUE(TravelTarget*, "travel target");
    if (travelTarget->isTraveling() && ChooseRpgTargetAction::isFollowValid(bot, *travelTarget->getPosition()))
        return false;

    if (guidP.distance(bot) < INTERACTION_DISTANCE)
        return false;

    if (!ChooseRpgTargetAction::isFollowValid(bot, wo))
        return false;

    if (!AI_VALUE(bool, "can move around"))
        return false;

    return true;
}
