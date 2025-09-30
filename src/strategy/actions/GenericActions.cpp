/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericActions.h"
#include "PlayerbotAI.h"
#include "Player.h"
#include "Pet.h"
#include "PlayerbotAIConfig.h"
#include "CreatureAI.h"
#include "Playerbots.h"
#include "CharmInfo.h"
#include "SharedDefines.h"
#include "ObjectGuid.h"
#include "SpellMgr.h"
#include "SpellInfo.h"
#include <vector>
#include <algorithm>

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
    PET_DEVOUR_MAGIC_7 = 48011,
    PET_SPIRIT_WOLF_LEAP = 58867
};

static std::vector<uint32> disabledPetSpells = {
    PET_PROWL_1, PET_PROWL_2, PET_PROWL_3,
    PET_COWER, PET_LEAP,
    PET_SPELL_LOCK_1, PET_SPELL_LOCK_2,
    PET_DEVOUR_MAGIC_1, PET_DEVOUR_MAGIC_2, PET_DEVOUR_MAGIC_3,
    PET_DEVOUR_MAGIC_4, PET_DEVOUR_MAGIC_5, PET_DEVOUR_MAGIC_6, PET_DEVOUR_MAGIC_7, PET_SPIRIT_WOLF_LEAP
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

    // Debug message if pet spells have been toggled and debug is enabled
    if (toggled && sPlayerbotAIConfig->petChatCommandDebug == 1)
        botAI->TellMaster("Pet autocast spells have been toggled.");

    return toggled;
}

bool PetAttackAction::Execute(Event event)
{
    Guardian* pet = bot->GetGuardianPet();
    if (!pet)
        return false;

    // Do not attack if the pet's stance is set to "passive".
    if (pet->GetReactState() == REACT_PASSIVE)
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    if (!bot->IsValidAttackTarget(target))
        return false;

    // This section has been commented because it was overriding the
    // pet's stance to "passive" every time the attack action was executed.
    // pet->SetReactState(REACT_PASSIVE);

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

bool SetPetStanceAction::Execute(Event /*event*/)
{
    // Prepare a list to hold all controlled pet and guardian creatures
    std::vector<Creature*> targets;

    // Add the bot's main pet (if it exists) to the target list
    Pet* pet = bot->GetPet();
    if (pet)
        targets.push_back(pet);

    // Loop through all units controlled by the bot (could be pets, guardians, etc.)
    for (Unit::ControlSet::const_iterator itr = bot->m_Controlled.begin(); itr != bot->m_Controlled.end(); ++itr)
    {
        // Only add creatures (skip players, vehicles, etc.)
        Creature* creature = dynamic_cast<Creature*>(*itr);
        if (!creature)
            continue;
        // Avoid adding the main pet twice
        if (pet && creature == pet)
            continue;
        targets.push_back(creature);
    }

    // If there are no controlled pets or guardians, notify the player and exit
    if (targets.empty())
    {
        botAI->TellError("You have no pet or guardian pet.");
        return false;
    }

    // Get the default pet stance from the configuration
    int32 stance = sPlayerbotAIConfig->defaultPetStance;
    ReactStates react = REACT_DEFENSIVE;
    std::string stanceText = "defensive (from config, fallback)";

    // Map the config stance integer to a ReactStates value and a message
    switch (stance)
    {
        case 0:
            react = REACT_PASSIVE;
            stanceText = "passive (from config)";
            break;
        case 1:
            react = REACT_DEFENSIVE;
            stanceText = "defensive (from config)";
            break;
        case 2:
            react = REACT_AGGRESSIVE;
            stanceText = "aggressive (from config)";
            break;
        default:
            react = REACT_DEFENSIVE;
            stanceText = "defensive (from config, fallback)";
            break;
    }

    // Apply the stance to all target creatures (pets/guardians)
    for (Creature* target : targets)
    {
        target->SetReactState(react);
        CharmInfo* charmInfo = target->GetCharmInfo();
        // If the creature has a CharmInfo, set the player-visible stance as well
        if (charmInfo)
            charmInfo->SetPlayerReactState(react);
    }

    // If debug is enabled in config, inform the master of the new stance
    if (sPlayerbotAIConfig->petChatCommandDebug == 1)
        botAI->TellMaster("Pet stance set to " + stanceText + " (applied to all pets/guardians).");

    return true;
}
