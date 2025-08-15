/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */
 
#pragma once

#include <string>
#include <functional>
#include "Common.h"

class Player;
class PlayerbotAI;

namespace ai::buff
{
    std::string MakeAuraQualifierForBuff(std::string const& name);

    std::string GroupVariantFor(std::string const& name);

    bool HasRequiredReagents(Player* bot, uint32 spellId);

    std::string UpgradeToGroupIfAppropriate(
        Player* bot,
        PlayerbotAI* botAI,
        std::string const& baseName,
        bool announceOnMissing = false,
        std::function<void(std::string const&)> announce = {}
    );
}
