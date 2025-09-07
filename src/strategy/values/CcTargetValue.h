/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_CCTARGETVALUE_H
#define _PLAYERBOT_CCTARGETVALUE_H

#include "NamedObjectContext.h"
#include "TargetValue.h"

class PlayerbotAI;
class Unit;

class CcTargetValue : public TargetValue, public Qualified
{
public:
    CcTargetValue(PlayerbotAI* botAI, std::string const name = "cc target") : TargetValue(botAI, name) {}

    Unit* Calculate() override;
};

#endif
