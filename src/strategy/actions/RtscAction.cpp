/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RtscAction.h"

#include "Playerbots.h"
#include "RTSCValues.h"

bool RTSCAction::Execute(Event event)
{
    std::string const command = event.getParam();

    Player* master = botAI->GetMaster();

    if (!master)
        return false;

    if (command != "reset" && !master->HasSpell(RTSC_MOVE_SPELL))
    {
        master->learnSpell(RTSC_MOVE_SPELL, false);
        botAI->TellMasterNoFacing("RTS control enabled.");
        botAI->TellMasterNoFacing("Aedm (Awesome energetic do move) spell trained.");
    }
    else if (command == "reset")
    {
        if (master->HasSpell(RTSC_MOVE_SPELL))
        {
            master->removeSpell(RTSC_MOVE_SPELL, SPEC_MASK_ALL, false);
            botAI->TellMasterNoFacing("RTS control spell removed.");
        }

        RESET_AI_VALUE(bool, "RTSC selected");
        RESET_AI_VALUE(std::string, "RTSC next spell action");

        for (auto value : botAI->GetAiObjectContext()->GetValues())
            if (value.find("RTSC saved location::") != std::string::npos)
                RESET_AI_VALUE(WorldPosition, value.c_str());

        return true;
    }

    bool selected = AI_VALUE(bool, "RTSC selected");

    if (command == "select" && !selected)
    {
        SET_AI_VALUE(bool, "RTSC selected", true);
        master->SendPlaySpellVisual(bot->GetGUID(), 5036);
        return true;
    }
    else if (command == "cancel")
    {
        RESET_AI_VALUE(bool, "RTSC selected");
        RESET_AI_VALUE(std::string, "RTSC next spell action");
        if (selected)
            master->SendPlaySpellVisual(bot->GetGUID(), 6372);
        return true;
    }
    else if (command == "toggle")
    {
        if (!selected)
        {
            SET_AI_VALUE(bool, "RTSC selected", true);
            master->SendPlaySpellVisual(bot->GetGUID(), 5036);
        }
        else
        {
            SET_AI_VALUE(bool, "RTSC selected", false);
            master->SendPlaySpellVisual(bot->GetGUID(), 6372);
        }

        return true;
    }
    else if (command.find("save here ") != std::string::npos)
    {
        std::string const locationName = command.substr(10);

        WorldPosition spellPosition(bot);
        SET_AI_VALUE2(WorldPosition, "RTSC saved location", locationName, spellPosition);

        Creature* wpCreature =
            bot->SummonCreature(15631, spellPosition.getX(), spellPosition.getY(), spellPosition.getZ(),
                                spellPosition.getO(), TEMPSUMMON_TIMED_DESPAWN, 2000.0f);
        wpCreature->SetObjectScale(0.5f);

        return true;
    }
    else if (command.find("unsave ") != std::string::npos)
    {
        std::string const locationName = command.substr(7);

        RESET_AI_VALUE2(WorldPosition, "RTSC saved location", locationName);

        return true;
    }

    if (command.find("save ") != std::string::npos || command == "move")
    {
        SET_AI_VALUE(std::string, "RTSC next spell action", command);

        return true;
    }

    if (command.find("show ") != std::string::npos)
    {
        std::string const locationName = command.substr(5);
        WorldPosition spellPosition = AI_VALUE2(WorldPosition, "RTSC saved location", locationName);

        if (spellPosition)
        {
            Creature* wpCreature =
                bot->SummonCreature(15631, spellPosition.getX(), spellPosition.getY(), spellPosition.getZ(),
                                    spellPosition.getO(), TEMPSUMMON_TIMED_DESPAWN, 2000.0f);
            wpCreature->SetObjectScale(0.5f);
        }

        return true;
    }

    if (command.find("show") != std::string::npos)
    {
        std::ostringstream out;
        out << "saved: ";

        for (auto value : botAI->GetAiObjectContext()->GetValues())
            if (value.find("RTSC saved location::") != std::string::npos)
                if (AI_VALUE2(WorldPosition, "RTSC saved location", value.substr(21).c_str()))
                    out << value.substr(21).c_str() << ",";

        out.seekp(-1, out.cur);
        out << ".";

        botAI->TellMasterNoFacing(out);
    }

    if (command.find("go ") != std::string::npos)
    {
        std::string const locationName = command.substr(3);
        WorldPosition spellPosition = AI_VALUE2(WorldPosition, "RTSC saved location", locationName);

        if (spellPosition)
            return MoveToSpell(spellPosition, false);

        return true;
    }
    else if (command == "last")
    {
        WorldPosition spellPosition = AI_VALUE(WorldPosition, "see spell location");
        if (spellPosition)
            return MoveToSpell(spellPosition);
    }

    return false;
}
