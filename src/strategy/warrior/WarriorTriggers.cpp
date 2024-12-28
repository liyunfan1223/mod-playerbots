/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "WarriorTriggers.h"

#include "Playerbots.h"

bool BloodrageBuffTrigger::IsActive()
{
    return AI_VALUE2(uint8, "health", "self target") >= sPlayerbotAIConfig->mediumHealth &&
           AI_VALUE2(uint8, "rage", "self target") < 20;
}

bool VigilanceTrigger::IsActive()
{
    Group* group = bot->GetGroup();
    if (!group)
    {
        // LOG_INFO("playerbots", "Bot {} <{}> is not in a group, Vigilance cannot be applied", 
        //         bot->GetGUID().ToString().c_str(), bot->GetName().c_str());
        return false;
    }

    Player* currentVigilanceTarget = nullptr;

    // Check if Vigilance is already applied by the bot
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && botAI->HasAura("vigilance", member, false, true)) // checkIsOwner = true
        {
            currentVigilanceTarget = member;
            break;
        }
    }

    // Find the highest-priority target
    Player* highestPriorityTarget = nullptr;

    // Step 1: Check Main Tank
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member != bot && botAI->IsMainTank(member))
        {
            highestPriorityTarget = member;
            break;
        }
    }

    // Step 2: Check Assist Tanks if no Main Tank is selected
    if (!highestPriorityTarget)
    {
        for (int index = 0; index < 2; ++index)
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member != bot && botAI->IsAssistTankOfIndex(member, index))
                {
                    highestPriorityTarget = member;
                    break;
                }
            }
            if (highestPriorityTarget)
                break;
        }
    }

    // Step 3: Fall Back to Highest DPS
    if (!highestPriorityTarget)
    {
        Player* highestGearScorePlayer = nullptr;
        uint32 highestGearScore = 0;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member != bot)
            {
                uint32 gearScore = botAI->GetEquipGearScore(member, false, false); // Exclude bags and bank
                if (gearScore > highestGearScore)
                {
                    highestGearScore = gearScore;
                    highestGearScorePlayer = member;
                }
            }
        }

        highestPriorityTarget = highestGearScorePlayer;
    }

    // Trigger if no Vigilance is active or the current target is not the highest-priority target
    if (!currentVigilanceTarget || currentVigilanceTarget != highestPriorityTarget)
    {
        LOG_INFO("playerbots", "Bot {} <{}> needs to reassign Vigilance to {} <{}>", 
                 bot->GetGUID().ToString().c_str(), bot->GetName().c_str(),
                 highestPriorityTarget ? highestPriorityTarget->GetGUID().ToString().c_str() : "none", 
                 highestPriorityTarget ? highestPriorityTarget->GetName().c_str() : "none");
        return true;
    }

    return false; // No need to reassign Vigilance
}
