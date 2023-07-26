/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "NearestGameObjects.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Playerbots.h"

class AnyGameObjectInObjectRangeCheck
{
    public:
        AnyGameObjectInObjectRangeCheck(WorldObject const* obj, float range) : i_obj(obj), i_range(range) { }
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

GuidVector NearestGameObjects::Calculate()
{
    std::list<GameObject*> targets;
    AnyGameObjectInObjectRangeCheck u_check(bot, range);
    Acore::GameObjectListSearcher<AnyGameObjectInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, range);

    GuidVector result;
    for (GameObject* go : targets)
    {
        // if (ignoreLos || bot->IsWithinLOSInMap(go))
        result.push_back(go->GetGUID());
    }

    return result;
}
