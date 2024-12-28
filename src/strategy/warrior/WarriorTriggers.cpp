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
    // Get a potential target that doesn't already have Vigilance
    Unit* target = context->GetValue<Unit*>("party member without aura", "vigilance")->Get();
    if (!target || target == bot) // Prevent self-casting
        return false;

    // Check if the bot has Vigilance on another member
    Group* group = bot->GetGroup();
    if (group)
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && botAI->HasAura("vigilance", member, false, true)) // checkIsOwner = true
                return false; // Vigilance is already applied by this bot
        }
    }

    // Return true if there's a valid target and no conflicts
    return true;
}

