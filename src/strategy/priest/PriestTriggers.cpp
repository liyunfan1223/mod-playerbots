/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PriestTriggers.h"

#include "Playerbots.h"

bool PowerWordFortitudeOnPartyTrigger::IsActive()
{
    return BuffOnPartyTrigger::IsActive() && !botAI->HasAura("power word : fortitude", GetTarget()) &&
           !botAI->HasAura("prayer of fortitude", GetTarget());
}

bool PowerWordFortitudeTrigger::IsActive()
{
    return BuffTrigger::IsActive() && !botAI->HasAura("power word: fortitude", GetTarget()) &&
           !botAI->HasAura("prayer of fortitude", GetTarget());
}

bool DivineSpiritOnPartyTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target || !target->IsPlayer() || target->GetLevel() < 20)
        return false;
    return BuffOnPartyTrigger::IsActive() && !botAI->HasAura("divine spirit", target) &&
           !botAI->HasAura("prayer of spirit", target);
}

bool DivineSpiritTrigger::IsActive()
{
    return BuffTrigger::IsActive() && !botAI->HasAura("divine spirit", GetTarget()) &&
           !botAI->HasAura("prayer of spirit", GetTarget());
}

bool PrayerOfFortitudeTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target || !target->IsPlayer() || target->GetLevel() < 40)
        return false;
    Group* group = botAI->GetBot()->GetGroup();
    if (!group)
        return false;

    return BuffOnPartyTrigger::IsActive() && !botAI->HasAura("power word: fortitude", target) &&
           group->GetMembersCount() - botAI->GetBuffedCount((Player*)GetTarget(), "prayer of fortitude") > 3;
}

bool PrayerOfSpiritTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target || !target->IsPlayer() || target->GetLevel() < 40)
        return false;
    Group* group = botAI->GetBot()->GetGroup();
    if (!group)
        return false;
    
    return BuffOnPartyTrigger::IsActive() && !botAI->HasAura("divine spirit", target) &&
           // botAI->GetManaPercent() > 50 &&
           group->GetMembersCount() - botAI->GetBuffedCount((Player*)GetTarget(), "prayer of spirit") > 3;
}

bool InnerFireTrigger::IsActive()
{
    Unit* target = GetTarget();
    return SpellTrigger::IsActive() && !botAI->HasAura(spell, target);
}

bool ShadowformTrigger::IsActive() { return !botAI->HasAura("shadowform", bot); }

bool ShadowfiendTrigger::IsActive() { return BoostTrigger::IsActive() && !bot->HasSpellCooldown(34433); }

BindingHealTrigger::BindingHealTrigger(PlayerbotAI* botAI)
    : PartyMemberLowHealthTrigger(botAI, "binding heal", sPlayerbotAIConfig->lowHealth, 0)
{
}

bool BindingHealTrigger::IsActive()
{
    return PartyMemberLowHealthTrigger::IsActive() &&
           AI_VALUE2(uint8, "health", "self target") < sPlayerbotAIConfig->mediumHealth;
}
