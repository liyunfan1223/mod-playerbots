/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_MOVEMENTACTIONS_H
#define _PLAYERBOT_MOVEMENTACTIONS_H

#include <cmath>

#include "Action.h"
#include "LastMovementValue.h"
#include "PlayerbotAIConfig.h"

class Player;
class PlayerbotAI;
class Unit;
class WorldObject;
class Position;

#define ANGLE_45_DEG (static_cast<float>(M_PI) / 4.f)
#define ANGLE_90_DEG M_PI_2
#define ANGLE_120_DEG (2.f * static_cast<float>(M_PI) / 3.f)

class MovementAction : public Action
{
public:
    MovementAction(PlayerbotAI* botAI, std::string const name);

protected:
    bool JumpTo(uint32 mapId, float x, float y, float z, MovementPriority priority = MovementPriority::MOVEMENT_NORMAL);
    bool MoveNear(uint32 mapId, float x, float y, float z, float distance = sPlayerbotAIConfig->contactDistance, MovementPriority priority = MovementPriority::MOVEMENT_NORMAL);
    bool MoveToLOS(WorldObject* target, bool ranged = false);
    bool MoveTo(uint32 mapId, float x, float y, float z, bool idle = false, bool react = false,
                bool normal_only = false, bool exact_waypoint = false, MovementPriority priority = MovementPriority::MOVEMENT_NORMAL);
    bool MoveTo(WorldObject* target, float distance = 0.0f, MovementPriority priority = MovementPriority::MOVEMENT_NORMAL);
    bool MoveNear(WorldObject* target, float distance = sPlayerbotAIConfig->contactDistance, MovementPriority priority = MovementPriority::MOVEMENT_NORMAL);
    float GetFollowAngle();
    bool Follow(Unit* target, float distance = sPlayerbotAIConfig->followDistance);
    bool Follow(Unit* target, float distance, float angle);
    bool ChaseTo(WorldObject* obj, float distance = 0.0f, float angle = 0.0f);
    bool ReachCombatTo(Unit* target, float distance = 0.0f);
    float MoveDelay(float distance);
    void WaitForReach(float distance);
    void SetNextMovementDelay(float delayMillis);
    bool IsMovingAllowed(WorldObject* target);
    bool IsMovingAllowed(uint32 mapId, float x, float y, float z);
    bool IsDuplicateMove(uint32 mapId, float x, float y, float z);
    bool IsWaitingForLastMove(MovementPriority priority);
    bool IsMovingAllowed();
    bool Flee(Unit* target);
    void ClearIdleState();
    void UpdateMovementState();
    bool MoveAway(Unit* target, float distance = sPlayerbotAIConfig -> fleeDistance);
    bool MoveFromGroup(float distance);
    bool Move(float angle, float distance);
    bool MoveInside(uint32 mapId, float x, float y, float z, float distance = sPlayerbotAIConfig->followDistance, MovementPriority priority = MovementPriority::MOVEMENT_NORMAL);
    void CreateWp(Player* wpOwner, float x, float y, float z, float o, uint32 entry, bool important = false);
    Position BestPositionForMeleeToFlee(Position pos, float radius);
    Position BestPositionForRangedToFlee(Position pos, float radius);
    bool FleePosition(Position pos, float radius);
    bool CheckLastFlee(float curAngle, std::list<FleeInfo>& infoList);

protected:
    struct CheckAngle
    {
        float angle;
        bool strict;
    };

private:
    // float SearchBestGroundZForPath(float x, float y, float z, bool generatePath, float range = 20.0f, bool
    // normal_only = false, float step = 8.0f);
    const Movement::PointsArray SearchForBestPath(float x, float y, float z, float& modified_z, int maxSearchCount = 5,
                                                  bool normal_only = false, float step = 8.0f);
};

class FleeAction : public MovementAction
{
public:
    FleeAction(PlayerbotAI* botAI, float distance = sPlayerbotAIConfig->spellDistance)
        : MovementAction(botAI, "flee"), distance(distance)
    {
    }

    bool Execute(Event event) override;
    bool isUseful() override;

private:
    float distance;
};

class FleeWithPetAction : public MovementAction
{
public:
    FleeWithPetAction(PlayerbotAI* botAI) : MovementAction(botAI, "flee with pet") {}

    bool Execute(Event event) override;
};

class AvoidAoeAction : public MovementAction
{
public:
    AvoidAoeAction(PlayerbotAI* botAI, int moveInterval = 1000)
        : MovementAction(botAI, "avoid aoe"), moveInterval(moveInterval)
    {
    }

    bool isUseful() override;
    bool Execute(Event event) override;

protected:
    bool AvoidAuraWithDynamicObj();
    bool AvoidGameObjectWithDamage();
    bool AvoidUnitWithDamageAura();
    time_t lastTellTimer = 0;
    int lastMoveTimer = 0;
    int moveInterval;
};


class CombatFormationMoveAction : public MovementAction
{
public:
    CombatFormationMoveAction(PlayerbotAI* botAI, std::string name = "combat formation move", int moveInterval = 1000)
        : MovementAction(botAI, name), moveInterval(moveInterval)
    {
    }

    bool isUseful() override;
    bool Execute(Event event) override;

protected:
    Position AverageGroupPos(float dis = sPlayerbotAIConfig->sightDistance, bool ranged = false, bool self = false);
    Player* NearestGroupMember(float dis = sPlayerbotAIConfig->sightDistance);
    float AverageGroupAngle(Unit* from, bool ranged = false, bool self = false);
    Position GetNearestPosition(const std::vector<Position>& positions);
    int lastMoveTimer = 0;
    int moveInterval;
};

class TankFaceAction : public CombatFormationMoveAction
{
public:
    TankFaceAction(PlayerbotAI* botAI) : CombatFormationMoveAction(botAI, "tank face") {}

    bool Execute(Event event) override;
};

class RearFlankAction : public MovementAction
{
// 90 degree minimum angle prevents any frontal cleaves/breaths and avoids parry-hasting the boss.
// 120 degree maximum angle leaves a 120 degree symmetrical cone at the tail end which is usually enough to avoid tail swipes.
// Some dragons or mobs may have different danger zone angles, override if needed.
public:
    RearFlankAction(PlayerbotAI* botAI, float distance = 0.0f, float minAngle = ANGLE_90_DEG, float maxAngle = ANGLE_120_DEG)
        : MovementAction(botAI, "rear flank")
        {
            this->distance = distance;
            this->minAngle = minAngle;
            this->maxAngle = maxAngle;
        }

    bool Execute(Event event) override;
    bool isUseful() override;

protected:
    float distance, minAngle, maxAngle;
};

class DisperseSetAction : public Action
{
public:
    DisperseSetAction(PlayerbotAI* botAI, std::string const name = "disperse set") : Action(botAI, name) {}

    bool Execute(Event event) override;
    float DEFAULT_DISPERSE_DISTANCE_RANGED = 5.0f;
    float DEFAULT_DISPERSE_DISTANCE_MELEE = 2.0f;
};

class RunAwayAction : public MovementAction
{
public:
    RunAwayAction(PlayerbotAI* botAI) : MovementAction(botAI, "runaway") {}

    bool Execute(Event event) override;
};

class MoveToLootAction : public MovementAction
{
public:
    MoveToLootAction(PlayerbotAI* botAI) : MovementAction(botAI, "move to loot") {}

    bool Execute(Event event) override;
};

class MoveOutOfEnemyContactAction : public MovementAction
{
public:
    MoveOutOfEnemyContactAction(PlayerbotAI* botAI) : MovementAction(botAI, "move out of enemy contact") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class SetFacingTargetAction : public Action
{
public:
    SetFacingTargetAction(PlayerbotAI* botAI) : Action(botAI, "set facing") {}

    bool Execute(Event event) override;
    bool isUseful() override;
    bool isPossible() override;
};

class SetBehindTargetAction : public CombatFormationMoveAction
{
public:
    SetBehindTargetAction(PlayerbotAI* botAI) : CombatFormationMoveAction(botAI, "set behind") {}

    bool Execute(Event event) override;
};

class MoveOutOfCollisionAction : public MovementAction
{
public:
    MoveOutOfCollisionAction(PlayerbotAI* botAI) : MovementAction(botAI, "move out of collision") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MoveRandomAction : public MovementAction
{
public:
    MoveRandomAction(PlayerbotAI* botAI) : MovementAction(botAI, "move random") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MoveInsideAction : public MovementAction
{
public:
    MoveInsideAction(PlayerbotAI* ai, float x, float y, float distance = 5.0f) : MovementAction(ai, "move inside")
    {
        this->x = x;
        this->y = y;
        this->distance = distance;
    }
    virtual bool Execute(Event event);

protected:
    float x, y, distance;
};

class RotateAroundTheCenterPointAction : public MovementAction
{
public:
    RotateAroundTheCenterPointAction(PlayerbotAI* ai, std::string name, float center_x, float center_y,
                                     float radius = 40.0f, uint32 intervals = 16, bool clockwise = true,
                                     float start_angle = 0)
        : MovementAction(ai, name)
    {
        this->center_x = center_x;
        this->center_y = center_y;
        this->radius = radius;
        this->intervals = intervals;
        this->clockwise = clockwise;
        this->call_counters = 0;
        for (int i = 0; i < intervals; i++)
        {
            float angle = start_angle + 2 * M_PI * i / intervals;
            waypoints.push_back(std::make_pair(center_x + cos(angle) * radius, center_y + sin(angle) * radius));
        }
    }
    virtual bool Execute(Event event);

protected:
    virtual uint32 GetCurrWaypoint() { return 0; }
    uint32 FindNearestWaypoint();
    float center_x, center_y, radius;
    uint32 intervals, call_counters;
    bool clockwise;
    std::vector<std::pair<float, float>> waypoints;
};

class MoveFromGroupAction : public MovementAction
{
public:
    MoveFromGroupAction(PlayerbotAI* botAI, std::string const name = "move from group") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

#endif
