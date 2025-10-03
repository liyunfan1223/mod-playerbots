/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
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
    if (!bot->HasSpell(50720))
    {
        return false;
    }

    Group* group = bot->GetGroup();
    if (!group)
    {
        return false;
    }

    Player* currentVigilanceTarget = nullptr;
    Player* mainTank = nullptr;
    Player* assistTank1 = nullptr;
    Player* assistTank2 = nullptr;
    Player* highestGearScorePlayer = nullptr;
    uint32 highestGearScore = 0;

    // Iterate once through the group to gather all necessary information
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive())
            continue;

        // Check if member has Vigilance applied by the bot
        if (!currentVigilanceTarget && botAI->HasAura("vigilance", member, false, true))
        {
            currentVigilanceTarget = member;
        }

        // Identify Main Tank
        if (!mainTank && botAI->IsMainTank(member))
        {
            mainTank = member;
        }

        // Identify Assist Tanks
        if (assistTank1 == nullptr && botAI->IsAssistTankOfIndex(member, 0))
        {
            assistTank1 = member;
        }
        else if (assistTank2 == nullptr && botAI->IsAssistTankOfIndex(member, 1))
        {
            assistTank2 = member;
        }

        // Determine Highest Gear Score
        uint32 gearScore = botAI->GetEquipGearScore(member/*, false, false*/);
        if (gearScore > highestGearScore)
        {
            highestGearScore = gearScore;
            highestGearScorePlayer = member;
        }
    }

    // Determine the highest-priority target
    Player* highestPriorityTarget = mainTank ? mainTank :
                                      (assistTank1 ? assistTank1 :
                                      (assistTank2 ? assistTank2 : highestGearScorePlayer));

    // Trigger if no Vigilance is active or the current target is not the highest-priority target
    if (!currentVigilanceTarget || currentVigilanceTarget != highestPriorityTarget)
    {
        return true;
    }

    return false; // No need to reassign Vigilance
}

bool ShatteringThrowTrigger::IsActive()
{
    // Spell cooldown check
    if (!bot->HasSpell(64382))
    {
        return false;
    }

    // Spell cooldown check
    if (bot->HasSpellCooldown(64382))
    {
        return false;
    }

    GuidVector enemies = AI_VALUE(GuidVector, "possible targets");

    for (ObjectGuid const& guid : enemies)
    {
        Unit* enemy = botAI->GetUnit(guid);
        if (!enemy || !enemy->IsAlive() || enemy->IsFriendlyTo(bot))
            continue;

        // Check if the enemy is within 25 yards and has the specific auras
        if (bot->IsWithinDistInMap(enemy, 25.0f) &&
            (enemy->HasAura(642) ||   // Divine Shield
             enemy->HasAura(45438) || // Ice Block
             enemy->HasAura(41450)))  // Blessing of Protection
        {
            return true;
        }
    }

    return false; // No valid targets within range
}

