/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CharacterPackets.h"
#include "Log.h"
#include "Opcodes.h"
#include "Player.h"
#include "Playerbots.h"
#include "ScriptMgr.h"
#include "WorldPacket.h"
#include "WorldSession.h"

class PlayerbotsSelfBotAfkServerScript : public ServerScript
{
public:
    PlayerbotsSelfBotAfkServerScript()
        : ServerScript("PlayerbotsSelfBotAfkServerScript", { SERVERHOOK_CAN_PACKET_RECEIVE }) {}

    bool CanPacketReceive(WorldSession* session, WorldPacket const& packet) override
    {
        if (packet.GetOpcode() != CMSG_LOGOUT_REQUEST)
            return true;

        Player* player = session ? session->GetPlayer() : nullptr;
        if (!player || !IsSelfBot(player) || !player->isAFK())
            return true;

        LOG_DEBUG("playerbots", "SelfBot {} stays in world: refused the AFK logout", player->GetName());

        WorldPackets::Character::LogoutResponse logoutResponse;
        logoutResponse.LogoutResult = 2;
        logoutResponse.Instant = false;
        session->SendPacket(logoutResponse.Write());

        return false;
    }
};

void AddPlayerbotsSelfBotAfkScripts()
{
    new PlayerbotsSelfBotAfkServerScript();
}
