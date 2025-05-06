/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_FORMATIONS_H
#define _PLAYERBOT_FORMATIONS_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "PlayerbotAIConfig.h"
#include "TravelMgr.h"

class Player;
class PlayerbotAI;

class Formation : public AiNamedObject
{
public:
    Formation(PlayerbotAI* botAI, std::string const name) : AiNamedObject(botAI, name) {}
    virtual ~Formation() = default;
    virtual std::string const GetTargetName() { return ""; }
    virtual WorldLocation GetLocation() { return NullLocation; }
    virtual float GetMaxDistance() { return sPlayerbotAIConfig->followDistance; }
    static WorldLocation NullLocation;
    static bool IsNullLocation(WorldLocation const& loc);

protected:
    float GetFollowAngle();
};

class FollowFormation : public Formation
{
public:
    FollowFormation(PlayerbotAI* botAI, std::string const name) : Formation(botAI, name) {}
};

class MoveFormation : public Formation
{
public:
    MoveFormation(PlayerbotAI* botAI, std::string const name) : Formation(botAI, name) {}

protected:
    WorldLocation MoveLine(std::vector<Player*> line, float diff, float cx, float cy, float cz, float orientation,
                           float range);
    WorldLocation MoveSingleLine(std::vector<Player*> line, float diff, float cx, float cy, float cz, float orientation,
                                 float range);
};

class MoveAheadFormation : public MoveFormation
{
public:
    MoveAheadFormation(PlayerbotAI* botAI, std::string const name) : MoveFormation(botAI, name) {}

    WorldLocation GetLocation() override;
    virtual WorldLocation GetLocationInternal() { return NullLocation; }
};

class FormationValue : public ManualSetValue<Formation*>
{
public:
    FormationValue(PlayerbotAI* botAI);
    ~FormationValue();

    std::string const Save() override;
    bool Load(std::string const value) override;
};

class SetFormationAction : public Action
{
public:
    SetFormationAction(PlayerbotAI* botAI) : Action(botAI, "set formation") {}

    bool Execute(Event event) override;
};

#endif
