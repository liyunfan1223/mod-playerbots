/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PositionAction.h"

#include "Event.h"
#include "Playerbots.h"
#include "PositionValue.h"

void TellPosition(PlayerbotAI* botAI, std::string const name, PositionInfo pos)
{
    std::ostringstream out;
    out << "Position " << name;

    if (pos.isSet())
    {
        float x = pos.x;
        float y = pos.y;
        Map2ZoneCoordinates(x, y, botAI->GetBot()->GetZoneId());
        out << " is set to " << x << "," << y;
    }
    else
        out << " is not set";

    botAI->TellMaster(out);
}

bool PositionAction::Execute(Event event)
{
    std::string const param = event.getParam();
    if (param.empty())
        return false;

    Player* master = GetMaster();
    if (!master)
        return false;

    PositionMap& posMap = context->GetValue<PositionMap&>("position")->Get();
    if (param == "?")
    {
        for (PositionMap::iterator i = posMap.begin(); i != posMap.end(); ++i)
        {
            if (i->second.isSet())
                TellPosition(botAI, i->first, i->second);
        }

        return true;
    }

    std::vector<std::string> params = split(param, ' ');
    if (params.size() != 2)
    {
        botAI->TellMaster("Whisper position <name> ?/set/reset");
        return false;
    }

    std::string const name = params[0];
    std::string const action = params[1];
    PositionInfo pos = posMap[name];
    if (action == "?")
    {
        TellPosition(botAI, name, pos);
        return true;
    }

    std::vector<std::string> coords = split(action, ',');
    if (coords.size() == 3)
    {
        pos.Set(atoi(coords[0].c_str()), atoi(coords[1].c_str()), atoi(coords[2].c_str()), botAI->GetBot()->GetMapId());
        posMap[name] = pos;

        std::ostringstream out;
        out << "Position " << name << " is set";
        botAI->TellMaster(out);
        return true;
    }

    if (action == "set")
    {
        pos.Set(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), botAI->GetBot()->GetMapId());
        posMap[name] = pos;

        std::ostringstream out;
        out << "Position " << name << " is set";
        botAI->TellMaster(out);
        return true;
    }

    if (action == "reset")
    {
        pos.Reset();
        posMap[name] = pos;

        std::ostringstream out;
        out << "Position " << name << " is reset";
        botAI->TellMaster(out);
        return true;
    }

    return false;
}

bool MoveToPositionAction::Execute(Event event)
{
    PositionInfo pos = context->GetValue<PositionMap&>("position")->Get()[qualifier];
    if (!pos.isSet())
    {
        std::ostringstream out;
        out << "Position " << qualifier << " is not set";
        botAI->TellMaster(out);
        return false;
    }

    return MoveTo(bot->GetMapId(), pos.x, pos.y, pos.z, idle);
}

bool MoveToPositionAction::isUseful()
{
    PositionInfo pos = context->GetValue<PositionMap&>("position")->Get()[qualifier];
    float distance = AI_VALUE2(float, "distance", std::string("position_") + qualifier);
    return pos.isSet() && distance > sPlayerbotAIConfig->followDistance && distance < sPlayerbotAIConfig->reactDistance;
}

bool SetReturnPositionAction::Execute(Event event)
{
    PositionMap& posMap = context->GetValue<PositionMap&>("position")->Get();
    PositionInfo returnPos = posMap["return"];
    PositionInfo randomPos = posMap["random"];
    if (returnPos.isSet() && !randomPos.isSet())
    {
        float angle = 2 * M_PI * urand(0, 1000) / 100.0f;
        float dist = sPlayerbotAIConfig->followDistance * urand(0, 1000) / 1000.0f;
        float x = returnPos.x + cos(angle) * dist;
        float y = returnPos.y + sin(angle) * dist;
        float z = bot->GetPositionZ();
        bot->UpdateAllowedPositionZ(x, y, z);

        if (!bot->IsWithinLOS(x, y, z))
            return false;

        randomPos.Set(x, y, z, bot->GetMapId());
        posMap["random"] = randomPos;
        return true;
    }

    return false;
}

bool SetReturnPositionAction::isUseful()
{
    PositionMap& posMap = context->GetValue<PositionMap&>("position")->Get();
    return posMap["return"].isSet() && !posMap["random"].isSet();
}

bool ReturnAction::isUseful()
{
    PositionInfo pos = context->GetValue<PositionMap&>("position")->Get()[qualifier];
    return pos.isSet() && AI_VALUE2(float, "distance", "position_random") > sPlayerbotAIConfig->followDistance;
}
