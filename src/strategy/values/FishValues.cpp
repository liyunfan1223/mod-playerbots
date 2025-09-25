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
  if (!bot || !botAI)
    return false;
    
  int32 SkillFishing = bot->GetSkillValue(SKILL_FISHING);

  if (SkillFishing == 0)
    return false;

  return true;
}

bool CanOpenBobberValue::Calculate()
{
  if (!bot || !botAI)
    return false;

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
  if (!bot || !botAI)
    return false;
  
  Player* master = botAI->GetMaster();
  
  if (!master) // Check if master exists
    return false;
    
  bool distance = master->GetDistance(bot) < 10.0f;
  if (!distance)
  {
    return true;
  }
  return false;
}