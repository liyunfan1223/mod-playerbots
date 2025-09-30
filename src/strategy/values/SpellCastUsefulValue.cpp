/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "SpellCastUsefulValue.h"

#include "LastSpellCastValue.h"
#include "Playerbots.h"

bool SpellCastUsefulValue::Calculate()
{
    uint32 spellid = AI_VALUE2(uint32, "spell id", qualifier);
    if (!spellid)
        return true;  // there can be known alternatives

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellid);
    if (!spellInfo)
        return true;  // there can be known alternatives

    if ((spellInfo->Attributes & SPELL_ATTR0_ON_NEXT_SWING_NO_DAMAGE) != 0 ||
        (spellInfo->Attributes & SPELL_ATTR0_ON_NEXT_SWING) != 0)
    {
        if (Spell* spell = bot->GetCurrentSpell(CURRENT_MELEE_SPELL))
            if (spell->m_spellInfo->Id == spellid && spell->IsNextMeleeSwingSpell() &&
                bot->HasUnitState(UNIT_STATE_MELEE_ATTACKING))
                return false;
    }
    else
    {
        // uint32 lastSpellId = AI_VALUE(LastSpellCast&, "last spell cast").id;
        // if (spellid == lastSpellId)
        //     if (Spell* const pSpell = bot->FindCurrentSpellBySpellId(lastSpellId))
        //         return false;
    }

    if (spellInfo->IsAutoRepeatRangedSpell() && bot->GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL) &&
        bot->GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL)->m_spellInfo->Id == spellid)
    {
        return false;
    }

    // TODO: workaround
    if (qualifier == "windfury weapon" || qualifier == "flametongue weapon" || qualifier == "frostbrand weapon" ||
        qualifier == "rockbiter weapon" || qualifier == "earthliving weapon" || qualifier == "spellstone")
    {
        if (Item* item = AI_VALUE2(Item*, "item for spell", spellid))
            if (item->IsInWorld() && item->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
                return false;
    }

    std::set<uint32>& skipSpells = AI_VALUE(std::set<uint32>&, "skip spells list");
    if (skipSpells.find(spellid) != skipSpells.end())
        return false;

    std::string const spellName = spellInfo->SpellName[0];
    for (uint32 skipSpellId : skipSpells)
    {
        SpellInfo const* skipSpellInfo = sSpellMgr->GetSpellInfo(skipSpellId);
        if (!skipSpellInfo)
            continue;

        std::wstring wnamepart;
        if (!Utf8toWStr(skipSpellInfo->SpellName[0], wnamepart))
            continue;

        wstrToLower(wnamepart);
        if (!spellName.empty() && spellName.length() == wnamepart.length() && Utf8FitTo(spellName, wnamepart))
            return false;
    }

    return true;
}
