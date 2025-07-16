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
    LOG_ERROR("playerbots","Entering CanFishValue::Calculate");

  if (!bot)
  {
    return false;
  }

  if (!botAI)
        {
        return false;
    }
  uint32 SkillFishing = bot->GetSkillValue(SKILL_FISHING);

  if (SkillFishing == 0)
    {
        botAI->TellError("I don't know how to fish");
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
    if (!bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
        return false;
    Spell* spell = bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
    if (spell && spell->m_spellInfo && spell->m_spellInfo->Id != FISHING_SPELL)
        return false;

    return true;
}
