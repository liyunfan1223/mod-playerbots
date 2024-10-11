/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "BattleGroundJoinAction.h"

#include "ArenaTeam.h"
#include "ArenaTeamMgr.h"
#include "BattlegroundMgr.h"
#include "Event.h"
#include "GroupMgr.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "PositionValue.h"
#include "UpdateTime.h"

bool BGJoinAction::Execute(Event event)
{
    uint32 queueType = AI_VALUE(uint32, "bg type");
    if (!queueType)  // force join to fill bg
    {
        if (bgList.empty())
            return false;

        BattlegroundQueueTypeId queueTypeId = (BattlegroundQueueTypeId)bgList[urand(0, bgList.size() - 1)];
        BattlegroundTypeId bgTypeId = BattlegroundMgr::BGTemplateId(queueTypeId);
        BattlegroundBracketId bracketId;
        bool isArena = false;
        bool isRated = false;

        Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
        if (!bg)
            return false;

        uint32 mapId = bg->GetMapId();
        PvPDifficultyEntry const* pvpDiff = GetBattlegroundBracketByLevel(mapId, bot->GetLevel());
        if (!pvpDiff)
            return false;

        bracketId = pvpDiff->GetBracketId();

        if (ArenaType type = ArenaType(BattlegroundMgr::BGArenaType(queueTypeId)))
        {
            isArena = true;

            std::vector<uint32>::iterator i = find(ratedList.begin(), ratedList.end(), queueTypeId);
            if (i != ratedList.end())
                isRated = true;

            if (isRated && !gatherArenaTeam(type))
                return false;

            botAI->GetAiObjectContext()->GetValue<uint32>("arena type")->Set(isRated);
        }

        // set bg type and bm guid
        // botAI->GetAiObjectContext()->GetValue<ObjectGuid>("bg master")->Set(bmGUID);
        botAI->GetAiObjectContext()->GetValue<uint32>("bg type")->Set(queueTypeId);
        queueType = queueTypeId;
    }

    return JoinQueue(queueType);
}

bool BGJoinAction::gatherArenaTeam(ArenaType type)
{
    ArenaTeam* arenateam = sArenaTeamMgr->GetArenaTeamByCaptain(bot->GetGUID(), type);

    if (!arenateam)
        return false;

    if (arenateam->GetMembersSize() < ((uint32)arenateam->GetType()))
        return false;

    GuidVector members;

    // search for arena team members and make them online
    for (ArenaTeam::MemberList::iterator itr = arenateam->GetMembers().begin(); itr != arenateam->GetMembers().end();
         ++itr)
    {
        bool offline = false;
        Player* member = ObjectAccessor::FindConnectedPlayer(itr->Guid);
        if (!member)
        {
            offline = true;
        }
        // if (!member && !sObjectMgr->GetPlayerAccountIdByGUID(itr->guid))
        //     continue;

        if (offline)
            sRandomPlayerbotMgr->AddPlayerBot(itr->Guid, 0);

        if (member)
        {
            PlayerbotAI* memberBotAI = GET_PLAYERBOT_AI(member);
            if (!memberBotAI)
                continue;

            if (member->GetGroup() && memberBotAI->HasRealPlayerMaster())
                continue;

            if (!sPlayerbotAIConfig->IsInRandomAccountList(member->GetSession()->GetAccountId()))
                continue;

            if (member->IsInCombat())
                continue;

            if (member->GetGUID() == bot->GetGUID())
                continue;

            if (member->InBattleground())
                continue;

            if (member->InBattlegroundQueue())
                continue;

            if (member->GetGroup())
                member->GetGroup()->RemoveMember(member->GetGUID());

            memberBotAI->Reset();
        }

        if (member)
            members.push_back(member->GetGUID());
    }

    if (!members.size() || (int)members.size() < (int)(arenateam->GetType() - 1))
    {
        LOG_INFO("playerbots", "Team #{} <{}> has not enough members for match", arenateam->GetId(),
                 arenateam->GetName().c_str());
        return false;
    }

    Group* group = new Group();

    // disband leaders group
    if (bot->GetGroup())
        bot->GetGroup()->Disband(true);

    if (!group->Create(bot))
    {
        LOG_INFO("playerbots", "Team #{} <{}>: Can't create group for arena queue", arenateam->GetId(),
                 arenateam->GetName());
        return false;
    }
    else
        sGroupMgr->AddGroup(group);

    LOG_INFO("playerbots", "Bot {} <{}>: Leader of <{}>", bot->GetGUID().ToString().c_str(), bot->GetName(),
             arenateam->GetName());

    for (auto i = begin(members); i != end(members); ++i)
    {
        if (*i == bot->GetGUID())
            continue;

        // if (count >= (int)arenateam->GetType())
        // break;

        if (group->GetMembersCount() >= (uint32)arenateam->GetType())
            break;

        Player* member = ObjectAccessor::FindConnectedPlayer(*i);
        if (!member)
            continue;

        if (member->GetLevel() < 70)
            continue;

        if (!group->AddMember(member))
            continue;

        PlayerbotAI* memberBotAI = GET_PLAYERBOT_AI(member);
        if (!memberBotAI)
            continue;

        memberBotAI->Reset();
        member->TeleportTo(bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), 0);

        LOG_INFO("playerbots", "Bot {} <{}>: Member of <{}>", member->GetGUID().ToString().c_str(),
                 member->GetName().c_str(), arenateam->GetName().c_str());
    }

    if (group && group->GetMembersCount() >= (uint32)arenateam->GetType())
    {
        LOG_INFO("playerbots", "Team #{} <{}> Group is ready for match", arenateam->GetId(),
                 arenateam->GetName().c_str());
        return true;
    }
    else
    {
        LOG_INFO("playerbots", "Team #{} <{}> Group is not ready for match (not enough members)", arenateam->GetId(),
                 arenateam->GetName().c_str());
        group->Disband();
    }

    return false;
}

bool BGJoinAction::canJoinBg(BattlegroundQueueTypeId queueTypeId, BattlegroundBracketId bracketId)
{
    // check if bot can join this bracket for the specific Battleground/Arena type
    BattlegroundTypeId bgTypeId = BattlegroundMgr::BGTemplateId(queueTypeId);

    // check if already in queue
    if (bot->InBattlegroundQueueForBattlegroundQueueType(queueTypeId))
        return false;

    // check too low/high level
    if (!bot->GetBGAccessByLevel(bgTypeId))
        return false;

    // check if the bracket exists for the bot's level for the specific Battleground/Arena type
    Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
    uint32 mapId = bg->GetMapId();
    PvPDifficultyEntry const* pvpDiff = GetBattlegroundBracketByLevel(mapId, bot->GetLevel());
    if (!pvpDiff)
        return false;

    BattlegroundBracketId bracket_temp = pvpDiff->GetBracketId();

    if (bracket_temp != bracketId)
        return false;

    return true;
}

bool BGJoinAction::shouldJoinBg(BattlegroundQueueTypeId queueTypeId, BattlegroundBracketId bracketId)
{
    BattlegroundTypeId bgTypeId = BattlegroundMgr::BGTemplateId(queueTypeId);
    Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
    if (!bg)
        return false;

    TeamId teamId = bot->GetTeamId();
    bool noLag = sWorldUpdateTime.GetAverageUpdateTime() < (sRandomPlayerbotMgr->GetPlayers().empty() ?
        sPlayerbotAIConfig->botActiveAloneSmartScaleDiffEmpty :
        sPlayerbotAIConfig->botActiveAloneSmartScaleDiffWithPlayer) * 1.1;

    uint32 BracketSize = bg->GetMaxPlayersPerTeam() * 2;
    uint32 TeamSize = bg->GetMaxPlayersPerTeam();

    // If performance diff is enabled, only queue if there is no lag
    if (sPlayerbotAIConfig->botActiveAloneSmartScale && !noLag)
        return false;

    // If the bot is in a group, only the leader can queue
    if (bot->GetGroup() && !bot->GetGroup()->IsLeader(bot->GetGUID()))
        return false;

    // Check if bots should join Arena
    ArenaType type = ArenaType(BattlegroundMgr::BGArenaType(queueTypeId));
    if (type != ARENA_TYPE_NONE)
    {
        BracketSize = (uint32)(type * 2);
        TeamSize = (uint32)type;

        // Check if bots should join Rated Arena (Only captains can queue)
        uint32 ratedArenaBotCount = sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].ratedArenaBotCount;
        uint32 ratedArenaPlayerCount =
            sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].ratedArenaPlayerCount;
        uint32 ratedArenaInstanceCount =
            sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].ratedArenaInstanceCount;
        uint32 activeRatedArenaQueue =
            sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].activeRatedArenaQueue;

        bool isRated = (ratedArenaBotCount + ratedArenaPlayerCount) <
                       (BracketSize * (activeRatedArenaQueue + ratedArenaInstanceCount));

        if (isRated)
        {
            if (sArenaTeamMgr->GetArenaTeamByCaptain(bot->GetGUID(), type))
            {
                sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].ratedArenaBotCount += TeamSize;
                ratedList.push_back(queueTypeId);
                return true;
            }
        }

        // Check if bots should join Skirmish Arena
        // We have extra bots queue because same faction can vs each other but can't be in the same group.
        uint32 skirmishArenaBotCount =
            sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].skirmishArenaBotCount;
        uint32 skirmishArenaPlayerCount =
            sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].skirmishArenaPlayerCount;
        uint32 skirmishArenaInstanceCount =
            sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].skirmishArenaInstanceCount;
        uint32 activeSkirmishArenaQueue =
            sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].activeSkirmishArenaQueue;
        uint32 maxRequiredSkirmishBots = BracketSize * (activeSkirmishArenaQueue + skirmishArenaInstanceCount);
        if (maxRequiredSkirmishBots != 0)
            maxRequiredSkirmishBots = maxRequiredSkirmishBots + TeamSize;

        if ((skirmishArenaBotCount + skirmishArenaPlayerCount) < maxRequiredSkirmishBots)
        {
            return true;
        }

        return false;
    }

    // Check if bots should join Battleground
    uint32 bgAllianceBotCount = sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].bgAllianceBotCount;
    uint32 bgAlliancePlayerCount = sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].bgAlliancePlayerCount;
    uint32 bgHordeBotCount = sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].bgHordeBotCount;
    uint32 bgHordePlayerCount = sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].bgHordePlayerCount;
    uint32 activeBgQueue = sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].activeBgQueue;
    uint32 bgInstanceCount = sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].bgInstanceCount;

    if (teamId == TEAM_ALLIANCE)
    {
        if ((bgAllianceBotCount + bgAlliancePlayerCount) < TeamSize * (activeBgQueue + bgInstanceCount))
            return true;
    }
    else
    {
        if ((bgHordeBotCount + bgHordePlayerCount) < TeamSize * (activeBgQueue + bgInstanceCount))
            return true;
    }

    return false;
}

bool BGJoinAction::isUseful()
{
    // do not try if BG bots disabled
    if (!sPlayerbotAIConfig->randomBotJoinBG)
        return false;

    // can't queue while in BG/Arena
    if (bot->InBattleground())
        return false;

    // can't queue while in BG/Arena queue
    if (bot->InBattlegroundQueue())
        return false;

    // do not try right after login (currently not working)
    if ((time(nullptr) - bot->GetInGameTime()) < 120)
        return false;

    // check level
    if (bot->GetLevel() < 10)
        return false;

    // do not try if with player master
    if (GET_PLAYERBOT_AI(bot)->HasActivePlayerMaster())
        return false;

    // do not try if in group, if in group only leader can queue
    if (bot->GetGroup() && !bot->GetGroup()->IsLeader(bot->GetGUID()))
        return false;

    // do not try if in combat
    if (bot->IsInCombat())
        return false;

    // check Deserter debuff
    if (!bot->CanJoinToBattleground())
        return false;

    // check if has free queue slots (pointless as already making sure not in queue)
    // keeping just in case.
    if (!bot->HasFreeBattlegroundQueueId())
        return false;

    // do not try if in dungeon
    // Map* map = bot->GetMap();
    // if (map && map->Instanceable())
    //     return false;

    bgList.clear();
    ratedList.clear();

    for (int bracket = BG_BRACKET_ID_FIRST; bracket < MAX_BATTLEGROUND_BRACKETS; ++bracket)
    {
        for (int queueType = BATTLEGROUND_QUEUE_AV; queueType < MAX_BATTLEGROUND_QUEUE_TYPES; ++queueType)
        {
            BattlegroundQueueTypeId queueTypeId = BattlegroundQueueTypeId(queueType);
            BattlegroundBracketId bracketId = BattlegroundBracketId(bracket);

            if (!canJoinBg(queueTypeId, bracketId))
                continue;

            if (shouldJoinBg(queueTypeId, bracketId))
                bgList.push_back(queueTypeId);
        }
    }

    if (!bgList.empty())
        return true;

    return false;
}

bool BGJoinAction::JoinQueue(uint32 type)
{
    // ignore if player is already in BG, is logging out, or already being teleport
    if (!bot || (!bot->IsInWorld() && !bot->IsBeingTeleported()) || bot->InBattleground())
        return false;

    // get BG TypeId
    BattlegroundQueueTypeId queueTypeId = BattlegroundQueueTypeId(type);
    BattlegroundTypeId bgTypeId = BattlegroundMgr::BGTemplateId(queueTypeId);
    BattlegroundBracketId bracketId;

    Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
    if (!bg)
        return false;

    uint32 mapId = bg->GetMapId();
    PvPDifficultyEntry const* pvpDiff = GetBattlegroundBracketByLevel(mapId, bot->GetLevel());
    if (!pvpDiff)
        return false;

    bracketId = pvpDiff->GetBracketId();

    uint32 BracketSize = bg->GetMaxPlayersPerTeam() * 2;
    uint32 TeamSize = bg->GetMaxPlayersPerTeam();
    TeamId teamId = bot->GetTeamId();

    // check if already in queue
    if (bot->InBattlegroundQueueForBattlegroundQueueType(queueTypeId))
        return false;

    // check bg req level
    if (!bot->GetBGAccessByLevel(bgTypeId))
        return false;

    // get BG MapId
    uint32 bgTypeId_ = bgTypeId;
    uint32 instanceId = 0;  // 0 = First Available

    bool isPremade = false;
    bool isArena = false;
    bool isRated = false;
    uint8 arenaslot = 0;
    uint8 asGroup = false;

    std::string _bgType;

    // check if arena
    ArenaType arenaType = ArenaType(BattlegroundMgr::BGArenaType(queueTypeId));
    if (arenaType != ARENA_TYPE_NONE)
        isArena = true;

    // get battlemaster
    // Unit* unit = botAI->GetUnit(AI_VALUE2(CreatureData const*, "bg master", bgTypeId));
    Unit* unit = botAI->GetUnit(sRandomPlayerbotMgr->GetBattleMasterGUID(bot, bgTypeId));
    if (!unit && isArena)
    {
        botAI->GetAiObjectContext()->GetValue<uint32>("bg type")->Set(0);
        LOG_DEBUG("playerbots", "Bot {} could not find Battlemaster to join", bot->GetGUID().ToString().c_str());
        return false;
    }

    // This breaks groups as refresh includes a remove from group function call.
    // refresh food/regs
    // sRandomPlayerbotMgr->Refresh(bot);

    bool joinAsGroup = bot->GetGroup() && bot->GetGroup()->GetLeaderGUID() == bot->GetGUID();

    // in wotlk only arena requires battlemaster guid
    ObjectGuid guid = isArena ? unit->GetGUID() : bot->GetGUID();

    switch (bgTypeId)
    {
        case BATTLEGROUND_AV:
            _bgType = "AV";
            break;
        case BATTLEGROUND_WS:
            _bgType = "WSG";
            break;
        case BATTLEGROUND_AB:
            _bgType = "AB";
            break;
        case BATTLEGROUND_EY:
            _bgType = "EotS";
            break;
        case BATTLEGROUND_RB:
            _bgType = "Random";
            break;
        case BATTLEGROUND_SA:
            _bgType = "SotA";
            break;
        case BATTLEGROUND_IC:
            _bgType = "IoC";
            break;
        default:
            break;
    }

    if (isArena)
    {
        isArena = true;
        BracketSize = type * 2;
        TeamSize = type;
        isRated = botAI->GetAiObjectContext()->GetValue<uint32>("arena type")->Get();

        if (joinAsGroup)
            asGroup = true;

        switch (arenaType)
        {
            case ARENA_TYPE_2v2:
                arenaslot = 0;
                _bgType = "2v2";
                break;
            case ARENA_TYPE_3v3:
                arenaslot = 1;
                _bgType = "3v3";
                break;
            case ARENA_TYPE_5v5:
                arenaslot = 2;
                _bgType = "5v5";
                break;
            default:
                break;
        }
    }

    LOG_INFO("playerbots", "Bot {} {}:{} <{}> queued {} {}", bot->GetGUID().ToString().c_str(),
             bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName().c_str(), _bgType.c_str(),
             isRated   ? "Rated Arena"
             : isArena ? "Arena"
                       : "");

    if (isArena)
    {
        if (!isRated)
        {
            sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].skirmishArenaBotCount++;
        }
    }
    else if (!joinAsGroup)
    {
        if (teamId == TEAM_ALLIANCE)
            sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].bgAllianceBotCount++;
        else
            sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].bgHordeBotCount++;
    }
    else
    {
        if (teamId == TEAM_ALLIANCE)
            sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].bgAllianceBotCount +=
                bot->GetGroup()->GetMembersCount();
        else
            sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].bgHordeBotCount +=
                bot->GetGroup()->GetMembersCount();
    }

    botAI->GetAiObjectContext()->GetValue<uint32>("bg type")->Set(0);

    if (!isArena)
    {
        WorldPacket packet(CMSG_BATTLEMASTER_JOIN, 20);
        packet << bot->GetGUID() << bgTypeId_ << instanceId << joinAsGroup;
        bot->GetSession()->HandleBattlemasterJoinOpcode(packet);
    }
    else
    {
        WorldPacket arena_packet(CMSG_BATTLEMASTER_JOIN_ARENA, 20);
        arena_packet << unit->GetGUID() << arenaslot << asGroup << uint8(isRated);
        bot->GetSession()->HandleBattlemasterJoinArena(arena_packet);
    }

    return true;
}

// Not sure if this has ever worked, but it should be similar to BGJoinAction::shouldJoinBg
bool FreeBGJoinAction::shouldJoinBg(BattlegroundQueueTypeId queueTypeId, BattlegroundBracketId bracketId)
{
    BattlegroundTypeId bgTypeId = BattlegroundMgr::BGTemplateId(queueTypeId);
    Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
    if (!bg)
        return false;

    TeamId teamId = bot->GetTeamId();
    bool noLag = sWorldUpdateTime.GetAverageUpdateTime() < (sRandomPlayerbotMgr->GetPlayers().empty() ?
        sPlayerbotAIConfig->botActiveAloneSmartScaleDiffEmpty :
        sPlayerbotAIConfig->botActiveAloneSmartScaleDiffWithPlayer) * 1.1;

    uint32 BracketSize = bg->GetMaxPlayersPerTeam() * 2;
    uint32 TeamSize = bg->GetMaxPlayersPerTeam();

    // If performance diff is enabled, only queue if there is no lag
    if (sPlayerbotAIConfig->botActiveAloneSmartScale && !noLag)
        return false;

    // If the bot is in a group, only the leader can queue
    if (bot->GetGroup() && !bot->GetGroup()->IsLeader(bot->GetGUID()))
        return false;

    // Check if bots should join Arena
    ArenaType type = ArenaType(BattlegroundMgr::BGArenaType(queueTypeId));
    if (type != ARENA_TYPE_NONE)
    {
        BracketSize = (uint32)(type * 2);
        TeamSize = (uint32)type;

        // Check if bots should join Rated Arena (Only captains can queue)
        uint32 ratedArenaBotCount = sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].ratedArenaBotCount;
        uint32 ratedArenaPlayerCount =
            sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].ratedArenaPlayerCount;
        uint32 ratedArenaInstanceCount =
            sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].ratedArenaInstanceCount;
        uint32 activeRatedArenaQueue =
            sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].activeRatedArenaQueue;

        bool isRated = (ratedArenaBotCount + ratedArenaPlayerCount) <
                       (BracketSize * (activeRatedArenaQueue + ratedArenaInstanceCount));

        if (isRated)
        {
            if (sArenaTeamMgr->GetArenaTeamByCaptain(bot->GetGUID(), type))
            {
                sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].ratedArenaBotCount += TeamSize;
                ratedList.push_back(queueTypeId);
                return true;
            }
        }

        // Check if bots should join Skirmish Arena
        // We have extra bots queue because same faction can vs each other but can't be in the same group.
        uint32 skirmishArenaBotCount =
            sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].skirmishArenaBotCount;
        uint32 skirmishArenaPlayerCount =
            sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].skirmishArenaPlayerCount;
        uint32 skirmishArenaInstanceCount =
            sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].skirmishArenaInstanceCount;
        uint32 activeSkirmishArenaQueue =
            sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].activeSkirmishArenaQueue;
        uint32 maxRequiredSkirmishBots = BracketSize * (activeSkirmishArenaQueue + skirmishArenaInstanceCount);
        if (maxRequiredSkirmishBots != 0)
            maxRequiredSkirmishBots = maxRequiredSkirmishBots + TeamSize;

        if ((skirmishArenaBotCount + skirmishArenaPlayerCount) < maxRequiredSkirmishBots)
        {
            return true;
        }

        return false;
    }

    // Check if bots should join Battleground
    uint32 bgAllianceBotCount = sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].bgAllianceBotCount;
    uint32 bgAlliancePlayerCount = sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].bgAlliancePlayerCount;
    uint32 bgHordeBotCount = sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].bgHordeBotCount;
    uint32 bgHordePlayerCount = sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].bgHordePlayerCount;
    uint32 activeBgQueue = sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].activeBgQueue;
    uint32 bgInstanceCount = sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].bgInstanceCount;

    if (teamId == TEAM_ALLIANCE)
    {
        if ((bgAllianceBotCount + bgAlliancePlayerCount) < TeamSize * (activeBgQueue + bgInstanceCount))
            return true;
    }
    else
    {
        if ((bgHordeBotCount + bgHordePlayerCount) < TeamSize * (activeBgQueue + bgInstanceCount))
            return true;
    }

    return false;
}

bool BGLeaveAction::Execute(Event event)
{
    if (!(bot->InBattlegroundQueue() || bot->InBattleground()))
        return false;

    // botAI->ChangeStrategy("-bg", BOT_STATE_NON_COMBAT);

    if (BGStatusAction::LeaveBG(botAI))
        return true;

    // leave queue if not in BG
    BattlegroundQueueTypeId queueTypeId = bot->GetBattlegroundQueueTypeId(0);
    BattlegroundTypeId _bgTypeId = BattlegroundMgr::BGTemplateId(queueTypeId);
    uint8 type = false;
    uint16 unk = 0x1F90;
    uint8 unk2 = 0x0;
    bool isArena = false;
    bool IsRandomBot = sRandomPlayerbotMgr->IsRandomBot(bot);

    ArenaType arenaType = ArenaType(BattlegroundMgr::BGArenaType(queueTypeId));
    if (arenaType)
    {
        isArena = true;
        type = arenaType;
    }

    uint32 queueType = AI_VALUE(uint32, "bg type");
    if (!queueType)
        return false;

    LOG_INFO("playerbots", "Bot {} {}:{} <{}> leaves {} queue", bot->GetGUID().ToString().c_str(),
             bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName().c_str(),
             isArena ? "Arena" : "BG");

    WorldPacket packet(CMSG_BATTLEFIELD_PORT, 20);
    packet << type << unk2 << (uint32)_bgTypeId << unk << uint8(0);
    bot->GetSession()->HandleBattleFieldPortOpcode(packet);

    if (IsRandomBot)
        botAI->SetMaster(nullptr);

    botAI->ResetStrategies(!IsRandomBot);
    botAI->GetAiObjectContext()->GetValue<uint32>("bg type")->Set(0);
    botAI->GetAiObjectContext()->GetValue<uint32>("bg role")->Set(0);
    botAI->GetAiObjectContext()->GetValue<uint32>("arena type")->Set(0);

    return true;
}

bool BGStatusAction::LeaveBG(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    Battleground* bg = bot->GetBattleground();
    if (!bg)
        return false;
    bool isArena = bg->isArena();
    bool isRandomBot = sRandomPlayerbotMgr->IsRandomBot(bot);

    if (isRandomBot)
        botAI->SetMaster(nullptr);

    botAI->ChangeStrategy("-warsong", BOT_STATE_COMBAT);
    botAI->ChangeStrategy("-warsong", BOT_STATE_NON_COMBAT);
    botAI->ChangeStrategy("-arathi", BOT_STATE_COMBAT);
    botAI->ChangeStrategy("-arathi", BOT_STATE_NON_COMBAT);
    botAI->ChangeStrategy("-eye", BOT_STATE_COMBAT);
    botAI->ChangeStrategy("-eye", BOT_STATE_NON_COMBAT);
    botAI->ChangeStrategy("-isle", BOT_STATE_COMBAT);
    botAI->ChangeStrategy("-isle", BOT_STATE_NON_COMBAT);
    botAI->ChangeStrategy("-Battleground", BOT_STATE_COMBAT);
    botAI->ChangeStrategy("-Battleground", BOT_STATE_NON_COMBAT);
    botAI->ChangeStrategy("-arena", BOT_STATE_COMBAT);
    botAI->ChangeStrategy("-arena", BOT_STATE_NON_COMBAT);

    LOG_INFO("playerbots", "Bot {} {}:{} <{}> leaves {}", bot->GetGUID().ToString().c_str(),
             bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName(),
             isArena ? "Arena" : "BG");

    WorldPacket packet(CMSG_LEAVE_BATTLEFIELD);
    packet << uint8(0);
    packet << uint8(0);  // BattlegroundTypeId-1 ?
    packet << uint32(0);
    packet << uint16(0);

    bot->GetSession()->HandleBattlefieldLeaveOpcode(packet);

    botAI->ResetStrategies(!isRandomBot);
    botAI->GetAiObjectContext()->GetValue<uint32>("bg type")->Set(0);
    botAI->GetAiObjectContext()->GetValue<uint32>("bg role")->Set(0);
    botAI->GetAiObjectContext()->GetValue<uint32>("arena type")->Set(0);
    PositionMap& posMap = botAI->GetAiObjectContext()->GetValue<PositionMap&>("position")->Get();
    PositionInfo pos = botAI->GetAiObjectContext()->GetValue<PositionMap&>("position")->Get()["bg objective"];
    pos.Reset();
    posMap["bg objective"] = pos;
    return true;
}

bool BGStatusAction::isUseful() { return bot->InBattlegroundQueue(); }

bool BGStatusAction::Execute(Event event)
{
    uint32 QueueSlot;
    uint32 instanceId;
    uint32 mapId;
    uint32 statusid;
    uint32 Time1;
    uint32 Time2;
    std::string _bgType;

    uint64 arenaByte;
    uint8 arenaTeam;
    uint8 isRated;
    uint64 unk0;
    uint8 minlevel;
    uint8 maxlevel;

    WorldPacket p(event.getPacket());
    statusid = 0;
    p >> QueueSlot;  // queue id (0...2) - player can be in 3 queues in time
    p >> arenaByte;
    if (arenaByte == 0)
        return false;
    p >> minlevel;
    p >> maxlevel;
    p >> instanceId;
    p >> isRated;
    p >> statusid;

    // check status
    switch (statusid)
    {
        case STATUS_WAIT_QUEUE:  // status_in_queue
            p >> Time1;          // average wait time, milliseconds
            p >> Time2;          // time in queue, updated every minute!, milliseconds
            break;
        case STATUS_WAIT_JOIN:  // status_invite
            p >> mapId;         // map id
            p >> unk0;
            p >> Time1;  // time to remove from queue, milliseconds
            break;
        case STATUS_IN_PROGRESS:  // status_in_progress
            p >> mapId;           // map id
            p >> unk0;
            p >> Time1;  // time to bg auto leave, 0 at bg start, 120000 after bg end, milliseconds
            p >> Time2;  // time from bg start, milliseconds
            p >> arenaTeam;
            break;
        default:
            LOG_ERROR("playerbots", "Unknown BG status!");
            break;
    }

    bool IsRandomBot = sRandomPlayerbotMgr->IsRandomBot(bot);
    BattlegroundQueueTypeId queueTypeId = bot->GetBattlegroundQueueTypeId(QueueSlot);
    BattlegroundTypeId _bgTypeId = BattlegroundMgr::BGTemplateId(queueTypeId);
    if (!queueTypeId)
        return false;

    BattlegroundBracketId bracketId;
    Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(_bgTypeId);
    mapId = bg->GetMapId();
    PvPDifficultyEntry const* pvpDiff = GetBattlegroundBracketByLevel(mapId, bot->GetLevel());
    if (pvpDiff)
        bracketId = pvpDiff->GetBracketId();

    bool isArena = false;
    uint8 type = false;  // arenatype if arena
    uint16 unk = 0x1F90;
    uint8 unk2 = 0x0;
    uint8 action = 0x1;

    ArenaType arenaType = ArenaType(BattlegroundMgr::BGArenaType(queueTypeId));
    if (arenaType)
    {
        isArena = true;
        type = arenaType;
    }

    switch (_bgTypeId)
    {
        case BATTLEGROUND_AV:
            _bgType = "AV";
            break;
        case BATTLEGROUND_WS:
            _bgType = "WSG";
            break;
        case BATTLEGROUND_AB:
            _bgType = "AB";
            break;
        case BATTLEGROUND_EY:
            _bgType = "EotS";
            break;
        case BATTLEGROUND_RB:
            _bgType = "Random";
            break;
        case BATTLEGROUND_SA:
            _bgType = "SotA";
            break;
        case BATTLEGROUND_IC:
            _bgType = "IoC";
            break;
        default:
            break;
    }

    switch (arenaType)
    {
        case ARENA_TYPE_2v2:
            _bgType = "2v2";
            break;
        case ARENA_TYPE_3v3:
            _bgType = "3v3";
            break;
        case ARENA_TYPE_5v5:
            _bgType = "5v5";
            break;
        default:
            break;
    }

    TeamId teamId = bot->GetTeamId();

    if (Time1 == TIME_TO_AUTOREMOVE)  // Battleground is over, bot needs to leave
    {
        LOG_INFO("playerbots", "Bot {} <{}> ({} {}): Received BG status TIME_TO_AUTOREMOVE for {} {}",
                 bot->GetGUID().ToString().c_str(), bot->GetName(), bot->GetLevel(),
                 bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", isArena ? "Arena" : "BG", _bgType);

        if (LeaveBG(botAI))
            return true;
    }

    if (statusid == STATUS_WAIT_QUEUE)  // bot is in queue
    {
        LOG_INFO("playerbots", "Bot {} {}:{} <{}>: Received BG status WAIT_QUEUE (wait time: {}) for {} {}",
                 bot->GetGUID().ToString().c_str(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->GetLevel(),
                 bot->GetName(), Time2, isArena ? "Arena" : "BG", _bgType);
        // temp fix for crash
        // return true;

        BattlegroundQueue& bgQueue = sBattlegroundMgr->GetBattlegroundQueue(queueTypeId);
        GroupQueueInfo ginfo;
        if (bgQueue.GetPlayerGroupInfoData(bot->GetGUID(), &ginfo))
        {
            if (ginfo.IsInvitedToBGInstanceGUID && !bot->InBattleground())
            {
                // BattlegroundMgr::GetBattleground() does not return battleground if bgTypeId==BATTLEGROUND_AA
                Battleground* bg = sBattlegroundMgr->GetBattleground(
                    ginfo.IsInvitedToBGInstanceGUID, _bgTypeId == BATTLEGROUND_AA ? BATTLEGROUND_TYPE_NONE : _bgTypeId);
                if (bg)
                {
                    if (isArena)
                    {
                        _bgTypeId = bg->GetBgTypeID();
                    }

                    LOG_INFO("playerbots", "Bot {} {}:{} <{}>: Force join {} {}", bot->GetGUID().ToString().c_str(),
                             bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName(),
                             isArena ? "Arena" : "BG", _bgType);
                    WorldPacket emptyPacket;
                    bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);
                    action = 0x1;

                    WorldPacket packet(CMSG_BATTLEFIELD_PORT, 20);
                    packet << type << unk2 << (uint32)_bgTypeId << unk << action;
                    bot->GetSession()->HandleBattleFieldPortOpcode(packet);

                    botAI->ResetStrategies(false);
                    if (!bot->GetBattleground())
                    {
                        // first bot to join wont have battleground and PlayerbotAI::ResetStrategies() wont set them up
                        // properly, set bg for "bg strategy check" to fix that
                        botAI->ChangeStrategy("+bg", BOT_STATE_NON_COMBAT);
                    }
                    context->GetValue<uint32>("bg role")->Set(urand(0, 9));
                    PositionMap& posMap = context->GetValue<PositionMap&>("position")->Get();
                    PositionInfo pos = context->GetValue<PositionMap&>("position")->Get()["bg objective"];
                    pos.Reset();
                    posMap["bg objective"] = pos;

                    return true;
                }
            }
        }

        Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(_bgTypeId);
        if (!bg)
            return false;

        bool leaveQ = false;
        uint32 timer;
        if (isArena)
            timer = TIME_TO_AUTOREMOVE;
        else
            timer = TIME_TO_AUTOREMOVE + 1000 * (bg->GetMaxPlayersPerTeam() * 8);

        if (Time2 > timer && isArena)  // disabled for BG
            leaveQ = true;

        if (leaveQ && ((bot->GetGroup() && bot->GetGroup()->IsLeader(bot->GetGUID())) ||
                       !(bot->GetGroup() || botAI->GetMaster())))
        {
            TeamId teamId = bot->GetTeamId();
            bool realPlayers = false;
            if (isRated)
                realPlayers = sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].ratedArenaPlayerCount > 0;
            else
                realPlayers =
                    sRandomPlayerbotMgr->BattlegroundData[queueTypeId][bracketId].skirmishArenaPlayerCount > 0;

            if (realPlayers)
                return false;

            LOG_INFO("playerbots", "Bot {} {}:{} <{}> waited too long and leaves queue ({} {}).",
                     bot->GetGUID().ToString().c_str(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->GetLevel(),
                     bot->GetName(), isArena ? "Arena" : "BG", _bgType);

            WorldPacket packet(CMSG_BATTLEFIELD_PORT, 20);
            action = 0;
            packet << type << unk2 << (uint32)_bgTypeId << unk << action;
            bot->GetSession()->HandleBattleFieldPortOpcode(packet);

            botAI->ResetStrategies(!IsRandomBot);
            botAI->GetAiObjectContext()->GetValue<uint32>("bg type")->Set(0);
            botAI->GetAiObjectContext()->GetValue<uint32>("bg role")->Set(0);
            botAI->GetAiObjectContext()->GetValue<uint32>("arena type")->Set(0);

            return true;
        }
    }

    if (statusid == STATUS_IN_PROGRESS)  // placeholder for Leave BG if it takes too long
    {
        LOG_INFO("playerbots", "Bot {} {}:{} <{}>: Received BG status IN_PROGRESS for {} {}",
                 bot->GetGUID().ToString().c_str(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->GetLevel(),
                 bot->GetName(), isArena ? "Arena" : "BG", _bgType);
        return false;
    }

    if (statusid == STATUS_WAIT_JOIN)  // bot may join
    {
        LOG_INFO("playerbots", "Bot {} {}:{} <{}>: Received BG status WAIT_JOIN for {} {}",
                 bot->GetGUID().ToString().c_str(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->GetLevel(),
                 bot->GetName(), isArena ? "Arena" : "BG", _bgType);

        if (isArena)
        {
            isArena = true;
            BattlegroundQueue& bgQueue = sBattlegroundMgr->GetBattlegroundQueue(queueTypeId);

            GroupQueueInfo ginfo;
            if (!bgQueue.GetPlayerGroupInfoData(bot->GetGUID(), &ginfo))
            {
                LOG_ERROR("playerbots", "Bot {} {}:{} <{}>: Missing QueueInfo for {} {}",
                          bot->GetGUID().ToString().c_str(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H",
                          bot->GetLevel(), bot->GetName(), isArena ? "Arena" : "BG", _bgType);
                return false;
            }

            if (ginfo.IsInvitedToBGInstanceGUID)
            {
                // BattlegroundMgr::GetBattleground() does not return battleground if bgTypeId==BATTLEGROUND_AA
                Battleground* bg = sBattlegroundMgr->GetBattleground(
                    ginfo.IsInvitedToBGInstanceGUID, _bgTypeId == BATTLEGROUND_AA ? BATTLEGROUND_TYPE_NONE : _bgTypeId);
                if (!bg)
                {
                    LOG_ERROR("playerbots", "Bot {} {}:{} <{}>: Missing QueueInfo for {} {}",
                              bot->GetGUID().ToString().c_str(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H",
                              bot->GetLevel(), bot->GetName(), isArena ? "Arena" : "BG", _bgType);
                    return false;
                }

                _bgTypeId = bg->GetBgTypeID();
            }
        }

        LOG_INFO("playerbots", "Bot {} {}:{} <{}> joined {} - {}", bot->GetGUID().ToString().c_str(),
                 bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName(),
                 isArena ? "Arena" : "BG", _bgType);

        WorldPacket emptyPacket;
        bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);

        action = 0x1;

        WorldPacket packet(CMSG_BATTLEFIELD_PORT, 20);
        packet << type << unk2 << (uint32)_bgTypeId << unk << action;
        bot->GetSession()->HandleBattleFieldPortOpcode(packet);

        botAI->ResetStrategies(false);
        if (!bot->GetBattleground())
        {
            // first bot to join wont have battleground and PlayerbotAI::ResetStrategies() wont set them up properly,
            // set bg for "bg strategy check" to fix that
            botAI->ChangeStrategy("+bg", BOT_STATE_NON_COMBAT);
        }
        context->GetValue<uint32>("bg role")->Set(urand(0, 9));
        PositionMap& posMap = context->GetValue<PositionMap&>("position")->Get();
        PositionInfo pos = context->GetValue<PositionMap&>("position")->Get()["bg objective"];
        PositionInfo pos2 = context->GetValue<PositionMap&>("position")->Get()["bg siege"];
        pos.Reset();
        pos2.Reset();
        posMap["bg objective"] = pos;
        posMap["bg siege"] = pos2;

        return true;
    }

    return true;
}

bool BGStatusCheckAction::Execute(Event event)
{
    if (bot->IsBeingTeleported())
        return false;

    WorldPacket packet(CMSG_BATTLEFIELD_STATUS);
    bot->GetSession()->HandleBattlefieldStatusOpcode(packet);

    LOG_INFO("playerbots", "Bot {} <{}> ({} {}) : Checking BG invite status", bot->GetGUID().ToString().c_str(),
             bot->GetName(), bot->GetLevel(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H");

    return true;
}

bool BGStatusCheckAction::isUseful() { return bot->InBattlegroundQueue(); }

bool BGStrategyCheckAction::Execute(Event event)
{
    bool inside_bg = bot->InBattleground() && bot->GetBattleground();
    ;
    if (!inside_bg && botAI->HasStrategy("battleground", BOT_STATE_NON_COMBAT))
    {
        botAI->ResetStrategies();
        return true;
    }
    if (inside_bg && !botAI->HasStrategy("battleground", BOT_STATE_NON_COMBAT))
    {
        botAI->ResetStrategies();
        return false;
    }
    return false;
}
