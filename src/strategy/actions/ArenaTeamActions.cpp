/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ArenaTeamActions.h"

#include "ArenaTeamMgr.h"
#include "Playerbots.h"

bool ArenaTeamAcceptAction::Execute(Event event)
{
    WorldPacket p(event.getPacket());
    p.rpos(0);
    Player* inviter = nullptr;
    std::string Invitedname;
    p >> Invitedname;

    if (normalizePlayerName(Invitedname))
        inviter = ObjectAccessor::FindPlayerByName(Invitedname.c_str());

    if (!inviter)
        return false;

    ArenaTeam* at = sArenaTeamMgr->GetArenaTeamById(bot->GetArenaTeamIdInvited());
    if (!at)
        return false;

    bool accept = true;

    if (bot->GetArenaTeamId(at->GetSlot()))
    {
        // bot is already in an arena team
        bot->Say("Sorry, I am already in such team", LANG_UNIVERSAL);
        accept = false;
    }

    if (accept)
    {
        WorldPacket data(CMSG_ARENA_TEAM_ACCEPT);
        bot->GetSession()->HandleArenaTeamAcceptOpcode(data);
        bot->Say("Thanks for the invite!", LANG_UNIVERSAL);
        LOG_INFO("playerbots", "Bot {} <{}> accepts Arena Team invite", bot->GetGUID().ToString().c_str(),
                 bot->GetName().c_str());
        return true;
    }
    else
    {
        WorldPacket data(CMSG_ARENA_TEAM_DECLINE);
        bot->GetSession()->HandleArenaTeamDeclineOpcode(data);
        LOG_INFO("playerbots", "Bot {} <{}> declines Arena Team invite", bot->GetGUID().ToString().c_str(),
                 bot->GetName().c_str());
        return false;
    }

    return false;
}
