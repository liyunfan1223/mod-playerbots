/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "MageActions.h"

#include "Playerbots.h"
#include "ServerFacade.h"

Value<Unit*>* CastPolymorphAction::GetTargetValue() { return context->GetValue<Unit*>("cc target", getName()); }

bool CastFrostNovaAction::isUseful()
{
    return sServerFacade->IsDistanceLessOrEqualThan(AI_VALUE2(float, "distance", GetTargetName()), 10.f);
}

bool CastConeOfColdAction::isUseful()
{
    bool facingTarget = AI_VALUE2(bool, "facing", "current target");
    bool targetClose = sServerFacade->IsDistanceLessOrEqualThan(AI_VALUE2(float, "distance", GetTargetName()), 10.f);
    return facingTarget && targetClose;
}

bool CastArcaneIntellectOnPartyAction::Execute(Event event)
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    Group* group = botAI->GetBot()->GetGroup();

    if (group)
    {
        if (botAI->CanCastSpell("dalaran brilliance", target))
            return botAI->CastSpell("dalaran brilliance", target);

        if (botAI->CanCastSpell("arcane brilliance", target))
            return botAI->CastSpell("arcane brilliance", target);
    }

    // If not in a group or we cannot cast brilliance, fall back to arcane intellect
    return botAI->CastSpell("arcane intellect", target);
}
