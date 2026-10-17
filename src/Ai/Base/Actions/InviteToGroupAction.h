/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

/*
 * Ported from the CMaNGOS playerbots project (https://github.com/cmangos/playerbots), GPL v2,
 * with modifications for AzerothCore.
 * Original authors:
 *   Sebastiaan Keek (mostlikely4r) <sebastiaan.keek@gmail.com> - original author
 *   David Parra Ausina (Flekz) <davidparraausina@gmail.com>
 *   celguar <celguar@gmail.com>
 */

#ifndef PLAYERBOTS_INVITETOGROUPACTION_H
#define PLAYERBOTS_INVITETOGROUPACTION_H

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
    JoinGroupAction(PlayerbotAI* botAI, std::string name = "join") : InviteToGroupAction(botAI, name) {}
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
