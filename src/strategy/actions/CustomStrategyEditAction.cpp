/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "CustomStrategyEditAction.h"

#include "CustomStrategy.h"
#include "Event.h"
#include "Playerbots.h"

bool CustomStrategyEditAction::Execute(Event event)
{
    std::string text = event.getParam();
    size_t pos = text.find(" ");
    if (pos == std::string::npos)
        return PrintHelp();

    std::string const name = text.substr(0, pos);
    text = text.substr(pos + 1);

    pos = text.find(" ");
    if (pos == std::string::npos)
        pos = text.size();

    std::string const idx = text.substr(0, pos);
    text = pos >= text.size() ? "" : text.substr(pos + 1);

    return idx == "?" ? Print(name) : Edit(name, atoi(idx.c_str()), text);
}

bool CustomStrategyEditAction::PrintHelp()
{
    botAI->TellMaster("=== Custom strategies ===");

    uint32 owner = botAI->GetBot()->GetGUID().GetCounter();

    PlayerbotsDatabasePreparedStatement* stmt =
        PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_CUSTOM_STRATEGY_BY_OWNER);
    stmt->SetData(0, owner);
    if (PreparedQueryResult result = PlayerbotsDatabase.Query(stmt))
    {
        do
        {
            Field* fields = result->Fetch();
            std::string const name = fields[0].Get<std::string>();
            botAI->TellMaster(name);
        } while (result->NextRow());
    }

    botAI->TellMaster("Usage: cs <name> <idx> <command>");
    return false;
}

bool CustomStrategyEditAction::Print(std::string const name)
{
    std::ostringstream out;
    out << "=== " << name << " ===";
    botAI->TellMaster(out.str());

    uint32 owner = botAI->GetBot()->GetGUID().GetCounter();

    PlayerbotsDatabasePreparedStatement* stmt =
        PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_CUSTOM_STRATEGY_BY_OWNER_AND_NAME);
    stmt->SetData(0, owner);
    stmt->SetData(1, name);
    if (PreparedQueryResult result = PlayerbotsDatabase.Query(stmt))
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 idx = fields[0].Get<uint32>();
            std::string const action = fields[1].Get<std::string>();

            PrintActionLine(idx, action);
        } while (result->NextRow());
    }

    return true;
}

bool CustomStrategyEditAction::Edit(std::string const name, uint32 idx, std::string const command)
{
    uint32 owner = botAI->GetBot()->GetGUID().GetCounter();

    PlayerbotsDatabasePreparedStatement* stmt =
        PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_CUSTOM_STRATEGY_BY_OWNER_AND_NAME_AND_IDX);
    stmt->SetData(0, owner);
    stmt->SetData(1, name);
    stmt->SetData(2, idx);
    if (PreparedQueryResult result = PlayerbotsDatabase.Query(stmt))
    {
        if (command.empty())
        {
            stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_DEL_CUSTOM_STRATEGY);
            stmt->SetData(0, name);
            stmt->SetData(1, owner);
            stmt->SetData(2, idx);
            PlayerbotsDatabase.Execute(stmt);
        }
        else
        {
            stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_UPD_CUSTOM_STRATEGY);
            stmt->SetData(0, command);
            stmt->SetData(1, name);
            stmt->SetData(2, owner);
            stmt->SetData(3, idx);
            PlayerbotsDatabase.Execute(stmt);
        }
    }
    else
    {
        stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_INS_CUSTOM_STRATEGY);
        stmt->SetData(0, name);
        stmt->SetData(1, owner);
        stmt->SetData(2, idx);
        stmt->SetData(3, command);
        PlayerbotsDatabase.Execute(stmt);
    }

    PrintActionLine(idx, command);

    std::ostringstream ss;
    ss << "custom::" << name;

    if (Strategy* strategy = botAI->GetAiObjectContext()->GetStrategy(ss.str()))
    {
        if (CustomStrategy* cs = dynamic_cast<CustomStrategy*>(strategy))
        {
            cs->Reset();
            botAI->ReInitCurrentEngine();
        }
    }

    return true;
}

bool CustomStrategyEditAction::PrintActionLine(uint32 idx, std::string const command)
{
    std::ostringstream out;
    out << "#" << idx << " " << command;
    botAI->TellMaster(out.str());
    return true;
}
