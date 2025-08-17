/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PositionValue.h"

#include "Playerbots.h"

PositionValue::PositionValue(PlayerbotAI* botAI, std::string const name)
    : ManualSetValue<PositionMap&>(botAI, positions, name)
{
}

std::string const PositionValue::Save()
{
    std::ostringstream out;
    bool first = true;
    for (PositionMap::iterator i = value.begin(); i != value.end(); ++i)
    {
        std::string const name = i->first;
        PositionInfo pos = i->second;
        if (pos.isSet())
        {
            if (!first)
                out << "^";
            else
                first = false;

            out << name << "=" << pos.x << "," << pos.y << "," << pos.z << "," << pos.mapId;
        }
    }

    return out.str();
}

bool PositionValue::Load(std::string const text)
{
    value.clear();

    std::vector<std::string> ss = split(text, '^');
    for (std::vector<std::string>::iterator i = ss.begin(); i != ss.end(); ++i)
    {
        std::vector<std::string> s1 = split(*i, '=');
        if (s1.size() != 2)
            continue;

        std::string const name = s1[0];

        std::vector<std::string> s2 = split(s1[1], ',');
        if (s2.size() != 4)
            continue;

        double x = atof(s2[0].c_str());
        double y = atof(s2[1].c_str());
        double z = atof(s2[2].c_str());
        uint32 mapId = atoi(s2[3].c_str());

        value[name].Set(x, y, z, mapId);
    }

    return true;
}

WorldPosition CurrentPositionValue::Calculate() { return WorldPosition(bot); }

PositionInfo SinglePositionValue::Calculate()
{
    PositionMap& posMap = AI_VALUE(PositionMap&, "position");
    return posMap[getQualifier()];
}

void SinglePositionValue::Set(PositionInfo value)
{
    PositionMap& posMap = AI_VALUE(PositionMap&, "position");
    PositionInfo pos = posMap[getQualifier()];
    pos = value;
    posMap[getQualifier()] = pos;
}

void SinglePositionValue::Reset()
{
    PositionMap& posMap = AI_VALUE(PositionMap&, "position");
    PositionInfo pos = posMap[getQualifier()];
    pos.Reset();
    posMap[getQualifier()] = pos;
}
