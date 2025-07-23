/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */
#include "FishWithMasterAction.h"
#include "FishingAction.h"
#include "Event.h"
#include "Playerbots.h"
#include "Common.h"

bool FishWithMasterAction::Execute(Event event)
{
    if (!botAI)
        return false;

    WorldPacket p(event.getPacket());
    p.rpos(0);
    uint8 castCount = 0, castFlags = 0;
    uint32 spellId = 0;
    p >> castCount >> spellId >> castFlags;

    if (spellId == FISHING_SPELL)
    {
        uint32 SkillFishing = bot->GetSkillValue(SKILL_FISHING);

        if (SkillFishing == 0)
            return false;

        int32 zone_skill = sObjectMgr->GetFishingBaseSkillLevel(bot->GetAreaId());
        
        if (!zone_skill)
            zone_skill = sObjectMgr->GetFishingBaseSkillLevel(bot->GetZoneId());
        if (SkillFishing < zone_skill)
            return false;

        return FishingAction(botAI).Execute(event);
    }
    return false;
}
