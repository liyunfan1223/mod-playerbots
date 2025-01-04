/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PaladinTriggers.h"
#include "BlessingManager.h"

#include "PaladinActions.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

bool SealTrigger::IsActive()
{
    Unit* target = GetTarget();
    return !botAI->HasAura("seal of justice", target) && !botAI->HasAura("seal of command", target) &&
           !botAI->HasAura("seal of vengeance", target) && !botAI->HasAura("seal of corruption", target) &&
           !botAI->HasAura("seal of righteousness", target) && !botAI->HasAura("seal of light", target) &&
           (!botAI->HasAura("seal of wisdom", target) || AI_VALUE2(uint8, "mana", "self target") > 70);
}

bool CrusaderAuraTrigger::IsActive()
{
    Unit* target = GetTarget();
    return AI_VALUE2(bool, "mounted", "self target") && !botAI->HasAura("crusader aura", target);
}

bool BlessingTrigger::IsActive()
{
    Unit* target = GetTarget();
    return SpellTrigger::IsActive() && !botAI->HasAnyAuraOf(target, "blessing of might", "blessing of wisdom",
                                                            "blessing of kings", "blessing of sanctuary", nullptr);
}

bool CastGreaterBlessingTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Require at least 6 total members (the bot plus 5 others), or an explicit raid check
    if (group->GetMembersCount() <= 5 && !group->isRaidGroup())
        return false;

    // Get the blessing distribution for all Paladins
    auto assignment = AssignBlessingsForGroup(botAI);

    // Find the assigned blessings for *this* Paladin
    auto palIt = assignment.find(bot);
    if (palIt == assignment.end())
        return false; // This Paladin has no assigned blessings

    // For each (class -> blessing) assigned to our bot,
    // see if at least one raid member of that class is missing the aura
    for (auto& kv : palIt->second)
    {
        uint8 classId = kv.first;
        GreaterBlessingType gBlessing = kv.second;

        // Convert that blessing to a relevant aura name
        std::string auraName;
        switch (gBlessing)
        {
            case GREATER_BLESSING_OF_MIGHT:
                auraName = "greater blessing of might";
                break;
            case GREATER_BLESSING_OF_WISDOM:
                auraName = "greater blessing of wisdom";
                break;
            case GREATER_BLESSING_OF_KINGS:
                auraName = "greater blessing of kings";
                break;
            case GREATER_BLESSING_OF_SANCTUARY:
                auraName = "greater blessing of sanctuary";
                break;
        }

        // Scan raid members
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsInWorld())
                continue;

            // Check if the member's class matches the assigned class
            if (member->getClass() == classId)
            {
                // Check if the member *lacks* the aura
                if (!botAI->HasAura(auraName, member))
                    return true; // Found a target missing the assigned Greater Blessing
            }
        }
    }

    // No missing blessings found
    return false;
}
