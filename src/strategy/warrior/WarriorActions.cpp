/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "WarriorActions.h"

#include "Playerbots.h"

bool CastSunderArmorAction::isUseful()
{
    Aura* aura = botAI->GetAura("sunder armor", GetTarget(), false, true);
    return !aura || aura->GetStackAmount() < 5 || aura->GetDuration() <= 6000;
}

Value<Unit*>* CastVigilanceAction::GetTargetValue()
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    // Check if Vigilance is already active and owned by the bot
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && botAI->HasAura("vigilance", member, false, true)) // checkIsOwner = true
            return nullptr; // Bot already has Vigilance on someone
    }

    Unit* target = nullptr;

    // Step 1: Prioritize Main Tank
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member != bot && botAI->IsMainTank(member) &&
            !botAI->HasAura("vigilance", member, false, true)) // checkIsOwner = true
        {
            target = member;
            break;
        }
    }

    if (!target) // Step 2: Check Assist Tanks by Index
    {
        for (int index = 0; index < 2; ++index)
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member != bot && botAI->IsAssistTankOfIndex(member, index) &&
                    !botAI->HasAura("vigilance", member, false, true)) // checkIsOwner = true
                {
                    target = member;
                    break;
                }
            }
            if (target)
                break;
        }
    }

    if (!target) // Step 3: Fall Back to Highest DPS (based on gear score)
    {
        Player* highestGearScorePlayer = nullptr;
        uint32 highestGearScore = 0;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member != bot && !botAI->HasAura("vigilance", member, false, true)) // checkIsOwner = true
            {
                uint32 gearScore = botAI->GetEquipGearScore(member, false, false); // Exclude bags and bank
                if (gearScore > highestGearScore)
                {
                    highestGearScore = gearScore;
                    highestGearScorePlayer = member;
                }
            }
        }

        target = highestGearScorePlayer;
    }

    // Return the target wrapped as a Value<Unit*>* if valid, otherwise nullptr
    return target ? new ManualSetValue<Unit*>(context, target) : nullptr;
}

bool CastVigilanceAction::Execute(Event event)
{
    Unit* target = GetTarget();
    if (!target || target == bot)
        return false;

    return botAI->CastSpell("vigilance", target);
}
