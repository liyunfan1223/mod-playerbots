/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "InviteToGroupAction.h"

#include "Event.h"
#include "GuildMgr.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "BroadcastHelper.h"

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

    if (bot == player)
        return false;

    if (!GET_PLAYERBOT_AI(player) && !botAI->GetSecurity()->CheckLevelFor(PLAYERBOT_SECURITY_INVITE, true, player))
        return false;

    if (Group* group = player->GetGroup())
        if (!group->isRaidGroup() && group->GetMembersCount() > 4)
            group->ConvertToRaid();

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

        if (player == bot)
            continue;

        if (botAI)
        {
            if (botAI->GetGrouperType() == GrouperType::SOLO &&
                !botAI->HasRealPlayerMaster())  // Do not invite solo players.
                continue;

            if (botAI->HasActivePlayerMaster())  // Do not invite alts of active players.
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

        Group* group = bot->GetGroup();
        Guild* guild = sGuildMgr->GetGuildById(bot->GetGuildId());
        if (!botAI->HasActivePlayerMaster())
        {
            if (guild && bot->GetGuildId() == player->GetGuildId())
            {
                BroadcastHelper::BroadcastGuildGroupOrRaidInvite(botAI, bot, player, group);
            }
            else
            {
                std::map<std::string, std::string> placeholders;
                placeholders["%player"] = player->GetName();

                if (group && group->isRaidGroup())
                    bot->Say(BOT_TEXT2("join_raid", placeholders), (bot->GetTeamId() == TEAM_ALLIANCE ? LANG_COMMON : LANG_ORCISH));
                else
                    bot->Say(BOT_TEXT2("join_group", placeholders), (bot->GetTeamId() == TEAM_ALLIANCE ? LANG_COMMON : LANG_ORCISH));
            }
        }

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

    if (botAI->HasActivePlayerMaster())  // Alts do not invite randomly
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
    Guild* guild = sGuildMgr->GetGuildById(bot->GetGuildId());

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
            if (botAI->GetGrouperType() == GrouperType::SOLO &&
                !botAI->HasRealPlayerMaster())  // Do not invite solo players.
                continue;

            if (botAI->HasActivePlayerMaster())  // Do not invite alts of active players.
                continue;

            if (player->GetLevel() >
                bot->GetLevel() + 5)  // Only invite higher levels that need money so they can grind money and help out.
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

        Group* group = bot->GetGroup();
        BroadcastHelper::BroadcastGuildGroupOrRaidInvite(botAI, bot, player, group);
        return Invite(player);
    }

    return false;
}

bool InviteGuildToGroupAction::isUseful()
{
    return bot->GetGuildId() && InviteNearbyToGroupAction::isUseful();
};

bool JoinGroupAction::Execute(Event event)
{
    Player* master = event.getOwner();
    Group* group = master->GetGroup();

    if (group && (group->IsFull() || bot->GetGroup() == group))
        return false;

    if (bot->GetGroup())
        if (!botAI->DoSpecificAction("leave", event, true))
            return false;

    return Invite(bot);
}
