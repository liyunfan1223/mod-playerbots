/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ShamanTriggers.h"

#include "Playerbots.h"

/*
std::vector<std::string> ShamanWeaponTrigger::spells;

bool ShamanWeaponTrigger::IsActive()
{
    if (spells.empty())
    {
        spells.push_back("frostbrand weapon");
        spells.push_back("rockbiter weapon");
        spells.push_back("flametongue weapon");
        spells.push_back("earthliving weapon");
        spells.push_back("windfury weapon");
    }

    for (std::vector<std::string>::iterator i = spells.begin(); i != spells.end(); ++i)
    {
        uint32 spellId = AI_VALUE2(uint32, "spell id", spell);
        if (!spellId)
            continue;

        if (AI_VALUE2(Item*, "item for spell", spellId))
            return true;
    }

    return false;
}
*/

bool MainHandWeaponNoImbueTrigger::IsActive()
{
    Item* const itemForSpell = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    if (!itemForSpell || itemForSpell->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
        return false;
    return true;
}

bool OffHandWeaponNoImbueTrigger::IsActive()
{
    Item* const itemForSpell = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
    if (!itemForSpell || itemForSpell->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT) ||
        itemForSpell->GetTemplate()->InventoryType != INVTYPE_WEAPON)
        return false;
    return true;
}

bool ShockTrigger::IsActive()
{
    return SpellTrigger::IsActive() && !botAI->HasAura("flame shock", GetTarget(), false, true) &&
           !botAI->HasAura("frost shock", GetTarget(), false, true);
}

bool TotemTrigger::IsActive()
{
    return AI_VALUE(uint8, "attacker count") >= attackerCount && !AI_VALUE2(bool, "has totem", name) &&
           !botAI->HasAura(name, bot);
}

bool ManaSpringTotemTrigger::IsActive()
{
    return AI_VALUE(uint8, "attacker count") >= attackerCount && !AI_VALUE2(bool, "has totem", "mana tide totem") &&
           !AI_VALUE2(bool, "has totem", name);
}

bool WaterWalkingTrigger::IsActive() { return BuffTrigger::IsActive() && AI_VALUE2(bool, "swimming", "self target"); }

bool WaterBreathingTrigger::IsActive() { return BuffTrigger::IsActive() && AI_VALUE2(bool, "swimming", "self target"); }

bool WaterWalkingOnPartyTrigger::IsActive()
{
    return BuffOnPartyTrigger::IsActive() && AI_VALUE2(bool, "swimming", "self target");
}

bool WaterBreathingOnPartyTrigger::IsActive()
{
    return BuffOnPartyTrigger::IsActive() && AI_VALUE2(bool, "swimming", "self target");
}

bool NoFireTotemTrigger::IsActive()
{
    return !AI_VALUE2(bool, "has totem", "magma totem") && !AI_VALUE2(bool, "has totem", "flametongue totem") &&
           !AI_VALUE2(bool, "has totem", "searing totem") && !AI_VALUE2(bool, "has totem", "fire elemental totem") &&
           !AI_VALUE2(bool, "has totem", "frost resistance totem") && !AI_VALUE2(bool, "has totem", "totem of wrath");
}

bool NoWaterTotemTrigger::IsActive()
{
    return !AI_VALUE2(bool, "has totem", "fire resistance totem") && !AI_VALUE2(bool, "has totem", "mana tide totem") &&
           !AI_VALUE2(bool, "has totem", "cleansing totem") && !AI_VALUE2(bool, "has totem", "mana spring totem") &&
           !AI_VALUE2(bool, "has totem", "healing stream totem");
}

bool NoAirTotemTrigger::IsActive()
{
    return !AI_VALUE2(bool, "has totem", "wrath of air totem") && !AI_VALUE2(bool, "has totem", "windfury totem");
}
