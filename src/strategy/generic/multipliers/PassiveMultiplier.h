/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PASSIVEMULTIPLIER_H
#define _PLAYERBOT_PASSIVEMULTIPLIER_H

#include <vector>

#include "Multiplier.h"

class Action;
class PlayerbotAI;

class PassiveMultiplier : public Multiplier
{
public:
    PassiveMultiplier(PlayerbotAI* botAI);

    float GetValue(Action* action) override;

private:
    static std::vector<std::string> allowedActions;
    static std::vector<std::string> allowedParts;
};

#endif
