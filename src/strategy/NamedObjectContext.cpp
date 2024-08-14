/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "NamedObjectContext.h"

#include "Playerbots.h"

void Qualified::Qualify(const int& qual)
{
    std::ostringstream out;
    out << qual;
    qualifier = out.str();
}

std::string const Qualified::MultiQualify(const std::vector<std::string>& qualifiers, std::string_view separator, std::string_view brackets)
{
    std::stringstream out;
    for (uint8 i = 0; i < qualifiers.size(); i++)
    {
        const std::string& qualifier = qualifiers[i];
        if (i == qualifiers.size() - 1)
        {
            out << qualifier;
        }
        else
        {
            out << qualifier << separator;
        }
    }

    if (brackets.empty())
    {
        return out.str();
    }
    else
    {
        return brackets[0] + out.str() + brackets[1];
    }
}

std::vector<std::string> Qualified::getMultiQualifiers(std::string_view qualifier1)
{
    std::istringstream iss(qualifier1.data());
    return {std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{}};
}

int32 Qualified::getMultiQualifier(std::string_view qualifier1, const uint32& pos)
{
    return std::stoi(getMultiQualifiers(qualifier1)[pos]);
}
