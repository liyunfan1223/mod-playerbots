/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PetAction.h"

#include "CharmInfo.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "Pet.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "SharedDefines.h"

bool PetAction::Execute(Event event)
{
    // Extract the command parameter from the event (e.g., "aggressive", "defensive", "attack", etc.)
    std::string param = event.getParam();
    if (param.empty() && !defaultCmd.empty())
    {
        param = defaultCmd;
    }

    if (param.empty())
    {
        // If no parameter is provided, show usage instructions and return.
        botAI->TellError("Usage: pet <aggressive|defensive|passive|stance|attack|follow|stay>");
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
        botAI->TellError("You have no pet or guardian pet.");
        return false;
    }

    ReactStates react;
    std::string stanceText;

    // Handle stance commands: aggressive, defensive, or passive.
    if (param == "aggressive")
    {
        react = REACT_AGGRESSIVE;
        stanceText = "aggressive";
    }
    else if (param == "defensive")
    {
        react = REACT_DEFENSIVE;
        stanceText = "defensive";
    }
    else if (param == "passive")
    {
        react = REACT_PASSIVE;
        stanceText = "passive";
    }
    // The "stance" command simply reports the current stance of each pet/guardian.
    else if (param == "stance")
    {
        for (Creature* target : targets)
        {
            std::string type = target->IsPet() ? "pet" : "guardian";
            std::string name = target->GetName();
            std::string stance;
            switch (target->GetReactState())
            {
                case REACT_AGGRESSIVE:
                    stance = "aggressive";
                    break;
                case REACT_DEFENSIVE:
                    stance = "defensive";
                    break;
                case REACT_PASSIVE:
                    stance = "passive";
                    break;
                default:
                    stance = "unknown";
                    break;
            }
            botAI->TellMaster("Current stance of " + type + " \"" + name + "\": " + stance + ".");
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
            {
                targetUnit = botAI->GetUnit(masterTargetGuid);
            }
        }

        // If no valid target is selected, show an error and return.
        if (!targetUnit)
        {
            botAI->TellError("No valid target selected by master.");
            return false;
        }
        if (!targetUnit->IsAlive())
        {
            botAI->TellError("Target is not alive.");
            return false;
        }
        if (!bot->IsValidAttackTarget(targetUnit))
        {
            botAI->TellError("Target is not a valid attack target for the bot.");
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
        if (didAttack && sPlayerbotAIConfig->petChatCommandDebug == 1)
            botAI->TellMaster("Pet commanded to attack your target.");
        else if (!didAttack)
            botAI->TellError("Pet did not attack. (Already attacking or unable to attack target)");
        return didAttack;
    }
    // The "follow" command makes all pets/guardians follow the bot.
    else if (param == "follow")
    {
        botAI->PetFollow();
        if (sPlayerbotAIConfig->petChatCommandDebug == 1)
            botAI->TellMaster("Pet commanded to follow.");
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
        if (sPlayerbotAIConfig->petChatCommandDebug == 1)
            botAI->TellMaster("Pet commanded to stay.");
        return true;
    }
    // Unknown command: show usage instructions and return.
    else
    {
        botAI->TellError("Unknown pet command: " + param +
                         ". Use: pet <aggressive|defensive|passive|stance|attack|follow|stay>");
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
    if (sPlayerbotAIConfig->petChatCommandDebug == 1)
        botAI->TellMaster("Pet stance set to " + stanceText + ".");

    return true;
}
