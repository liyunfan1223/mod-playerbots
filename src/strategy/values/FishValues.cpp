/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "FishValues.h"
#include "PlayerbotAI.h"
#include "RandomPlayerbotMgr.h"
#include "Map.h"
#include "Spell.h"
#include "FishingAction.h"

bool CanFishValue::Calculate()
{
    int32 SkillFishing = bot->GetSkillValue(SKILL_FISHING);

    if (SkillFishing == 0)
        return false;

    if (bot->isSwimming())
        return false;
    if (bot->IsInCombat())
        return false;
        
    return true;
}

bool IsNearWaterValue::Calculate()
{
    WorldPosition nearwater = findWaterRadial(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetMap(), bot->GetPhaseMask(), 10.0f, 20.0f, 2.5f, false);
    if (nearwater.GetPositionX() != 0.0f && nearwater.GetPositionY() != 0.0f)
        {
            return true;
        }
    return false;
}

bool CanUseFishingBobberValue::Calculate()
{
    GuidVector gos = AI_VALUE(GuidVector, "nearest game objects no los");
    for (const auto& guid : gos)
    {
        if (GameObject* go = botAI->GetGameObject(guid))
        {
            if (go->GetEntry() != FISHING_BOBBER)
                continue;
            if (go->GetOwnerGUID() != bot->GetGUID())
                continue;

            if (go->getLootState() == GO_READY)
                return true;

            // Not ready yet → delay next check
            time_t bobberActiveTime = go->GetRespawnTime() - FISHING_BOBBER_READY_TIME;
            if (bobberActiveTime > time(0))
                botAI->SetNextCheckDelay((bobberActiveTime - time(0)) * IN_MILLISECONDS + 500);
            else
                botAI->SetNextCheckDelay(1000);

            return false;
        }
    }
    return false;
}

bool DoneFishingValue::Calculate()
{
    WorldPosition nearwater = findWaterRadial(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetMap(), bot->GetPhaseMask(), 10.0f, 40.0f, 2.5f, false);
    return (nearwater.GetPositionX() == 0.0f && nearwater.GetPositionY() == 0.0f);
}