#include "Playerbots.h"
#include "PitOfSaronActions.h"
#include "PitOfSaronStrategy.h"
#include "SharedDefines.h"

bool IckAndKrickAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "Ick");
    if (!boss)
        return false;

    std::vector<Unit*> orbs;
    bool orb = false;

    // First gather all orbs
    GuidVector npcs1 = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs1)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->IsAlive() && unit->HasAura(SPELL_EXPLODING_ORB_VISUAL))
        {
            orb = true;
            break;
        }
    }

    bool pursuit = bot->HasAura(SPELL_PURSUIT) || (!botAI->IsTank(bot) && boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_PURSUIT));
    bool poisonNova = boss->HasUnitState(UNIT_STATE_CASTING) && (boss->FindCurrentSpellBySpellId(SPELL_POISON_NOVA_POS) || boss->FindCurrentSpellBySpellId(SPELL_POISON_NOVA_POS_HC));
    bool explosiveBarrage = orb || boss->HasUnitState(UNIT_STATE_CASTING) && (boss->FindCurrentSpellBySpellId(SPELL_EXPLOSIVE_BARRAGE_ICK) || boss->FindCurrentSpellBySpellId(SPELL_EXPLOSIVE_BARRAGE_KRICK));
    bool isTank = botAI->IsTank(bot);

    if (pursuit && Pursuit(pursuit, boss))
        return true;

    if (poisonNova && PoisonNova(poisonNova, boss))
        return true;

    if (explosiveBarrage && ExplosiveBarrage(explosiveBarrage, boss))
        return true;

    if (isTank && !pursuit && !poisonNova && !explosiveBarrage)
    {
        if (TankPosition(boss))
            return true;
    }

    return false;
}

bool IckAndKrickAction::TankPosition(Unit* boss)
{
    if (botAI->HasAggro(boss))
    {
        float distance = bot->GetExactDist2d(ICKANDKRICK_TANK_POSITION.GetPositionX(), ICKANDKRICK_TANK_POSITION.GetPositionY());
        if (distance > 7.0f)
        {
            // Calculate direction vector
            float dirX = ICKANDKRICK_TANK_POSITION.GetPositionX() - bot->GetPositionX();
            float dirY = ICKANDKRICK_TANK_POSITION.GetPositionY() - bot->GetPositionY();
            float length = sqrt(dirX * dirX + dirY * dirY);
            dirX /= length;
            dirY /= length;

            // Move in increments of 3.0f
            float moveX = bot->GetPositionX() + dirX * 3.0f;
            float moveY = bot->GetPositionY() + dirY * 3.0f;

            return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}

bool IckAndKrickAction::Pursuit(bool pursuit, Unit* boss)
{
    // Only execute this action when pursuit is active and for non-tank players
    if (!pursuit || botAI->IsTank(bot))
        return false;

    // Get the tank position as a reference point
    Position tankPosition = ICKANDKRICK_TANK_POSITION;

    // Calculate distance from boss
    float distanceToBoss = bot->GetExactDist2d(boss);

    // If boss is close, move in a circular pattern
    if (distanceToBoss < 20.0f)
    {
        // Current bot position
        float x = bot->GetPositionX();
        float y = bot->GetPositionY();

        // Calculate vector from tank to bot
        float dx = x - tankPosition.GetPositionX();
        float dy = y - tankPosition.GetPositionY();

        // Normalize the vector
        float distance = std::sqrt(dx * dx + dy * dy);
        if (distance > 0)
        {
            dx /= distance;
            dy /= distance;
        }

        // Rotate the vector by 90 degrees to create circular movement
        // (x,y) rotated 90 degrees becomes (-y,x)
        float rotatedX = -dy;
        float rotatedY = dx;

        // Create a target position along the circular path
        // We want to maintain a specific radius from the tank
        float targetRadius = 20.0f;  // 20 feet radius circle around tank

        Position targetPos;
        targetPos.Relocate(tankPosition.GetPositionX() + rotatedX * targetRadius,
                           tankPosition.GetPositionY() + rotatedY * targetRadius, bot->GetPositionZ());

        // Move to the calculated circular position
        botAI->Reset();
        return MoveTo(bot->GetMapId(), targetPos.GetPositionX(), targetPos.GetPositionY(), bot->GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_COMBAT);

    }

    return false;
}

bool IckAndKrickAction::PoisonNova(bool poisonNova, Unit* boss)
{
    if (bot->GetExactDist2d(boss) < 20.0f && poisonNova)
        return MoveAway(boss, 11.0f);
    else if (poisonNova)
        return true;

    return false;
}


bool IckAndKrickAction::ExplosiveBarrage(bool explosiveBarrage, Unit* boss)
{
    std::vector<Unit*> orbs;
    Unit* closestOrb = nullptr;
    float closestDistance = std::numeric_limits<float>::max();

    // First gather all orbs
    GuidVector npcs1 = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs1)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->IsAlive() && unit->HasAura(SPELL_EXPLODING_ORB_VISUAL))
        {
            orbs.push_back(unit);
            float dist = bot->GetDistance(unit);
            if (dist < closestDistance)
            {
                closestDistance = dist;
                closestOrb = unit;
            }
        }
    }

    if (!orbs.empty() && closestOrb && bot->GetExactDist2d(closestOrb) < 7.0f)
    {
        // Get player positions to avoid moving toward them
        std::vector<Position> playerPositions;
        Group* group = bot->GetGroup();
        if (group)
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                if (Player* member = ref->GetSource())
                {
                    if (member != bot && member->IsAlive() && bot->GetMap() == member->GetMap() &&
                        bot->GetExactDist2d(member) < 20.0f)
                    {
                        playerPositions.push_back(member->GetPosition());
                    }
                }
            }
        }

        // Calculate several potential escape positions
        const int NUM_ANGLES = 16;  // Check 16 different directions
        const float SAFE_DISTANCE = 7.0f;
        const float MAX_BOSS_DISTANCE = 30.0f;  // Maximum allowed distance from boss
        const float ANGLE_INCREMENT = 2 * M_PI / NUM_ANGLES;

        Position bestPos;
        bool foundValidPos = false;
        float bestScore = -1.0f;

        for (int i = 0; i < NUM_ANGLES; i++)
        {
            float angle = i * ANGLE_INCREMENT;
            Position potentialPos;
            potentialPos.m_positionX = bot->GetPositionX() + SAFE_DISTANCE * cos(angle);
            potentialPos.m_positionY = bot->GetPositionY() + SAFE_DISTANCE * sin(angle);
            potentialPos.m_positionZ = bot->GetPositionZ();

            // Check if position is valid (not in a wall)
            if (!bot->IsWithinLOS(potentialPos.m_positionX, potentialPos.m_positionY, potentialPos.m_positionZ))
                continue;

            // Check if position is within maximum allowed distance from boss
            if (boss && sqrt(pow(potentialPos.m_positionX - boss->GetPositionX(), 2) +
                             pow(potentialPos.m_positionY - boss->GetPositionY(), 2)) > MAX_BOSS_DISTANCE)
                continue;

            // Score this position based on:
            // 1. Distance from all orbs (farther is better)
            // 2. Distance from other players (farther is better)
            // 3. Distance from boss (closer is better, but still safe)
            float score = 0.0f;

            // Check distance from all orbs
            float minOrbDist = std::numeric_limits<float>::max();
            for (Unit* orb : orbs)
            {
                float orbDist = sqrt(pow(potentialPos.m_positionX - orb->GetPositionX(), 2) +
                                     pow(potentialPos.m_positionY - orb->GetPositionY(), 2));
                minOrbDist = std::min(minOrbDist, orbDist);
            }
            score += minOrbDist * 2.0f;  // Weight orb distance more heavily

            // Check distance from other players
            for (const Position& playerPos : playerPositions)
            {
                float playerDist = sqrt(pow(potentialPos.m_positionX - playerPos.m_positionX, 2) +
                                        pow(potentialPos.m_positionY - playerPos.m_positionY, 2));
                score += std::min(playerDist, 10.0f);  // Cap player distance contribution
            }

            // Factor in proximity to boss (closer is better, as long as we're safe from orbs)
            if (boss)
            {
                float bossDist = sqrt(pow(potentialPos.m_positionX - boss->GetPositionX(), 2) +
                                      pow(potentialPos.m_positionY - boss->GetPositionY(), 2));
                // Add points for being closer to boss (inverse relationship)
                // but only if we're safely away from orbs
                if (minOrbDist > SAFE_DISTANCE)
                {
                    score += (MAX_BOSS_DISTANCE - bossDist) * 0.5f;
                }
            }

            // If this is the best position so far, remember it
            if (score > bestScore)
            {
                bestScore = score;
                bestPos = potentialPos;
                foundValidPos = true;
            }
        }

        // If we found a valid position, move there
        if (foundValidPos)
        {
            botAI->Reset();
            // Use MoveTo instead of FleePosition since we already calculated the exact position
            return MoveTo(bot->GetMapId(), bestPos.m_positionX, bestPos.m_positionY, bot->GetPositionZ(), false, false,
                          false, true, MovementPriority::MOVEMENT_COMBAT);
        }
        else
        {
            return FleePosition(closestOrb->GetPosition(), 7.0f, 250U);
        }
    }

    return false;
}

bool TyrannusAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "scourgelord tyrannus");
    if (!boss)
        return false;

    bool rangedSpread = false;

    if (botAI->IsRanged(bot) && boss->HealthBelowPct(99))
        rangedSpread = true;

    if (rangedSpread && RangedSpread(rangedSpread))
        return true;

    return false;
}

bool TyrannusAction::RangedSpread(bool rangedSpread)
{
    float radius = 10.0f;
    float moveIncrement = 3.0f;

    GuidVector members = AI_VALUE(GuidVector, "group members");
    if (botAI->IsRanged(bot) && rangedSpread)
    {
        // Ranged: spread from other members
        for (auto& member : members)
        {
            Unit* unit = botAI->GetUnit(member);
            if (!unit || !unit->IsAlive() || unit == bot)
                continue;

            float dist = bot->GetExactDist2d(unit);
            if (dist < radius)
            {
                float moveDistance = std::min(moveIncrement, radius - dist + 1.0f);
                return FleePosition(unit->GetPosition(), moveDistance, 250U);
            }
        }
    }

    return false;
}
