/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "NearestCorpsesValue.h"

#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Playerbots.h"

class AnyDeadUnitInObjectRangeCheck
{
public:
    AnyDeadUnitInObjectRangeCheck(WorldObject const* obj, float range) : i_obj(obj), i_range(range) {}
    WorldObject const& GetFocusObject() const { return *i_obj; }
    bool operator()(Unit* u) { return !u->IsAlive(); }

private:
    WorldObject const* i_obj;
    float i_range;
};

void NearestCorpsesValue::FindUnits(std::list<Unit*>& targets)
{
    AnyDeadUnitInObjectRangeCheck u_check(bot, range);
    Acore::UnitListSearcher<AnyDeadUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, range);
}

bool NearestCorpsesValue::AcceptUnit(Unit* unit) { return true; }
