/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "InviteToGroupAction.h"
#include "Event.h"
#include "GuildMgr.h"
#include "Playerbots.h"
#include "ServerFacade.h"

bool InviteToGroupAction::Execute(Event event)
{
    Player* master = event.getOwner();
    if (!master)
        return false;

    return Invite(master);
}

bool InviteToGroupAction::Invite(Player* player)
{
    if (!player || !player->IsInWorld())
        return false;

    if (!GET_PLAYERBOT_AI(player) && !botAI->GetSecurity()->CheckLevelFor(PLAYERBOT_SECURITY_INVITE, true, player))
        return false;

    WorldPacket p;
    uint32 roles_mask = 0;
    p << player->GetName();
    p << roles_mask;
    bot->GetSession()->HandleGroupInviteOpcode(p);

    return true;
}

bool InviteNearbyToGroupAction::Execute(Event event)
{
    GuidVector nearGuids = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest friendly players")->Get();
    for (auto& i : nearGuids)
    {
        Player* player = ObjectAccessor::FindPlayer(i);
        if (!player)
            continue;

        if (player->GetGroup())
            continue;

        if (botAI)
        {
            if (botAI->GetGrouperType() == GrouperType::SOLO && !botAI->HasRealPlayerMaster()) // Do not invite solo players.
                continue;

            if (botAI->HasActivePlayerMaster()) // Do not invite alts of active players.
                continue;
        }
        else
        {
            if (!sPlayerbotAIConfig->randomBotGroupNearby)
                return false;
        }

        if (abs(int32(player->GetLevel() - bot->GetLevel())) > 2)
            continue;

        if (sServerFacade->GetDistance2d(bot, player) > sPlayerbotAIConfig->sightDistance)
            continue;

        return Invite(player);
    }

    return false;
}

bool InviteNearbyToGroupAction::isUseful()
{
    if (bot->InBattleground())
        return false;

    if (bot->InBattlegroundQueue())
        return false;

    GrouperType grouperType = botAI->GetGrouperType();
    if (grouperType == GrouperType::SOLO || grouperType == GrouperType::MEMBER)
        return false;

    if (Group* group = bot->GetGroup())
    {
        if (group->IsFull())
            return false;

        if (botAI->GetGroupMaster() != bot)
            return false;

        uint32 memberCount = group->GetMembersCount();
        if (memberCount >= uint8(grouperType))
            return false;
    }

    if (botAI->HasActivePlayerMaster()) // Alts do not invite randomly
        return false;

    return true;
}

std::vector<Player*> InviteGuildToGroupAction::getGuildMembers()
{
    Guild* guild = sGuildMgr->GetGuildById(bot->GetGuildId());

    FindGuildMembers worker;
    guild->BroadcastWorker(worker);

    return worker.GetResult();
}

bool InviteGuildToGroupAction::Execute(Event event)
{
    for (auto& member : getGuildMembers())
    {
        Player* player = member;

        if (!player)
            continue;

        if (player->GetGroup())
            continue;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);

        if (botAI)
        {
            if (botAI->GetGrouperType() == GrouperType::SOLO && !botAI->HasRealPlayerMaster()) //Do not invite solo players.
                continue;

            if (botAI->HasActivePlayerMaster()) //Do not invite alts of active players.
                continue;

            if (player->GetLevel() > bot->GetLevel() + 5) // Only invite higher levels that need money so they can grind money and help out.
            {
                AiObjectContext* botContext = botAI->GetAiObjectContext();

                if (!botContext->GetValue<bool>("should get money")->Get())
                    continue;
            }
        }
        else
        {
            if (!sPlayerbotAIConfig->randomBotGroupNearby)
                return false;
        }

        if (abs(int32(player->GetLevel() - bot->GetLevel())) > 4)
            continue;

        if (!botAI && sServerFacade->GetDistance2d(bot, player) > sPlayerbotAIConfig->sightDistance)
            continue;

        return Invite(player);
    }

    return false;
}

bool InviteGuildToGroupAction::isUseful()
{
    return bot->GetGuildId() && InviteNearbyToGroupAction::isUseful();
};
