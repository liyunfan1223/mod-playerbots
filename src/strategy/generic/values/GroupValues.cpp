/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GroupValues.h"

#include "Playerbots.h"
#include "ServerFacade.h"

GuidVector GroupMembersValue::Calculate()
{
    GuidVector members;

    Group* group = bot->GetGroup();
    if (group)
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            members.push_back(ref->GetSource()->GetGUID());
        }
    }
    else
        members.push_back(bot->GetGUID());

    return members;
}

bool IsFollowingPartyValue::Calculate()
{
    if (botAI->GetGroupMaster() == bot)
        return true;

    if (botAI->HasStrategy("follow", BOT_STATE_NON_COMBAT))
        return true;

    return false;
};

bool IsNearLeaderValue::Calculate()
{
    Player* groupMaster = botAI->GetGroupMaster();

    if (!groupMaster)
        return false;

    if (groupMaster == bot)
        return true;

    return sServerFacade->GetDistance2d(bot, botAI->GetGroupMaster()) < sPlayerbotAIConfig->sightDistance;
}

bool BoolANDValue::Calculate()
{
    std::vector<std::string> values = split(getQualifier(), ',');

    for (auto value : values)
    {
        if (!AI_VALUE(bool, value))
            return false;
    }

    return true;
}

uint32 GroupBoolCountValue::Calculate()
{
    uint32 count = 0;

    for (ObjectGuid guid : AI_VALUE(GuidVector, "group members"))
    {
        Player* player = ObjectAccessor::FindPlayer(guid);

        if (!player)
            continue;

        if (player->GetMapId() != bot->GetMapId())
            continue;

        if (!GET_PLAYERBOT_AI(player))
            continue;

        if (PAI_VALUE2(bool, "and", getQualifier()))
            return count++;
    }

    return count;
};

bool GroupBoolANDValue::Calculate()
{
    for (ObjectGuid guid : AI_VALUE(GuidVector, "group members"))
    {
        Player* player = ObjectAccessor::FindPlayer(guid);

        if (!player)
            continue;

        if (player->GetMapId() != bot->GetMapId())
            continue;

        if (!GET_PLAYERBOT_AI(player))
            continue;

        if (!PAI_VALUE2(bool, "and", getQualifier()))
            return false;
    }

    return true;
};

bool GroupBoolORValue::Calculate()
{
    for (ObjectGuid guid : AI_VALUE(GuidVector, "group members"))
    {
        Player* player = ObjectAccessor::FindPlayer(guid);

        if (!player)
            continue;

        if (player->GetMapId() != bot->GetMapId())
            continue;

        if (!GET_PLAYERBOT_AI(player))
            continue;

        if (PAI_VALUE2(bool, "and", getQualifier()))
            return true;
    }

    return false;
};

bool GroupReadyValue::Calculate()
{
    bool inDungeon = !WorldPosition(bot).isOverworld();

    for (ObjectGuid guid : AI_VALUE(GuidVector, "group members"))
    {
        Player* member = ObjectAccessor::FindPlayer(guid);

        if (!member)
            continue;

        if (inDungeon)  // In dungeons all following members need to be alive before continueing.
        {
            PlayerbotAI* memberAi = GET_PLAYERBOT_AI(member);

            bool isFollowing = memberAi ? memberAi->HasStrategy("follow", BOT_STATE_NON_COMBAT) : true;

            if (!member->IsAlive() && isFollowing)
                return false;
        }

        // We only wait for members that are in range otherwise we might be waiting for bots stuck in dead loops
        // forever.
        if (botAI->GetGroupMaster() &&
            sServerFacade->GetDistance2d(member, botAI->GetGroupMaster()) > sPlayerbotAIConfig->sightDistance)
            continue;

        if (member->GetHealthPct() < sPlayerbotAIConfig->almostFullHealth)
            return false;

        if (!member->GetPower(POWER_MANA))
            continue;

        float mana = (static_cast<float>(member->GetPower(POWER_MANA)) / member->GetMaxPower(POWER_MANA)) * 100;

        if (mana < sPlayerbotAIConfig->mediumMana)
            return false;
    }

    return true;
};
