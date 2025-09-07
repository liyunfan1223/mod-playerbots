/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "OutfitListValue.h"

#include "Playerbots.h"

std::string const OutfitListValue::Save()
{
    std::ostringstream out;
    bool first = true;
    for (Outfit::iterator i = value.begin(); i != value.end(); ++i)
    {
        if (!first)
            out << "^";
        else
            first = false;

        out << *i;
    }

    return out.str();
}

bool OutfitListValue::Load(std::string const text)
{
    value.clear();

    std::vector<std::string> ss = split(text, '^');
    for (std::vector<std::string>::iterator i = ss.begin(); i != ss.end(); ++i)
    {
        value.push_back(*i);
    }

    return true;
}
