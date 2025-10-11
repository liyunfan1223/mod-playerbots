/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "MageActions.h"
#include "MovementActions.h"
#include <cmath>
#include "UseItemAction.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "Battleground.h"
#include "Log.h"

// Function declared in RitualActions.cpp

// Custom cooldown for rituals (15 minutes)
static std::unordered_map<ObjectGuid, uint32> lastRefreshmentRitualUsage;
// Movement state for rituals
static std::unordered_map<ObjectGuid, bool> hasMovedForRefreshmentRitual;
// Orientation state for rituals (to avoid overlap)
static std::unordered_map<ObjectGuid, bool> hasChangedOrientationForRitual;
// Track if bot has completed ritual interaction to avoid loops
extern std::unordered_map<uint64, bool> hasCompletedRitualInteraction;

#include "ServerFacade.h"
#include "SharedDefines.h"
#include "RitualActions.h"

// Global constants for spells and items
namespace MageConstants
{
    // Ritual of Refreshment spells
    const uint32 RITUAL_REFRESHMENT_RANK_1 = 43987;
    const uint32 RITUAL_REFRESHMENT_RANK_2 = 58659;
    
    // Refreshment Table game objects
    const uint32 REFRESHMENT_TABLE_RANK_1 = 186812;
    const uint32 REFRESHMENT_TABLE_RANK_2 = 193061;
    
    // Refreshment Portal game objects
    const uint32 REFRESHMENT_PORTAL_RANK_1 = 186811;
    const uint32 REFRESHMENT_PORTAL_RANK_2 = 193062;
    
    // Arcane Powder item
    const uint32 ARCANE_POWDER_ITEM = 17020;
    
    // Conjured food and drink items
    const uint32 CONJURED_MANA_BISCUIT = 43523;
    const uint32 CONJURED_MANA_STRUDEL = 43518;
    const uint32 CONJURED_MANA_COOKIE = 43517;
    const uint32 CONJURED_MANA_CAKE = 43516;
    const uint32 CONJURED_MANA_PIE = 43515;
    const uint32 CONJURED_MANA_BREAD = 43514;
    const uint32 CONJURED_MANA_MUFFIN = 43513;
    const uint32 CONJURED_MANA_DONUT = 43512;
    const uint32 CONJURED_MANA_BAGEL = 43511;
    const uint32 CONJURED_MANA_PRETZEL = 43510;
    const uint32 CONJURED_MANA_WATER = 43519;
    const uint32 CONJURED_MANA_JUICE = 43520;
    const uint32 CONJURED_MANA_TEA = 43521;
    const uint32 CONJURED_MANA_COFFEE = 43522;
    // Focus Magic spell (aura)
    const uint32 FOCUS_MAGIC_AURA = 54646;

    // Mana gem items
    const uint32 MANA_SAPPHIRE = 33312;
    const uint32 MANA_EMERALD  = 22044;
    const uint32 MANA_RUBY     = 8008;
    const uint32 MANA_CITRINE  = 8007;
    const uint32 MANA_JADE     = 5513;
    const uint32 MANA_AGATE    = 5514;
}

Value<Unit*>* CastPolymorphAction::GetTargetValue() { return context->GetValue<Unit*>("cc target", getName()); }

bool UseManaSapphireAction::isUseful()
{
    return AI_VALUE2(bool, "combat", "self target") && bot->GetItemCount(MageConstants::MANA_SAPPHIRE, false) > 0;
}

bool UseManaEmeraldAction::isUseful()
{
    return AI_VALUE2(bool, "combat", "self target") && bot->GetItemCount(MageConstants::MANA_EMERALD, false) > 0;
}

bool UseManaRubyAction::isUseful()
{
    return AI_VALUE2(bool, "combat", "self target") && bot->GetItemCount(MageConstants::MANA_RUBY, false) > 0;
}

bool UseManaCitrineAction::isUseful()
{
    return AI_VALUE2(bool, "combat", "self target") && bot->GetItemCount(MageConstants::MANA_CITRINE, false) > 0;
}

bool UseManaJadeAction::isUseful()
{
    return AI_VALUE2(bool, "combat", "self target") && bot->GetItemCount(MageConstants::MANA_JADE, false) > 0;
}

bool UseManaAgateAction::isUseful()
{
    return AI_VALUE2(bool, "combat", "self target") && bot->GetItemCount(MageConstants::MANA_AGATE, false) > 0;
}

bool CastFrostNovaAction::isUseful()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || !target->IsInWorld())
        return false;

    if (target->ToCreature() && target->ToCreature()->HasMechanicTemplateImmunity(1 << (MECHANIC_FREEZE - 1)))
        return false;

    if (target->isFrozen())
        return false;

    return sServerFacade->IsDistanceLessOrEqualThan(AI_VALUE2(float, "distance", GetTargetName()), 10.f);
}

bool CastConeOfColdAction::isUseful()
{
    bool facingTarget = AI_VALUE2(bool, "facing", "current target");
    bool targetClose = sServerFacade->IsDistanceLessOrEqualThan(AI_VALUE2(float, "distance", GetTargetName()), 10.f);
    return facingTarget && targetClose;
}

bool CastDragonsBreathAction::isUseful()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;
    
    bool facingTarget = AI_VALUE2(bool, "facing", "current target");
    bool targetClose = bot->IsWithinCombatRange(target, 10.0f);
    return facingTarget && targetClose;
}

bool CastBlastWaveAction::isUseful()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;
    
    bool targetClose = bot->IsWithinCombatRange(target, 10.0f);
    return targetClose;
}

Unit* CastFocusMagicOnPartyAction::GetTarget()
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Unit* casterDps = nullptr;
    Unit* healer = nullptr;
    Unit* target = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive())
            continue;

        if (member->GetMap() != bot->GetMap() || bot->GetDistance(member) > sPlayerbotAIConfig->spellDistance)
            continue;

        if (member->HasAura(MageConstants::FOCUS_MAGIC_AURA))
            continue;

        if (member->getClass() == CLASS_MAGE)
            return member;

        if (!casterDps && botAI->IsCaster(member) && botAI->IsDps(member))
            casterDps = member;

        if (!healer && botAI->IsHeal(member))
            healer = member;

        if (!target)
            target = member;
    }

    if (casterDps)
        return casterDps;

    if (healer)
        return healer;

    return target;
}

bool CastBlinkBackAction::Execute(Event event)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;
    
    // can cast spell check passed in isUseful()
    bot->SetOrientation(bot->GetAngle(target) + M_PI);
    return CastSpellAction::Execute(event);
}

// Ritual Actions Implementation

// Helper function to detect which ritual spell the bot has
uint32 CastRitualOfRefreshmentAction::GetBotRitualSpellId(Player* bot)
{    
    // Prefer higher rank if available
    if (bot->HasSpell(MageConstants::RITUAL_REFRESHMENT_RANK_2)) {
        return MageConstants::RITUAL_REFRESHMENT_RANK_2;
    } else if (bot->HasSpell(MageConstants::RITUAL_REFRESHMENT_RANK_1)) {
        return MageConstants::RITUAL_REFRESHMENT_RANK_1;
    }
    
    return 0; // No ritual spell found
}

bool CastRitualOfRefreshmentAction::isUseful()
{
    // Detect which rank the bot has
    uint32 botSpellId = GetBotRitualSpellId(bot);
    
    // Fast-fail preconditions
    if (!CastSpellAction::isUseful() || !CanUseRituals(bot) || botSpellId == 0)
    {
        return false;
    }
    
    // Cooldown checks (spell or custom 2 minutes)
    uint32 currentTime = time(nullptr);
    auto it = lastRefreshmentRitualUsage.find(bot->GetGUID());
    bool underCustomCooldown = (it != lastRefreshmentRitualUsage.end()) && ((currentTime - it->second) < 120);
    if (bot->GetSpellCooldownDelay(botSpellId) > 0 || underCustomCooldown)
    {
        return false;
    }
    
    // Only allows one mage to perform the ritual if there are other mages in the group
    if (Group* group = bot->GetGroup())
    {
        Player* selectedMage = nullptr;
        uint32 lowestGuid = UINT32_MAX;
        
        // Find the mage with the lowest GUID (priority by group entry order)
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            if (Player* member = itr->GetSource())
            {
                if (member->getClass() == CLASS_MAGE && member->IsAlive())
                {
                    // In Battlegrounds, only consider same faction
                    bool isSameFaction = (member->GetTeamId() == bot->GetTeamId());
                    bool shouldConsider = bot->GetMap()->IsBattleground() ? isSameFaction : true;
                    
                    if (shouldConsider && member->GetGUID().GetCounter() < lowestGuid)
                    {
                        lowestGuid = member->GetGUID().GetCounter();
                        selectedMage = member;
                    }
                }
            }
        }
        
        // Only the selected mage can perform the ritual
        if (selectedMage && selectedMage != bot)
        {
            return false; // Not the selected mage
        }
    }
    
    if (!HasRitualComponent(bot, MageConstants::RITUAL_REFRESHMENT_RANK_1))
    {
        // Give Arcane Powder to bot - 2 units required
        bot->AddItem(MageConstants::ARCANE_POWDER_ITEM, 2);
    }
    
    // Check if already has refreshment table nearby
    GameObject* existingTable = bot->FindNearestGameObject(MageConstants::REFRESHMENT_TABLE_RANK_1, 30.0f);
    if (!existingTable)
        existingTable = bot->FindNearestGameObject(MageConstants::REFRESHMENT_TABLE_RANK_2, 30.0f);
    
    return !existingTable;
}

bool CastRitualOfRefreshmentAction::Execute(Event event)
{
    // Check if bot is currently channeling the ritual
    if (bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
    {
        // Don't interrupt the channeling - wait for it to complete naturally
        botAI->SetNextCheckDelay(2000); // Check again in 2 seconds
        return false;
    }
    
    // Change mage orientation to avoid portal overlap
    // Only do this once per ritual
    auto orientationIt = hasChangedOrientationForRitual.find(bot->GetGUID());
    if (orientationIt == hasChangedOrientationForRitual.end() || !orientationIt->second)
    {
        // Rotate mage 90 degrees to avoid overlap
        float currentOrientation = bot->GetOrientation();
        float newOrientation = currentOrientation + M_PI / 2; // 90 degrees
        bot->SetOrientation(newOrientation);
        bot->SetFacingTo(newOrientation);
        
        hasChangedOrientationForRitual[bot->GetGUID()] = true;     
    }
    
    // Detect which rank the bot has
    uint32 botSpellId = GetBotRitualSpellId(bot);
    // Rank string not used; only need the spell id for cooldown and cast
    if (botSpellId == 0)
    {
        return false;
    }
    
    // Check cooldown for the detected spell
    uint32 cooldown = bot->GetSpellCooldownDelay(botSpellId);
    if (cooldown > 0)
    {
        return false;
    }
    
    bool result = CastSpellAction::Execute(event);
    if (result)
    {        
        // Mark that this bot has completed ritual interaction
        uint64 botGuid = bot->GetGUID().GetRawValue();
        hasCompletedRitualInteraction[botGuid] = true;
        
        // Set custom cooldown (2 minutes)
        uint32 currentTime = time(nullptr);
        lastRefreshmentRitualUsage[bot->GetGUID()] = currentTime;
        // Reset movement and orientation state for next ritual
        hasMovedForRefreshmentRitual[bot->GetGUID()] = false;
        hasChangedOrientationForRitual[bot->GetGUID()] = false;
        
        // Record ritual creation time to prevent immediate re-triggering
        static std::unordered_map<ObjectGuid, uint32> lastRefreshmentRitualCreation;
        lastRefreshmentRitualCreation[bot->GetGUID()] = currentTime;
    }
    
    return result;
}
