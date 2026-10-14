/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RogueTriggers.h"
#include "GenericTriggers.h"
#include "Playerbots.h"
#include "ServerFacade.h"

namespace
{
constexpr uint32 SPELL_STEALTH = 1784;
constexpr uint32 SPELL_SPRINT_RANK_1 = 2983;
}

// bool AdrenalineRushTrigger::isPossible()
// {
//     return !botAI->HasAura("stealth", bot);
// }

bool UnstealthTrigger::IsActive()
{
    if (!botAI->HasAura("stealth", bot))
        return false;

    if (!AI_VALUE2(bool, "moving", "self target"))
        return false;

    if (!AI_VALUE(uint8, "attacker count"))
        return true;

    return botAI->GetMaster() &&
           ServerFacade::instance().IsDistanceGreaterThan(AI_VALUE2(float, "distance", "group leader"), 10.0f) &&
           AI_VALUE2(bool, "moving", "group leader");
}

bool StealthTrigger::IsActive()
{
    if (bot->HasAura(SPELL_STEALTH) || bot->IsInCombat() || bot->HasSpellCooldown(SPELL_STEALTH))
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

    return target && ServerFacade::instance().GetDistance2d(bot, target) < distance;
}

bool SapTrigger::IsPossible() { return bot->GetLevel() > 10 && botAI->HasSpell("sap") && !bot->IsInCombat(); }

bool SprintTrigger::IsPossible() { return bot->HasSpell(SPELL_SPRINT_RANK_1); }

bool SprintTrigger::IsActive()
{
    if (bot->HasSpellCooldown(SPELL_SPRINT_RANK_1))
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
           (ServerFacade::instance().IsDistanceGreaterThan(AI_VALUE2(float, "distance", "dps target"), distance) ||
            ServerFacade::instance().IsDistanceGreaterThan(AI_VALUE2(float, "distance", "enemy player target"), distance));
}

bool ExposeArmorTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    return DebuffTrigger::IsActive() && !botAI->HasAura("sunder armor", target, false, false, -1, true) &&
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
