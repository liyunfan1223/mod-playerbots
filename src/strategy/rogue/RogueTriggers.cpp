/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "RogueTriggers.h"

#include "GenericTriggers.h"
#include "Playerbots.h"
#include "ServerFacade.h"

// bool AdrenalineRushTrigger::isPossible()
// {
//     return !botAI->HasAura("stealth", bot);
// }

bool UnstealthTrigger::IsActive()
{
    if (!botAI->HasAura("stealth", bot))
        return false;

    return botAI->HasAura("stealth", bot) && !AI_VALUE(uint8, "attacker count") &&
           (AI_VALUE2(bool, "moving", "self target") &&
            ((botAI->GetMaster() &&
              sServerFacade->IsDistanceGreaterThan(AI_VALUE2(float, "distance", "master target"), 10.0f) &&
              AI_VALUE2(bool, "moving", "master target")) ||
             !AI_VALUE(uint8, "attacker count")));
}

bool StealthTrigger::IsActive()
{
    if (botAI->HasAura("stealth", bot) || bot->IsInCombat() || bot->HasSpellCooldown(1784))
        return false;

    float distance = 30.f;

    Unit* target = AI_VALUE(Unit*, "enemy player target");
    if (target && !target->IsInWorld())
    {
        return false;
    }
    if (!target)
        target = AI_VALUE(Unit*, "grind target");

    if (!target)
        target = AI_VALUE(Unit*, "dps target");

    if (!target)
        return false;

    if (target && target->GetVictim())
        distance -= 10;

    if (target->isMoving() && target->GetVictim())
        distance -= 10;

    if (bot->InBattleground())
        distance += 15;

    if (bot->InArena())
        distance += 15;

    return target && sServerFacade->GetDistance2d(bot, target) < distance;
}

bool SapTrigger::IsPossible() { return bot->GetLevel() > 10 && bot->HasSpell(6770) && !bot->IsInCombat(); }

bool SprintTrigger::IsPossible() { return bot->HasSpell(2983); }

bool SprintTrigger::IsActive()
{
    if (bot->HasSpellCooldown(2983))
        return false;

    float distance = botAI->GetMaster() ? 45.0f : 35.0f;
    if (botAI->HasAura("stealth", bot))
        distance -= 10;

    bool targeted = false;

    Unit* dps = AI_VALUE(Unit*, "dps target");
    Unit* enemyPlayer = AI_VALUE(Unit*, "enemy player target");

    if (enemyPlayer && !enemyPlayer->IsInWorld())
    {
        return false;
    }
    if (dps)
        targeted = (dps == AI_VALUE(Unit*, "current target"));

    if (enemyPlayer && !targeted)
        targeted = (enemyPlayer == AI_VALUE(Unit*, "current target"));

    if (!targeted)
        return false;

    if ((dps && dps->IsInCombat()) || enemyPlayer)
        distance -= 10;

    return AI_VALUE2(bool, "moving", "self target") &&
           (AI_VALUE2(bool, "moving", "dps target") || AI_VALUE2(bool, "moving", "enemy player target")) && targeted &&
           (sServerFacade->IsDistanceGreaterThan(AI_VALUE2(float, "distance", "dps target"), distance) ||
            sServerFacade->IsDistanceGreaterThan(AI_VALUE2(float, "distance", "enemy player target"), distance));
}

bool ExposeArmorTrigger::IsActive()
{
    return DebuffTrigger::IsActive() && !botAI->HasAura("sunder armor", bot, false, false, -1, true) &&
           AI_VALUE2(uint8, "combo", "current target") <= 3;
}

bool MainHandWeaponNoEnchantTrigger::IsActive()
{
    Item* const itemForSpell = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    if (!itemForSpell || itemForSpell->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
        return false;
    return true;
}

bool OffHandWeaponNoEnchantTrigger::IsActive()
{
    Item* const itemForSpell = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
    if (!itemForSpell || itemForSpell->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
        return false;
    return true;
}
