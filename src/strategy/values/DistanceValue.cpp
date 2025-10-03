/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "DistanceValue.h"

#include "Formations.h"
#include "LootObjectStack.h"
#include "Playerbots.h"
#include "PositionValue.h"
#include "ServerFacade.h"
#include "Stances.h"

float DistanceValue::Calculate()
{
    if (qualifier == "loot target")
    {
        LootObject loot = AI_VALUE(LootObject, qualifier);
        if (loot.IsEmpty())
            return 0.0f;

        WorldObject* obj = loot.GetWorldObject(bot);
        if (!obj || !obj->IsInWorld())
            return 0.0f;

        return sServerFacade->GetDistance2d(botAI->GetBot(), obj);
    }

    if (qualifier.find("position_") == 0)
    {
        std::string const position = qualifier.substr(9);
        PositionInfo pos = context->GetValue<PositionMap&>("position")->Get()[position];
        if (!pos.isSet())
            return 0.0f;

        if (botAI->GetBot()->GetMapId() != pos.mapId)
            return 0.0f;

        return sServerFacade->GetDistance2d(botAI->GetBot(), pos.x, pos.y);
    }

    Unit* target = nullptr;
    if (qualifier == "rpg target")
    {
        GuidPosition rpgTarget = AI_VALUE(GuidPosition, qualifier);
        return rpgTarget.distance(bot);
    }
    else if (qualifier == "travel target")
    {
        TravelTarget* travelTarget = AI_VALUE(TravelTarget*, qualifier);
        return travelTarget->distance(botAI->GetBot());
    }
    else if (qualifier == "last long move")
    {
        WorldPosition target = AI_VALUE(WorldPosition, qualifier);
        return target.distance(botAI->GetBot());
    }
    else if (qualifier == "home bind")
    {
        WorldPosition target = AI_VALUE(WorldPosition, qualifier);
        return target.distance(botAI->GetBot());
    }
    else if (qualifier == "current target")
    {
        Unit* target = AI_VALUE(Unit*, qualifier);
        if (!target || !target->IsInWorld())
            return 0.0f;

        return bot->GetDistance2d(target);
    }
    else
    {
        target = AI_VALUE(Unit*, qualifier);
        if (target && target == GetMaster() && target != bot)
        {
            Formation* formation = AI_VALUE(Formation*, "formation");
            WorldLocation loc = formation->GetLocation();
            return sServerFacade->GetDistance2d(botAI->GetBot(), loc.GetPositionX(), loc.GetPositionY());
        }
    }

    if (!target || !target->IsInWorld())
        return 0.0f;

    if (target == botAI->GetBot())
        return 0.0f;

    return sServerFacade->GetDistance2d(botAI->GetBot(), target);
}

bool InsideTargetValue::Calculate()
{
    Unit* target = AI_VALUE(Unit*, qualifier);
    if (!target || !target->IsInWorld() || target == botAI->GetBot())
        return false;

    float dist = sServerFacade->GetDistance2d(botAI->GetBot(), target->GetPositionX(), target->GetPositionY());
    return sServerFacade->IsDistanceLessThan(dist, target->GetCombatReach());
}
