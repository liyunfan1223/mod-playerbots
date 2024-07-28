/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "NamedObjectContext.h"

#include "Playerbots.h"

void Qualified::Qualify(int qual)
{
    std::ostringstream out;
    out << qual;
    qualifier = out.str();
}

std::string const Qualified::MultiQualify(std::vector<std::string> qualifiers)
{
    std::ostringstream out;
    for (auto& qualifier : qualifiers)
        out << qualifier << (&qualifier != &qualifiers.back() ? " " : "");

    return out.str();
}

std::vector<std::string> Qualified::getMultiQualifiers(std::string const qualifier1)
{
    std::istringstream iss(qualifier1);
    return {std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{}};
}

int32 Qualified::getMultiQualifier(std::string const qualifier1, uint32 pos)
{
    return std::stoi(getMultiQualifiers(qualifier1)[pos]);
}
