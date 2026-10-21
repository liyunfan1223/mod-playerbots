// Ick and Krick bot tactics: kite Pursuit around the room, clear Poison Nova, dodge the orbs and hold the tank spot.

#include <limits>

#include "Group.h"
#include "Playerbots.h"
#include "PoSActions.h"
#include "PoSMultipliers.h"
#include "PoSTriggers.h"

namespace
{
Position const ICKANDKRICK_TANK_POSITION = Position(816.8508f, 102.331505f, 509.1586f);
// boss_ick evades when its target is more than 80 yd from here
Position const ICKANDKRICK_ROOM_CENTER = Position(836.65f, 115.08f, 510.0f);

bool IsBarrageCasting(Unit* boss)
{
    return boss->HasUnitState(UNIT_STATE_CASTING) && (boss->FindCurrentSpellBySpellId(SPELL_EXPLOSIVE_BARRAGE_ICK) ||
                                                      boss->FindCurrentSpellBySpellId(SPELL_EXPLOSIVE_BARRAGE_KRICK));
}
}  // namespace

bool IckAndKrickTrigger::IsActive() { return AI_VALUE2(Unit*, "find target", "Ick"); }

bool IckAndKrickAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "Ick");
    if (!boss)
        return false;

    bool orb = false;
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (ObjectGuid const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() && unit->HasAura(SPELL_EXPLODING_ORB_VISUAL))
        {
            orb = true;
            break;
        }
    }

    // Only the fixated player kites, tank included; everyone else keeps fighting
    bool pursuit = bot->HasAura(SPELL_PURSUIT);
    bool poisonNova = IsPoisonNovaCasting(boss);
    bool explosiveBarrage = orb || IsBarrageCasting(boss);
    if (pursuit && Pursuit(boss))
        return true;

    if (poisonNova && PoisonNova(boss))
        return true;

    if (explosiveBarrage && ExplosiveBarrage(boss))
        return true;

    return PlayerbotAI::IsTank(bot) && !pursuit && !poisonNova && !explosiveBarrage && TankPosition(boss);
}

bool IckAndKrickAction::TankPosition(Unit* boss)
{
    if (!botAI->HasAggro(boss))
        return false;

    float spotX = ICKANDKRICK_TANK_POSITION.GetPositionX();
    float spotY = ICKANDKRICK_TANK_POSITION.GetPositionY();
    float distance = bot->GetExactDist2d(spotX, spotY);
    if (distance <= PitOfSaron::IckTankSpotTolerance)
        return false;

    // A Toxic Waste pool on the spot: tank him where he is until it fades rather than drag him into it
    std::vector<PitOfSaron::Hazard> hazards = FindPoSGroundHazards(bot);
    if (PitOfSaron::InHazard(hazards, spotX, spotY, 3.0f))
        return false;

    float moveX = bot->GetPositionX() + (spotX - bot->GetPositionX()) / distance * PitOfSaron::IckTankStep;
    float moveY = bot->GetPositionY() + (spotY - bot->GetPositionY()) / distance * PitOfSaron::IckTankStep;
    if (PitOfSaron::InHazard(hazards, moveX, moveY, 1.5f))
        return false;

    return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT);
}

bool IckAndKrickAction::Pursuit(Unit* boss)
{
    // Keep running a circle around the room for the whole 12 s fixate, always on the side away from Ick;
    // the room centre is the script's evade anchor, so the circle never pulls him out of it
    if (bot->GetExactDist2d(boss) > PitOfSaron::PursuitKeepRunningRange && bot->isMoving())
        return true;

    Position const center = ICKANDKRICK_ROOM_CENTER;
    float botAngle =
        std::atan2(bot->GetPositionY() - center.GetPositionY(), bot->GetPositionX() - center.GetPositionX());
    float bossAngle =
        std::atan2(boss->GetPositionY() - center.GetPositionY(), boss->GetPositionX() - center.GetPositionX());
    float ahead = PitOfSaron::PursuitDirection(botAngle, bossAngle);

    std::vector<PitOfSaron::Hazard> hazards = FindPoSGroundHazards(bot);
    for (float radius : PitOfSaron::PursuitRadii)
    {
        for (float step : PitOfSaron::PursuitSteps)
        {
            float angle = botAngle + ahead * step;
            float x = center.GetPositionX() + radius * std::cos(angle);
            float y = center.GetPositionY() + radius * std::sin(angle);
            float z = bot->GetPositionZ();
            if (PitOfSaron::InHazard(hazards, x, y, 1.5f))
                continue;

            bot->UpdateAllowedPositionZ(x, y, z);
            if (std::fabs(z - bot->GetPositionZ()) > PitOfSaron::MaxClimb || !bot->IsWithinLOS(x, y, z + 1.0f))
                continue;

            botAI->Reset();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_FORCED);
        }
    }

    return MoveAway(boss, PitOfSaron::PursuitFallbackDistance);
}

bool IckAndKrickAction::PoisonNova(Unit* boss)
{
    float step = PitOfSaron::NovaStepOut(bot->GetExactDist2d(boss));
    if (step > 0.0f)
        return MoveAway(boss, step);

    // Clear of the nova: wait it out, unless in a pool; the hazard step then keeps the same distance
    return !PitOfSaron::InHazard(FindPoSGroundHazards(bot), bot->GetPositionX(), bot->GetPositionY(), 1.0f);
}

bool IckAndKrickAction::ExplosiveBarrage(Unit* boss)
{
    std::vector<Unit*> orbs;
    Unit* closestOrb = nullptr;
    float closestDistance = std::numeric_limits<float>::max();
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (ObjectGuid const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || !unit->HasAura(SPELL_EXPLODING_ORB_VISUAL))
            continue;

        orbs.push_back(unit);
        float dist = bot->GetDistance(unit);
        if (dist < closestDistance)
        {
            closestDistance = dist;
            closestOrb = unit;
        }
    }

    if (!closestOrb || bot->GetExactDist2d(closestOrb) >= PitOfSaron::OrbDangerRange)
        return false;

    // Stay apart from the others so one orb does not catch two players
    std::vector<Position> players;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member != bot && member->IsAlive() && bot->GetMap() == member->GetMap() &&
                bot->GetExactDist2d(member) < PitOfSaron::BarrageNeighbourRange)
                players.push_back(member->GetPosition());
        }
    }

    float bestScore = -1.0f;
    float bestX = 0.0f, bestY = 0.0f;
    for (int i = 0; i < PitOfSaron::BarrageDirections; ++i)
    {
        float angle = float(i) * 2.0f * float(M_PI) / float(PitOfSaron::BarrageDirections);
        float x = bot->GetPositionX() + PitOfSaron::BarrageStepDistance * std::cos(angle);
        float y = bot->GetPositionY() + PitOfSaron::BarrageStepDistance * std::sin(angle);
        if (!bot->IsWithinLOS(x, y, bot->GetPositionZ()))
            continue;

        float bossDist = boss->GetDistance2d(x, y);
        if (bossDist > PitOfSaron::BarrageMaxBossDistance)
            continue;

        float nearestOrb = std::numeric_limits<float>::max();
        for (Unit* orb : orbs)
            nearestOrb = std::min(nearestOrb, orb->GetDistance2d(x, y));

        float spacing = 0.0f;
        for (Position const& player : players)
            spacing += PitOfSaron::PlayerSpacing(player.GetExactDist2d(x, y));

        float score = PitOfSaron::BarrageSpotScore(nearestOrb, spacing, true, bossDist);
        if (score > bestScore)
        {
            bestScore = score;
            bestX = x;
            bestY = y;
        }
    }

    if (bestScore < 0.0f)
        return FleePosition(closestOrb->GetPosition(), PitOfSaron::OrbDangerRange, 250U);

    botAI->Reset();
    return MoveTo(bot->GetMapId(), bestX, bestY, bot->GetPositionZ(), false, false, false, true,
                  MovementPriority::MOVEMENT_COMBAT);
}

float IckAndKrickMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ick");
    if (!boss)
        return 1.0f;

    PitOfSaron::IckAction kind = PitOfSaron::IckAction::Other;
    if (dynamic_cast<IckAndKrickAction*>(action))
        kind = PitOfSaron::IckAction::IckAndKrick;
    else if (dynamic_cast<PoSAvoidGroundHazardAction*>(action))
        kind = PitOfSaron::IckAction::AvoidHazard;

    bool close = bot->GetExactDist2d(boss) < PitOfSaron::IckCloseRange;
    PitOfSaron::IckFight fight;
    fight.novaClose = close && IsPoisonNovaCasting(boss);
    fight.pursuedClose = close && bot->HasAura(SPELL_PURSUIT);
    fight.barrage = IsBarrageCasting(boss);
    fight.healer = PlayerbotAI::IsHeal(bot);
    return PitOfSaron::IckAllows(kind, fight) ? 1.0f : 0.0f;
}
