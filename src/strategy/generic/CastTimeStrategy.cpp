/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "CastTimeStrategy.h"
#include "GenericSpellActions.h"
#include "Playerbots.h"

float CastTimeMultiplier::GetValue(Action* action)
{
    if (action == nullptr)
        return 1.0f;

    uint8 targetHealth = AI_VALUE2(uint8, "health", "current target");
    std::string const name = action->getName();

    if (action->GetTarget() != AI_VALUE(Unit*, "current target"))
        return 1.0f;

    if (targetHealth < sPlayerbotAIConfig->criticalHealth && dynamic_cast<CastSpellAction*>(action))
    {
        uint32 spellId = AI_VALUE2(uint32, "spell id", name);
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
            return 1.0f;

        if ((spellInfo->Targets & TARGET_FLAG_DEST_LOCATION) != 0|| (spellInfo->Targets & TARGET_FLAG_SOURCE_LOCATION) != 0)
            return 1.0f;

        uint32 castTime = spellInfo->CalcCastTime();
        // if (castTime >= 3000)
        //     return 0.0f;

        if (castTime >= 1500)
            return 0.5f;

        if (castTime >= 1000)
            return 0.25f;
    }

    return 1.0f;
}

void CastTimeStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    multipliers.push_back(new CastTimeMultiplier(botAI));
}
