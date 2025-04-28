/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "BossAuraTriggers.h"

#include <HunterBuffStrategies.h>
#include <PaladinBuffStrategies.h>
#include <Unit.h>

#include "Playerbots.h"

bool BossFireResistanceTrigger::IsActive()
{
    // Check boss and it is alive
    Unit* boss = AI_VALUE2(Unit*, "find target", bossName);
    if (!boss || !boss->IsAlive())
        return false;

    // Check if bot is paladin
    if (bot->getClass() != CLASS_PALADIN)
        return false;

    // Check if bot have fire resistance aura
    if (bot->HasAura(SPELL_FIRE_RESISTANCE_AURA))
        return false;

    // Check if bot dont have already have fire resistance strategy
    PaladinFireResistanceStrategy paladinFireResistanceStrategy(botAI);
    if (botAI->HasStrategy(paladinFireResistanceStrategy.getName(), BotState::BOT_STATE_COMBAT))
        return false;

    // Check that the bot actually knows the spell
    if (!bot->HasActiveSpell(SPELL_FIRE_RESISTANCE_AURA))
        return false;

    // Get the group and ensure it's a raid group
    Group* group = bot->GetGroup();
    if (!group || !group->isRaidGroup())
        return false;

    // Iterate through group members to find the first alive paladin
    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* member = gref->GetSource();
        if (!member || !member->IsAlive())
            continue;

        // Check if the member is a hunter
        if (member->getClass() == CLASS_PALADIN)
        {
            // Return true only if the current bot is the first alive paladin
            return member == bot;
        }
    }

    return false;
}

bool BossFrostResistanceTrigger::IsActive()
{
    // Check boss and it is alive
    Unit* boss = AI_VALUE2(Unit*, "find target", bossName);
    if (!boss || !boss->IsAlive())
        return false;

    // Check if bot is paladin
    if (bot->getClass() != CLASS_PALADIN)
        return false;

    // Check if bot have frost resistance aura
    if (bot->HasAura(SPELL_FROST_RESISTANCE_AURA))
        return false;

    // Check if bot dont have already have frost resistance strategy
    PaladinFrostResistanceStrategy paladinFrostResistanceStrategy(botAI);
    if (botAI->HasStrategy(paladinFrostResistanceStrategy.getName(), BotState::BOT_STATE_COMBAT))
        return false;

    // Check that the bot actually knows the spell
    if (!bot->HasActiveSpell(SPELL_FROST_RESISTANCE_AURA))
        return false;

    // Get the group and ensure it's a raid group
    Group* group = bot->GetGroup();
    if (!group || !group->isRaidGroup())
        return false;

    // Iterate through group members to find the first alive paladin
    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* member = gref->GetSource();
        if (!member || !member->IsAlive())
            continue;

        // Check if the member is a hunter
        if (member->getClass() == CLASS_PALADIN)
        {
            // Return true only if the current bot is the first alive paladin
            return member == bot;
        }
    }

    return false;
}

bool BossNatureResistanceTrigger::IsActive()
{
    // Check boss and it is alive
    Unit* boss = AI_VALUE2(Unit*, "find target", bossName);
    if (!boss || !boss->IsAlive())
        return false;

    // Check if bot is alive
    if (!bot->IsAlive())
        return false;

    // Check if bot is hunter
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    // Check if bot have nature resistance aura
    if (bot->HasAura(SPELL_ASPECT_OF_THE_WILD))
        return false;

    // Check if bot dont have already setted nature resistance aura
    HunterNatureResistanceStrategy hunterNatureResistanceStrategy(botAI);
    if (botAI->HasStrategy(hunterNatureResistanceStrategy.getName(), BotState::BOT_STATE_COMBAT))
        return false;

    // Check that the bot actually knows Aspect of the Wild
    if (!bot->HasActiveSpell(SPELL_ASPECT_OF_THE_WILD))
        return false;

    // Get the group and ensure it's a raid group
    Group* group = bot->GetGroup();
    if (!group || !group->isRaidGroup())
        return false;

    // Iterate through group members to find the first alive hunter
    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* member = gref->GetSource();
        if (!member || !member->IsAlive())
            continue;

        // Check if the member is a hunter
        if (member->getClass() == CLASS_HUNTER)
        {
            // Return true only if the current bot is the first alive hunter
            return member == bot;
        }
    }

    return false;
}
