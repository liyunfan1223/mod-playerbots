/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ObjectAccessor.h"
#include "Opcodes.h"
#include "Player.h"
#include "Playerbots.h"
#include "ScriptMgr.h"

namespace
{
    static Player* FindOnlineAltbotByGuid(ObjectGuid guid)
    {
        if (!guid)
            return nullptr;

        Player* p = ObjectAccessor::FindPlayer(guid);
        if (!p)
            return nullptr;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(p);
        if (!botAI || IsSelfBot(p))
            return nullptr;

        return p;
    }

    static void ForceLogoutViaPlayerbotHolder(Player* target)
    {
        if (!target)
            return;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(target);

        if (!botAI)
            return;

        if (Player* master = botAI->GetMaster())
        {
            if (PlayerbotMgr* mgr = GET_PLAYERBOT_MGR(master))
            {
                mgr->LogoutPlayerBot(target->GetGUID());
                return;
            }
        }

        sRandomPlayerbotMgr.LogoutPlayerBot(target->GetGUID());
    }
}

class PlayerbotsSecureLoginServerScript : public ServerScript
{
public:
    PlayerbotsSecureLoginServerScript()
        : ServerScript("PlayerbotsSecureLoginServerScript", { SERVERHOOK_CAN_PACKET_RECEIVE }) {}

    bool CanPacketReceive(WorldSession* /*session*/, WorldPacket const& packet) override
    {
        if (packet.GetOpcode() != CMSG_PLAYER_LOGIN)
            return true;

        WorldPacket pkt(packet);
        ObjectGuid loginGuid;
        pkt >> loginGuid;

        if (!loginGuid)
            return true;

        Player* existingAltbot = FindOnlineAltbotByGuid(loginGuid);
        if (existingAltbot)
            ForceLogoutViaPlayerbotHolder(existingAltbot);

        return true;
    }
};

void AddPlayerbotsSecureLoginScripts()
{
    new PlayerbotsSecureLoginServerScript();
}
