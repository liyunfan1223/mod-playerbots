/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PlayerbotTextMgr.h"
#include "Playerbots.h"

void replaceAll(std::string& str, std::string const from, std::string const to);

void PlayerbotTextMgr::LoadTemplates()
{
    LOG_INFO("playerbots", "Loading playerbots texts...");

    uint32 count = 0;
    if (PreparedQueryResult result = PlayerbotsDatabase.Query(PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_TEXT)))
    {
        do
        {
            Field* fields = result->Fetch();
            std::string const key = fields[0].Get<std::string>();
            std::string const text = fields[1].Get<std::string>();
            templates[key].push_back(text);
            ++count;
        }
        while (result->NextRow());
    }

    LOG_INFO("playerbots", "{} playerbots texts loaded", count);
}

std::string const PlayerbotTextMgr::Format(std::string const key, std::map<std::string, std::string> placeholders)
{
    if (templates.empty())
        LoadTemplates();

    std::vector<std::string>& list = templates[key];
    if (list.empty())
    {
        std::ostringstream out;
        out << "Unknown text: " << key;
        return out.str();
    }

    std::string str = list[urand(0, list.size() - 1)];
    for (std::map<std::string, std::string>::iterator i = placeholders.begin(); i != placeholders.end(); ++i)
        replaceAll(str, i->first, i->second);

    return str;
}
