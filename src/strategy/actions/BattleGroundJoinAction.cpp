/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
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

bool BGJoinAction::Execute(Event event)
{
    uint32 queueType = AI_VALUE(uint32, "bg type");
    if (!queueType) // force join to fill bg
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
        PvPDifficultyEntry const* pvpDiff = GetBattlegroundBracketByLevel(mapId, bot->getLevel());
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
        //botAI->GetAiObjectContext()->GetValue<ObjectGuid>("bg master")->Set(bmGUID);
        botAI->GetAiObjectContext()->GetValue<uint32>("bg type")->Set(queueTypeId);
        queueType = queueTypeId;
    }

    return JoinQueue(queueType);
}

bool BGJoinAction::gatherArenaTeam(ArenaType type)
{
    ArenaTeam* arenateam = nullptr;
    for (uint32 arena_slot = 0; arena_slot < MAX_ARENA_SLOT; ++arena_slot)
    {
        ArenaTeam* temp = sArenaTeamMgr->GetArenaTeamById(bot->GetArenaTeamId(arena_slot));
        if (!temp)
            continue;

        if (temp->GetCaptain() != bot->GetGUID())
            continue;

        if (temp->GetType() != type)
            continue;

        if (temp->GetMembersSize() < ((uint32) temp->GetType()))
            continue;

        arenateam = temp;
    }

    if (!arenateam)
        return false;

    GuidVector members;

    // search for arena team members and make them online
    for (ArenaTeam::MemberList::iterator itr = arenateam->GetMembers().begin(); itr != arenateam->GetMembers().end(); ++itr)
    {
        bool offline = false;
        Player* member = ObjectAccessor::FindConnectedPlayer(itr->Guid);
        if (!member)
        {
            offline = true;
        }
        //if (!member && !sObjectMgr->GetPlayerAccountIdByGUID(itr->guid))
        //    continue;

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

            if (member->GetGroup())
                member->GetGroup()->RemoveMember(member->GetGUID());

            memberBotAI->Reset();
        }

        if (member)
            members.push_back(member->GetGUID());
    }

    if (!members.size() || (int)members.size() < (int)(arenateam->GetType() - 1))
    {
        LOG_INFO("playerbots", "Team #{} <{}> has not enough members for match", arenateam->GetId(), arenateam->GetName().c_str());
        return false;
    }

    Group* group = new Group();
    uint32 count = 1;

    // disband leaders group
    if (bot->GetGroup())
        bot->GetGroup()->Disband(true);

    if (!group->Create(bot))
    {
        LOG_INFO("playerbots", "Team #{} <{}>: Can't create group for arena queue", arenateam->GetId(), arenateam->GetName());
        return false;
    }
    else
        sGroupMgr->AddGroup(group);

    LOG_INFO("playerbots", "Bot {} <{}>: Leader of <{}>", bot->GetGUID().ToString().c_str(), bot->GetName(), arenateam->GetName());


    for (auto i = begin(members); i != end(members); ++i)
    {
        if (*i == bot->GetGUID())
            continue;

        //if (count >= (int)arenateam->GetType())
            //break;

        if (group->GetMembersCount() >= (uint32) arenateam->GetType())
            break;

        Player* member = ObjectAccessor::FindConnectedPlayer(*i);
        if (!member)
            continue;

        if (member->getLevel() < 70)
            continue;

        if (!group->AddMember(member))
            continue;

        PlayerbotAI* memberBotAI = GET_PLAYERBOT_AI(member);
        if (!memberBotAI)
            continue;

        memberBotAI->Reset();
        member->TeleportTo(bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), 0);


        LOG_INFO("playerbots", "Bot {} <{}>: Member of <{}>", member->GetGUID().ToString().c_str(), member->GetName().c_str(), arenateam->GetName().c_str());

        count++;
    }

    if (group && group->GetMembersCount() >= (uint32)arenateam->GetType())
    {
        LOG_INFO("playerbots", "Team #{} <{}> Group is ready for match", arenateam->GetId(), arenateam->GetName().c_str());
        return true;
    }
    else
    {
        LOG_INFO("playerbots", "Team #{} <{}> Group is not ready for match (not enough members)", arenateam->GetId(), arenateam->GetName().c_str());
        group->Disband();
    }

    return false;
}

bool BGJoinAction::canJoinBg(BattlegroundQueueTypeId queueTypeId, BattlegroundBracketId bracketId)
{
    // check if bot can join this bg/bracket
    BattlegroundTypeId bgTypeId = BattlegroundMgr::BGTemplateId(queueTypeId);

    // check if already in queue
    if (bot->InBattlegroundQueueForBattlegroundQueueType(queueTypeId))
        return false;

    // check too low/high level
    if (!bot->GetBGAccessByLevel(bgTypeId))
        return false;

    // check bracket
    Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
    uint32 mapId = bg->GetMapId();
    PvPDifficultyEntry const* pvpDiff = GetBattlegroundBracketByLevel(mapId, bot->getLevel());
    if (!pvpDiff)
        return false;

    BattlegroundBracketId bracket_temp = pvpDiff->GetBracketId();

    if (bracket_temp != bracketId)
        return false;

    return true;
}

bool BGJoinAction::shouldJoinBg(BattlegroundQueueTypeId queueTypeId, BattlegroundBracketId bracketId)
{
    // check if bot should join (queue has real players)
    BattlegroundTypeId bgTypeId = BattlegroundMgr::BGTemplateId(queueTypeId);
    Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
    if (!bg)
        return false;

    bool isArena = false;
    bool isRated = false;

    ArenaType type = ArenaType(BattlegroundMgr::BGArenaType(queueTypeId));
    if (type != ARENA_TYPE_NONE)
        isArena = true;

    bool hasPlayers = (sRandomPlayerbotMgr->BgPlayers[queueTypeId][bracketId][TEAM_ALLIANCE] + sRandomPlayerbotMgr->BgPlayers[queueTypeId][bracketId][TEAM_HORDE]) > 0;
    bool hasBots = (sRandomPlayerbotMgr->BgBots[queueTypeId][bracketId][TEAM_ALLIANCE] + sRandomPlayerbotMgr->BgBots[queueTypeId][bracketId][TEAM_HORDE]) >= bg->GetMinPlayersPerTeam();
    if (!sPlayerbotAIConfig->randomBotAutoJoinBG && !hasPlayers)
        return false;

    if (!(hasPlayers || hasBots))
        return false;

    uint32 BracketSize = bg->GetMaxPlayersPerTeam() * 2;
    uint32 TeamSize = bg->GetMaxPlayersPerTeam();

    uint32 ACount = sRandomPlayerbotMgr->BgBots[queueTypeId][bracketId][TEAM_ALLIANCE] + sRandomPlayerbotMgr->BgPlayers[queueTypeId][bracketId][TEAM_ALLIANCE];
    uint32 HCount = sRandomPlayerbotMgr->BgBots[queueTypeId][bracketId][TEAM_HORDE] + sRandomPlayerbotMgr->BgPlayers[queueTypeId][bracketId][TEAM_HORDE];

    uint32 BgCount = ACount + HCount;
    uint32 SCount = 0;
    uint32 RCount = 0;

    TeamId teamId = bot->GetTeamId();

    if (isArena)
    {
        uint32 rated_players = sRandomPlayerbotMgr->BgPlayers[queueTypeId][bracketId][TEAM_HORDE];
        if (rated_players)
        {
            isRated = true;
        }

        isArena = true;
        BracketSize = (uint32)(type * 2);
        TeamSize = type;
        ACount = sRandomPlayerbotMgr->ArenaBots[queueTypeId][bracketId][isRated ? TEAM_HORDE : TEAM_ALLIANCE][TEAM_ALLIANCE];
        HCount = sRandomPlayerbotMgr->ArenaBots[queueTypeId][bracketId][isRated ? TEAM_HORDE : TEAM_ALLIANCE][TEAM_HORDE];
        BgCount = sRandomPlayerbotMgr->BgBots[queueTypeId][bracketId][isRated ? TEAM_HORDE : TEAM_ALLIANCE] + sRandomPlayerbotMgr->BgPlayers[queueTypeId][bracketId][isRated ? TEAM_HORDE : TEAM_ALLIANCE];
        SCount = sRandomPlayerbotMgr->BgBots[queueTypeId][bracketId][TEAM_ALLIANCE] + sRandomPlayerbotMgr->BgPlayers[queueTypeId][bracketId][TEAM_ALLIANCE];
        RCount = sRandomPlayerbotMgr->BgBots[queueTypeId][bracketId][TEAM_HORDE] + sRandomPlayerbotMgr->BgPlayers[queueTypeId][bracketId][TEAM_HORDE];
    }

    // do not try if not a captain of arena team
    if (isRated)
    {
        if (!sArenaTeamMgr->GetArenaTeamByCaptain(bot->GetGUID()))
            return false;

        // check if bot has correct team
        ArenaTeam* arenateam = nullptr;
        for (uint32 arena_slot = 0; arena_slot < MAX_ARENA_SLOT; ++arena_slot)
        {
            ArenaTeam* temp = sArenaTeamMgr->GetArenaTeamById(bot->GetArenaTeamId(arena_slot));
            if (!temp)
                continue;

            if (temp->GetType() != type)
                continue;

            arenateam = temp;
        }

        if (!arenateam)
            return false;

        ratedList.push_back(queueTypeId);
    }

    // hack fix crash in queue remove event
    if (bot->GetGroup() && !bot->GetGroup()->IsLeader(bot->GetGUID()))
        return false;

    bool needBots = sRandomPlayerbotMgr->NeedBots[queueTypeId][bracketId][isArena ? isRated ? TEAM_HORDE : TEAM_ALLIANCE : teamId];

    // add more bots if players are not invited or 1st BG instance is full
    if (needBots/* || (hasPlayers && BgCount > BracketSize && (BgCount % BracketSize) != 0)*/)
        return true;

    // do not join if BG queue is full
    if (BgCount >= BracketSize && (ACount >= TeamSize) && (HCount >= TeamSize))
    {
        return false;
    }

    if (!isArena && ((ACount >= TeamSize && teamId == TEAM_ALLIANCE) || (HCount >= TeamSize && teamId == TEAM_HORDE)))
    {
        return false;
    }

    if (isArena && (((ACount >= TeamSize && HCount > 0) && teamId == TEAM_ALLIANCE) || ((HCount >= TeamSize && ACount > 0) && teamId == TEAM_HORDE)))
    {
        return false;
    }

    if (isArena && (((ACount > TeamSize && HCount == 0) && teamId == TEAM_HORDE) || ((HCount > TeamSize && ACount == 0) && teamId == TEAM_ALLIANCE)))
    {
        return false;
    }

    if (isArena && ((!isRated && SCount >= BracketSize) || (isRated && RCount >= BracketSize)))
    {
        return false;
    }

    return true;
}

bool BGJoinAction::isUseful()
{
    // do not try if BG bots disabled
    if (!sPlayerbotAIConfig->randomBotJoinBG)
        return false;

    // can't queue in BG
    if (bot->InBattleground())
        return false;

    // do not try right after login
    if ((time(nullptr) - bot->GetInGameTime()) < 30)
        return false;

    // check level
    if (bot->getLevel() < 10)
        return false;

    // do not try if with player master or in combat/group
    if (GET_PLAYERBOT_AI(bot)->HasActivePlayerMaster())
        return false;

   if (bot->GetGroup() && !bot->GetGroup()->IsLeader(bot->GetGUID()))
        return false;

    if (bot->IsInCombat())
        return false;

    // check Deserter debuff
    if (!bot->CanJoinToBattleground())
        return false;

    // check if has free queue slots
    if (!bot->HasFreeBattlegroundQueueId())
        return false;

    // do not try if in dungeon
     //Map* map = bot->GetMap();
    //if (map && map->Instanceable())
    //    return false;

    bgList.clear();
    ratedList.clear();

    for (int i = BG_BRACKET_ID_FIRST; i < MAX_BATTLEGROUND_BRACKETS; ++i)
    {
        for (int j = BATTLEGROUND_QUEUE_AV; j < MAX_BATTLEGROUND_QUEUE_TYPES; ++j)
        {
            BattlegroundQueueTypeId queueTypeId = BattlegroundQueueTypeId(j);
            BattlegroundTypeId bgTypeId = BattlegroundMgr::BGTemplateId(queueTypeId);
            BattlegroundBracketId bracketId = BattlegroundBracketId(i);

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
    PvPDifficultyEntry const* pvpDiff = GetBattlegroundBracketByLevel(mapId, bot->getLevel());
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
    uint32 instanceId = 0; // 0 = First Available
    bool joinAsGroup = bot->GetGroup() && bot->GetGroup()->GetLeaderGUID() == bot->GetGUID();
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
    Unit* unit = botAI->GetUnit(AI_VALUE2(CreatureData const*, "bg master", bgTypeId));
    if (!unit && isArena)
    { 
        botAI->GetAiObjectContext()->GetValue<uint32>("bg type")->Set(0);
        LOG_DEBUG("playerbots", "Bot {} could not find Battlemaster to join", bot->GetGUID().ToString().c_str());
        return false;
    }

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

    LOG_INFO("playerbots", "Bot {} {}:{} <{}> queued {} {}",
        bot->GetGUID().ToString().c_str(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->getLevel(), bot->GetName().c_str(), _bgType.c_str(),
        isRated ? "Rated Arena" : isArena ? "Arena" : "");

    // refresh food/regs
    sRandomPlayerbotMgr->Refresh(bot);

    if (isArena)
    {
        if (isRated)
        {
            sRandomPlayerbotMgr->BgBots[queueTypeId][bracketId][isRated ? TEAM_HORDE : TEAM_ALLIANCE] += TeamSize;
            sRandomPlayerbotMgr->ArenaBots[queueTypeId][bracketId][isRated ? TEAM_HORDE : TEAM_ALLIANCE][teamId] += TeamSize;
        }
        else
        {
            sRandomPlayerbotMgr->BgBots[queueTypeId][bracketId][isRated ? TEAM_HORDE : TEAM_ALLIANCE]++;
            sRandomPlayerbotMgr->ArenaBots[queueTypeId][bracketId][isRated ? TEAM_HORDE : TEAM_ALLIANCE][teamId]++;
        }
    }
    else if (!joinAsGroup)
        sRandomPlayerbotMgr->BgBots[queueTypeId][bracketId][teamId]++;
    else
        sRandomPlayerbotMgr->BgBots[queueTypeId][bracketId][teamId] += bot->GetGroup()->GetMembersCount();

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

bool FreeBGJoinAction::shouldJoinBg(BattlegroundQueueTypeId queueTypeId, BattlegroundBracketId bracketId)
{
    BattlegroundTypeId bgTypeId = BattlegroundMgr::BGTemplateId(queueTypeId);
    Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
    if (!bg)
        return false;

    bool isArena = false;
    bool isRated = false;

    ArenaType type = ArenaType(BattlegroundMgr::BGArenaType(queueTypeId));
    if (type != ARENA_TYPE_NONE)
        isArena = true;

    uint32 BracketSize = bg->GetMaxPlayersPerTeam() * 2;
    uint32 TeamSize = bg->GetMaxPlayersPerTeam();

    uint32 ACount = sRandomPlayerbotMgr->BgBots[queueTypeId][bracketId][TEAM_ALLIANCE] + sRandomPlayerbotMgr->BgPlayers[queueTypeId][bracketId][TEAM_ALLIANCE];
    uint32 HCount = sRandomPlayerbotMgr->BgBots[queueTypeId][bracketId][TEAM_HORDE] + sRandomPlayerbotMgr->BgPlayers[queueTypeId][bracketId][TEAM_HORDE];

    uint32 BgCount = ACount + HCount;
    uint32 SCount, RCount = 0;

    TeamId teamId = bot->GetTeamId();

    if (isArena)
    {
        uint32 rated_players = sRandomPlayerbotMgr->BgPlayers[queueTypeId][bracketId][TEAM_HORDE];
        if (rated_players)
        {
            isRated = true;
        }

        isArena = true;
        BracketSize = (uint32)(type * 2);
        TeamSize = type;
        ACount = sRandomPlayerbotMgr->ArenaBots[queueTypeId][bracketId][isRated ? TEAM_HORDE : TEAM_ALLIANCE][TEAM_ALLIANCE];
        HCount = sRandomPlayerbotMgr->ArenaBots[queueTypeId][bracketId][isRated ? TEAM_HORDE : TEAM_ALLIANCE][TEAM_HORDE];
        BgCount = sRandomPlayerbotMgr->BgBots[queueTypeId][bracketId][isRated ? TEAM_HORDE : TEAM_ALLIANCE] + sRandomPlayerbotMgr->BgPlayers[queueTypeId][bracketId][isRated ? TEAM_HORDE : TEAM_ALLIANCE];
        SCount = sRandomPlayerbotMgr->BgBots[queueTypeId][bracketId][TEAM_ALLIANCE] + sRandomPlayerbotMgr->BgPlayers[queueTypeId][bracketId][TEAM_ALLIANCE];
        RCount = sRandomPlayerbotMgr->BgBots[queueTypeId][bracketId][TEAM_HORDE] + sRandomPlayerbotMgr->BgPlayers[queueTypeId][bracketId][TEAM_HORDE];
    }

    // do not try if not a captain of arena team

    if (isRated)
    {
        if (!sArenaTeamMgr->GetArenaTeamByCaptain(bot->GetGUID()))
            return false;

        // check if bot has correct team
        ArenaTeam* arenateam = nullptr;
        for (uint32 arena_slot = 0; arena_slot < MAX_ARENA_SLOT; ++arena_slot)
        {
            ArenaTeam* temp = sArenaTeamMgr->GetArenaTeamById(bot->GetArenaTeamId(arena_slot));
            if (!temp)
                continue;

            if (temp->GetType() != type)
                continue;

            arenateam = temp;
        }

        if (!arenateam)
            return false;

        ratedList.push_back(queueTypeId);
    }

    // hack fix crash in queue remove event
    if (bot->GetGroup() && !bot->GetGroup()->IsLeader(bot->GetGUID()))
        return false;

    bool needBots = sRandomPlayerbotMgr->NeedBots[queueTypeId][bracketId][isArena ? isRated ? TEAM_HORDE : TEAM_ALLIANCE : teamId];

    // add more bots if players are not invited or 1st BG instance is full
    if (needBots/* || (hasPlayers && BgCount > BracketSize && (BgCount % BracketSize) != 0)*/)
        return true;

    // do not join if BG queue is full
    if (BgCount >= BracketSize && (ACount >= TeamSize) && (HCount >= TeamSize))
    {
        return false;
    }

    if (!isArena && ((ACount >= TeamSize && teamId == TEAM_ALLIANCE) || (HCount >= TeamSize && teamId == TEAM_HORDE)))
    {
        return false;
    }

    if (isArena && (((ACount >= TeamSize && HCount > 0) && teamId == TEAM_ALLIANCE) || ((HCount >= TeamSize && ACount > 0) && teamId == TEAM_HORDE)))
    {
        return false;
    }

    if (isArena && (((ACount > TeamSize && HCount == 0) && teamId == TEAM_HORDE) || ((HCount > TeamSize && ACount == 0) && teamId == TEAM_ALLIANCE)))
    {
        return false;
    }

    if (isArena && ((!isRated && SCount >= BracketSize) || (isRated && RCount >= BracketSize)))
    {
        return false;
    }

    return true;
}

bool BGLeaveAction::Execute(Event event)
{
    if (!(bot->InBattlegroundQueue() || bot->InBattleground()))
        return false;

    //botAI->ChangeStrategy("-bg", BOT_STATE_NON_COMBAT);

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

    if (bot->InBattleground())
    {
        LOG_INFO("playerbots", "Bot {} {}:{} <{}> leaves {}",
            bot->GetGUID().ToString().c_str(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->getLevel(), bot->GetName(), isArena ? "Arena" : "BG");

        WorldPacket leave(CMSG_LEAVE_BATTLEFIELD);
        leave << uint8(0) << uint8(0) << uint32(0) << uint16(0);
        bot->GetSession()->HandleBattlefieldLeaveOpcode(leave);

        if (IsRandomBot)
            botAI->SetMaster(nullptr);

        botAI->ResetStrategies(!IsRandomBot);
        botAI->GetAiObjectContext()->GetValue<uint32>("bg type")->Set(0);
        botAI->GetAiObjectContext()->GetValue<uint32>("bg role")->Set(0);
        botAI->GetAiObjectContext()->GetValue<uint32>("arena type")->Set(0);

        return true;
    }

    uint32 queueType = AI_VALUE(uint32, "bg type");
    if (!queueType)
        return false;

    LOG_INFO("playerbots", "Bot {} {}:{} <{}> leaves {} queue",
        bot->GetGUID().ToString().c_str(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->getLevel(), bot->GetName().c_str(), isArena ? "Arena" : "BG");

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

bool BGStatusAction::isUseful()
{
    return bot->InBattlegroundQueue();
}

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
    p >> QueueSlot; // queue id (0...2) - player can be in 3 queues in time
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
        case STATUS_WAIT_QUEUE:             // status_in_queue
            p >> Time1;                     // average wait time, milliseconds
            p >> Time2;                     // time in queue, updated every minute!, milliseconds
            break;
        case STATUS_WAIT_JOIN:              // status_invite
            p >> mapId;                     // map id
            p >> unk0;
            p >> Time1;                     // time to remove from queue, milliseconds
            break;
        case STATUS_IN_PROGRESS:            // status_in_progress
            p >> mapId;                     // map id
            p >> unk0;
            p >> Time1;                     // time to bg auto leave, 0 at bg start, 120000 after bg end, milliseconds
            p >> Time2;                     // time from bg start, milliseconds
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
    PvPDifficultyEntry const* pvpDiff = GetBattlegroundBracketByLevel(mapId, bot->getLevel());
    if (pvpDiff)
        bracketId = pvpDiff->GetBracketId();

    bool isArena = false;
    uint8 type = false;                                             // arenatype if arena
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

    if (Time1 == TIME_TO_AUTOREMOVE) //Battleground is over, bot needs to leave
    {
        LOG_INFO("playerbots", "Bot {} <{}> ({} {}): Received BG status TIME_REMOVE for {} {}",
            bot->GetGUID().ToString().c_str(), bot->GetName(), bot->getLevel(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", isArena ? "Arena" : "BG", _bgType);

        if (Battleground* bg = bot->GetBattleground())
        {
            if (isArena)
            {
                sRandomPlayerbotMgr->ArenaBots[queueTypeId][bracketId][isRated ? TEAM_HORDE : TEAM_ALLIANCE][teamId]--;
                teamId = isRated ? TEAM_HORDE : TEAM_ALLIANCE;
            }

            sRandomPlayerbotMgr->BgBots[queueTypeId][bracketId][teamId]--;
            sRandomPlayerbotMgr->BgPlayers[queueTypeId][bracketId][teamId] = 0;
        }

        // remove warsong strategy
        if (IsRandomBot)
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

        LOG_INFO("playerbots", "Bot {} {}:{} <{}> leaves {} - {}",
            bot->GetGUID().ToString().c_str(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->getLevel(), bot->GetName(), isArena ? "Arena" : "BG", _bgType);

        WorldPacket packet(CMSG_LEAVE_BATTLEFIELD);
        packet << uint8(0);
        packet << uint8(0);                           // BattlegroundTypeId-1 ?
        packet << uint32(0);
        packet << uint16(0);

        bot->GetSession()->HandleBattlefieldLeaveOpcode(packet);

        botAI->ResetStrategies(!IsRandomBot);
        botAI->GetAiObjectContext()->GetValue<uint32>("bg type")->Set(0);
        botAI->GetAiObjectContext()->GetValue<uint32>("bg role")->Set(0);
        botAI->GetAiObjectContext()->GetValue<uint32>("arena type")->Set(0);
        PositionMap& posMap = context->GetValue<PositionMap&>("position")->Get();
        PositionInfo pos = context->GetValue<PositionMap&>("position")->Get()["bg objective"];
        pos.Reset();
        posMap["bg objective"] = pos;
    }

    if (statusid == STATUS_WAIT_QUEUE) // bot is in queue
    {
        LOG_INFO("playerbots", "Bot {} {}:{} <{}>: Received BG status WAIT_QUEUE (wait time: {}) for {} {}",
            bot->GetGUID().ToString().c_str(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->getLevel(), bot->GetName(), Time2, isArena ? "Arena" : "BG", _bgType);
        // temp fix for crash
        //return true;

        BattlegroundQueue& bgQueue = sBattlegroundMgr->GetBattlegroundQueue(queueTypeId);
        GroupQueueInfo ginfo;
        if (bgQueue.GetPlayerGroupInfoData(bot->GetGUID(), &ginfo))
        {
            if (ginfo.IsInvitedToBGInstanceGUID && !bot->InBattleground())
            {
                Battleground* bg = sBattlegroundMgr->GetBattleground(ginfo.IsInvitedToBGInstanceGUID, _bgTypeId);
                if (bg)
                {
                    if (isArena)
                    {
                        _bgTypeId = bg->GetBgTypeID();
                    }

                    LOG_INFO("playerbots", "Bot {} {}:{} <{}>: Force join {} {}",
                        bot->GetGUID().ToString().c_str(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->getLevel(), bot->GetName(), isArena ? "Arena" : "BG", _bgType);
                    WorldPacket emptyPacket;
                    bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);
                    action = 0x1;

                    // bg started so players should get invites by now
                    sRandomPlayerbotMgr->NeedBots[queueTypeId][bracketId][isArena ? TeamId(isRated) : bot->GetTeamId()] = false;

                    WorldPacket packet(CMSG_BATTLEFIELD_PORT, 20);
                    packet << type << unk2 << (uint32)_bgTypeId << unk << action;
                    bot->GetSession()->HandleBattleFieldPortOpcode(packet);

                    botAI->ResetStrategies(false);
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

        if (Time2 > timer && isArena) // disabled for BG
            leaveQ = true;

        if (leaveQ && ((bot->GetGroup() && bot->GetGroup()->IsLeader(bot->GetGUID())) || !(bot->GetGroup() || botAI->GetMaster())))
        {
            TeamId teamId = bot->GetTeamId();
            bool realPlayers = sRandomPlayerbotMgr->BgPlayers[queueTypeId][bracketId][teamId];
            if (realPlayers)
                return false;

            LOG_INFO("playerbots", "Bot {} {}:{} <{}> waited too long and leaves queue ({} {}).",
                bot->GetGUID().ToString().c_str(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->getLevel(), bot->GetName(), isArena ? "Arena" : "BG", _bgType);

            WorldPacket packet(CMSG_BATTLEFIELD_PORT, 20);
            action = 0;
            packet << type << unk2 << (uint32)_bgTypeId << unk << action;
            bot->GetSession()->HandleBattleFieldPortOpcode(packet);

            botAI->ResetStrategies(!IsRandomBot);
            botAI->GetAiObjectContext()->GetValue<uint32>("bg type")->Set(0);
            botAI->GetAiObjectContext()->GetValue<uint32>("bg role")->Set(0);
            botAI->GetAiObjectContext()->GetValue<uint32>("arena type")->Set(0);
            sRandomPlayerbotMgr->BgBots[queueTypeId][bracketId][teamId]--;

            return true;
        }
    }

    if (statusid == STATUS_IN_PROGRESS) // placeholder for Leave BG if it takes too long
    {
        LOG_INFO("playerbots", "Bot {} {}:{} <{}>: Received BG status IN_PROGRESS for {} {}",
            bot->GetGUID().ToString().c_str(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->getLevel(), bot->GetName(), isArena ? "Arena" : "BG", _bgType);
        return false;
    }

    if (statusid == STATUS_WAIT_JOIN) //bot may join
    {
        LOG_INFO("playerbots", "Bot {} {}:{} <{}>: Received BG status WAIT_JOIN for {} {}",
            bot->GetGUID().ToString().c_str(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->getLevel(), bot->GetName(), isArena ? "Arena" : "BG", _bgType);

        if (isArena)
        {
            isArena = true;
            BattlegroundQueue& bgQueue = sBattlegroundMgr->GetBattlegroundQueue(queueTypeId);

            GroupQueueInfo ginfo;
            if (!bgQueue.GetPlayerGroupInfoData(bot->GetGUID(), &ginfo))
            {
                LOG_ERROR("playerbots", "Bot {} {}:{} <{}>: Missing QueueInfo for {} {}",
                    bot->GetGUID().ToString().c_str(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->getLevel(), bot->GetName(), isArena ? "Arena" : "BG", _bgType);
                return false;
            }

            if (ginfo.IsInvitedToBGInstanceGUID)
            {
                Battleground* bg = sBattlegroundMgr->GetBattleground(ginfo.IsInvitedToBGInstanceGUID, _bgTypeId);
                if (!bg)
                {
                    LOG_ERROR("playerbots", "Bot {} {}:{} <{}>: Missing QueueInfo for {} {}",
                        bot->GetGUID().ToString().c_str(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->getLevel(), bot->GetName(), isArena ? "Arena" : "BG", _bgType);
                    return false;
                }

                _bgTypeId = bg->GetBgTypeID();
            }
        }

        LOG_INFO("playerbots", "Bot {} {}:{} <{}> joined {} - {}",
            bot->GetGUID().ToString().c_str(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->getLevel(), bot->GetName(), isArena ? "Arena" : "BG", _bgType);

        WorldPacket emptyPacket;
        bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);

        action = 0x1;

        // bg started so players should get invites by now
        sRandomPlayerbotMgr->NeedBots[queueTypeId][bracketId][isArena ? isRated ? TEAM_HORDE : TEAM_ALLIANCE : teamId] = false;

        WorldPacket packet(CMSG_BATTLEFIELD_PORT, 20);
        packet << type << unk2 << (uint32)_bgTypeId << unk << action;
        bot->GetSession()->HandleBattleFieldPortOpcode(packet);

        botAI->ResetStrategies(false);
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

    LOG_INFO("playerbots", "Bot {} <{}> ({} {}) : Checking BG invite status",
        bot->GetGUID().ToString().c_str(), bot->GetName(), bot->getLevel(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H");

    return true;
}

bool BGStatusCheckAction::isUseful()
{
    return bot->InBattlegroundQueue();
}

bool BGStrategyCheckAction::Execute(Event event)
{
    bool inside_bg = bot->InBattleground() && bot->GetBattleground();;
    if (!inside_bg && botAI->HasStrategy("battleground", BOT_STATE_NON_COMBAT)) {
        botAI->ResetStrategies();
        return true;
    }
    if (inside_bg && !botAI->HasStrategy("battleground", BOT_STATE_NON_COMBAT)) {
        botAI->ResetStrategies();
        return false;
    }
    return false;
}