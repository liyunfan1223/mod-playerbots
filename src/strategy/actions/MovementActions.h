/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_MOVEMENTACTIONS_H
#define _PLAYERBOT_MOVEMENTACTIONS_H

#include "Action.h"
#include "PlayerbotAIConfig.h"
#include <cmath>

class Player;
class PlayerbotAI;
class Unit;
class WorldObject;

class MovementAction : public Action
{
    public:
        MovementAction(PlayerbotAI* botAI, std::string const name);

    protected:
        bool MoveNear(uint32 mapId, float x, float y, float z, float distance = sPlayerbotAIConfig->contactDistance);
        bool MoveToLOS(WorldObject* target, bool ranged = false);
        bool MoveTo(uint32 mapId, float x, float y, float z, bool idle = false, bool react = false, bool normal_only = false);
        bool MoveTo(Unit* target, float distance = 0.0f);
        bool MoveNear(WorldObject* target, float distance = sPlayerbotAIConfig->contactDistance);
        float GetFollowAngle();
        bool Follow(Unit* target, float distance = sPlayerbotAIConfig->followDistance);
        bool Follow(Unit* target, float distance, float angle);
        bool ChaseTo(WorldObject* obj, float distance = 0.0f, float angle = 0.0f);
        float MoveDelay(float distance);
        void WaitForReach(float distance);
        bool IsMovingAllowed(Unit* target);
        bool IsMovingAllowed(uint32 mapId, float x, float y, float z);
        bool IsMovingAllowed();
        bool Flee(Unit *target);
        void ClearIdleState();
        void UpdateMovementState();
        bool MoveAway(Unit* target);
        bool MoveInside(uint32 mapId, float x, float y, float z, float distance = sPlayerbotAIConfig->followDistance);
        void CreateWp(Player* wpOwner, float x, float y, float z, float o, uint32 entry, bool important = false);
    private:
        float SearchBestGroundZForPath(float x, float y, float z, bool generatePath, float range = 15.0f, bool normal_only = false, float step = 3.0f);
};

class FleeAction : public MovementAction
{
    public:
        FleeAction(PlayerbotAI* botAI, float distance = sPlayerbotAIConfig->spellDistance) : MovementAction(botAI, "flee"), distance(distance) { }

        bool Execute(Event event) override;
        bool isUseful() override;

	private:
		float distance;
};

class FleeWithPetAction : public MovementAction
{
    public:
        FleeWithPetAction(PlayerbotAI* botAI) : MovementAction(botAI, "flee with pet") { }

        bool Execute(Event event) override;
};

class RunAwayAction : public MovementAction
{
    public:
        RunAwayAction(PlayerbotAI* botAI) : MovementAction(botAI, "runaway") { }

        bool Execute(Event event) override;
};

class MoveToLootAction : public MovementAction
{
    public:
        MoveToLootAction(PlayerbotAI* botAI) : MovementAction(botAI, "move to loot") { }

        bool Execute(Event event) override;
};

class MoveOutOfEnemyContactAction : public MovementAction
{
    public:
        MoveOutOfEnemyContactAction(PlayerbotAI* botAI) : MovementAction(botAI, "move out of enemy contact") { }

        bool Execute(Event event) override;
        bool isUseful() override;
};

class SetFacingTargetAction : public Action
{
    public:
        SetFacingTargetAction(PlayerbotAI* botAI) : Action(botAI, "set facing") { }

        bool Execute(Event event) override;
        bool isUseful() override;
        bool isPossible() override;
};

class SetBehindTargetAction : public MovementAction
{
    public:
        SetBehindTargetAction(PlayerbotAI* botAI) : MovementAction(botAI, "set behind") { }

        bool Execute(Event event) override;
        bool isUseful() override;
        bool isPossible() override;
};

class MoveOutOfCollisionAction : public MovementAction
{
    public:
        MoveOutOfCollisionAction(PlayerbotAI* botAI) : MovementAction(botAI, "move out of collision") { }

        bool Execute(Event event) override;
        bool isUseful() override;
};

class MoveRandomAction : public MovementAction
{
    public:
        MoveRandomAction(PlayerbotAI* botAI) : MovementAction(botAI, "move random") { }

        bool Execute(Event event) override;
        bool isUseful() override;
};

class MoveInsideAction : public MovementAction
{
    public:
        MoveInsideAction(PlayerbotAI* ai, float x, float y, float distance = 5.0f) : MovementAction(ai, "move inside") {
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
    RotateAroundTheCenterPointAction(PlayerbotAI* ai, std::string name, 
        float center_x, float center_y, float radius = 40.0f, 
        uint32 intervals = 16, bool clockwise = true, float start_angle = 0) : MovementAction(ai, name) {
        this->center_x = center_x;
        this->center_y = center_y;
        this->radius = radius;
        this->intervals = intervals;
        this->clockwise = clockwise;
        this->call_counters = 0;
        for (int i = 0; i < intervals; i++) {
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
#endif
