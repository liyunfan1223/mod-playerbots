/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "WarlockTriggers.h"
#include "GenericTriggers.h"
#include "Playerbots.h"

bool SpellstoneTrigger::IsActive() { return BuffTrigger::IsActive() && AI_VALUE2(uint32, "item count", getName()) > 0; }

bool FirestoneTrigger::IsActive() { return BuffTrigger::IsActive() && AI_VALUE2(uint32, "item count", getName()) > 0; }

bool WarlockConjuredItemTrigger::IsActive()
{
    return ItemCountTrigger::IsActive() && AI_VALUE2(uint32, "item count", "soul shard") > 0;
}

// Checks if the target marked with the moon icon can be banished
bool BanishTrigger::IsActive()
{
    Unit* ccTarget = context->GetValue<Unit*>("cc target", "banish")->Get();
    Unit* moonTarget = context->GetValue<Unit*>("rti cc target")->Get();
    return ccTarget && moonTarget && ccTarget == moonTarget && HasCcTargetTrigger::IsActive();
}

// Checks if the target marked with the moon icon can be feared
bool FearTrigger::IsActive()
{
    Unit* ccTarget = context->GetValue<Unit*>("cc target", "fear")->Get();
    Unit* moonTarget = context->GetValue<Unit*>("rti cc target")->Get();
    return ccTarget && moonTarget && ccTarget == moonTarget && HasCcTargetTrigger::IsActive();
}

bool DemonArmorTrigger::IsActive()
{
    Unit* target = GetTarget();
    return !botAI->HasAura("demon skin", target) && !botAI->HasAura("demon armor", target) &&
           !botAI->HasAura("fel armor", target);
}

bool SoulLinkTrigger::IsActive()
{
    Unit* target = GetTarget();
    return !botAI->HasAura("soul link", target);
}

bool DemonicEmpowermentTrigger::IsActive()
{
    Pet* pet = bot->GetPet();
    if (!pet)
        return false;
    return !botAI->HasAura("demonic empowerment", pet);
}

bool DecimationTrigger::IsActive()
{
    Aura* aura = botAI->GetAura(getName(), GetTarget(), false, true);
    return aura && aura->GetDuration() > 3000;
}

// Checks if the bot's mana is below 85% and health is above a low health threshold
bool LifeTapTrigger::IsActive()
{
    if (AI_VALUE2(uint8, "health", "self target") <= sPlayerbotAIConfig->lowHealth)
        return false;

    if (!AI_VALUE2(bool, "has mana", "self target"))
        return false;
    if (AI_VALUE2(uint8, "mana", "self target") >= 85)
        return false;

    return true;
}

// Checks if the Life Tap Glyph buff is active
bool LifeTapGlyphBuffTrigger::IsActive()
{
    if (!botAI->HasAura(63320, bot))
        return false;

    return BuffTrigger::IsActive();
}

// Checks if the target has a conflicting debuff that is equal to Curse of the Elements
bool CurseOfTheElementsTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target || !target->IsAlive() || !target->IsInWorld())
        return false;

    // List of all spell IDs for Ebon Plague, Earth and Moon, and Curse of the Elements
    static const uint32 CurseOfTheElementsExclusiveDebuffs[] = {// Ebon Plague
                                                                51735, 51734, 51726,
                                                                // Earth and Moon
                                                                48511, 48513, 48514,
                                                                // Curse of the Elements
                                                                1490, 11721, 11722, 27228, 47865};

    // Check if target has any of the exclusive debuffs
    for (uint32 spellId : CurseOfTheElementsExclusiveDebuffs)
    {
        if (target->HasAura(spellId))
            return false;
    }

    // Use default BuffTrigger logic for the rest (only trigger if debuff is missing or expiring)
    return BuffTrigger::IsActive();
}
