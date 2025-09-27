/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RPGVALUES_H
#define _PLAYERBOT_RPGVALUES_H

#include "Value.h"

class PlayerbotAI;

class NextRpgActionValue : public ManualSetValue<std::string>
{
public:
    NextRpgActionValue(PlayerbotAI* botAI, std::string const defaultValue = "",
                       std::string const name = "next rpg action")
        : ManualSetValue(botAI, defaultValue, name){};
};

#endif
