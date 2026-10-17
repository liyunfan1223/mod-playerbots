/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PriestActions.h"

#include "Event.h"
#include "Playerbots.h"
#include "PriestValues.h"

Unit* CastMassDispelOnEnemyAction::GetTarget()
{
    Unit* target = botAI->GetUnit(AI_VALUE(ObjectGuid, "mass dispel target"));
    return MassDispelTargetValue::IsValidTarget(botAI, target) ? target : nullptr;
}

bool CastMassDispelOnEnemyAction::isPossible()
{
    Unit* target = GetTarget();
    if (!target || (botAI->IsInVehicle() && !botAI->IsInVehicle(false, false, true)) ||
        bot->HasUnitState(UNIT_STATE_LOST_CONTROL) || bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL) || bot->isMoving())
        return false;

    // Mass Dispel is ground-targeted: check the destination, not a missing destination on a unit-only spell.
    uint32 const spellId = AI_VALUE2(uint32, "spell id", spell);
    return botAI->CanCastSpell(spellId, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());
}

bool CastRemoveShadowformAction::Execute(Event /*event*/)
{
    botAI->RemoveAura("shadowform");
    return true;
}

bool CastFadeAction::isUseful()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    return bot->GetGroup() && !(target && target->IsPlayer());  // players have no threat
}

bool CastRemoveShadowformAction::isUseful() { return botAI->HasAura("shadowform", AI_VALUE(Unit*, "self target")); }

Unit* CastPowerWordShieldOnAlmostFullHealthBelowAction::GetTarget()
{
    Group* group = bot->GetGroup();
    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* player = gref->GetSource();
        if (!player)
            continue;
        if (player->isDead())
        {
            continue;
        }
        if (player->GetHealthPct() > sPlayerbotAIConfig.AlmostFullHealth)
        {
            continue;
        }
        if (player->GetDistance2d(bot) > sPlayerbotAIConfig.SpellDistance)
        {
            continue;
        }
        if (botAI->HasAnyAuraOf(player, "weakened soul", "power word: shield", nullptr))
        {
            continue;
        }
        return player;
    }
    return nullptr;
}

bool CastPowerWordShieldOnAlmostFullHealthBelowAction::isUseful()
{
    Group* group = bot->GetGroup();
    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* player = gref->GetSource();
        if (!player)
            continue;
        if (player->isDead())
        {
            continue;
        }
        if (player->GetHealthPct() > sPlayerbotAIConfig.AlmostFullHealth)
        {
            continue;
        }
        if (player->GetDistance2d(bot) > sPlayerbotAIConfig.SpellDistance)
        {
            continue;
        }
        if (botAI->HasAnyAuraOf(player, "weakened soul", "power word: shield", nullptr))
        {
            continue;
        }
        return true;
    }
    return false;
}

Unit* CastPowerWordShieldOnNotFullAction::GetTarget()
{
    Group* group = bot->GetGroup();
    MinValueCalculator calc(100);
    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* player = gref->GetSource();
        if (!player)
            continue;
        if (player->isDead() || player->IsFullHealth())
        {
            continue;
        }
        if (player->GetDistance2d(bot) > sPlayerbotAIConfig.SpellDistance)
        {
            continue;
        }
        if (botAI->HasAnyAuraOf(player, "weakened soul", "power word: shield", nullptr))
        {
            continue;
        }
        calc.probe(player->GetHealthPct(), player);
    }
    return (Unit*)calc.param;
}

bool CastPowerWordShieldOnNotFullAction::isUseful()
{
    return GetTarget();
}
