/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_CUSTOMSTRATEGYEDITACTION_H
#define _PLAYERBOT_CUSTOMSTRATEGYEDITACTION_H

#include "Action.h"

class PlayerbotAI;

class CustomStrategyEditAction : public Action
{
public:
    CustomStrategyEditAction(PlayerbotAI* botAI) : Action(botAI, "cs") {}

    bool Execute(Event event) override;

private:
    bool PrintHelp();
    bool PrintActionLine(uint32 idx, std::string const command);
    bool Print(std::string const name);
    bool Edit(std::string const name, uint32 idx, std::string const command);
};

#endif
