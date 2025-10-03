/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "CureTriggers.h"

#include "Playerbots.h"
#include "WorldBuffAction.h"

bool NeedCureTrigger::IsActive()
{
    Unit* target = GetTarget();
    return target && target->IsInWorld() && botAI->HasAuraToDispel(target, dispelType);
}

Value<Unit*>* PartyMemberNeedCureTrigger::GetTargetValue()
{
    return context->GetValue<Unit*>("party member to dispel", dispelType);
}

bool PartyMemberNeedCureTrigger::IsActive()
{
    Unit* target = GetTarget();
    return target && target->IsInWorld();
}

bool NeedWorldBuffTrigger::IsActive() { return !WorldBuffAction::NeedWorldBuffs(bot).empty(); }
