/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "SkipSpellsListAction.h"

#include "Event.h"
#include "LootAction.h"
#include "Playerbots.h"
#include "SkipSpellsListValue.h"

bool SkipSpellsListAction::Execute(Event event)
{
    std::string cmd = event.getParam();
    std::set<uint32>& skipSpells = AI_VALUE(std::set<uint32>&, "skip spells list");

    SpellIds spellIds = parseIds(cmd);
    if (!spellIds.empty())
    {
        skipSpells.clear();
        for (uint32 spellId : spellIds)
        {
            skipSpells.insert(spellId);
        }

        cmd = "?";
    }

    if (cmd == "reset")
    {
        skipSpells.clear();
        botAI->TellMaster("Ignored spell list is empty");
        return true;
    }

    if (cmd.empty() || cmd == "?")
    {
        std::ostringstream out;
        if (skipSpells.empty())
        {
            botAI->TellMaster("Ignored spell list is empty");
            return true;
        }

        out << "Ignored spell list: ";

        bool first = true;
        for (uint32 spellId : skipSpells)
        {
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
            if (!spellInfo)
                continue;

            if (first)
                first = false;
            else
                out << ", ";

            out << chat->FormatSpell(spellInfo);
        }

        botAI->TellMaster(out);
    }
    else
    {
        bool remove = cmd.size() > 1 && cmd.substr(0, 1) == "-";
        if (remove)
            cmd = cmd.substr(1);

        uint32 spellId = chat->parseSpell(cmd);
        if (!spellId)
        {
            botAI->TellError("Unknown spell");
            return false;
        }

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
            return false;

        if (remove)
        {
            std::set<uint32>::iterator j = skipSpells.find(spellId);
            if (j != skipSpells.end())
            {
                skipSpells.erase(j);

                std::ostringstream out;
                out << chat->FormatSpell(spellInfo) << " removed from ignored spells";
                botAI->TellMaster(out);
                return true;
            }
        }
        else
        {
            std::set<uint32>::iterator j = skipSpells.find(spellId);
            if (j == skipSpells.end())
            {
                skipSpells.insert(spellId);

                std::ostringstream out;
                out << chat->FormatSpell(spellInfo) << " added to ignored spells";
                botAI->TellMaster(out);
                return true;
            }
        }
    }

    return false;
}

SpellIds SkipSpellsListAction::parseIds(std::string const text)
{
    SpellIds spellIds;

    uint8 pos = 0;
    while (pos < text.size())
    {
        int32 endPos = text.find(',', pos);
        if (endPos == -1)
            endPos = text.size();

        std::string const idC = text.substr(pos, endPos - pos);
        uint32 id = atol(idC.c_str());
        pos = endPos + 1;
        if (id)
            spellIds.insert(id);
    }

    return spellIds;
}
