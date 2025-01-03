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
    {
        return new ManualSetValue<Unit*>(botAI, nullptr);
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
        uint32 gearScore = botAI->GetEquipGearScore(member, false, false);
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

    // If no valid target, return nullptr
    if (!highestPriorityTarget)
    {
        return new ManualSetValue<Unit*>(botAI, nullptr);
    }

    // If the current target is already the highest-priority target, do nothing
    if (currentVigilanceTarget == highestPriorityTarget)
    {
        return new ManualSetValue<Unit*>(botAI, nullptr);
    }

    // Assign the new target
    Unit* targetUnit = highestPriorityTarget->ToUnit();
    if (targetUnit)
    {
        return new ManualSetValue<Unit*>(botAI, targetUnit);
    }

    return new ManualSetValue<Unit*>(botAI, nullptr);
}

bool CastVigilanceAction::Execute(Event event)
{
    Unit* target = GetTarget();
    if (!target || target == bot)
        return false;

    return botAI->CastSpell("vigilance", target);
}

bool CastRetaliationAction::isUseful()
{
    uint8 meleeAttackers = 0;
    GuidVector attackers = AI_VALUE(GuidVector, "attackers");

    for (ObjectGuid const& guid : attackers)
    {
        Unit* attacker = botAI->GetUnit(guid);
        if (!attacker || !attacker->IsAlive() || attacker->GetVictim() != bot)
            continue;

        // Check if the attacker is melee-based using unit_class
        if (attacker->GetTypeId() == TYPEID_UNIT)
        {
            Creature* creature = attacker->ToCreature();
            if (creature && (creature->IsClass(CLASS_WARRIOR)
                || creature->IsClass(CLASS_ROGUE)
                || creature->IsClass(CLASS_PALADIN)))
            {
                ++meleeAttackers;
            }
        }
        else if (attacker->GetTypeId() == TYPEID_PLAYER)
        {
            Player* playerAttacker = attacker->ToPlayer();
            if (playerAttacker && botAI->IsMelee(playerAttacker)) // Reuse existing Player melee check
            {
                ++meleeAttackers;
            }
        }

        // Early exit if we already have enough melee attackers
        if (meleeAttackers >= 2)
            break;
    }

    // Only cast Retaliation if there are at least 2 melee attackers and the buff is not active
    return meleeAttackers >= 2 && !botAI->HasAura("retaliation", bot);
}

Unit* CastShatteringThrowAction::GetTarget()
{
    GuidVector enemies = AI_VALUE(GuidVector, "possible targets");

    for (ObjectGuid const& guid : enemies)
    {
        Unit* enemy = botAI->GetUnit(guid);
        if (!enemy || !enemy->IsAlive() || enemy->IsFriendlyTo(bot))
            continue;

        if (bot->IsWithinDistInMap(enemy, 25.0f) &&
            (enemy->HasAura(642) ||   // Divine Shield
             enemy->HasAura(45438) || // Ice Block
             enemy->HasAura(41450)))  // Blessing of Protection
        {
            LOG_INFO("playerbots", "Bot Name = {}, CastShatteringThrowAction::GetTarget: Valid target found: Name = {}, GUID = {}", 
                bot->GetName(), 
                enemy->GetName().empty() ? "Unknown" : enemy->GetName(), 
                guid.GetRawValue());
            return enemy;
        }
    }

    return nullptr; // No valid target
}

bool CastShatteringThrowAction::isUseful()
{
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
            LOG_INFO("playerbots", "Bot Name = {}, ShatteringThrowAction::isUseful: Valid target found: Name = {}, GUID = {}", 
                bot->GetName(), 
                enemy->GetName().empty() ? "Unknown" : enemy->GetName(), 
                guid.GetRawValue());

            return true;
        }
    }

    return false; // No valid targets within range
}

bool CastShatteringThrowAction::isPossible()
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    // Spell cooldown check
    if (!bot->HasSpell(64382))
    {
        LOG_INFO("playerbots",
            "CastShatteringThrowAction::isPossible - Spell not known. Bot: {}, Spell ID: {}",
            bot->GetName(), 64382);
        return false;
    }

    // Spell cooldown check
    if (bot->HasSpellCooldown(64382))
    {
        LOG_INFO("playerbots",
            "CastShatteringThrowAction::isPossible - Spell is on cooldown. Bot: {}, Spell ID: {}",
            bot->GetName(), 64382);
        return false;
    }

    // Range check: Shattering Throw is 30 yards    
    if (!bot->IsWithinDistInMap(target, 30.0f))
    {
        return false;
    }

    // Check line of sight
    if (!bot->IsWithinLOSInMap(target))
    {
        return false;
    }

    // If the minimal checks above pass, simply return true.
    LOG_INFO("playerbots",
        "CastShatteringThrowAction::isPossible - Passed, returning true. Bot: {}, Target: {}",
        bot->GetName(), target->GetName().empty() ? "Unknown" : target->GetName());
    return true;
}

bool CastShatteringThrowAction::Execute(Event event)
{
    LOG_INFO("playerbots", "Bot Name = {}, Executing Shattering Throw function", bot->GetName());
    Unit* target = GetTarget();
    if (!target)
        return false;

    LOG_INFO("playerbots", "Bot Name = {}, Casting Shattering Throw at target: Name = {}, GUID = {}", 
             bot->GetName(), 
             target->GetName().empty() ? "Unknown" : target->GetName(), 
             target->GetGUID().GetRawValue());

    return botAI->CastSpell("shattering throw", target);
}
