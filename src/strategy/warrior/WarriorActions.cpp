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

    // Determine the target to apply Vigilance
    Player* selectedTarget = nullptr;

    // Step 1: Check if Vigilance is already applied by the bot
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && botAI->HasAura("vigilance", member, false, true)) // checkIsOwner = true
            return nullptr; // Vigilance already applied, no need to reapply
    }

    // Step 2: Prioritize Main Tank
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member != bot && botAI->IsMainTank(member) &&
            !botAI->HasAura("vigilance", member, false, true)) // checkIsOwner = true
        {
            selectedTarget = member;
            LOG_INFO("playerbots", "Bot {} <{}> selected Main Tank {} <{}> for Vigilance", 
                     bot->GetGUID().ToString().c_str(), bot->GetName().c_str(),
                     member->GetGUID().ToString().c_str(), member->GetName().c_str());
            break;
        }
    }

    // Step 3: Check Assist Tanks if no Main Tank is selected
    if (!selectedTarget)
    {
        for (int index = 0; index < 2; ++index)
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member != bot && botAI->IsAssistTankOfIndex(member, index) &&
                    !botAI->HasAura("vigilance", member, false, true)) // checkIsOwner = true
                {
                    selectedTarget = member;
                    LOG_INFO("playerbots", "Bot {} <{}> selected Assist Tank {} <{}> for Vigilance", 
                             bot->GetGUID().ToString().c_str(), bot->GetName().c_str(),
                             member->GetGUID().ToString().c_str(), member->GetName().c_str());
                    break;
                }
            }
            if (selectedTarget)
                break;
        }
    }

    // Step 4: Fall Back to Highest DPS if no tanks are valid targets
    if (!selectedTarget)
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

        selectedTarget = highestGearScorePlayer;
    }

    // Step 5: Convert to Unit* and return the appropriate target
    if (selectedTarget)
    {
        Unit* targetUnit = selectedTarget->ToUnit();
        if (targetUnit)
        {
            LOG_INFO("playerbots", "Bot {} <{}> will cast Vigilance on {} <{}>", 
                     bot->GetGUID().ToString().c_str(), bot->GetName().c_str(),
                     targetUnit->GetGUID().ToString().c_str(), targetUnit->GetName().c_str());
            return new ManualSetValue<Unit*>(context, targetUnit); // Wrap and return the target
        }
        else
        {
            LOG_INFO("playerbots", "Bot {} <{}> selected invalid target (not a Unit*)", 
                     bot->GetGUID().ToString().c_str(), bot->GetName().c_str());
        }
    }
    
    return nullptr;
}

bool CastVigilanceAction::Execute(Event event)
{
    Unit* target = GetTarget();
    if (!target || target == bot)
        return false;

    LOG_INFO("playerbots", "Bot {} <{}> attempts to cast Vigilance on {} <{}>", 
                 bot->GetGUID().ToString().c_str(), bot->GetName().c_str(),
                 target->GetGUID().ToString().c_str(), target->GetName().c_str());

    return botAI->CastSpell("vigilance", target);
}
