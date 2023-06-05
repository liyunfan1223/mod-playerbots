/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_MOVEMENTACTIONS_H
#define _PLAYERBOT_MOVEMENTACTIONS_H

#include "Action.h"
#include "PlayerbotAIConfig.h"

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
        bool MoveTo(uint32 mapId, float x, float y, float z, bool idle = false, bool react = false);
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
};

class FleeAction : public MovementAction
{
    public:
        FleeAction(PlayerbotAI* botAI, float distance = sPlayerbotAIConfig->spellDistance) : MovementAction(botAI, "flee"), distance(distance) { }

        bool Execute(Event event) override;

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

#endif
