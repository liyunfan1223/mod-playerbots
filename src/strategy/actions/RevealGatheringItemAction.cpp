/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RevealGatheringItemAction.h"

#include "CellImpl.h"
#include "ChatHelper.h"
#include "Event.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Playerbots.h"
#include "ServerFacade.h"

class AnyGameObjectInObjectRangeCheck
{
public:
    AnyGameObjectInObjectRangeCheck(WorldObject const* obj, float range) : i_obj(obj), i_range(range) {}
    WorldObject const& GetFocusObject() const { return *i_obj; }
    bool operator()(GameObject* go)
    {
        if (go && i_obj->IsWithinDistInMap(go, i_range) && go->isSpawned() && go->GetGOInfo())
            return true;

        return false;
    }

private:
    WorldObject const* i_obj;
    float i_range;
};

bool RevealGatheringItemAction::Execute(Event event)
{
    if (!bot->GetGroup())
        return false;

    std::list<GameObject*> targets;
    AnyGameObjectInObjectRangeCheck u_check(bot, sPlayerbotAIConfig->grindDistance);
    Acore::GameObjectListSearcher<AnyGameObjectInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, sPlayerbotAIConfig->reactDistance);

    std::vector<GameObject*> result;
    for (GameObject* go : targets)
    {
        if (!go || !go->isSpawned() ||
            sServerFacade->IsDistanceLessOrEqualThan(sServerFacade->GetDistance2d(bot, go),
                                                     sPlayerbotAIConfig->lootDistance))
            continue;

        if (LockEntry const* lockInfo = sLockStore.LookupEntry(go->GetGOInfo()->GetLockId()))
        {
            for (uint8 i = 0; i < 8; ++i)
            {
                if (lockInfo->Type[i] == LOCK_KEY_SKILL)
                {
                    uint32 skillId = SkillByLockType(LockType(lockInfo->Index[i]));
                    uint32 reqSkillValue = std::max(2u, lockInfo->Skill[i]);
                    if ((skillId == SKILL_MINING || skillId == SKILL_HERBALISM) &&
                        botAI->HasSkill((SkillType)skillId) && uint32(bot->GetSkillValue(skillId)) >= reqSkillValue)
                    {
                        result.push_back(go);
                        break;
                    }
                }
            }
        }

        if (go->GetGoType() == GAMEOBJECT_TYPE_FISHINGNODE && botAI->HasSkill(SKILL_FISHING))
            result.push_back(go);
    }

    if (result.empty())
        return false;

    GameObject* go = result[urand(0, result.size() - 1)];
    if (!go)
        return false;

    std::ostringstream msg;
    msg << "I see a " << ChatHelper::FormatGameobject(go) << ". ";

    switch (go->GetGoType())
    {
        case GAMEOBJECT_TYPE_CHEST:
            msg << "Let's look at it.";
            break;
        case GAMEOBJECT_TYPE_FISHINGNODE:
            msg << "Let's fish a bit.";
            break;
        default:
            msg << "Should we go nearer?";
    }

    // everything is fine, do it
    botAI->Ping(go->GetPositionX(), go->GetPositionY());
    bot->Say(msg.str(), LANG_UNIVERSAL);
    return true;
}
