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

#include "InviteToGroupAction.h"
#include "BroadcastHelper.h"
#include "Event.h"
#include "GuildMgr.h"
#include "PlayerbotOperations.h"
#include "PlayerbotWorldThreadProcessor.h"
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
        if (GET_PLAYERBOT_AI(player) && !IsSelfBot(player))
            if (!group->isRaidGroup() && group->GetMembersCount() > 4)
            {
                auto convertOp = std::make_unique<GroupConvertToRaidOperation>(inviter->GetGUID());
                PlayerbotWorldThreadProcessor::instance().QueueOperation(std::move(convertOp));
            }
    }

    WorldPacket p;
    uint32 roles_mask = 0;
    p << player->GetName();
    p << roles_mask;
    inviter->GetSession()->HandleGroupInviteOpcode(p);

    return true;
}

bool InviteNearbyToGroupAction::Execute(Event /*event*/)
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

        if (!PlayerbotAIConfig::Instance().RandomBotInvitePlayer && IsSelfBot(player))
            continue;

        Group* group = bot->GetGroup();

        if (player->isDND())
            continue;

        if (player->IsBeingTeleported())
            continue;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);

        if (botAI)
        {
            // A solo-grouper bot with no real master (regular player or SelfBot master) does not invite.
            if (botAI->GetGrouperType() == GrouperType::SOLO && !botAI->HasGameClientMaster())
                continue;

            if (IsRealPlayer(botAI->GetMaster()))  // An active player's altbot does not auto-invite.
                continue;
        }

        if (abs(int32(player->GetLevel() - bot->GetLevel())) > 2)
            continue;

        if (ServerFacade::instance().GetDistance2d(bot, player) > PlayerbotAIConfig::Instance().SightDistance)
            continue;

        // When inviting the 5th member of the group convert to raid for future invites.
        if (group && botAI->GetGrouperType() > GrouperType::LEADER_5 && !group->isRaidGroup() &&
            bot->GetGroup()->GetMembersCount() > 3)
        {
            auto convertOp = std::make_unique<GroupConvertToRaidOperation>(bot->GetGUID());
            PlayerbotWorldThreadProcessor::instance().QueueOperation(std::move(convertOp));
        }

        if (PlayerbotAIConfig::Instance().InviteChat && RandomPlayerbotMgr::instance().IsRandomBot(bot))
        {
            std::map<std::string, std::string> placeholders;
            placeholders["%player"] = player->GetName();

            if (group && group->isRaidGroup())
                bot->Say(PlayerbotTextMgr::instance().GetBotText("join_raid", placeholders),
                         (bot->GetTeamId() == TEAM_ALLIANCE ? LANG_COMMON : LANG_ORCISH));
            else
                bot->Say(PlayerbotTextMgr::instance().GetBotText("join_group", placeholders),
                         (bot->GetTeamId() == TEAM_ALLIANCE ? LANG_COMMON : LANG_ORCISH));
        }

        return Invite(bot, player);
    }

    return false;
}

bool InviteNearbyToGroupAction::isUseful()
{
    if (!PlayerbotAIConfig::Instance().RandomBotGroupNearby)
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

        if (botAI->GetGroupLeader() != bot)
            return false;

        uint32 memberCount = group->GetMembersCount();

        if (memberCount >= uint8(grouperType))
            return false;
    }

    if (IsRealPlayer(botAI->GetMaster()))  // An active player's altbot does not auto-invite.
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

bool InviteGuildToGroupAction::Execute(Event /*event*/)
{
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

        if (!PlayerbotAIConfig::Instance().RandomBotInvitePlayer && IsSelfBot(player))
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
            // Do not invite solo players.
            if (playerAi->GetGrouperType() == GrouperType::SOLO && !playerAi->HasGameClientMaster())
                continue;

            if (IsRealPlayer(playerAi->GetMaster()))  // Do not invite bots that belong to an active player.
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

        if (!playerAi && ServerFacade::instance().GetDistance2d(bot, player) > PlayerbotAIConfig::Instance().SightDistance)
            continue;

        Group* group = bot->GetGroup();
        // When inviting the 5th member of the group convert to raid for future invites.
        if (group && botAI->GetGrouperType() > GrouperType::LEADER_5 && !group->isRaidGroup() &&
            bot->GetGroup()->GetMembersCount() > 3)
        {
            auto convertOp = std::make_unique<GroupConvertToRaidOperation>(bot->GetGUID());
            PlayerbotWorldThreadProcessor::instance().QueueOperation(std::move(convertOp));
        }

        if (PlayerbotAIConfig::Instance().InviteChat &&
            (RandomPlayerbotMgr::instance().IsRandomBot(bot) || !IsRealPlayer(botAI->GetMaster())))
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
        if (botAI->HasGameClientMaster())
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

    BotRoles role = PlayerbotAI::IsTank(requester, true)
                        ? BOT_ROLE_TANK
                        : (PlayerbotAI::IsHeal(requester, true) ? BOT_ROLE_HEALER : BOT_ROLE_DPS);
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
            {
                auto convertOp = std::make_unique<GroupConvertToRaidOperation>(requester->GetGUID());
                PlayerbotWorldThreadProcessor::instance().QueueOperation(std::move(convertOp));
            }
        }

        Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
        for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
        {
            // Only add group member targets that are alive and near the player
            Player* player = ObjectAccessor::FindPlayer(itr->guid);

            if (!botAI->IsSafe(player))
                return false;

            role = PlayerbotAI::IsTank(player, true) ? BOT_ROLE_TANK
                                               : (PlayerbotAI::IsHeal(player, true) ? BOT_ROLE_HEALER : BOT_ROLE_DPS);
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

    role = PlayerbotAI::IsTank(bot, true) ? BOT_ROLE_TANK
                                          : (PlayerbotAI::IsHeal(bot, true) ? BOT_ROLE_HEALER : BOT_ROLE_DPS);
    cls = (Classes)bot->getClass();

    if (allowedRoles[role] == 0)
        return false;

    if (allowedClassNr[cls].find(role) != allowedClassNr[cls].end() && allowedClassNr[cls][role] == 0)
        return false;

    if (bot->GetGroup())
    {
        if (botAI->HasGameClientMaster())
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
