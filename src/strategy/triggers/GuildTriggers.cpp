/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GuildTriggers.h"

#include "GuildMgr.h"
#include "Playerbots.h"

bool PetitionTurnInTrigger::IsActive()
{
    return !bot->GetGuildId() && AI_VALUE2(uint32, "item count", chat->FormatQItem(5863)) &&
           AI_VALUE(uint8, "petition signs") >= sWorld->getIntConfig(CONFIG_MIN_PETITION_SIGNS);
}

bool BuyTabardTrigger::IsActive()
{
    return bot->GetGuildId() && !AI_VALUE2(uint32, "item count", chat->FormatQItem(5976));
}

bool LeaveLargeGuildTrigger::IsActive()
{
    if (!bot->GetGuildId())
        return false;

    if (botAI->IsRealPlayer())
        return false;

    if (botAI->IsAlt())
        return false;

    if (botAI->IsInRealGuild())
        return false;

    GuilderType type = botAI->GetGuilderType();

    Guild* guild = sGuildMgr->GetGuildById(bot->GetGuildId());

    Player* leader = ObjectAccessor::FindPlayer(guild->GetLeaderGUID());

    // Only leave the guild if we know the leader is not a real player.
    if (!leader || !GET_PLAYERBOT_AI(leader) || !GET_PLAYERBOT_AI(leader)->IsRealPlayer())
        return false;

    PlayerbotAI* leaderBotAI = GET_PLAYERBOT_AI(leader);
    if (!leaderBotAI || leaderBotAI->IsRealPlayer())
        return false;

    if (type == GuilderType::SOLO && guild->GetLeaderGUID() != bot->GetGUID())
        return true;

    uint32 members = guild->GetMemberSize();
    uint32 maxMembers = uint8(type);

    return members > maxMembers;
}
