/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PetAction.h"
#include <algorithm>
#include <iomanip>
#include <sstream>
#include "Pet.h"
#include "SpellMgr.h"
#include "DBCStructure.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotFactory.h"
#include <random>
#include <cctype>
#include "WorldSession.h"

bool IsExoticPet(const CreatureTemplate* creature)
{
    // Use the IsExotic() method from CreatureTemplate
    return creature && creature->IsExotic();
}

bool HasBeastMastery(Player* bot)
{
    // Beast Mastery talent aura ID for WotLK is 53270
    return bot->HasAura(53270);
}

bool PetAction::Execute(Event event)
{
    std::string param = event.getParam();
    std::istringstream iss(param);
    std::string mode, value;
    iss >> mode;
    std::getline(iss, value);
    value.erase(0, value.find_first_not_of(" "));  // trim leading spaces

    bool found = false;

    // Reset lastPetName/Id each time
    lastPetName = "";
    lastPetId = 0;

    if (mode == "name" && !value.empty())
    {
        found = SetPetByName(value);
    }
    else if (mode == "id" && !value.empty())
    {
        try
        {
            uint32 id = std::stoul(value);
            found = SetPetById(id);
        }
        catch (...)
        {
            botAI->TellError("Invalid pet id.");
        }
    }
    else if (mode == "family" && !value.empty())
    {
        found = SetPetByFamily(value);
    }
    else if (mode == "rename" && !value.empty())
    {
        found = RenamePet(value);
    }
    else
    {
        botAI->TellError("Usage: pet name <name> | pet id <id> | pet family <family> | pet rename <new name> ");
        return false;
    }

    if (!found)
        return false;

    // For non-rename commands, initialize pet and give feedback
    if (mode != "rename")
    {
        Player* bot = botAI->GetBot();
        PlayerbotFactory factory(bot, bot->GetLevel());
        factory.InitPet();
        factory.InitPetTalents();

        if (!lastPetName.empty() && lastPetId != 0)
        {
            std::ostringstream oss;
            oss << "Pet changed to " << lastPetName << ", ID: " << lastPetId << ".";
            botAI->TellMaster(oss.str());
        }
        else
        {
            botAI->TellMaster("Pet changed and initialized!");
        }
    }

    return true;
}

bool PetAction::SetPetByName(const std::string& name)
{
    // Convert the input to lowercase for case-insensitive comparison
    std::string lowerName = name;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

    CreatureTemplateContainer const* creatures = sObjectMgr->GetCreatureTemplates();
    Player* bot = botAI->GetBot();

    for (auto itr = creatures->begin(); itr != creatures->end(); ++itr)
    {
        const CreatureTemplate& creature = itr->second;
        std::string creatureName = creature.Name;
        std::transform(creatureName.begin(), creatureName.end(), creatureName.begin(), ::tolower);

        // Only match if names match (case-insensitive)
        if (creatureName == lowerName)
        {
            // Check if the pet is tameable at all
            if (!creature.IsTameable(true))
                continue;

            // Exotic pet check with talent requirement
            if (IsExoticPet(&creature) && !HasBeastMastery(bot))
            {
                botAI->TellError("I cannot use exotic pets unless I have the Beast Mastery talent.");
                return false;
            }

            // Final tameable check based on hunter's actual ability
            if (!creature.IsTameable(bot->CanTameExoticPets()))
                continue;

            lastPetName = creature.Name;
            lastPetId = creature.Entry;
            return CreateAndSetPet(creature.Entry);
        }
    }

    botAI->TellError("No tameable pet found with name: " + name);
    return false;
}

bool PetAction::SetPetById(uint32 id)
{
    CreatureTemplate const* creature = sObjectMgr->GetCreatureTemplate(id);
    Player* bot = botAI->GetBot();

    if (creature)
    {
        // Check if the pet is tameable at all
        if (!creature->IsTameable(true))
        {
            botAI->TellError("No tameable pet found with id: " + std::to_string(id));
            return false;
        }

        // Exotic pet check with talent requirement
        if (IsExoticPet(creature) && !HasBeastMastery(bot))
        {
            botAI->TellError("I cannot use exotic pets unless I have the Beast Mastery talent.");
            return false;
        }

        // Final tameable check based on hunter's actual ability
        if (!creature->IsTameable(bot->CanTameExoticPets()))
        {
            botAI->TellError("No tameable pet found with id: " + std::to_string(id));
            return false;
        }

        lastPetName = creature->Name;
        lastPetId = creature->Entry;
        return CreateAndSetPet(creature->Entry);
    }

    botAI->TellError("No tameable pet found with id: " + std::to_string(id));
    return false;
}

bool PetAction::SetPetByFamily(const std::string& family)
{
    std::string lowerFamily = family;
    std::transform(lowerFamily.begin(), lowerFamily.end(), lowerFamily.begin(), ::tolower);

    CreatureTemplateContainer const* creatures = sObjectMgr->GetCreatureTemplates();
    Player* bot = botAI->GetBot();

    std::vector<const CreatureTemplate*> candidates;
    bool foundExotic = false;

    for (auto itr = creatures->begin(); itr != creatures->end(); ++itr)
    {
        const CreatureTemplate& creature = itr->second;

        if (!creature.IsTameable(true))  // allow exotics for search
            continue;

        CreatureFamilyEntry const* familyEntry = sCreatureFamilyStore.LookupEntry(creature.family);
        if (!familyEntry)
            continue;

        std::string familyName = familyEntry->Name[0];
        std::transform(familyName.begin(), familyName.end(), familyName.begin(), ::tolower);

        if (familyName != lowerFamily)
            continue;

        // Exotic/BM check
        if (IsExoticPet(&creature))
        {
            foundExotic = true;
            if (!HasBeastMastery(bot))
                continue;
        }

        if (!creature.IsTameable(bot->CanTameExoticPets()))
            continue;

        candidates.push_back(&creature);
    }

    if (candidates.empty())
    {
        if (foundExotic && !HasBeastMastery(bot))
            botAI->TellError("I cannot use exotic pets unless I have the Beast Mastery talent.");
        else
            botAI->TellError("No tameable pet found with family: " + family);
        return false;
    }

    // Randomly select one from candidates
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, candidates.size() - 1);

    const CreatureTemplate* selected = candidates[dis(gen)];

    lastPetName = selected->Name;
    lastPetId = selected->Entry;
    return CreateAndSetPet(selected->Entry);
}

bool PetAction::RenamePet(const std::string& newName)
{
    Player* bot = botAI->GetBot();
    Pet* pet = bot->GetPet();
    if (!pet)
    {
        botAI->TellError("You have no pet to rename.");
        return false;
    }

    // Length check (WoW max pet name is 12 characters)
    if (newName.empty() || newName.length() > 12)
    {
        botAI->TellError("Pet name must be between 1 and 12 alphabetic characters.");
        return false;
    }

    // Alphabetic character check
    for (char c : newName)
    {
        if (!std::isalpha(static_cast<unsigned char>(c)))
        {
            botAI->TellError("Pet name must only contain alphabetic characters (A-Z, a-z).");
            return false;
        }
    }

    // Normalize case: capitalize first letter, lower the rest
    std::string normalized = newName;
    normalized[0] = std::toupper(normalized[0]);
    for (size_t i = 1; i < normalized.size(); ++i)
        normalized[i] = std::tolower(normalized[i]);

    // Forbidden name check
    if (sObjectMgr->IsReservedName(normalized))
    {
        botAI->TellError("That pet name is forbidden. Please choose another name.");
        return false;
    }

    // Set the pet's name, save to DB, and send instant client update
    pet->SetName(normalized);
    pet->SavePetToDB(PET_SAVE_AS_CURRENT);
    bot->GetSession()->SendPetNameQuery(pet->GetGUID(), pet->GetEntry());

    botAI->TellMaster("Your pet has been renamed to " + normalized + "!");
    botAI->TellMaster("If you do not see the new name, please dismiss and recall your pet.");

    // Dismiss pet
    bot->RemovePet(nullptr, PET_SAVE_AS_CURRENT, true);
    // Recall pet using Hunter's Call Pet spell (spellId 883)
    if (bot->getClass() == CLASS_HUNTER && bot->HasSpell(883))
    {
        bot->CastSpell(bot, 883, true);
    }

    return true;
}

bool PetAction::CreateAndSetPet(uint32 creatureEntry)
{
    Player* bot = botAI->GetBot();
    if (bot->getClass() != CLASS_HUNTER || bot->GetLevel() < 10)
    {
        botAI->TellError("Only level 10+ hunters can have pets.");
        return false;
    }

    CreatureTemplate const* creature = sObjectMgr->GetCreatureTemplate(creatureEntry);
    if (!creature)
    {
        botAI->TellError("Creature template not found.");
        return false;
    }

    // Remove current pet(s)
    if (bot->GetPetStable() && bot->GetPetStable()->CurrentPet)
    {
        bot->RemovePet(nullptr, PET_SAVE_AS_CURRENT);
        bot->RemovePet(nullptr, PET_SAVE_NOT_IN_SLOT);
    }
    if (bot->GetPetStable() && bot->GetPetStable()->GetUnslottedHunterPet())
    {
        bot->GetPetStable()->UnslottedPets.clear();
        bot->RemovePet(nullptr, PET_SAVE_AS_CURRENT);
        bot->RemovePet(nullptr, PET_SAVE_NOT_IN_SLOT);
    }

    // Actually create the new pet
    Pet* pet = bot->CreateTamedPetFrom(creatureEntry, 0);
    if (!pet)
    {
        botAI->TellError("Failed to create pet.");
        return false;
    }

    // Set pet level and add to world
    pet->SetUInt32Value(UNIT_FIELD_LEVEL, bot->GetLevel() - 1);
    pet->GetMap()->AddToMap(pet->ToCreature());
    pet->SetUInt32Value(UNIT_FIELD_LEVEL, bot->GetLevel());
    bot->SetMinion(pet, true);
    pet->InitTalentForLevel();
    pet->SavePetToDB(PET_SAVE_AS_CURRENT);
    bot->PetSpellInitialize();

    // Set stats
    pet->InitStatsForLevel(bot->GetLevel());
    pet->SetLevel(bot->GetLevel());
    pet->SetPower(POWER_HAPPINESS, pet->GetMaxPower(Powers(POWER_HAPPINESS)));
    pet->SetHealth(pet->GetMaxHealth());

    // Enable autocast for active spells
    for (PetSpellMap::const_iterator itr = pet->m_spells.begin(); itr != pet->m_spells.end(); ++itr)
    {
        if (itr->second.state == PETSPELL_REMOVED)
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(itr->first);
        if (!spellInfo)
            continue;

        if (spellInfo->IsPassive())
            continue;

        pet->ToggleAutocast(spellInfo, true);
    }

    return true;
}
