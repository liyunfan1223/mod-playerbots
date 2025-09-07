/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "InviteToGroupAction.h"

#include "BroadcastHelper.h"
#include "Event.h"
#include "GuildMgr.h"
#include "Log.h"
#include "Playerbots.h"
#include "ServerFacade.h"

bool InviteToGroupAction::Invite(Player* inviter, Player* player)
{
    if (!player)
        return false;

    if (inviter == player)
        return false;

    if (!GET_PLAYERBOT_AI(player) && !botAI->GetSecurity()->CheckLevelFor(PLAYERBOT_SECURITY_INVITE, true, player))
        return false;

    if (Group* group = inviter->GetGroup())
    {
        if (GET_PLAYERBOT_AI(player) && !GET_PLAYERBOT_AI(player)->IsRealPlayer())
            if (!group->isRaidGroup() && group->GetMembersCount() > 4)
                group->ConvertToRaid();
    }

    WorldPacket p;
    uint32 roles_mask = 0;
    p << player->GetName();
    p << roles_mask;
    inviter->GetSession()->HandleGroupInviteOpcode(p);

    return true;
}

bool InviteNearbyToGroupAction::Execute(Event event)
{
    GuidVector nearGuids = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest friendly players")->Get();
    for (auto& i : nearGuids)
    {
        Player* player = ObjectAccessor::FindPlayer(i);
        if (!player)
            continue;

        if (player == bot)
            continue;

        if (player->GetMapId() != bot->GetMapId())
            continue;

        if (player->GetGroup())
            continue;

        if (!sPlayerbotAIConfig->randomBotInvitePlayer && GET_PLAYERBOT_AI(player)->IsRealPlayer())
            continue;

        Group* group = bot->GetGroup();

        if (player->isDND())
            continue;

        if (player->IsBeingTeleported())
            continue;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);

        if (botAI)
        {
            if (botAI->GetGrouperType() == GrouperType::SOLO &&
                !botAI->HasRealPlayerMaster())  // Do not invite solo players.
                continue;

            if (botAI->HasActivePlayerMaster())  // Do not invite alts of active players.
                continue;
        }

        if (abs(int32(player->GetLevel() - bot->GetLevel())) > 2)
            continue;

        if (sServerFacade->GetDistance2d(bot, player) > sPlayerbotAIConfig->sightDistance)
            continue;

        // When inviting the 5th member of the group convert to raid for future invites.
        if (group && botAI->GetGrouperType() > GrouperType::LEADER_5 && !group->isRaidGroup() &&
            bot->GetGroup()->GetMembersCount() > 3)
            group->ConvertToRaid();

        if (sPlayerbotAIConfig->inviteChat && sRandomPlayerbotMgr->IsRandomBot(bot))
        {
            std::map<std::string, std::string> placeholders;
            placeholders["%player"] = player->GetName();

            if (group && group->isRaidGroup())
                bot->Say(BOT_TEXT2("join_raid", placeholders),
                         (bot->GetTeamId() == TEAM_ALLIANCE ? LANG_COMMON : LANG_ORCISH));
            else
                bot->Say(BOT_TEXT2("join_group", placeholders),
                         (bot->GetTeamId() == TEAM_ALLIANCE ? LANG_COMMON : LANG_ORCISH));
        }

        return Invite(bot, player);
    }

    return false;
}

bool InviteNearbyToGroupAction::isUseful()
{
    if (!sPlayerbotAIConfig->randomBotGroupNearby)
        return false;

    if (bot->InBattleground())
        return false;

    if (bot->InBattlegroundQueue())
        return false;

    GrouperType grouperType = botAI->GetGrouperType();

    if (grouperType == GrouperType::SOLO || grouperType == GrouperType::MEMBER)
        return false;

    Group* group = bot->GetGroup();

    if (group)
    {
        if (group->isRaidGroup() && group->IsFull())
            return false;

        if (botAI->GetGroupMaster() != bot)
            return false;

        uint32 memberCount = group->GetMembersCount();

        if (memberCount >= uint8(grouperType))
            return false;
    }

    if (botAI->HasActivePlayerMaster())  // Alts do not invite randomly
        return false;

    return true;
}

std::vector<Player*> InviteGuildToGroupAction::getGuildMembers()
{
    Guild* guild = sGuildMgr->GetGuildById(bot->GetGuildId());

    FindGuildMembers worker;
    guild->BroadcastWorker(worker);

    return worker.GetResult();
}

bool InviteGuildToGroupAction::Execute(Event event)
{
    Guild* guild = sGuildMgr->GetGuildById(bot->GetGuildId());

    for (auto& member : getGuildMembers())
    {
        Player* player = member;

        if (!player)
            continue;

        if (player == bot)
            continue;

        if (player->GetGroup())
            continue;

        if (player->isDND())
            continue;

        if (!sPlayerbotAIConfig->randomBotInvitePlayer && GET_PLAYERBOT_AI(player)->IsRealPlayer())
            continue;

        if (player->IsBeingTeleported())
            continue;

        if (player->GetMapId() != bot->GetMapId() && player->GetLevel() < 30)
            continue;

        if (WorldPosition(player).distance(bot) > 1000 && player->GetLevel() < 15)
            continue;

        PlayerbotAI* playerAi = GET_PLAYERBOT_AI(player);

        if (playerAi)
        {
            if (playerAi->GetGrouperType() == GrouperType::SOLO &&
                !playerAi->HasRealPlayerMaster())  // Do not invite solo players.
                continue;

            if (playerAi->HasActivePlayerMaster())  // Do not invite alts of active players.
                continue;

            if (player->GetLevel() >
                bot->GetLevel() + 5)  // Invite higher levels that need money so they can grind money and help out.
            {
                if (!PAI_VALUE(bool, "should get money"))
                    continue;
            }
        }

        if (bot->GetLevel() >
            player->GetLevel() + 5)  // Do not invite members that too low level or risk dragging them to deadly places.
            continue;

        if (!playerAi && sServerFacade->GetDistance2d(bot, player) > sPlayerbotAIConfig->sightDistance)
            continue;

        Group* group = bot->GetGroup();
        // When inviting the 5th member of the group convert to raid for future invites.
        if (group && botAI->GetGrouperType() > GrouperType::LEADER_5 && !group->isRaidGroup() &&
            bot->GetGroup()->GetMembersCount() > 3)
        {
            group->ConvertToRaid();
        }

        if (sPlayerbotAIConfig->inviteChat &&
            (sRandomPlayerbotMgr->IsRandomBot(bot) || !botAI->HasActivePlayerMaster()))
        {
            BroadcastHelper::BroadcastGuildGroupOrRaidInvite(botAI, bot, player, group);
        }

        return Invite(bot, player);
    }

    return false;
}

bool JoinGroupAction::Execute(Event event)
{
    if (bot->InBattleground())
        return false;

    if (bot->InBattlegroundQueue())
        return false;

    Player* master = event.getOwner();

    Group* group = master->GetGroup();

    if (group)
    {
        if (group->IsFull())
            return false;

        if (bot->GetGroup() == group)
            return false;
    }

    if (bot->GetGroup())
    {
        if (botAI->HasRealPlayerMaster())
            return false;

        if (!botAI->DoSpecificAction("leave", event, true))
            return false;
    }

    return Invite(master, bot);
}

bool LfgAction::Execute(Event event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();

    if (bot->InBattleground())
        return false;

    if (bot->InBattlegroundQueue())
        return false;

    if (!botAI->IsSafe(requester))
        return false;

    if (requester->GetLevel() == DEFAULT_MAX_LEVEL && bot->GetLevel() != DEFAULT_MAX_LEVEL)
        return false;

    if (requester->GetLevel() > bot->GetLevel() + 4 || bot->GetLevel() > requester->GetLevel() + 4)
        return false;

    std::string param = event.getParam();

    if (!param.empty() && param != "40" && param != "25" && param != "20" && param != "10" && param != "5")
        return false;

    Group* group = requester->GetGroup();

    std::unordered_map<Classes, std::unordered_map<BotRoles, uint32>> allowedClassNr;
    std::unordered_map<BotRoles, uint32> allowedRoles;

    allowedRoles[BOT_ROLE_TANK] = 1;
    allowedRoles[BOT_ROLE_HEALER] = 1;
    allowedRoles[BOT_ROLE_DPS] = 3;

    BotRoles role = botAI->IsTank(requester, false)
                        ? BOT_ROLE_TANK
                        : (botAI->IsHeal(requester, false) ? BOT_ROLE_HEALER : BOT_ROLE_DPS);
    Classes cls = (Classes)requester->getClass();

    if (group)
    {
        if (param.empty() && group->isRaidGroup())
            // Default to WotLK Raiding. Max size 25
            param = "25";
        // Select optimal group layout.
        if (param == "40")
        {
            allowedRoles[BOT_ROLE_TANK] = 4;
            allowedRoles[BOT_ROLE_HEALER] = 16;
            allowedRoles[BOT_ROLE_DPS] = 20;
            /*
            allowedClassNr[CLASS_PALADIN][BOT_ROLE_TANK] = 0;
            allowedClassNr[CLASS_DRUID][BOT_ROLE_TANK] = 1;

            allowedClassNr[CLASS_DRUID][BOT_ROLE_HEALER] = 3;
            allowedClassNr[CLASS_PALADIN][BOT_ROLE_HEALER] = 4;
            allowedClassNr[CLASS_SHAMAN][BOT_ROLE_HEALER] = 4;
            allowedClassNr[CLASS_PRIEST][BOT_ROLE_HEALER] = 11;

            allowedClassNr[CLASS_WARRIOR][BOT_ROLE_DPS] = 8;
            allowedClassNr[CLASS_PALADIN][BOT_ROLE_DPS] = 4;
            allowedClassNr[CLASS_HUNTER][BOT_ROLE_DPS] = 4;
            allowedClassNr[CLASS_ROGUE][BOT_ROLE_DPS] = 6;
            allowedClassNr[CLASS_PRIEST][BOT_ROLE_DPS] = 1;
            allowedClassNr[CLASS_SHAMAN][BOT_ROLE_DPS] = 4;
            allowedClassNr[CLASS_MAGE][BOT_ROLE_DPS] = 15;
            allowedClassNr[CLASS_WARLOCK][BOT_ROLE_DPS] = 4;
            allowedClassNr[CLASS_DRUID][BOT_ROLE_DPS] = 1;
            */
        }
        else if (param == "25")
        {
            allowedRoles[BOT_ROLE_TANK] = 3;
            allowedRoles[BOT_ROLE_HEALER] = 7;
            allowedRoles[BOT_ROLE_DPS] = 15;
        }
        else if (param == "20")
        {
            allowedRoles[BOT_ROLE_TANK] = 2;
            allowedRoles[BOT_ROLE_HEALER] = 5;
            allowedRoles[BOT_ROLE_DPS] = 13;
        }
        else if (param == "10")
        {
            allowedRoles[BOT_ROLE_TANK] = 2;
            allowedRoles[BOT_ROLE_HEALER] = 3;
            allowedRoles[BOT_ROLE_DPS] = 5;
        }

        if (group->IsFull())
        {
            if (param.empty() || param == "5" || group->isRaidGroup())
                return false;  // Group or raid is full so stop trying.
            else
                group->ConvertToRaid();  // We want a raid but are in a group so convert and continue.
        }

        Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
        for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
        {
            // Only add group member targets that are alive and near the player
            Player* player = ObjectAccessor::FindPlayer(itr->guid);

            if (!botAI->IsSafe(player))
                return false;

            role = botAI->IsTank(player, false) ? BOT_ROLE_TANK
                                                : (botAI->IsHeal(player, false) ? BOT_ROLE_HEALER : BOT_ROLE_DPS);
            cls = (Classes)player->getClass();

            if (allowedRoles[role] > 0)
                allowedRoles[role]--;

            if (allowedClassNr[cls].find(role) != allowedClassNr[cls].end() && allowedClassNr[cls][role] > 0)
                allowedClassNr[cls][role]--;
        }
    }
    else
    {
        if (allowedRoles[role] > 0)
            allowedRoles[role]--;

        if (allowedClassNr[cls].find(role) != allowedClassNr[cls].end() && allowedClassNr[cls][role] > 0)
            allowedClassNr[cls][role]--;
    }

    role = botAI->IsTank(bot, false) ? BOT_ROLE_TANK : (botAI->IsHeal(bot, false) ? BOT_ROLE_HEALER : BOT_ROLE_DPS);
    cls = (Classes)bot->getClass();

    if (allowedRoles[role] == 0)
        return false;

    if (allowedClassNr[cls].find(role) != allowedClassNr[cls].end() && allowedClassNr[cls][role] == 0)
        return false;

    if (bot->GetGroup())
    {
        if (botAI->HasRealPlayerMaster())
            return false;

        if (!botAI->DoSpecificAction("leave", event, true))
            return false;
    }

    bool invite = Invite(requester, bot);

    if (invite)
    {
        Event acceptEvent("accept invitation", requester ? requester->GetGUID() : ObjectGuid::Empty);
        if (!botAI->DoSpecificAction("accept invitation", acceptEvent, true))
            return false;

        std::map<std::string, std::string> placeholders;
        placeholders["%role"] = (role & BOT_ROLE_TANK ? "tank" : (role & BOT_ROLE_HEALER ? "healer" : "dps"));
        placeholders["%spotsleft"] = std::to_string(allowedRoles[role] - 1);

        std::ostringstream out;
        if (allowedRoles[role] > 1)
        {
            out << "Joining as " << placeholders["%role"] << ", " << placeholders["%spotsleft"] << " "
                << placeholders["%role"] << " spots left.";
            botAI->TellMasterNoFacing(out.str());

            //botAI->DoSpecificAction("autogear");
            //botAI->DoSpecificAction("maintenance");
        }
        else
        {
            out << "Joining as " << placeholders["%role"] << ".";
            botAI->TellMasterNoFacing(out.str());

            //botAI->DoSpecificAction("autogear");
            //botAI->DoSpecificAction("maintenance");
        }

        return true;
    }

    return false;
}
