/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "AlwaysLootListValue.h"

#include "Playerbots.h"

std::string const AlwaysLootListValue::Save()
{
    std::ostringstream out;
    bool first = true;
    for (std::set<uint32>::iterator i = value.begin(); i != value.end(); ++i)
    {
        if (!first)
            out << ",";
        else
            first = false;

        out << *i;
    }

    return out.str();
}

bool AlwaysLootListValue::Load(std::string const text)
{
    value.clear();

    std::vector<std::string> ss = split(text, ',');
    for (std::vector<std::string>::iterator i = ss.begin(); i != ss.end(); ++i)
    {
        value.insert(atoi(i->c_str()));
    }
    return true;
}
