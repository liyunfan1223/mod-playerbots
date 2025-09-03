/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ThreatValues.h"

#include "Playerbots.h"
#include "ThreatMgr.h"

uint8 ThreatValue::Calculate()
{
    if (qualifier == "aoe")
    {
        uint8 maxThreat = 0;
        GuidVector attackers = context->GetValue<GuidVector>("attackers")->Get();
        for (ObjectGuid const guid : attackers)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (!unit || !unit->IsAlive())
                continue;

            uint8 threat = Calculate(unit);
            if (!maxThreat || threat > maxThreat)
                maxThreat = threat;
        }

        return maxThreat;
    }

    Unit* target = AI_VALUE(Unit*, qualifier);
    return Calculate(target);
}

uint8 ThreatValue::Calculate(Unit* target)
{
    if (!target)
        return 0;

    if (target->GetGUID().IsPlayer())
        return 0;

    Group* group = bot->GetGroup();
    if (!group)
        return 0;

    float botThreat = target->GetThreatMgr().GetThreat(bot);
    float maxThreat = -1.0f;
    bool hasTank = false;

    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* player = gref->GetSource();
        if (!player || !player->IsAlive() || player == bot)
            continue;

        if (botAI->IsTank(player))
        {
            hasTank = true;
            float threat = target->GetThreatMgr().GetThreat(player);
            if (maxThreat < threat)
                maxThreat = threat;
        }
    }

    if (maxThreat <= 0 && !hasTank)
        return 0;

    // calculate normal threat for fleeing targets
    bool fleeing = target->GetMotionMaster()->GetCurrentMovementGeneratorType() == FLEEING_MOTION_TYPE ||
                   target->GetMotionMaster()->GetCurrentMovementGeneratorType() == TIMED_FLEEING_MOTION_TYPE;

    // return high threat if tank has no threat
    if (target->IsInCombat() && maxThreat <= 0 && botThreat <= 0 && hasTank && !fleeing)
        return 100;

    // return low threat if mob if fleeing
    if (hasTank && fleeing)
        return 0;

    return botThreat * 100 / maxThreat;
}
