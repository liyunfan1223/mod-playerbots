/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "MageTriggers.h"

#include "MageActions.h"
#include "Playerbots.h"

bool ArcaneIntellectOnPartyTrigger::IsActive()
{
    return BuffOnPartyTrigger::IsActive() && !botAI->HasAura("arcane brilliance", GetTarget());
}

bool ArcaneIntellectTrigger::IsActive()
{
    return BuffTrigger::IsActive() && !botAI->HasAura("arcane brilliance", GetTarget());
}

bool MageArmorTrigger::IsActive()
{
    Unit* target = GetTarget();
    return !botAI->HasAura("ice armor", target) && !botAI->HasAura("frost armor", target) &&
           !botAI->HasAura("molten armor", target) && !botAI->HasAura("mage armor", target);
}

bool FingersOfFrostSingleTrigger::IsActive()
{
    // Fingers of Frost "stack" count is always 1.
    // The value is instead stored in the charges.
    Aura* aura = botAI->GetAura("fingers of frost", bot, false, true, -1);
    return (aura && aura->GetCharges() == 1);
}

bool ArcaneBlastStackTrigger::IsActive()
{
    Aura* aura = botAI->GetAura(getName(), GetTarget(), false, true, 3);
    if (!aura)
        return false;
    if (aura->GetStackAmount() >= 4)
        return true;
    bool hasMissileBarrage = botAI->HasAura(44401, bot);
    return hasMissileBarrage;
}

bool FrostNovaOnTargetTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target || !target->IsAlive() || !target->IsInWorld())
    {
        return false;
    }
    return botAI->HasAura(spell, target);
}

bool FrostbiteOnTargetTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target || !target->IsAlive() || !target->IsInWorld())
    {
        return false;
    }
    return botAI->HasAura(spell, target);
}

bool NoFocusMagicTrigger::IsActive()
{
    if (!bot->HasSpell(54646))
        return false;
    
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive())
            continue;

        if (member->HasAura(54646, bot->GetGUID()))
            return false;
    }
    return true;
}
