/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "NaxxActions.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "SharedDefines.h"

bool GluthChooseTargetAction::Execute(Event /*event*/)
{
    if (!helper.UpdateBossAI())
        return false;

    GuidVector attackers = context->GetValue<GuidVector>("possible targets")->Get();
    Unit* target = nullptr;
    Unit* target_boss = nullptr;
    std::vector<Unit*> target_zombies;
    for (GuidVector::iterator i = attackers.begin(); i != attackers.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (!unit)
            continue;

        if (!unit->IsAlive())
            continue;

        if (botAI->EqualLowercaseName(unit->GetName(), "zombie chow"))
            target_zombies.push_back(unit);

        if (botAI->EqualLowercaseName(unit->GetName(), "gluth"))
            target_boss = unit;
    }
    if (PlayerbotAI::IsMainTank(bot) || PlayerbotAI::IsAssistTankOfIndex(bot, 0))
        target = target_boss;
    else if (PlayerbotAI::IsAssistTankOfIndex(bot, 1))
    {
        for (Unit* t : target_zombies)
        {
            if (t->GetHealthPct() > helper.decimatedZombiePct && t->GetVictim() != bot && t->GetDistance2d(bot) <= 10.0f)
            {
                if (!target || t->GetDistance2d(bot) < target->GetDistance2d(bot))
                    target = t;
            }
        }
    }
    else if (botAI->GetClassIndex(bot, CLASS_HUNTER) == 0 || botAI->GetClassIndex(bot, CLASS_HUNTER) == 1)
    {
        // prevent zombie go straight to gluth
        for (Unit* t : target_zombies)
        {
            if (t->GetHealthPct() > helper.decimatedZombiePct && t->GetVictim() == target_boss &&
                t->GetDistance2d(bot) <= sPlayerbotAIConfig.SpellDistance)
            {
                if (!target || t->GetDistance2d(bot) < target->GetDistance2d(bot))
                    target = t;
            }
        }
        if (!target)
            target = target_boss;
    }
    else
    {
        for (Unit* t : target_zombies)
        {
            if (t->GetHealthPct() <= helper.decimatedZombiePct)
            {
                if (target == nullptr ||
                    target->GetDistance2d(helper.mainTankPos25.first, helper.mainTankPos25.second) >
                        t->GetDistance2d(helper.mainTankPos25.first, helper.mainTankPos25.second))
                    target = t;
            }
        }
        if (target == nullptr)
            target = target_boss;
    }
    if (!target || context->GetValue<Unit*>("current target")->Get() == target)
        return false;

    if (target_boss && target == target_boss)
        return Attack(target, true);

    return Attack(target, false);
    // return Attack(target);
}

bool GluthPositionAction::Execute(Event /*event*/)
{
    if (!helper.UpdateBossAI())
        return false;

    bool raid25 = bot->GetRaidDifficulty() == RAID_DIFFICULTY_25MAN_NORMAL;
    if (PlayerbotAI::IsMainTank(bot) || PlayerbotAI::IsAssistTankOfIndex(bot, 0))
    {
        if (AI_VALUE2(bool, "has aggro", "boss target"))
        {
            if (raid25)
            {
                if (MoveTo(NAXX_MAP_ID, helper.mainTankPos25.first, helper.mainTankPos25.second, bot->GetPositionZ(), false, false, false,
                           false, MovementPriority::MOVEMENT_COMBAT))
                    return true;

                return MoveInside(NAXX_MAP_ID, helper.mainTankPos25.first, helper.mainTankPos25.second, bot->GetPositionZ(), 2.0f,
                                  MovementPriority::MOVEMENT_COMBAT);
            }
            else
            {
                if (MoveTo(NAXX_MAP_ID, helper.mainTankPos10.first, helper.mainTankPos10.second, bot->GetPositionZ(), false, false, false,
                           false, MovementPriority::MOVEMENT_COMBAT))
                    return true;

                return MoveInside(NAXX_MAP_ID, helper.mainTankPos10.first, helper.mainTankPos10.second, bot->GetPositionZ(), 2.0f,
                                  MovementPriority::MOVEMENT_COMBAT);
            }
        }
    }
    else if (PlayerbotAI::IsAssistTankOfIndex(bot, 1))
    {
        if (helper.BeforeDecimate())
        {
            if (MoveTo(bot->GetMapId(), helper.beforeDecimatePos.first, helper.beforeDecimatePos.second, bot->GetPositionZ(), false, false,
                       false, false, MovementPriority::MOVEMENT_COMBAT))
                return true;

            return MoveInside(bot->GetMapId(), helper.beforeDecimatePos.first, helper.beforeDecimatePos.second, bot->GetPositionZ(), 2.0f,
                              MovementPriority::MOVEMENT_COMBAT);
        }
        else
        {
            if (AI_VALUE2(bool, "has aggro", "current target"))
            {
                uint32 nearest = FindNearestWaypoint();
                uint32 next_point = (nearest + 1) % intervals;
                return MoveTo(bot->GetMapId(), waypoints[next_point].first, waypoints[next_point].second, bot->GetPositionZ(),
                              false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
            }
        }
    }
    else if (PlayerbotAI::IsRangedDps(bot))
    {
        if (raid25)
        {
            if (botAI->GetClassIndex(bot, CLASS_HUNTER) == 0)
                return MoveInside(NAXX_MAP_ID, helper.leftSlowDownPos.first, helper.leftSlowDownPos.second, bot->GetPositionZ(), 0.0f,
                                  MovementPriority::MOVEMENT_COMBAT);

            if (botAI->GetClassIndex(bot, CLASS_HUNTER) == 1)
                return MoveInside(NAXX_MAP_ID, helper.rightSlowDownPos.first, helper.rightSlowDownPos.second, bot->GetPositionZ(), 0.0f,
                                  MovementPriority::MOVEMENT_COMBAT);
        }
        return MoveInside(NAXX_MAP_ID, helper.rangedPos.first, helper.rangedPos.second, bot->GetPositionZ(), 3.0f,
                          MovementPriority::MOVEMENT_COMBAT);
    }
    else if (PlayerbotAI::IsHeal(bot))
        return MoveInside(NAXX_MAP_ID, helper.healPos.first, helper.healPos.second, bot->GetPositionZ(), 0.0f,
                          MovementPriority::MOVEMENT_COMBAT);
    return false;
}

bool GluthSlowdownAction::Execute(Event /*event*/)
{
    if (!helper.UpdateBossAI())
        return false;

    bool raid25 = bot->GetRaidDifficulty() == RAID_DIFFICULTY_25MAN_NORMAL;
    if (!raid25)
        return false;

    if (helper.JustStartCombat())
        return false;

    switch (bot->getClass())
    {
        case CLASS_HUNTER:
            return botAI->CastSpell("frost trap", bot);
            break;
        default:
            break;
    }
    return false;
}
