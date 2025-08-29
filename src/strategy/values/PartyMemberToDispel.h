/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PARTYMEMBERTODISPEL_H
#define _PLAYERBOT_PARTYMEMBERTODISPEL_H

#include "NamedObjectContext.h"
#include "PartyMemberValue.h"

class PlayerbotAI;
class Unit;

class PartyMemberToDispel : public PartyMemberValue, public Qualified
{
public:
    PartyMemberToDispel(PlayerbotAI* botAI, std::string const name = "party member to dispel")
        : PartyMemberValue(botAI, name, 1000), Qualified()
    {
    }

protected:
    Unit* Calculate() override;
};

#endif
