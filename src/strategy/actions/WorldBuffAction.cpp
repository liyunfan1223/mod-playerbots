/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "WorldBuffAction.h"
#include "AiFactory.h"
#include "Event.h"
#include "Playerbots.h"

bool WorldBuffAction::Execute(Event event)
{
    std::string const text = event.getParam();

    std::vector<uint32> buffs = NeedWorldBuffs(bot);  // Get matching buffs

    for (auto& wb : buffs)
    {
        bot->AddAura(wb, bot);
    }
    return false;
}

std::vector<uint32> WorldBuffAction::NeedWorldBuffs(Unit* unit)
{
    std::vector<uint32> retVec;

    if (sPlayerbotAIConfig->worldBuffs.empty())
        return retVec;

    FactionTemplateEntry const* humanFaction = sFactionTemplateStore.LookupEntry(1);
    uint32 factionId =
        (Unit::GetFactionReactionTo(unit->GetFactionTemplateEntry(), humanFaction) >= REP_NEUTRAL) ? 1 : 2;

    Player* bot = unit->ToPlayer();
    if (!bot)
        return retVec;

    uint8 botClass = bot->getClass();
    uint8 botLevel = bot->GetLevel();

    uint8 tab = AiFactory::GetPlayerSpecTab(bot);

    // We'll store the final "effective" spec ID here.
    // For non-Feral druids (and all other classes),
    // effectiveSpec = tab by default.
    uint8 effectiveSpec = tab;

    // If this is a druid in the Feral tab, decide Bear vs. Cat
    if (botClass == CLASS_DRUID && tab == 1)  // 1 = feral
    {
        bool isBear = bot->HasTalent(16931, bot->GetActiveSpec()); // Thick Hide rank 3
        if (!isBear)
        {
            // If not bear, then treat it as "cat" spec = 3
            effectiveSpec = 3;
        }
        // If bear, effectiveSpec remains 1
    }

    // If this is a Death Knight in the Blood tab, decide Tank vs. DPS
    if (botClass == CLASS_DEATH_KNIGHT && tab == 0)  // 0 = Blood
    {
        bool isTank = bot->HasTalent(55226, bot->GetActiveSpec()); // Blade Barrier rank 5
        if (!isTank)
        {
            // If not tank, then treat it as DPS spec = 3
            effectiveSpec = 3;
        }
        // If tank, effectiveSpec remains unchanged
    }

    for (auto const& wb : sPlayerbotAIConfig->worldBuffs)
    {
        // Faction check
        if (wb.factionId != 0 && wb.factionId != factionId)
            continue;

        // Class check
        if (wb.classId != 0 && wb.classId != botClass)
            continue;

        // Level check
        if (wb.minLevel != 0 && wb.minLevel > botLevel)
            continue;
        if (wb.maxLevel != 0 && wb.maxLevel < botLevel)
            continue;

        // Already has aura?
        if (bot->HasAura(wb.spellId))
            continue;

        // Final check: does the world-buff spec ID match our effective spec?
        if (wb.specId == effectiveSpec)
            retVec.push_back(wb.spellId);
    }

    return retVec;
}

