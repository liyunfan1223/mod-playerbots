/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "WorldBuffAction.h"

#include "AiFactory.h"
#include "Event.h"
#include "Playerbots.h"

bool WorldBuffAction::Execute(Event event)
{
    std::string const text = event.getParam();

    for (auto& wb : NeedWorldBuffs(bot))
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

    Player* player = unit->ToPlayer();
    if (!player)
        return retVec;

    uint8 playerClass = player->getClass();
    uint8 playerLevel = player->GetLevel();

    uint8 tab = AiFactory::GetPlayerSpecTab(player);

    // We'll store the final "effective" spec ID here.
    // For non-Feral druids (and all other classes),
    // effectiveSpec = tab by default.
    uint8 effectiveSpec = tab;

    // If this is a druid in the Feral tab, decide Bear vs. Cat
    if (playerClass == CLASS_DRUID && tab == 1)  // 1 = feral
    {
        bool isBear = player->HasTalent(16931, bot->GetActiveSpec()); // Thick Hide rank 3
        if (!isBear)
        {
            // If not bear, then treat it as "cat" spec = 4
            effectiveSpec = 4;
        }
        // If bear, effectiveSpec remains 1
    }

    for (auto const& wb : sPlayerbotAIConfig->worldBuffs)
    {
        // Faction check
        if (wb.factionId != 0 && wb.factionId != factionId)
            continue;

        // Class check
        if (wb.classId != 0 && wb.classId != playerClass)
            continue;

        // Level check
        if (wb.minLevel != 0 && wb.minLevel > playerLevel)
            continue;
        if (wb.maxLevel != 0 && wb.maxLevel < playerLevel)
            continue;

        // Already has aura?
        if (player->HasAura(wb.spellId))
            continue;

        // Final check: does the world-buff spec ID match our effective spec?
        if (wb.specId == effectiveSpec)
            retVec.push_back(wb.spellId);
    }

    return retVec;
}

