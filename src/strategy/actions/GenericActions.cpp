/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GenericActions.h"

#include "CreatureAI.h"
#include "Playerbots.h"

enum PetSpells
{
    PET_PROWL_1 = 24450,
    PET_PROWL_2 = 24452,
    PET_PROWL_3 = 24453,
    PET_COWER = 1742,
    PET_LEAP = 47482,
    PET_SPELL_LOCK_1 = 19244,
    PET_SPELL_LOCK_2 = 19647,
    PET_DEVOUR_MAGIC_1 = 19505,
    PET_DEVOUR_MAGIC_2 = 19731,
    PET_DEVOUR_MAGIC_3 = 19734,
    PET_DEVOUR_MAGIC_4 = 19736,
    PET_DEVOUR_MAGIC_5 = 27276,
    PET_DEVOUR_MAGIC_6 = 27277,
    PET_DEVOUR_MAGIC_7 = 48011
};

static std::vector<uint32> disabledPetSpells = {
    PET_PROWL_1, PET_PROWL_2, PET_PROWL_3,
    PET_COWER, PET_LEAP,
    PET_SPELL_LOCK_1, PET_SPELL_LOCK_2,
    PET_DEVOUR_MAGIC_1, PET_DEVOUR_MAGIC_2, PET_DEVOUR_MAGIC_3,
    PET_DEVOUR_MAGIC_4, PET_DEVOUR_MAGIC_5, PET_DEVOUR_MAGIC_6, PET_DEVOUR_MAGIC_7
};

bool MeleeAction::isUseful()
{
    // do not allow if can't attack from vehicle
    if (botAI->IsInVehicle() && !botAI->IsInVehicle(false, false, true))
        return false;

    return true;
}

bool TogglePetSpellAutoCastAction::Execute(Event event)
{
    Pet* pet = bot->GetPet();
    if (!pet)
    {
        return false;
    }
    // hack on high level spell after low level initialization
    std::vector<unsigned int> shouldRemove;
    for (unsigned int& m_autospell : pet->m_autospells)
    {
        if (!pet->HasSpell(m_autospell))
        {
            shouldRemove.push_back(m_autospell);
        }
    }
    for (unsigned int spellId : shouldRemove)
    {
        auto autospellItr = std::find(pet->m_autospells.begin(), pet->m_autospells.end(), spellId);
        if (autospellItr != pet->m_autospells.end())
            pet->m_autospells.erase(autospellItr);
    }
    bool toggled = false;
    for (PetSpellMap::const_iterator itr = pet->m_spells.begin(); itr != pet->m_spells.end(); ++itr)
    {
        if (itr->second.state == PETSPELL_REMOVED)
            continue;

        uint32 spellId = itr->first;
        const SpellInfo* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo->IsAutocastable())
            continue;

        bool shouldApply = true;
        for (uint32 disabledSpell : disabledPetSpells)
        {
            if (spellId == disabledSpell)
            {
                shouldApply = false;
                break;
            }
        }
        bool isAutoCast = false;
        for (unsigned int& m_autospell : pet->m_autospells)
        {
            if (m_autospell == spellId)
            {
                isAutoCast = true;
                break;
            }
        }
        if (shouldApply != isAutoCast)
        {
            pet->ToggleAutocast(spellInfo, shouldApply);
            toggled = true;
        }
    }
    return toggled;
}

bool PetAttackAction::Execute(Event event)
{
    Guardian* pet = bot->GetGuardianPet();
    if (!pet)
    {
        return false;
    }

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
    {
        return false;
    }

    if (!bot->IsValidAttackTarget(target))
    {
        return false;
    }

    pet->SetReactState(REACT_PASSIVE);
    pet->ClearUnitState(UNIT_STATE_FOLLOW);
    pet->AttackStop();
    pet->SetTarget(target->GetGUID());

    pet->GetCharmInfo()->SetIsCommandAttack(true);
    pet->GetCharmInfo()->SetIsAtStay(false);
    pet->GetCharmInfo()->SetIsFollowing(false);
    pet->GetCharmInfo()->SetIsCommandFollow(false);
    pet->GetCharmInfo()->SetIsReturning(false);

    pet->ToCreature()->AI()->AttackStart(target);
    return true;
}
