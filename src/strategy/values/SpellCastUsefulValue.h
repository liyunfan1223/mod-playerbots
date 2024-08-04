/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_SPELLCASTUSEFULVALUE_H
#define _PLAYERBOT_SPELLCASTUSEFULVALUE_H

#include "NamedObjectContext.h"
#include "Value.h"

class PlayerbotAI;

class SpellCastUsefulValue : public BoolCalculatedValue, public Qualified
{
public:
    SpellCastUsefulValue(PlayerbotAI* botAI, std::string const name = "spell cast useful")
        : BoolCalculatedValue(botAI, name)
    {
    }

    bool Calculate() override;
};

#endif
