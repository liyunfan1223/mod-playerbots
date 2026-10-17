/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PetsAction.h"
#include "CharmInfo.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "Pet.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include <algorithm>
#include <vector>

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

bool PetsAction::Execute(Event event)
{
    // Extract the command parameter from the event (e.g., "aggressive", "defensive", "attack", etc.)
    std::string param = event.getParam();
    if (param.empty() && !defaultCmd.empty())
        param = defaultCmd;

    if (param.empty())
    {
        // If no parameter is provided, show usage instructions and return.
        std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "pet_usage_error", "Usage: pet <aggressive|defensive|passive|stance|attack|follow|stay>", {});
        botAI->TellError(text);
        return false;
    }

    Player* bot = botAI->GetBot();

    // Collect all controlled pets and guardians, except totems, into the targets vector.
    std::vector<Creature*> targets;
    Pet* pet = bot->GetPet();
    if (pet)
        targets.push_back(pet);

    for (Unit::ControlSet::const_iterator itr = bot->m_Controlled.begin(); itr != bot->m_Controlled.end(); ++itr)
    {
        Creature* creature = dynamic_cast<Creature*>(*itr);
        if (!creature)
            continue;
        if (pet && creature == pet)
            continue;
        if (creature->IsTotem())
            continue;
        targets.push_back(creature);
    }

    // If no pets or guardians are found, notify and return.
    if (targets.empty())
    {
        std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "pet_no_pet_error", "You have no pet or guardian pet.", {});
        botAI->TellError(text);
        return false;
    }

    ReactStates react;
    std::string stanceText;

    // Handle stance commands: aggressive, defensive, or passive.
    if (param == "aggressive")
    {
        react = REACT_AGGRESSIVE;
        stanceText = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "pet_stance_aggressive", "aggressive", {});
    }
    else if (param == "defensive")
    {
        react = REACT_DEFENSIVE;
        stanceText = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "pet_stance_defensive", "defensive", {});
    }
    else if (param == "passive")
    {
        react = REACT_PASSIVE;
        stanceText = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "pet_stance_passive", "passive", {});
    }
    // The "stance" command simply reports the current stance of each pet/guardian.
    else if (param == "stance")
    {
        for (Creature* target : targets)
        {
            std::string type = target->IsPet() ?
                PlayerbotTextMgr::instance().GetBotTextOrDefault("pet_type_pet", "pet", {}) :
                PlayerbotTextMgr::instance().GetBotTextOrDefault("pet_type_guardian", "guardian", {});
            std::string name = target->GetName();
            std::string stance;
            switch (target->GetReactState())
            {
                case REACT_AGGRESSIVE:
                    stance = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                        "pet_stance_aggressive", "aggressive", {});
                    break;
                case REACT_DEFENSIVE:
                    stance = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                        "pet_stance_defensive", "defensive", {});
                    break;
                case REACT_PASSIVE:
                    stance = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                        "pet_stance_passive", "passive", {});
                    break;
                default:
                    stance = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                        "pet_stance_unknown", "unknown", {});
                    break;
            }
            std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "pet_stance_report", "Current stance of %type \"%name\": %stance.",
                {{"type", type}, {"name", name}, {"stance", stance}});
            botAI->TellMaster(text);
        }
        return true;
    }
    // The "attack" command forces pets/guardians to attack the master's selected target.
    else if (param == "attack")
    {
        // Try to get the master's selected target.
        Player* master = botAI->GetMaster();
        Unit* targetUnit = nullptr;

        if (master)
        {
            ObjectGuid masterTargetGuid = master->GetTarget();
            if (!masterTargetGuid.IsEmpty())
                targetUnit = botAI->GetUnit(masterTargetGuid);
        }

        // If no valid target is selected, show an error and return.
        if (!targetUnit)
        {
            std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "pet_no_target_error", "No valid target selected by master.", {});
            botAI->TellError(text);
            return false;
        }
        if (!targetUnit->IsAlive())
        {
            std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "pet_target_dead_error", "Target is not alive.", {});
            botAI->TellError(text);
            return false;
        }
        if (!bot->IsValidAttackTarget(targetUnit))
        {
            std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "pet_invalid_target_error", "Target is not a valid attack target for the bot.", {});
            botAI->TellError(text);
            return false;
        }
        if (sPlayerbotAIConfig.IsPvpProhibited(bot->GetZoneId(), bot->GetAreaId()) &&
            (targetUnit->IsPlayer() || targetUnit->IsPet()) &&
            (!bot->duel || bot->duel->Opponent != targetUnit))
        {
            std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "pet_pvp_prohibited_error", "I cannot command my pet to attack players in PvP prohibited areas.", {});
            botAI->TellError(text);
            return false;
        }

        bool didAttack = false;
        // For each controlled pet/guardian, command them to attack the selected target.
        for (Creature* petCreature : targets)
        {
            CharmInfo* charmInfo = petCreature->GetCharmInfo();
            if (!charmInfo)
                continue;

            petCreature->ClearUnitState(UNIT_STATE_FOLLOW);
            // Only command attack if not already attacking the target, or if not currently under command attack.
            if (petCreature->GetVictim() != targetUnit ||
                (petCreature->GetVictim() == targetUnit && !charmInfo->IsCommandAttack()))
            {
                if (petCreature->GetVictim())
                    petCreature->AttackStop();

                if (!petCreature->IsPlayer() && petCreature->ToCreature()->IsAIEnabled)
                {
                    // For AI-enabled creatures (NPC pets/guardians): issue attack command and set flags.
                    charmInfo->SetIsCommandAttack(true);
                    charmInfo->SetIsAtStay(false);
                    charmInfo->SetIsFollowing(false);
                    charmInfo->SetIsCommandFollow(false);
                    charmInfo->SetIsReturning(false);

                    petCreature->ToCreature()->AI()->AttackStart(targetUnit);

                    didAttack = true;
                }
                else  // For charmed player pets/guardians
                {
                    if (petCreature->GetVictim() && petCreature->GetVictim() != targetUnit)
                        petCreature->AttackStop();

                    charmInfo->SetIsCommandAttack(true);
                    charmInfo->SetIsAtStay(false);
                    charmInfo->SetIsFollowing(false);
                    charmInfo->SetIsCommandFollow(false);
                    charmInfo->SetIsReturning(false);

                    petCreature->Attack(targetUnit, true);
                    didAttack = true;
                }
            }
        }
        // Inform the master if the command succeeded or failed.
        if (didAttack && sPlayerbotAIConfig.PetChatCommandDebug == 1)
        {
            std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "pet_attack_success", "Pet commanded to attack your target.", {});
            botAI->TellMaster(text);
        }
        else if (!didAttack)
        {
            std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "pet_attack_failed", "Pet did not attack. (Already attacking or unable to attack target)", {});
            botAI->TellError(text);
        }
        return didAttack;
    }
    // The "follow" command makes all pets/guardians follow the bot.
    else if (param == "follow")
    {
        botAI->PetFollow();
        if (sPlayerbotAIConfig.PetChatCommandDebug == 1)
        {
            std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "pet_follow_success", "Pet commanded to follow.", {});
            botAI->TellMaster(text);
        }
        return true;
    }
    // The "stay" command causes all pets/guardians to stop and stay in place.
    else if (param == "stay")
    {
        for (Creature* target : targets)
        {
            // If not already in controlled motion, stop movement and set to idle.
            bool controlledMotion =
                target->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_CONTROLLED) != NULL_MOTION_TYPE;
            if (!controlledMotion)
            {
                target->StopMovingOnCurrentPos();
                target->GetMotionMaster()->Clear(false);
                target->GetMotionMaster()->MoveIdle();
            }

            CharmInfo* charmInfo = target->GetCharmInfo();
            if (charmInfo)
            {
                // Set charm/pet state flags for "stay".
                charmInfo->SetCommandState(COMMAND_STAY);
                charmInfo->SetIsCommandAttack(false);
                charmInfo->SetIsCommandFollow(false);
                charmInfo->SetIsFollowing(false);
                charmInfo->SetIsReturning(false);
                charmInfo->SetIsAtStay(!controlledMotion);
                charmInfo->SaveStayPosition(controlledMotion);
                if (target->ToPet())
                    target->ToPet()->ClearCastWhenWillAvailable();

                charmInfo->SetForcedSpell(0);
                charmInfo->SetForcedTargetGUID();
            }
        }
        if (sPlayerbotAIConfig.PetChatCommandDebug == 1)
        {
            std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "pet_stay_success", "Pet commanded to stay.", {});
            botAI->TellMaster(text);
        }
        return true;
    }
    // Unknown command: show usage instructions and return.
    else
    {
        std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "pet_unknown_command_error", "Unknown pet command: %param. Use: pet <aggressive|defensive|passive|stance|attack|follow|stay>",
            {{"param", param}});
        botAI->TellError(text);
        return false;
    }

    // For stance commands, apply the chosen stance to all targets.
    for (Creature* target : targets)
    {
        target->SetReactState(react);
        CharmInfo* charmInfo = target->GetCharmInfo();
        if (charmInfo)
            charmInfo->SetPlayerReactState(react);
    }

    // Inform the master of the new stance if debug is enabled.
    if (sPlayerbotAIConfig.PetChatCommandDebug == 1)
    {
        std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "pet_stance_set_success", "Pet stance set to %stance.",
            {{"stance", stanceText}});
        botAI->TellMaster(text);
    }

    return true;
}

bool TogglePetSpellAutoCastAction::Execute(Event /*event*/)
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
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo || !spellInfo->IsAutocastable())
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
    if (toggled && sPlayerbotAIConfig.PetChatCommandDebug == 1)
        botAI->TellMaster("Pet autocast spells have been toggled.");

    return toggled;
}

bool PetAttackAction::Execute(Event /*event*/)
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
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "pet_no_pet_error", "You have no pet or guardian pet.", {}));
        return false;
    }

    // Get the default pet stance from the configuration
    int32 stance = sPlayerbotAIConfig.DefaultPetStance;
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
    if (sPlayerbotAIConfig.PetChatCommandDebug == 1)
        botAI->TellMaster("Pet stance set to " + stanceText + " (applied to all pets/guardians).");

    return true;
}
