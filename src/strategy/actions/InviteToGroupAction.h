/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_INVITETOGROUPACTION_H
#define _PLAYERBOT_INVITETOGROUPACTION_H

#include "Action.h"
#include "Player.h"

class PlayerbotAI;

class InviteToGroupAction : public Action
{
public:
    InviteToGroupAction(PlayerbotAI* botAI, std::string const name = "invite") : Action(botAI, name) {}

    bool Execute(Event event) override
    {
        Player* master = event.getOwner();
        return Invite(bot, master);
    }

    virtual bool Invite(Player* inviter, Player* player);
};

class JoinGroupAction : public InviteToGroupAction
{
public:
    JoinGroupAction(PlayerbotAI* ai, std::string name = "join") : InviteToGroupAction(ai, name) {}
    bool Execute(Event event) override;
};

class InviteNearbyToGroupAction : public InviteToGroupAction
{
public:
    InviteNearbyToGroupAction(PlayerbotAI* botAI, std::string const name = "invite nearby") : InviteToGroupAction(botAI, name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Generic guid member finder
class FindGuildMembers
{
public:
    FindGuildMembers(){};

    void operator()(Player* player) { data.push_back(player); };

    std::vector<Player*> const GetResult() { return data; };

private:
    std::vector<Player*> data;
};

class InviteGuildToGroupAction : public InviteNearbyToGroupAction
{
public:
    InviteGuildToGroupAction(PlayerbotAI* botAI, std::string const name = "invite guild")
        : InviteNearbyToGroupAction(botAI, name)
    {
    }

    bool Execute(Event event) override;
    bool isUseful() override { return bot->GetGuildId() && InviteNearbyToGroupAction::isUseful(); };

private:
    std::vector<Player*> getGuildMembers();
};

class LfgAction : public InviteToGroupAction
{
public:
    LfgAction(PlayerbotAI* botAI, std::string name = "lfg") : InviteToGroupAction(botAI, name) {}
    bool Execute(Event event) override;
};

#endif
