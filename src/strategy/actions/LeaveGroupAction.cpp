/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "LeaveGroupAction.h"
#include "Event.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

bool LeaveGroupAction::Execute(Event event)
{
    Player* master = event.getOwner();
    return Leave(master);
}

bool PartyCommandAction::Execute(Event event)
{
    WorldPacket& p = event.getPacket();
    p.rpos(0);
    uint32 operation;
    std::string member;

    p >> operation >> member;

    if (operation != PARTY_OP_LEAVE)
        return false;

    Player* master = GetMaster();
    if (master && member == master->GetName())
        return Leave(bot);

    return false;
}

bool UninviteAction::Execute(Event event)
{
    WorldPacket& p = event.getPacket();
    if (p.GetOpcode() == CMSG_GROUP_UNINVITE)
    {
        p.rpos(0);
        std::string membername;
        p >> membername;

        // player not found
        if (!normalizePlayerName(membername))
        {
            return false;
        }

        if (bot->GetName() == membername)
            return Leave(bot);
    }

    if (p.GetOpcode() == CMSG_GROUP_UNINVITE_GUID)
    {
        p.rpos(0);
        ObjectGuid guid;
        p >> guid;

        if (bot->GetGUID() == guid)
            return Leave(bot);
    }

    return false;
}

bool LeaveGroupAction::Leave(Player* player)
{
    if (player && !GET_PLAYERBOT_AI(player) && !botAI->GetSecurity()->CheckLevelFor(PLAYERBOT_SECURITY_INVITE, false, player))
        return false;

    bool aiMaster = GET_PLAYERBOT_AI(botAI->GetMaster()) != nullptr;

    botAI->TellMaster("Goodbye!", PLAYERBOT_SECURITY_TALK);

    bool randomBot = sRandomPlayerbotMgr->IsRandomBot(bot);
    bool shouldStay = randomBot && bot->GetGroup() && player == bot;
    if (!shouldStay)
    {
        WorldPacket p;
        p << uint32(PARTY_OP_LEAVE) << bot->GetName() << uint32(0);
        bot->GetSession()->HandleGroupDisbandOpcode(p);
    }

    if (randomBot)
    {
        GET_PLAYERBOT_AI(bot)->SetMaster(nullptr);
    }

    if (!aiMaster)
        botAI->ResetStrategies(!randomBot);

    botAI->Reset();

    return true;
}

bool LeaveFarAwayAction::Execute(Event event)
{
    return Leave(nullptr);
}

bool LeaveFarAwayAction::isUseful()
{
    if (!sPlayerbotAIConfig->randomBotGroupNearby)
        return false;

    if (bot->InBattleground())
        return false;

    if (bot->InBattlegroundQueue())
        return false;

    if (!bot->GetGroup())
        return false;

    Player* master = botAI->GetGroupMaster();
    Player* trueMaster = botAI->GetMaster();
    if (!master || (bot == master && !botAI->IsRealPlayer()))
        return false;

    PlayerbotAI* masterBotAI = nullptr;
    if (master)
        masterBotAI = GET_PLAYERBOT_AI(master);
    if (master && !masterBotAI)
        return false;

    if (trueMaster && !GET_PLAYERBOT_AI(trueMaster))
        return false;

    if (botAI->IsAlt() && (!masterBotAI || masterBotAI->IsRealPlayer())) // Don't leave group when alt grouped with player master.
        return false;

    if (botAI->GetGrouperType() == GrouperType::SOLO)
        return true;

    uint32 dCount = AI_VALUE(uint32, "death count");

    if (dCount > 9)
        return true;

    if (dCount > 4 && !botAI->HasRealPlayerMaster())
        return true;

    if (bot->GetGuildId() == master->GetGuildId())
    {
        if (bot->getLevel() > master->getLevel() + 5)
        {
            if (AI_VALUE(bool, "should get money"))
                return false;
        }
    }

    if (abs(int32(master->getLevel() - bot->getLevel())) > 4)
        return true;

    if (bot->GetMapId() != master->GetMapId() || bot->GetDistance2d(master) >= 2 * sPlayerbotAIConfig->rpgDistance) {
        return true;
    }
    
    return false;
}
