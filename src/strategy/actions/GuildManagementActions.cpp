/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GuildManagementActions.h"

#include "GuildMgr.h"
#include "GuildPackets.h"
#include "Playerbots.h"
#include "ServerFacade.h"

Player* GuidManageAction::GetPlayer(Event event)
{
    Player* player = nullptr;
    ObjectGuid guid = event.getObject();

    if (guid)
    {
        player = ObjectAccessor::FindPlayer(guid);

        if (player)
            return player;
    }

    std::string text = event.getParam();

    if (!text.empty())
    {
        if (normalizePlayerName(text))
        {
            player = ObjectAccessor::FindPlayerByName(text.c_str());

            if (player)
                return player;
        }

        return nullptr;
    }

    Player* master = GetMaster();
    if (!master)
        guid = bot->GetTarget();
    else
        guid = master->GetTarget();

    player = ObjectAccessor::FindPlayer(guid);

    if (player)
        return player;

    player = event.getOwner();

    if (player)
        return player;

    return nullptr;
}

bool GuidManageAction::Execute(Event event)
{
    Player* player = GetPlayer(event);

    if (!player || !PlayerIsValid(player) || player == bot)
        return false;

    WorldPacket data(opcode);
    data << player->GetName();
    SendPacket(data);

    return true;
}

bool GuidManageAction::PlayerIsValid(Player* member) { return !member->GetGuildId(); }

uint8 GuidManageAction::GetRankId(Player* member)
{
    return sGuildMgr->GetGuildById(member->GetGuildId())->GetMember(member->GetGUID())->GetRankId();
}

bool GuildInviteAction::isUseful()
{
    return bot->GetGuildId() && sGuildMgr->GetGuildById(bot->GetGuildId())->HasRankRight(bot, GR_RIGHT_INVITE);
}

void GuildInviteAction::SendPacket(WorldPacket packet)
{
    WorldPackets::Guild::GuildInviteByName data = WorldPacket(packet);
    bot->GetSession()->HandleGuildInviteOpcode(data);
}

bool GuildInviteAction::PlayerIsValid(Player* member) { return !member->GetGuildId(); }

bool GuildPromoteAction::isUseful()
{
    return bot->GetGuildId() && sGuildMgr->GetGuildById(bot->GetGuildId())->HasRankRight(bot, GR_RIGHT_PROMOTE);
}

void GuildPromoteAction::SendPacket(WorldPacket packet)
{
    WorldPackets::Guild::GuildPromoteMember data = WorldPacket(packet);
    bot->GetSession()->HandleGuildPromoteOpcode(data);
}

bool GuildPromoteAction::PlayerIsValid(Player* member)
{
    return member->GetGuildId() == bot->GetGuildId() && GetRankId(bot) < GetRankId(member) - 1;
}

bool GuildDemoteAction::isUseful()
{
    return bot->GetGuildId() && sGuildMgr->GetGuildById(bot->GetGuildId())->HasRankRight(bot, GR_RIGHT_DEMOTE);
}

void GuildDemoteAction::SendPacket(WorldPacket packet)
{
    WorldPackets::Guild::GuildDemoteMember data = WorldPacket(packet);
    bot->GetSession()->HandleGuildDemoteOpcode(data);
}

bool GuildDemoteAction::PlayerIsValid(Player* member)
{
    return member->GetGuildId() == bot->GetGuildId() && GetRankId(bot) < GetRankId(member);
}

bool GuildRemoveAction::isUseful()
{
    return bot->GetGuildId() && sGuildMgr->GetGuildById(bot->GetGuildId())->HasRankRight(bot, GR_RIGHT_REMOVE);
}

void GuildRemoveAction::SendPacket(WorldPacket packet)
{
    WorldPackets::Guild::GuildOfficerRemoveMember data = WorldPacket(packet);
    bot->GetSession()->HandleGuildRemoveOpcode(data);
}

bool GuildRemoveAction::PlayerIsValid(Player* member)
{
    return member->GetGuildId() == bot->GetGuildId() && GetRankId(bot) < GetRankId(member);
};

bool GuildManageNearbyAction::Execute(Event event)
{
    uint32 found = 0;

    Guild* guild = sGuildMgr->GetGuildById(bot->GetGuildId());
    Guild::Member* botMember = guild->GetMember(bot->GetGUID());

    GuidVector nearGuids = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest friendly players")->Get();
    for (auto& guid : nearGuids)
    {
        Player* player = ObjectAccessor::FindPlayer(guid);

        if (!player || bot == player)
            continue;

        if (player->GetGuildId())  // Promote or demote nearby members based on chance.
        {
            Guild::Member* member = guild->GetMember(player->GetGUID());
            uint32 dCount = AI_VALUE(uint32, "death count");

            if ((dCount < 2 || !urand(0, 10)) && guild->GetRankRights(botMember->GetRankId() & GR_RIGHT_PROMOTE))
            {
                if (!urand(0, 10))
                {
                    botAI->DoSpecificAction("guild promote", Event("guild management", guid), true);

                    continue;
                }
            }

            if ((dCount > 3 || !urand(0, 10)) && guild->GetRankRights(botMember->GetRankId() & GR_RIGHT_DEMOTE))
            {
                if (!urand(0, 10))
                {
                    botAI->DoSpecificAction("guild demote", Event("guild management", guid), true);

                    continue;
                }
            }

            continue;
        }

        if (!(guild->GetRankRights(botMember->GetRankId()) & GR_RIGHT_INVITE))
            continue;

        if (player->GetGuildIdInvited())
            continue;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);

        if (botAI)
        {
            if (botAI->GetGuilderType() == GuilderType::SOLO &&
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

        if (sServerFacade->GetDistance2d(bot, player) > sPlayerbotAIConfig->sightDistance)
            continue;

        if (botAI && botAI->DoSpecificAction("ginvite", Event("guild management", guid)))
            found++;
    }

    return found > 0;
}

bool GuildManageNearbyAction::isUseful()
{
    if (!bot->GetGuildId())
        return false;

    Guild* guild = sGuildMgr->GetGuildById(bot->GetGuildId());
    Guild::Member* botMember = guild->GetMember(bot->GetGUID());

    return guild->GetRankRights(botMember->GetRankId()) & (GR_RIGHT_DEMOTE | GR_RIGHT_PROMOTE | GR_RIGHT_INVITE);
}

bool GuildLeaveAction::Execute(Event event)
{
    Player* owner = event.getOwner();
    if (owner && !botAI->GetSecurity()->CheckLevelFor(PLAYERBOT_SECURITY_INVITE, false, owner, true))
    {
        botAI->TellError("Sorry, I am happy in my guild :)");
        return false;
    }

    WorldPackets::Guild::GuildLeave data = WorldPacket(CMSG_GUILD_LEAVE);
    bot->GetSession()->HandleGuildLeaveOpcode(data);
    return true;
}

bool GuildLeaveAction::isUseful() { return bot->GetGuildId(); }
