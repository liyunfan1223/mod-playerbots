/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "NearestGameObjects.h"

#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Playerbots.h"
#include "SharedDefines.h"
#include "SpellMgr.h"

GuidVector NearestGameObjects::Calculate()
{
    std::list<GameObject*> targets;
    AnyGameObjectInObjectRangeCheck u_check(bot, range);
    Acore::GameObjectListSearcher<AnyGameObjectInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, range);

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
    Cell::VisitObjects(bot, searcher, range);

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
