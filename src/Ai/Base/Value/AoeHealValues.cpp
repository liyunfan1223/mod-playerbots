/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AoeHealValues.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

uint8 AoeHealValue::Calculate()
{
    Group* group = bot->GetGroup();
    if (!group)
        return 0;

    float range = 0;
    if (qualifier == "low")
        range = sPlayerbotAIConfig.LowHealth;
    else if (qualifier == "medium")
        range = sPlayerbotAIConfig.MediumHealth;
    else if (qualifier == "critical")
        range = sPlayerbotAIConfig.CriticalHealth;
    else if (qualifier == "almost full")
        range = sPlayerbotAIConfig.AlmostFullHealth;

    uint8 count = 0;
    Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
    for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
    {
        Player* player = ObjectAccessor::FindPlayer(itr->guid);
        if (!player || !player->IsAlive())
            continue;

        if (player->GetDistance(bot) >= sPlayerbotAIConfig.SightDistance)
            continue;

        float percent = (static_cast<float>(player->GetHealth()) / player->GetMaxHealth()) * 100;
        if (percent <= range)
            ++count;
    }

    return count;
}
