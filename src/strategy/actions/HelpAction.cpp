/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "HelpAction.h"

#include "ChatActionContext.h"
#include "Event.h"
#include "Playerbots.h"

HelpAction::HelpAction(PlayerbotAI* botAI) : Action(botAI, "help") { chatContext = new ChatActionContext(); }

HelpAction::~HelpAction() { delete chatContext; }

bool HelpAction::Execute(Event event)
{
    TellChatCommands();
    TellStrategies();
    return true;
}

void HelpAction::TellChatCommands()
{
    std::ostringstream out;
    out << "Whisper any of: ";
    out << CombineSupported(chatContext->supports());
    out << ", [item], [quest] or [object] link";
    botAI->TellError(out.str());
}

void HelpAction::TellStrategies()
{
    std::ostringstream out;
    out << "Possible strategies (co/nc/dead commands): ";
    out << CombineSupported(botAI->GetAiObjectContext()->GetSupportedStrategies());
    botAI->TellError(out.str());
}

std::string const HelpAction::CombineSupported(std::set<std::string> commands)
{
    std::ostringstream out;

    for (std::set<std::string>::iterator i = commands.begin(); i != commands.end();)
    {
        out << *i;
        if (++i != commands.end())
            out << ", ";
    }

    return out.str();
}
