/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "VHActions.h"
#include "Playerbots.h"
#include "VHTriggers.h"

bool AttackErekemAction::Execute(Event /*event*/)
{
    // Focus boss first, adds after
    Unit* boss = AI_VALUE2(Unit*, "find target", "erekem");
    if (!boss) { return false; }

    if (AI_VALUE(Unit*, "current target") != boss)
        {
            return Attack(boss);
        }
    return false;
}

bool AttackIchorGlobuleAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ichoron");
    if (!boss) { return false; }

    Unit* currentTarget = AI_VALUE(Unit*, "current target");

    // Tank prioritise boss if it's up
    if (PlayerbotAI::IsTank(bot) && !boss->HasAura(SPELL_DRAINED))
    {
        if (AI_VALUE(Unit*, "current target") != boss)
        {
            return Attack(boss);
        }
        return false;
    }

    // Target is not findable from threat table using AI_VALUE2(),
    // therefore need to search manually for the unit name
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");

    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && unit->GetEntry() == NPC_ICHOR_GLOBULE)
        {
            // Check IDs here, NOT Unit* pointers:
            // Don't keep swapping between sentries.
            // If we're already attacking one, don't retarget another
            if (currentTarget && currentTarget->GetEntry() == NPC_ICHOR_GLOBULE)
            {
                return false;
            }
            return Attack(unit);
        }
    }
    // No ichor globules left alive, fall back to targeting boss
    if (currentTarget != boss)
    {
        return Attack(boss);
    }

    return false;
}

bool AttackVoidSentryAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "zuramat the obliterator");
    if (!boss) { return false; }

    Unit* currentTarget = AI_VALUE(Unit*, "current target");

    // Target is not findable from threat table using AI_VALUE2(),
    // therefore need to search manually for the unit name
    // GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");
    GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");

    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && unit->GetEntry() == NPC_VOID_SENTRY)
        {
            // Check IDs here, NOT Unit* pointers:
            // Don't keep swapping between sentries.
            // If we're already attacking one, don't retarget another
            if (currentTarget && currentTarget->GetEntry() == NPC_VOID_SENTRY)
            {
                return false;
            }
            return Attack(unit);
        }
    }
    // No void sentries left alive, fall back to targeting boss
    if (currentTarget != boss)
    {
        return Attack(boss);
    }

    return false;
}
