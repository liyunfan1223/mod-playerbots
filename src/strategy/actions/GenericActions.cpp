/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GenericActions.h"

#include "CreatureAI.h"
#include "Playerbots.h"

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
    bool toggled = false;
    for (PetSpellMap::const_iterator itr = pet->m_spells.begin(); itr != pet->m_spells.end(); ++itr)
    {
        if (itr->second.state == PETSPELL_REMOVED)
            continue;

        uint32 spellId = itr->first;
        const SpellInfo* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (spellInfo->IsPassive())
            continue;

        bool shouldApply = true;
        // imp's spell, felhunte's intelligence, cat stealth
        if (spellId == 4511 || spellId == 1742 || spellId == 54424 || spellId == 57564 || spellId == 57565 ||
            spellId == 57566 || spellId == 57567 || spellId == 24450)
        {
            shouldApply = false;
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