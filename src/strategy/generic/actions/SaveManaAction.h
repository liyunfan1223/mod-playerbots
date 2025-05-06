/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_SAVEMANAACTION_H
#define _PLAYERBOT_SAVEMANAACTION_H

#include "Action.h"

class PlayerbotAI;

class SaveManaAction : public Action
{
public:
    SaveManaAction(PlayerbotAI* botAI) : Action(botAI, "save mana") {}

    bool Execute(Event event) override;

private:
    std::string const Format(double value);
};

#endif
