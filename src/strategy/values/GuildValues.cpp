/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GuildValues.h"

#include "Playerbots.h"

uint8 PetitionSignsValue::Calculate()
{
    if (bot->GetGuildId())
        return 0;

    std::vector<Item*> petitions = AI_VALUE2(std::vector<Item*>, "inventory items", chat->FormatQItem(5863));
    if (petitions.empty())
        return 0;

    QueryResult result = CharacterDatabase.Query("SELECT playerguid FROM petition_sign WHERE petitionguid = {}",
                                                 petitions.front()->GetGUID().GetCounter());
    return result ? (uint8)result->GetRowCount() : 0;
}
