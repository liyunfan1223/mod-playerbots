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
  if (!bot ||!botAI)
    return false;
    
  int32 SkillFishing = bot->GetSkillValue(SKILL_FISHING);

  if (SkillFishing == 0)
  {
    return false;
  }

  int32 zone_skill = sObjectMgr->GetFishingBaseSkillLevel(bot->GetAreaId());
  if (!zone_skill)
    zone_skill = sObjectMgr->GetFishingBaseSkillLevel(bot->GetZoneId());

  if (SkillFishing < zone_skill)
  {
    botAI->TellError("I don't have enough skill to fish here");
    return false;
  }
  return true;
}

bool CanOpenBobberValue::Calculate()
{
  if (!bot || !botAI)
  {
    return false;
  }
  return true;
}

bool DoneFishingValue::Calculate()
{
  if (!bot ||!botAI)
    return false;
  
  Player* master = botAI->GetMaster();
  
  if (!master) // Check if master exists
    return false;
  ObjectGuid masterGUID;
  masterGUID = master->GetGUID();
  
  if (!masterGUID)
  {
    return false;// Need to add triggers for free fishing. 
  }
  bool distance = master->GetDistance(bot) < 10.0f;
  if (!distance)
  {
    return true;
  }
  return false;
}

