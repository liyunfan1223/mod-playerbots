#include "Playerbots.h"
#include "HallsOfLightningActions.h"
#include "HallsOfLightningStrategy.h"

bool BjarngrimTargetAction::Execute(Event event)
{
    Unit* target = nullptr;

    // Target is not findable from threat table using AI_VALUE2(),
    // therefore need to search manually for the unit name
    GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");

    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && unit->GetEntry() == NPC_STORMFORGED_LIEUTENANT)
        {
            target = unit;
            break;
        }
    }

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    // There are two, we don't want to ping-pong between them if we're attacking one already
    if (target && currentTarget && currentTarget->GetEntry() == NPC_STORMFORGED_LIEUTENANT)
    {
        return false;
    }

    if (AI_VALUE(Unit*, "current target") == target)
    {
        return false;
    }

    return Attack(target);
}

bool AvoidWhirlwindAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "general bjarngrim");
    if (!boss) { return false; }

    float distance = bot->GetExactDist2d(boss->GetPosition());
    float radius = 8.0f;
    float distanceExtra = 2.0f;

    if (distance < radius + distanceExtra)
    {
        return MoveAway(boss, radius + distanceExtra - distance);
    }

    return false;
}

bool VolkhanTargetAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "volkhan");
    if (!boss || AI_VALUE(Unit*, "current target") == boss)
    {
        return false;
    }
    
    return Attack(boss);
}

bool StaticOverloadSpreadAction::Execute(Event event)
{
    float radius = 8.0f;
    float distanceExtra = 2.0f;

    GuidVector members = AI_VALUE(GuidVector, "group members");
    for (auto& member : members)
    {
        if (bot->GetGUID() == member)
        {
            continue;
        }

        Unit* unit = botAI->GetUnit(member);
        if (unit && unit->HasAura(SPELL_STATIC_OVERLOAD)
            && bot->GetExactDist2d(unit) < radius)
        {
            return MoveAway(unit, radius + distanceExtra - bot->GetExactDist2d(unit));
        }
    }
    return false;
}

bool BallLightningSpreadAction::Execute(Event event)
{
    float radius = 6.0f;
    float distanceExtra = 1.0f;

    GuidVector members = AI_VALUE(GuidVector, "group members");
    for (auto& member : members)
    {
        if (bot->GetGUID() == member)
        {
            continue;
        }
        Unit* unit = botAI->GetUnit(member);
        if (unit && bot->GetExactDist2d(unit) < radius + distanceExtra)
        {
            return MoveAway(unit, radius + distanceExtra - bot->GetExactDist2d(unit));
        }
    }
    return false;
}

bool IonarTankPositionAction::isUseful() { return bot->GetExactDist2d(IONAR_TANK_POSITION) > 10.0f; }
bool IonarTankPositionAction::Execute(Event event)
{
    return MoveTo(bot->GetMapId(), IONAR_TANK_POSITION.GetPositionX(), IONAR_TANK_POSITION.GetPositionY(), IONAR_TANK_POSITION.GetPositionZ(),
                  false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
}

bool DispersePositionAction::isUseful() { return bot->GetExactDist2d(DISPERSE_POSITION) > 8.0f; }
bool DispersePositionAction::Execute(Event event)
{
    return MoveTo(bot->GetMapId(), DISPERSE_POSITION.GetPositionX(), DISPERSE_POSITION.GetPositionY(), DISPERSE_POSITION.GetPositionZ(),
                  false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
}

bool LokenStackAction::isUseful()
{
    // Minimum hunter range is 5, but values too close to this seem to cause issues..
    // Hunter bots will try and melee in between ranged attacks, or just melee entirely at 5 as they are in range.
    // 6.5 or 7.0 solves this for this boss.
    if(bot->getClass() == CLASS_HUNTER)
    {
        return AI_VALUE2(float, "distance", "current target") > 6.5f;
    }
    // else
    return AI_VALUE2(float, "distance", "current target") > 2.0f;
}
bool LokenStackAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "loken");
    if (!boss) { return false; }

    float maxMovement = 10.0f;
    if (!boss->HasUnitState(UNIT_STATE_CASTING))
    {
        if(bot->getClass() == CLASS_HUNTER)
        {
            return Move(bot->GetAngle(boss), fmin(bot->GetExactDist2d(boss) - 6.5f, maxMovement));
        }
        // else
        return Move(bot->GetAngle(boss), fmin(bot->GetExactDist2d(boss), maxMovement));
    }
    
    return false;
}


bool AvoidLightningNovaAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "loken");
    if (!boss) { return false; }

    float distance = bot->GetExactDist2d(boss->GetPosition());
    float radius = 20.0f;
    float distanceExtra = 2.0f;

    if (distance < radius + distanceExtra)
    {
        return MoveAway(boss, radius + distanceExtra - distance);
    }

    return false;
}
