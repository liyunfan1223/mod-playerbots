/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_GUILDMAMANEGEMENTACTION_H
#define _PLAYERBOT_GUILDMAMANEGEMENTACTION_H

#include "Action.h"
#include "Opcodes.h"

class Player;
class PlayerbotAI;
class WorldPacket;

class GuidManageAction : public Action
{
public:
    GuidManageAction(PlayerbotAI* botAI, std::string const name = "guild manage", uint16 opcode = CMSG_GUILD_INVITE)
        : Action(botAI, name), opcode(opcode)
    {
    }

    bool Execute(Event event) override;
    bool isUseful() override { return false; }

protected:
    virtual void SendPacket(WorldPacket data){};
    virtual Player* GetPlayer(Event event);
    virtual bool PlayerIsValid(Player* member);
    virtual uint8 GetRankId(Player* member);

    uint16 opcode;
};

class GuildInviteAction : public GuidManageAction
{
public:
    GuildInviteAction(PlayerbotAI* botAI, std::string const name = "guild invite", uint16 opcode = CMSG_GUILD_INVITE)
        : GuidManageAction(botAI, name, opcode)
    {
    }

    bool isUseful() override;

protected:
    void SendPacket(WorldPacket data) override;
    bool PlayerIsValid(Player* member) override;
};

class GuildPromoteAction : public GuidManageAction
{
public:
    GuildPromoteAction(PlayerbotAI* botAI, std::string const name = "guild promote", uint16 opcode = CMSG_GUILD_PROMOTE)
        : GuidManageAction(botAI, name, opcode)
    {
    }

    bool isUseful() override;

protected:
    void SendPacket(WorldPacket data) override;
    bool PlayerIsValid(Player* member) override;
};

class GuildDemoteAction : public GuidManageAction
{
public:
    GuildDemoteAction(PlayerbotAI* botAI, std::string const name = "guild demote", uint16 opcode = CMSG_GUILD_DEMOTE)
        : GuidManageAction(botAI, name, opcode)
    {
    }

    bool isUseful() override;

protected:
    void SendPacket(WorldPacket data) override;
    bool PlayerIsValid(Player* member) override;
};

class GuildRemoveAction : public GuidManageAction
{
public:
    GuildRemoveAction(PlayerbotAI* botAI, std::string const name = "guild remove", uint16 opcode = CMSG_GUILD_REMOVE)
        : GuidManageAction(botAI, name, opcode)
    {
    }

    bool isUseful() override;

protected:
    void SendPacket(WorldPacket data) override;
    bool PlayerIsValid(Player* member) override;
};

class GuildManageNearbyAction : public Action
{
public:
    GuildManageNearbyAction(PlayerbotAI* botAI) : Action(botAI, "guild manage nearby") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class GuildLeaveAction : public Action
{
public:
    GuildLeaveAction(PlayerbotAI* botAI) : Action(botAI, "guild leave") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
