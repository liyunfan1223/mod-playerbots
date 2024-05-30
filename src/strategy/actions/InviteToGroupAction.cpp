/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "InviteToGroupAction.h"
#include "Event.h"
#include "GuildMgr.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "PlayerbotAI.h"

bool InviteToGroupAction::Execute(Event event)
{
    Player* master = event.getOwner();
    if (!master)
        return false;

    return Invite(master, bot);
}

bool InviteToGroupAction::Invite(Player* inviter, Player* player)
{
    if (!player)
        return false;

    if (!GET_PLAYERBOT_AI(player) && !botAI->GetSecurity()->CheckLevelFor(PLAYERBOT_SECURITY_INVITE, true, player))
        return false;

    WorldPacket p;
    uint32 roles_mask = 0;
    p << player->GetName();
    p << roles_mask;
    bot->GetSession()->HandleGroupInviteOpcode(p);

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

        if (player->GetGroup())
            continue;

        if (botAI)
        {
            if (botAI->GetGrouperType() == GrouperType::SOLO && !botAI->HasRealPlayerMaster()) // Do not invite solo players.
                continue;

            if (botAI->HasActivePlayerMaster()) // Do not invite alts of active players.
                continue;
        }
        else
        {
            if (!sPlayerbotAIConfig->randomBotGroupNearby)
                return false;
        }

        if (abs(int32(player->getLevel() - bot->getLevel())) > 2)
            continue;

        if (sServerFacade->GetDistance2d(bot, player) > sPlayerbotAIConfig->sightDistance)
            continue;

        return Invite(player, bot);
    }

    return false;
}

bool LfgAction::Execute(Event event)
{
    Player* master = event.getOwner() ? event.getOwner() : GetMaster();
    if (bot->InBattleground())
        return false;

    if (bot->InBattlegroundQueue())
        return false;

    /* Not implemented yet.
    if (!ai->IsSafe(requester))
        return false;
    */

    if (master->GetLevel() == DEFAULT_MAX_LEVEL && bot->GetLevel() != DEFAULT_MAX_LEVEL)
        return false;

    if (master->GetLevel() > bot->GetLevel() + 4 || bot->GetLevel() > master->GetLevel() + 4)
        return false;

    std::string param = event.getParam();

    if (!param.empty() && param != "40" && param != "25" && param != "20" && param != "10" && param != "5")
    {
        botAI->TellError("Unknown group size. Valid sizes for lfg are 40, 25, 20, 10 and 5.", {});
        return false;
    }

    Group* group = master->GetGroup();

    std::unordered_map<Classes, std::unordered_map<BotRoles, uint32>> allowedClassNr;
    std::unordered_map<BotRoles, uint32> allowedRoles;

    allowedRoles[BOT_ROLE_TANK] = 1;
    allowedRoles[BOT_ROLE_HEALER] = 1;
    allowedRoles[BOT_ROLE_DPS] = 3;

    BotRoles role = botAI->IsTank(master, false) ? BOT_ROLE_TANK : (botAI->IsHeal(master, false) ? BOT_ROLE_HEALER : BOT_ROLE_DPS);
    Classes cls = (Classes)master->getClass();

    if (group)
    {
        //If no input use max raid for raid groups.
        if (param.empty() && group->isRaidGroup())
            /// Default to WotLK Raiding. Max size 25.
            param = "25";

        //Select optimal group layout.
        if (param == "40")
        {
            allowedRoles[BOT_ROLE_TANK] = 4;
            allowedRoles[BOT_ROLE_HEALER] = 16;
            allowedRoles[BOT_ROLE_DPS] = 20;

            //allowedClassNr[CLASS_WARRIOR][BOT_ROLE_TANK] = ;
            allowedClassNr[CLASS_PALADIN][BOT_ROLE_TANK] = 0;
            allowedClassNr[CLASS_DRUID][BOT_ROLE_TANK] = 1;
            //allowedClassNr[CLASS_DEATH_KNIGHT][BOT_ROLE_TANK] = ;

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
            //allowedClassNr[CLASS_DEATH_KNIGHT][BOT_ROLE_DPS] = ;
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
                return false; //Group or raid is full so stop trying.
            else
                group->ConvertToRaid(); //We want a raid but are in a group so convert and continue.
        }

        Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
        for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
        {
            // Only add group member targets that are alive and near the player
            Player* player = ObjectAccessor::FindPlayer(itr->guid);

            /* This whole function is not implemented in PlayerbotAI.h
            if (!botAI->IsSafe(player))
                return false;
            */

            role = botAI->IsTank(player, false) ? BOT_ROLE_TANK : (botAI->IsHeal(player, false) ? BOT_ROLE_HEALER : BOT_ROLE_DPS);
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

    bool invite = Invite(master, bot);

    if (invite)
    {
        if (!botAI->DoSpecificAction("accept invitation", event, true))
            return false;

        std::map<std::string, std::string> placeholders;
        placeholders["%role"] = (role == BOT_ROLE_TANK ? "tank" : (role == BOT_ROLE_HEALER ? "healer" : "dps"));
        placeholders["%spotsleft"] = std::to_string(allowedRoles[role] - 1);

        if (allowedRoles[role] > 1)
            botAI->TellMaster("Joining as %role, %spotsleft %role spots left.");
        else
            botAI->TellMaster("Joining as %role.");

        return true;
    }

    return false;
}

bool InviteNearbyToGroupAction::isUseful()
{
    if (bot->InBattleground())
        return false;

    if (bot->InBattlegroundQueue())
        return false;

    GrouperType grouperType = botAI->GetGrouperType();
    if (grouperType == GrouperType::SOLO || grouperType == GrouperType::MEMBER)
        return false;

    if (Group* group = bot->GetGroup())
    {
        if (group->IsFull())
            return false;

        if (botAI->GetGroupMaster() != bot)
            return false;

        uint32 memberCount = group->GetMembersCount();
        if (memberCount >= uint8(grouperType))
            return false;
    }

    if (botAI->HasActivePlayerMaster()) // Alts do not invite randomly
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
    for (auto& member : getGuildMembers())
    {
        Player* player = member;

        if (!player)
            continue;

        if (player->GetGroup())
            continue;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);

        if (botAI)
        {
            if (botAI->GetGrouperType() == GrouperType::SOLO && !botAI->HasRealPlayerMaster()) //Do not invite solo players.
                continue;

            if (botAI->HasActivePlayerMaster()) //Do not invite alts of active players.
                continue;

            if (player->getLevel() > bot->getLevel() + 5) // Only invite higher levels that need money so they can grind money and help out.
            {
                AiObjectContext* botContext = botAI->GetAiObjectContext();

                if (!botContext->GetValue<bool>("should get money")->Get())
                    continue;
            }
        }
        else
        {
            if (!sPlayerbotAIConfig->randomBotGroupNearby)
                return false;
        }

        if (abs(int32(player->getLevel() - bot->getLevel())) > 4)
            continue;

        if (!botAI && sServerFacade->GetDistance2d(bot, player) > sPlayerbotAIConfig->sightDistance)
            continue;

        return Invite(bot, player);
    }

    return false;
}

bool InviteGuildToGroupAction::isUseful()
{
    return bot->GetGuildId() && InviteNearbyToGroupAction::isUseful();
};
