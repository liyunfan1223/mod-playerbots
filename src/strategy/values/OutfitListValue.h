/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_OUTFITLISTVALUE_H
#define _PLAYERBOT_OUTFITLISTVALUE_H

#include "Value.h"

class PlayerbotAI;

typedef std::vector<std::string> Outfit;

class OutfitListValue : public ManualSetValue<Outfit&>
{
public:
    OutfitListValue(PlayerbotAI* botAI, std::string const name = "outfit list")
        : ManualSetValue<Outfit&>(botAI, list, name)
    {
    }

    std::string const Save() override;
    bool Load(std::string const value) override;

private:
    Outfit list;
};

#endif
