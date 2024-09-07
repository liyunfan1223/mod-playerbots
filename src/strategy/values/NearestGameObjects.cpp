/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "NearestGameObjects.h"

#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Playerbots.h"
#include "SharedDefines.h"
#include "SpellMgr.h"

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

GuidVector NearestTrapWithDamageValue::Calculate()
{
    std::list<GameObject*> targets;
    AnyGameObjectInObjectRangeCheck u_check(bot, range);
    Acore::GameObjectListSearcher<AnyGameObjectInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, range);

    GuidVector result;
    for (GameObject* go : targets)
    {
        if (go->GetGoType() != GAMEOBJECT_TYPE_TRAP)
        {
            continue;
        }
        Unit* owner = go->GetOwner();
        if (owner && owner->IsFriendlyTo(bot))
        {
            continue;
        }
        const GameObjectTemplate* goInfo = go->GetGOInfo();
        if (!goInfo)
        {
            continue;
        }
        uint32 spellId = goInfo->trap.spellId;
        if (!spellId)
        {
            continue;
        }
        const SpellInfo* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo || spellInfo->IsPositive())
        {
            continue;
        }
        for (int i = 0; i < MAX_SPELL_EFFECTS; i++)
        {
            if (spellInfo->Effects[i].Effect == SPELL_EFFECT_APPLY_AURA)
            {
                if (spellInfo->Effects[i].ApplyAuraName == SPELL_AURA_PERIODIC_DAMAGE)
                {
                    result.push_back(go->GetGUID());
                    break;
                }
            }
            else if (spellInfo->Effects[i].Effect == SPELL_EFFECT_SCHOOL_DAMAGE)
            {
                result.push_back(go->GetGUID());
                break;
            }
        }
    }
    return result;
}
