/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_NEARESTGAMEOBJECTS_H
#define _PLAYERBOT_NEARESTGAMEOBJECTS_H

#include "PlayerbotAIConfig.h"
#include "Value.h"
#include "GameObject.h"

class PlayerbotAI;

class AnyGameObjectInObjectRangeCheck
{
public:
    AnyGameObjectInObjectRangeCheck(WorldObject const* obj, float range) : i_obj(obj), i_range(range) {}
    WorldObject const& GetFocusObject() const { return *i_obj; }
    bool operator()(GameObject* u)
    {
        if (u && i_obj->IsWithinDistInMap(u, i_range) && u->isSpawned() && u->GetGOInfo())
            return true;

        return false;
    }

private:
    WorldObject const* i_obj;
    float i_range;
};

class NearestGameObjects : public ObjectGuidListCalculatedValue
{
public:
    NearestGameObjects(PlayerbotAI* botAI, float range = sPlayerbotAIConfig->sightDistance, bool ignoreLos = false,
                       std::string const name = "nearest game objects")
        : ObjectGuidListCalculatedValue(botAI, name, 1 * 1000), range(range), ignoreLos(ignoreLos)
    {
    }

protected:
    GuidVector Calculate() override;

private:
    float range;
    bool ignoreLos;
};

class NearestTrapWithDamageValue : public ObjectGuidListCalculatedValue
{
public:
    NearestTrapWithDamageValue(PlayerbotAI* botAI, float range = 15.0f)
        : ObjectGuidListCalculatedValue(botAI, "nearest trap with damage", 1 * 1000), range(range)
    {
    }

protected:
    GuidVector Calculate() override;

private:
    float range;
};

#endif
