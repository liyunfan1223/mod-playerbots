/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "AttackerWithoutAuraTargetValue.h"
#include "Playerbots.h"

Unit* AttackerWithoutAuraTargetValue::Calculate()
{
    GuidVector attackers = botAI->GetAiObjectContext()->GetValue<GuidVector >("attackers")->Get();
    Unit* target = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    for (ObjectGuid const guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || unit == target)
            continue;

        if (bot->GetDistance(unit) > botAI->GetRange("spell"))
            continue;

        if (!botAI->HasAura(qualifier, unit, false, true))
            return unit;
    }

    return nullptr;
}
