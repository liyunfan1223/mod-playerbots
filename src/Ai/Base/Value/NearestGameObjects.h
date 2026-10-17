/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_NEARESTGAMEOBJECTS_H
#define PLAYERBOTS_NEARESTGAMEOBJECTS_H

#include "GameObject.h"
#include "PlayerbotAIConfig.h"
#include "Value.h"

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
    NearestGameObjects(PlayerbotAI* botAI, float range = sPlayerbotAIConfig.SightDistance, bool /*ignoreLos*/ = false,
                       std::string const name = "nearest game objects")
        : ObjectGuidListCalculatedValue(botAI, name, 1 * IN_MILLISECONDS), range(range)
    {
    }

protected:
    GuidVector Calculate() override;

private:
    float range;
};

class NearestTrapWithDamageValue : public ObjectGuidListCalculatedValue
{
public:
    NearestTrapWithDamageValue(PlayerbotAI* botAI, float range = 15.0f)
        : ObjectGuidListCalculatedValue(botAI, "nearest trap with damage", 1 * IN_MILLISECONDS), range(range)
    {
    }

protected:
    GuidVector Calculate() override;

private:
    float range;
};

#endif
