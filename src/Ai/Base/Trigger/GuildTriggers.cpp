/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
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

    if (IsSelfBot(bot))
        return false;

    if (botAI->IsAltBot())
        return false;

    if (botAI->IsInRealGuild())
        return false;

    GuilderType type = botAI->GetGuilderType();

    Guild* guild = sGuildMgr->GetGuildById(bot->GetGuildId());

    Player* leader = ObjectAccessor::FindPlayer(guild->GetLeaderGUID());

    // Only leave the guild if the leader is a bot (but not a SelfBot).
    PlayerbotAI* leaderBotAI = leader ? GET_PLAYERBOT_AI(leader) : nullptr;
    if (!leaderBotAI || IsSelfBot(leader))
        return false;

    if (type == GuilderType::SOLO && guild->GetLeaderGUID() != bot->GetGUID())
        return true;

    uint32 members = guild->GetMemberSize();
    uint32 maxMembers = uint8(type);

    return members > maxMembers;
}
