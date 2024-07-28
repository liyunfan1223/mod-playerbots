/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "WorldBuffAction.h"

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
        return std::move(retVec);

    FactionTemplateEntry const* humanFaction = sFactionTemplateStore.LookupEntry(1);
    uint32 factionId =
        (Unit::GetFactionReactionTo(unit->GetFactionTemplateEntry(), humanFaction) >= REP_NEUTRAL) ? 1 : 2;

    for (auto& wb : sPlayerbotAIConfig->worldBuffs)
    {
        if (wb.factionId != 0 && wb.factionId != factionId)
            continue;

        if (wb.classId != 0 && wb.classId != unit->getClass())
            continue;

        if (wb.minLevel != 0 && wb.minLevel > unit->GetLevel())
            continue;

        if (wb.maxLevel != 0 && wb.maxLevel < unit->GetLevel())
            continue;

        if (unit->HasAura(wb.spellId))
            continue;

        retVec.push_back(wb.spellId);
    }

    return std::move(retVec);
}
