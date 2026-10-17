/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ACActions.h"
#include "ACTriggers.h"
#include "EncounterHelpers.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include <algorithm>
#include <iterator>
#include <vector>

using namespace EncounterHelpers;

// Tank will position Shirrak at the specified coordinates, up the stairs
bool ShirrakTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* shirrak = AI_VALUE2(Unit*, "find target", "shirrak the dead watcher");
    if (!shirrak)
        return false;

    if (bot->GetVictim() != shirrak)
        return Attack(shirrak);

    if (shirrak->GetVictim() != bot || !bot->IsWithinMeleeRange(shirrak) ||
        bot->GetHealthPct() < 30.0f)
    {
        return false;
    }

    constexpr float arrivalDist = 3.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, SHIRRAK_TANK_POSITION, arrivalDist, shirrak, moveX, moveY, backwards))
    {
        return false;
    }

    return MoveTo(
        bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool ShirrakFleeFocusFireAction::Execute(Event /*event*/)
{
    Creature* flare = bot->FindNearestCreature(NPC_FOCUS_FIRE, FLARE_SEARCH_RADIUS);
    if (!flare)
        return false;

    float currentDistance = bot->GetExactDist2d(flare);
    constexpr float safeDistance = 12.0f;
    if (currentDistance >= safeDistance)
        return false;

    bot->CastStop();
    float distanceToMove = safeDistance - currentDistance;
    return MoveAway(flare, distanceToMove);
}

// Ranged should keep distance from Shirrak, staying at the edge of the stairs
bool ShirrakRangedKeepDistanceAction::Execute(Event /*event*/)
{
    Unit* shirrak = AI_VALUE2(Unit*, "find target", "shirrak the dead watcher");
    if (!shirrak)
        return false;

    if (bot->GetExactDist(shirrak) - shirrak->GetCombatReach() > sPlayerbotAIConfig.SpellDistance)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> rangedBots;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && PlayerbotAI::IsRanged(member))
            rangedBots.push_back(member);
    }

    auto findIt = std::find(rangedBots.begin(), rangedBots.end(), bot);
    size_t botIndex = (findIt != rangedBots.end()) ? std::distance(rangedBots.begin(), findIt) : 0;
    size_t count = rangedBots.size();

    constexpr float arcSpan = M_PI / 2.0f;
    float arcCenter = M_PI;
    float arcStart = arcCenter - (arcSpan / 2.0f);
    float angle = (count <= 1)
        ? arcCenter : (arcStart + (arcSpan * (float)botIndex / (float)(count - 1)));

    Position const& position = SHIRRAK_RANGED_POSITION;
    constexpr float spreadRadius = 3.0f;
    float targetX = position.GetPositionX() + cos(angle) * spreadRadius;
    float targetY = position.GetPositionY() + sin(angle) * spreadRadius;

    float distToSpot = bot->GetExactDist2d(targetX, targetY);
    if (distToSpot <= 4.0f)
        return false;

    float dX = targetX - bot->GetPositionX();
    float dY = targetY - bot->GetPositionY();
    float moveDist = std::min(2.0f, distToSpot);
    float moveX = bot->GetPositionX() + (dX / distToSpot) * moveDist;
    float moveY = bot->GetPositionY() + (dY / distToSpot) * moveDist;

    return MoveTo(
        bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}
