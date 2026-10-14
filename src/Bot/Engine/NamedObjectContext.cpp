/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "NamedObjectContext.h"
#include "Playerbots.h"

void Qualified::Qualify(int qual)
{
    std::ostringstream out;
    out << qual;
    qualifier = out.str();
}

std::string const Qualified::MultiQualify(std::vector<std::string> const& qualifiers, std::string const& separator, const std::string_view brackets)
{
    std::stringstream out;
    for (uint8 i = 0; i < qualifiers.size(); ++i)
    {
        std::string const& qualifier = qualifiers[i];
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

std::vector<std::string> Qualified::getMultiQualifiers(std::string const& qualifier1)
{
    std::istringstream iss(qualifier1);
    return {std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{}};
}

int32 Qualified::getMultiQualifier(std::string const& qualifier1, uint32 pos)
{
    return std::stoi(getMultiQualifiers(qualifier1)[pos]);
}
