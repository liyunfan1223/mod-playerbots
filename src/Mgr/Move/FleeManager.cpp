/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "FleeManager.h"
#include "Playerbots.h"
#include "ServerFacade.h"

FleeManager::FleeManager(Player* bot, float maxAllowedDistance, float /*followAngle*/, bool forceMaxDistance,
                         WorldPosition startPosition)
    : bot(bot),
      maxAllowedDistance(maxAllowedDistance),
      forceMaxDistance(forceMaxDistance),
      startPosition(startPosition ? startPosition : WorldPosition(bot))
{
}

void FleeManager::calculateDistanceToCreatures(FleePoint* point)
{
    point->minDistance = -1.0f;
    point->sumDistance = 0.0f;
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
    {
        return;
    }
    GuidVector units = *botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los");
    for (GuidVector::iterator i = units.begin(); i != units.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (!unit)
            continue;

        float d = ServerFacade::instance().GetDistance2d(unit, point->x, point->y);
        point->sumDistance += d;
        if (point->minDistance < 0 || point->minDistance > d)
            point->minDistance = d;
    }
}

bool intersectsOri(float angle, std::vector<float>& angles, float angleIncrement)
{
    for (std::vector<float>::iterator i = angles.begin(); i != angles.end(); ++i)
    {
        float ori = *i;
        if (abs(angle - ori) < angleIncrement)
            return true;
    }

    return false;
}

void FleeManager::calculatePossibleDestinations(std::vector<FleePoint*>& points)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
    {
        return;
    }
    Unit* target = *botAI->GetAiObjectContext()->GetValue<Unit*>("current target");

    float botPosX = startPosition.GetPositionX();
    float botPosY = startPosition.GetPositionY();
    float botPosZ = startPosition.GetPositionZ();

    FleePoint start(botPosX, botPosY, botPosZ);
    calculateDistanceToCreatures(&start);

    std::vector<float> enemyOri;
    GuidVector units = *botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los");
    for (GuidVector::iterator i = units.begin(); i != units.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (!unit)
            continue;

        float ori = bot->GetAngle(unit);
        enemyOri.push_back(ori);
    }

    float distIncrement = std::max(sPlayerbotAIConfig.FollowDistance,
                                   (maxAllowedDistance - sPlayerbotAIConfig.TooCloseDistance) / 10.0f);
    for (float dist = maxAllowedDistance; dist >= sPlayerbotAIConfig.TooCloseDistance; dist -= distIncrement)
    {
        float angleIncrement = std::max(M_PI / 20, M_PI / 4 / (1.0 + dist - sPlayerbotAIConfig.TooCloseDistance));
        for (float add = 0.0f; add < M_PI / 4 + angleIncrement; add += angleIncrement)
        {
            for (float angle = add; angle < add + 2 * static_cast<float>(M_PI) + angleIncrement;
                 angle += static_cast<float>(M_PI) / 4)
            {
                if (intersectsOri(angle, enemyOri, angleIncrement))
                    continue;

                float x = botPosX + cos(angle) * maxAllowedDistance, y = botPosY + sin(angle) * maxAllowedDistance,
                      z = botPosZ + CONTACT_DISTANCE;
                if (forceMaxDistance &&
                    ServerFacade::instance().IsDistanceLessThan(ServerFacade::instance().GetDistance2d(bot, x, y),
                                                      maxAllowedDistance - sPlayerbotAIConfig.TooCloseDistance))
                    continue;

                bot->UpdateAllowedPositionZ(x, y, z);

                Map* map = startPosition.getMap();
                if (map && map->IsInWater(bot->GetPhaseMask(), x, y, z, bot->GetCollisionHeight()))
                    continue;

                if (!bot->IsWithinLOS(x, y, z) || (target && !target->IsWithinLOS(x, y, z)))
                    continue;

                FleePoint* point = new FleePoint(x, y, z);
                calculateDistanceToCreatures(point);

                if (ServerFacade::instance().IsDistanceGreaterOrEqualThan(point->minDistance - start.minDistance,
                                                                sPlayerbotAIConfig.FollowDistance))
                    points.push_back(point);
                else
                    delete point;
            }
        }
    }
}

void FleeManager::cleanup(std::vector<FleePoint*>& points)
{
    for (std::vector<FleePoint*>::iterator i = points.begin(); i != points.end(); i++)
    {
        delete *i;
    }

    points.clear();
}

bool FleeManager::isBetterThan(FleePoint* point, FleePoint* other)
{
    return point->sumDistance - other->sumDistance > 0;
}

FleePoint* FleeManager::selectOptimalDestination(std::vector<FleePoint*>& points)
{
    FleePoint* best = nullptr;
    for (std::vector<FleePoint*>::iterator i = points.begin(); i != points.end(); i++)
    {
        FleePoint* point = *i;
        if (!best || isBetterThan(point, best))
            best = point;
    }

    return best;
}

bool FleeManager::CalculateDestination(float* rx, float* ry, float* rz)
{
    std::vector<FleePoint*> points;
    calculatePossibleDestinations(points);

    FleePoint* point = selectOptimalDestination(points);
    if (!point)
    {
        cleanup(points);
        return false;
    }

    *rx = point->x;
    *ry = point->y;
    *rz = point->z;

    cleanup(points);
    return true;
}

bool FleeManager::isUseful()
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
    {
        return false;
    }
    GuidVector units = *botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los");
    for (GuidVector::iterator i = units.begin(); i != units.end(); ++i)
    {
        Creature* creature = botAI->GetCreature(*i);
        if (!creature)
            continue;

        if (startPosition.sqDistance(WorldPosition(creature)) <
            creature->GetAttackDistance(bot) * creature->GetAttackDistance(bot))
            return true;

        // float d = ServerFacade::instance().GetDistance2d(unit, bot);
        // if (ServerFacade::instance().IsDistanceLessThan(d, sPlayerbotAIConfig.AggroDistance)) return true;
    }

    return false;
}
