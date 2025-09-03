/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RtiTargetValue.h"

#include "AttackersValue.h"
#include "Playerbots.h"
#include "ServerFacade.h"

int32 RtiTargetValue::GetRtiIndex(std::string const rti)
{
    int32 index = -1;
    if (rti == "star")
        index = 0;
    else if (rti == "circle")
        index = 1;
    else if (rti == "diamond")
        index = 2;
    else if (rti == "triangle")
        index = 3;
    else if (rti == "moon")
        index = 4;
    else if (rti == "square")
        index = 5;
    else if (rti == "cross")
        index = 6;
    else if (rti == "skull")
        index = 7;

    return index;
}

Unit* RtiTargetValue::Calculate()
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    std::string const rti = AI_VALUE(std::string, type);
    int32 index = GetRtiIndex(rti);

    if (index == -1)
        return nullptr;

    ObjectGuid guid = group->GetTargetIcon(index);
    if (!guid)
        return nullptr;

    //////////////////////////////////////////////////////begin: delete below check
    // Some units that need to be killed in battle are not on the list of attackers,
    // such as the Kor'kron Battle-Mage in Icecrown Citadel.

    // GuidVector attackers = context->GetValue<GuidVector >("attackers")->Get();
    // if (find(attackers.begin(), attackers.end(), guid) == attackers.end())
    //     return nullptr;
    //
    //////////////////////////////////////////////////////end: delete below check

    Unit* unit = botAI->GetUnit(guid);
    if (!unit || unit->isDead() || !bot->IsWithinLOSInMap(unit) || !AttackersValue::IsValidTarget(unit, bot) ||
        sServerFacade->IsDistanceGreaterThan(sServerFacade->GetDistance2d(bot, unit),
                                             sPlayerbotAIConfig->sightDistance))
        return nullptr;

    return unit;
}
