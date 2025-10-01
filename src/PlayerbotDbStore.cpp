/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PlayerbotDbStore.h"

#include <iostream>

#include "Playerbots.h"

void PlayerbotDbStore::Load(PlayerbotAI* botAI)
{
    ObjectGuid::LowType guid = botAI->GetBot()->GetGUID().GetCounter();

    PlayerbotsDatabasePreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_DB_STORE);
    stmt->SetData(0, guid);
    if (PreparedQueryResult result = PlayerbotsDatabase.Query(stmt))
    {
        std::vector<std::string> values;
        do
        {
            Field* fields = result->Fetch();
            std::string const key = fields[0].Get<std::string>();
            std::string const value = fields[1].Get<std::string>();

            if (key == "value")
                values.push_back(value);
            else if (key == "co")
            {
                botAI->ClearStrategies(BOT_STATE_COMBAT);
                botAI->ChangeStrategy("+chat", BOT_STATE_COMBAT);
                botAI->ChangeStrategy(value, BOT_STATE_COMBAT);
            }
            else if (key == "nc")
            {
                botAI->ClearStrategies(BOT_STATE_NON_COMBAT);
                botAI->ChangeStrategy("+chat", BOT_STATE_NON_COMBAT);
                botAI->ChangeStrategy(value, BOT_STATE_NON_COMBAT);
            }
            else if (key == "dead")
                botAI->ChangeStrategy(value, BOT_STATE_DEAD);
        } while (result->NextRow());

        botAI->GetAiObjectContext()->Load(values);
    }
}

void PlayerbotDbStore::Save(PlayerbotAI* botAI)
{
    ObjectGuid::LowType guid = botAI->GetBot()->GetGUID().GetCounter();

    Reset(botAI);

    PlayerbotsDatabasePreparedStatement* deleteStatement =
        PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_DEL_DB_STORE);
    deleteStatement->SetData(0, guid);
    PlayerbotsDatabase.Execute(deleteStatement);

    std::vector<std::string> data = botAI->GetAiObjectContext()->Save();
    for (std::vector<std::string>::iterator i = data.begin(); i != data.end(); ++i)
    {
        SaveValue(guid, "value", *i);
    }

    SaveValue(guid, "co", FormatStrategies("co", botAI->GetStrategies(BOT_STATE_COMBAT)));
    SaveValue(guid, "nc", FormatStrategies("nc", botAI->GetStrategies(BOT_STATE_NON_COMBAT)));
    SaveValue(guid, "dead", FormatStrategies("dead", botAI->GetStrategies(BOT_STATE_DEAD)));
}

std::string const PlayerbotDbStore::FormatStrategies(std::string const type, std::vector<std::string> strategies)
{
    std::ostringstream out;
    for (std::vector<std::string>::iterator i = strategies.begin(); i != strategies.end(); ++i)
        out << "+" << (*i).c_str() << ",";

    std::string const res = out.str();
    return res.substr(0, res.size() - 1);
}

void PlayerbotDbStore::Reset(PlayerbotAI* botAI)
{
    ObjectGuid::LowType guid = botAI->GetBot()->GetGUID().GetCounter();

    PlayerbotsDatabasePreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_DEL_DB_STORE);
    stmt->SetData(0, guid);
    PlayerbotsDatabase.Execute(stmt);
}

void PlayerbotDbStore::SaveValue(uint32 guid, std::string const key, std::string const value)
{
    PlayerbotsDatabasePreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_INS_DB_STORE);
    stmt->SetData(0, guid);
    stmt->SetData(1, key);
    stmt->SetData(2, value);
    PlayerbotsDatabase.Execute(stmt);
}
