/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PossibleTargetsValue.h"

#include "AttackersValue.h"
#include "CellImpl.h"
#include "DBCStructure.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Playerbots.h"
#include "SharedDefines.h"
#include "SpellAuraDefines.h"
#include "SpellAuraEffects.h"
#include "SpellMgr.h"
#include "Unit.h"

void PossibleTargetsValue::FindUnits(std::list<Unit*>& targets)
{
    Acore::AnyUnfriendlyUnitInObjectRangeCheck u_check(bot, bot, range);
    Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, range);
}

bool PossibleTargetsValue::AcceptUnit(Unit* unit) { return AttackersValue::IsPossibleTarget(unit, bot, range); }

void PossibleTriggersValue::FindUnits(std::list<Unit*>& targets)
{
    Acore::AnyUnfriendlyUnitInObjectRangeCheck u_check(bot, bot, range);
    Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, range);
}

bool PossibleTriggersValue::AcceptUnit(Unit* unit)
{
    if (!unit->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
    {
        return false;
    }
    Unit::AuraEffectList const& aurasPeriodicTriggerSpell =
        unit->GetAuraEffectsByType(SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    Unit::AuraEffectList const& aurasPeriodicTriggerWithValueSpell =
        unit->GetAuraEffectsByType(SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE);
    for (const Unit::AuraEffectList& list : {aurasPeriodicTriggerSpell, aurasPeriodicTriggerWithValueSpell})
    {
        for (auto i = list.begin(); i != list.end(); ++i)
        {
            AuraEffect* aurEff = *i;
            const SpellInfo* spellInfo = aurEff->GetSpellInfo();
            if (!spellInfo)
                continue;
            const SpellInfo* triggerSpellInfo =
                sSpellMgr->GetSpellInfo(spellInfo->Effects[aurEff->GetEffIndex()].TriggerSpell);
            if (!triggerSpellInfo)
                continue;
            for (int j = 0; j < MAX_SPELL_EFFECTS; j++)
            {
                if (triggerSpellInfo->Effects[j].Effect == SPELL_EFFECT_SCHOOL_DAMAGE)
                {
                    return true;
                }
            }
        }
    }
    return false;
}
