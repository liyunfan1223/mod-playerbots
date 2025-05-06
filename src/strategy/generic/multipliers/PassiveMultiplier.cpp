/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PassiveMultiplier.h"

#include "Action.h"
#include "AiObjectContext.h"

std::vector<std::string> PassiveMultiplier::allowedActions;
std::vector<std::string> PassiveMultiplier::allowedParts;

PassiveMultiplier::PassiveMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "passive")
{
    if (allowedActions.empty())
    {
        allowedActions.push_back("co");
        allowedActions.push_back("nc");
        allowedActions.push_back("reset botAI");
        allowedActions.push_back("check mount state");
    }

    if (allowedParts.empty())
    {
        allowedParts.push_back("follow");
        allowedParts.push_back("move from group");
        allowedParts.push_back("stay");
        allowedParts.push_back("chat shortcut");
    }
}

float PassiveMultiplier::GetValue(Action* action)
{
    if (!action)
        return 1.0f;

    std::string const name = action->getName();

    for (std::vector<std::string>::iterator i = allowedActions.begin(); i != allowedActions.end(); i++)
    {
        if (name == *i)
            return 1.0f;
    }

    for (std::vector<std::string>::iterator i = allowedParts.begin(); i != allowedParts.end(); i++)
    {
        if (name.find(*i) != std::string::npos)
            return 1.0f;
    }

    return 0;
}
