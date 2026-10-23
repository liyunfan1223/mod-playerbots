/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UKActions.h"
#include "Playerbots.h"

bool AttackFrostTombAction::isUseful() { return !PlayerbotAI::IsHeal(bot); }
bool AttackFrostTombAction::Execute(Event /*event*/)
{
    Unit* frostTomb = nullptr;

    // Target is not findable from threat table using AI_VALUE2(),
    // therefore need to search manually for the unit name
    GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");

    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->GetEntry() == NPC_FROST_TOMB)
        {
            frostTomb = unit;
            break;
        }
    }
    if (!frostTomb || AI_VALUE(Unit*, "current target") == frostTomb)
    {
        return false;
    }
    return Attack(frostTomb);
}

// TODO: Possibly add player stacking behaviour close to tank, to prevent Skarvald charging ranged
bool AttackDalronnAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "dalronn the controller");
    if (!boss) { return false; }

    if (AI_VALUE(Unit*, "current target") == boss)
    {
        return false;
    }
    return Attack(boss);
}

bool IngvarDodgeSmashAction::isUseful() { return !AI_VALUE2(bool, "behind", "current target"); }
bool IngvarDodgeSmashAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ingvar the plunderer");
    if (!boss) { return false; }

    float distance = bot->GetExactDist2d(boss->GetPosition());
    // Extra units to move into the boss, instead of being just 1 pixel past his midpoint.
    // Can be adjusted - this value tends to mirror how a human would play,
    // and visibly ensures you won't get hit while not creating excessive movements.
    float distanceExtra = 2.0f;
    return Move(bot->GetAngle(boss), distance + distanceExtra);
}

bool IngvarSmashReturnAction::isUseful() { return AI_VALUE2(bool, "behind", "current target"); }
bool IngvarSmashReturnAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ingvar the plunderer");
    if (!boss) { return false; }

    float distance = bot->GetExactDist2d(boss->GetPosition());
    return Move(bot->GetAngle(boss), distance + bot->GetMeleeReach());
}
