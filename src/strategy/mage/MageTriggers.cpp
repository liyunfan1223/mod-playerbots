/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "MageTriggers.h"
#include "MageActions.h"
#include "Playerbots.h"
#include "PlayerbotMgr.h"
#include "Player.h"
#include "Spell.h"
#include "Battleground.h"
#include "GameObject.h"
#include "Cell.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "DynamicObject.h"
#include "Value.h"
#include "SpellAuraEffects.h"
#include "ServerFacade.h"
#include "RitualActions.h"
#include <unordered_map>
#include <ctime>

bool NoManaGemTrigger::IsActive()
{
    static const std::vector<uint32> gemIds = {
        33312,  // Mana Sapphire
        22044,  // Mana Emerald
        8008,   // Mana Ruby
        8007,   // Mana Citrine
        5513,   // Mana Jade
        5514    // Mana Agate
    };

    for (uint32 gemId : gemIds)
    {
        if (bot->GetItemCount(gemId, false) > 0)  // false = only in bags
            return false;
    }
    return true;
}

bool ArcaneIntellectOnPartyTrigger::IsActive()
{
    return BuffOnPartyTrigger::IsActive() && !botAI->HasAura("arcane brilliance", GetTarget());
}

bool ArcaneIntellectTrigger::IsActive()
{
    return BuffTrigger::IsActive() && !botAI->HasAura("arcane brilliance", GetTarget());
}

bool MageArmorTrigger::IsActive()
{
    Unit* target = GetTarget();
    return !botAI->HasAura("ice armor", target) && !botAI->HasAura("frost armor", target) &&
           !botAI->HasAura("molten armor", target) && !botAI->HasAura("mage armor", target);
}

bool FrostNovaOnTargetTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target || !target->IsAlive() || !target->IsInWorld())
    {
        return false;
    }
    return botAI->HasAura(spell, target);
}

bool FrostbiteOnTargetTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target || !target->IsAlive() || !target->IsInWorld())
    {
        return false;
    }
    return botAI->HasAura(spell, target);
}

bool NoFocusMagicTrigger::IsActive()
{
    if (!bot->HasSpell(54646))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive())
            continue;

        if (member->HasAura(54646, bot->GetGUID()))
            return false;
    }
    return true;
}

bool DeepFreezeCooldownTrigger::IsActive()
{
    static const uint32 DEEP_FREEZE_SPELL_ID = 44572;

    // If the bot does NOT have Deep Freeze, treat as "on cooldown"
    if (!bot->HasSpell(DEEP_FREEZE_SPELL_ID))
        return true;

    // Otherwise, use the default cooldown logic
    return SpellCooldownTrigger::IsActive();
}

const std::set<uint32> FlamestrikeNearbyTrigger::FLAMESTRIKE_SPELL_IDS = {2120,  2121,  8422,  8423, 10215,
                                                                          10216, 27086, 42925, 42926};

bool FlamestrikeNearbyTrigger::IsActive()
{
    for (uint32 spellId : FLAMESTRIKE_SPELL_IDS)
    {
        Aura* aura = bot->GetAura(spellId, bot->GetGUID());
        if (!aura)
            continue;

        DynamicObject* dynObj = aura->GetDynobjOwner();
        if (!dynObj)
            continue;

        float dist = bot->GetDistance2d(dynObj->GetPositionX(), dynObj->GetPositionY());
        if (dist <= radius)
            return true;
    }
    return false;
}

bool ImprovedScorchTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target || !target->IsAlive() || !target->IsInWorld())
        return false;

    // List of all spell IDs for Improved Scorch, Winter's Chill, and Shadow Mastery
    static const uint32 ImprovedScorchExclusiveDebuffs[] = {
        17794, 17797, 17798, 17799, 17800, // Shadow Mastery
        12579, // Winter's Chill
        22959 // Improved Scorch
    };

    for (uint32 spellId : ImprovedScorchExclusiveDebuffs)
    {
        if (target->HasAura(spellId))
            return false;
    }

    // Use default DebuffTrigger logic for the rest (only trigger if debuff is missing or expiring)
    return DebuffTrigger::IsActive();
}

const std::set<uint32> BlizzardChannelCheckTrigger::BLIZZARD_SPELL_IDS = {
    10,     // Blizzard Rank 1
    6141,   // Blizzard Rank 2
    8427,   // Blizzard Rank 3
    10185,  // Blizzard Rank 4
    10186,  // Blizzard Rank 5
    10187,  // Blizzard Rank 6
    27085,  // Blizzard Rank 7
    42938,  // Blizzard Rank 8
    42939   // Blizzard Rank 9
};

bool BlizzardChannelCheckTrigger::IsActive()
{
    // Check if the bot is channeling a spell
    if (Spell* spell = bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
    {
        // Only trigger if the spell being channeled is Blizzard
        if (BLIZZARD_SPELL_IDS.count(spell->m_spellInfo->Id))
        {
            uint8 attackerCount = AI_VALUE(uint8, "attacker count");
            return attackerCount < minEnemies;
        }
    }

    // Not channeling Blizzard
    return false;
}

// Ritual Triggers Implementation

bool NoRefreshmentTableTrigger::IsActive()
{ 
    // Check if rituals can be used in current map
    if (!CanUseRituals(bot))
    {
        return false;
    }
    
    // Checking trigger...
    
    // Check if bot has the spell
    if (!bot->HasSpell(RITUAL_OF_REFRESHMENT_RANK_1))
    {
        return false;
    }
    
    // Check if bot has the required component, if not, give it to him
    if (!HasRitualComponent(bot, RITUAL_OF_REFRESHMENT_RANK_1))
    {
        // Give Light Feather to bot
        return bot->AddItem(RITUAL_LIGHT_FEATHER, 1);
    }
    
    // Check if there's already a refreshment table nearby (within 30 yards)
    // This is a simplified check - in a real implementation you'd search for GameObjects
    // For now, we'll assume no refreshment table exists if we're in a dungeon/raid
    
    // Check if there's already a refreshment table nearby (within 30 yards)
    // Try different possible IDs for Refreshment Table
    GameObject* existingTable = bot->FindNearestGameObject(RITUAL_REFRESHMENT_TABLE_ID_1, 30.0f);
    if (!existingTable)
        existingTable = bot->FindNearestGameObject(RITUAL_REFRESHMENT_TABLE_ID_2, 30.0f);
    if (!existingTable)
        existingTable = bot->FindNearestGameObject(RITUAL_REFRESHMENT_TABLE_ID_3, 30.0f);
    if (existingTable)
    {
        return false; // Refreshment table already exists nearby
    }
    
    // Only create ritual if we're in a group (more realistic)
    Group* group = bot->GetGroup();
    if (!group || group->GetMembersCount() <= 1)
    {
        return false;
    }
    
    // SIMPLIFIED TO MAXIMUM: Only check if there's a warlock in the group
    bool hasWarlock = false;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->getClass() == CLASS_WARLOCK)
        {
            hasWarlock = true;
            break;
        }
    }
    
    // If there's a warlock, check if it already finished its ritual and wait a bit
    if (hasWarlock)
    {
        // Check if there's already a Soul Well (means the warlock finished and bots clicked)
        GameObject* existingSoulWell = bot->FindNearestGameObject(181621, 30.0f); // Soul Well Rank 1
        if (!existingSoulWell)
            existingSoulWell = bot->FindNearestGameObject(193169, 30.0f); // Soul Well Rank 2 (variant 1)
        if (!existingSoulWell)
            existingSoulWell = bot->FindNearestGameObject(193170, 30.0f); // Soul Well Rank 2 (variant 2)
        if (!existingSoulWell)
            existingSoulWell = bot->FindNearestGameObject(193171, 30.0f); // Soul Well Rank 2 (variant 3)
        
        return existingSoulWell;
    }
    
    // If no warlock, mage can cast freely
    return true;
}
