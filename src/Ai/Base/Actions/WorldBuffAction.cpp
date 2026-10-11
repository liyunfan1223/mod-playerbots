/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
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

    if (sPlayerbotAIConfig.WorldBuffs.empty())
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

    for (auto const& wb : sPlayerbotAIConfig.WorldBuffs)
    {
        // Faction check
        if (wb.FactionId != 0 && wb.FactionId != factionId)
            continue;

        // Class check
        if (wb.ClassId != 0 && wb.ClassId != botClass)
            continue;

        // Level check
        if (wb.MinLevel != 0 && wb.MinLevel > botLevel)
            continue;
        if (wb.MaxLevel != 0 && wb.MaxLevel < botLevel)
            continue;

        // Already has aura?
        if (bot->HasAura(wb.SpellId))
            continue;

        // Final check: does the world-buff spec ID match our effective spec?
        if (wb.SpecId == effectiveSpec)
            retVec.push_back(wb.SpellId);
    }

    return retVec;
}
