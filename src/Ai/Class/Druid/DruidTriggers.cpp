/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "DruidTriggers.h"
#include "DynamicObject.h"
#include "Player.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "TargetValue.h"

bool MarkOfTheWildTrigger::IsActive()
{
    return BuffTrigger::IsActive() && !botAI->HasAura("gift of the wild", GetTarget());
}

bool ThornsOnPartyTrigger::IsActive()
{
    return BuffOnPartyTrigger::IsActive() && !botAI->HasAura("thorns", GetTarget());
}

bool EntanglingRootsKiteTrigger::IsActive()
{
    return DebuffTrigger::IsActive() && AI_VALUE(uint8, "attacker count") < 3 && !GetTarget()->GetPower(POWER_MANA);
}

bool ThornsTrigger::IsActive() { return BuffTrigger::IsActive() && !botAI->HasAura("thorns", GetTarget()); }

bool BearFormTrigger::IsActive() { return !botAI->HasAnyAuraOf(bot, "bear form", "dire bear form", nullptr); }

bool TreeFormTrigger::IsActive()
{
    return !bot->HasAura(SPELL_TREE_OF_LIFE);
}

bool CatFormTrigger::IsActive() { return !botAI->HasAura("cat form", bot); }

bool HealerShouldAttackAndNotBlanketingTrigger::IsActive()
{
    if (bot->GetGroup() && bot->HasSpell(SPELL_TREE_OF_LIFE) && botAI->HasStrategy("blanketing", BOT_STATE_COMBAT))
        return false;

    return HealerShouldAttackTrigger::IsActive();
}

Unit* BlanketHotTrigger::GetTarget() { return botAI->GetUnit(AI_VALUE2(ObjectGuid, "blanket hot target", _spell)); }

bool BlanketHotTrigger::IsActive()
{
    if (!bot->GetGroup() || !botAI->HasStrategy("resto", BOT_STATE_COMBAT))
        return false;

    return GetTarget();
}

bool AquaticFormTrigger::IsActive()
{
    return !bot->IsInCombat() && !botAI->HasAura("aquatic form", bot) &&
           bot->GetLiquidData().Status == LIQUID_MAP_UNDER_WATER;
}

bool ProwlTrigger::IsActive()
{
    if (botAI->HasAura("prowl", bot) || bot->IsInCombat())
        return false;

    if (!botAI->HasSpell("prowl"))
        return false;

    uint32 const prowlId = AI_VALUE2(uint32, "spell id", "prowl");
    if (bot->HasSpellCooldown(prowlId))
        return false;

    float distance = 30.f;

    Unit* target = AI_VALUE(Unit*, "enemy player target");
    if (target && !target->IsInWorld())
        return false;
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

const std::set<uint32> HurricaneChannelCheckTrigger::HURRICANE_SPELL_IDS = {
    16914,  // Hurricane Rank 1
    17401,  // Hurricane Rank 2
    17402,  // Hurricane Rank 3
    27012,  // Hurricane Rank 4
    48467   // Hurricane Rank 5
};

bool HurricaneChannelCheckTrigger::IsActive()
{
    if (Spell* spell = bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
    {
        if (!HURRICANE_SPELL_IDS.count(spell->m_spellInfo->Id))
            return false;

        // Find this bot's own Hurricane DynamicObject
        DynamicObject* dynObj = nullptr;
        for (uint32 spellId : HURRICANE_SPELL_IDS)
        {
            dynObj = bot->GetDynObject(spellId);
            if (dynObj)
                break;
        }

        if (!dynObj)
            return false;

        // Count attackers actually inside the Hurricane AoE
        return !HasEnoughAoeTargets(botAI, dynObj->GetPosition(), dynObj->GetRadius(), minEnemies);
    }

    return false;
}
