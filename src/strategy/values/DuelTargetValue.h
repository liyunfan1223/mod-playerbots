/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_DUELTARGETVALUE_H
#define _PLAYERBOT_DUELTARGETVALUE_H

#include "TargetValue.h"

class PlayerbotAI;
class Unit;

class DuelTargetValue : public TargetValue
{
public:
    DuelTargetValue(PlayerbotAI* botAI, std::string const name = "duel target") : TargetValue(botAI, name) {}

    Unit* Calculate() override;
};

#endif
