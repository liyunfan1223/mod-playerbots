/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "LfgActions.h"

#include "AiFactory.h"
#include "ItemVisitors.h"
#include "LFGMgr.h"
#include "Playerbots.h"

using namespace lfg;

bool LfgJoinAction::Execute(Event event) { return JoinLFG(); }

uint32 LfgJoinAction::GetRoles()
{
    if (!sRandomPlayerbotMgr->IsRandomBot(bot))
    {
        if (botAI->IsTank(bot))
            return PLAYER_ROLE_TANK;
        if (botAI->IsHeal(bot))
            return PLAYER_ROLE_HEALER;
        else
            return PLAYER_ROLE_DAMAGE;
    }

    uint8 spec = AiFactory::GetPlayerSpecTab(bot);
    switch (bot->getClass())
    {
        case CLASS_DRUID:
            if (spec == 2)
                return PLAYER_ROLE_HEALER;
            else if (spec == 1)
                return (PLAYER_ROLE_TANK | PLAYER_ROLE_DAMAGE);
            else
                return PLAYER_ROLE_DAMAGE;
            break;
        case CLASS_PALADIN:
            if (spec == 1)
                return PLAYER_ROLE_TANK;
            else if (!spec)
                return PLAYER_ROLE_HEALER;
            else
                return PLAYER_ROLE_DAMAGE;
            break;
        case CLASS_PRIEST:
            if (spec != 2)
                return PLAYER_ROLE_HEALER;
            else
                return PLAYER_ROLE_DAMAGE;
            break;
        case CLASS_SHAMAN:
            if (spec == 2)
                return PLAYER_ROLE_HEALER;
            else
                return PLAYER_ROLE_DAMAGE;
            break;
        case CLASS_WARRIOR:
            if (spec == 2)
                return PLAYER_ROLE_TANK;
            else
                return PLAYER_ROLE_DAMAGE;
            break;
        case CLASS_DEATH_KNIGHT:
            if (spec == 0)
                return PLAYER_ROLE_TANK;
            else
                return PLAYER_ROLE_DAMAGE;
            break;
        default:
            return PLAYER_ROLE_DAMAGE;
    }

    return PLAYER_ROLE_DAMAGE;
}

bool LfgJoinAction::JoinLFG()
{
    // check if already in lfg
    LfgState state = sLFGMgr->GetState(bot->GetGUID());
    if (state != LFG_STATE_NONE)
        return false;

    ItemCountByQuality visitor;
    IterateItems(&visitor, ITERATE_ITEMS_IN_EQUIP);
    bool random = urand(0, 100) < 20;
    bool heroic = urand(0, 100) < 50 &&
                  (visitor.count[ITEM_QUALITY_EPIC] >= 3 || visitor.count[ITEM_QUALITY_RARE] >= 10) &&
                  bot->GetLevel() >= 70;
    bool rbotAId = !heroic && (urand(0, 100) < 50 && visitor.count[ITEM_QUALITY_EPIC] >= 5 &&
                               (bot->GetLevel() == 60 || bot->GetLevel() == 70 || bot->GetLevel() == 80));

    LfgDungeonSet list;
    std::vector<uint32> selected;

    std::vector<uint32> dungeons = sRandomPlayerbotMgr->LfgDungeons[bot->GetTeamId()];
    if (!dungeons.size())
        return false;

    for (std::vector<uint32>::iterator i = dungeons.begin(); i != dungeons.end(); ++i)
    {
        LFGDungeonEntry const* dungeon = sLFGDungeonStore.LookupEntry(*i);
        if (!dungeon || (dungeon->TypeID != LFG_TYPE_RANDOM && dungeon->TypeID != LFG_TYPE_DUNGEON &&
                         dungeon->TypeID != LFG_TYPE_HEROIC && dungeon->TypeID != LFG_TYPE_RAID))
            continue;

        const auto& botLevel = bot->GetLevel();

        /*LFG_TYPE_RANDOM on classic is 15-58 so bot over level 25 will never queue*/
        if (dungeon->MinLevel && (botLevel < dungeon->MinLevel || botLevel > dungeon->MaxLevel) ||
            (botLevel > dungeon->MinLevel + 10 && dungeon->TypeID == LFG_TYPE_DUNGEON))
            continue;

        selected.push_back(dungeon->ID);
        list.insert(dungeon->ID);
    }

    if (!selected.size())
        return false;

    if (list.empty())
        return false;

    bool many = list.size() > 1;
    LFGDungeonEntry const* dungeon = sLFGDungeonStore.LookupEntry(*list.begin());

    // check role for console msg
    std::string _roles = "multiple roles";
    uint32 roleMask = GetRoles();
    if (roleMask & PLAYER_ROLE_TANK)
        _roles = "TANK";

    if (roleMask & PLAYER_ROLE_HEALER)
        _roles = "HEAL";

    if (roleMask & PLAYER_ROLE_DAMAGE)
        _roles = "DPS";

    LOG_INFO("playerbots", "Bot {} {}:{} <{}>: queues LFG, Dungeon as {} ({})", bot->GetGUID().ToString().c_str(),
             bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName().c_str(), _roles,
             many ? "several dungeons" : dungeon->Name[0]);

    // Set RbotAId Browser comment
    std::string const _gs = std::to_string(botAI->GetEquipGearScore(bot, false, false));
    sLFGMgr->JoinLfg(bot, roleMask, list, _gs);

    return true;
}

bool LfgRoleCheckAction::Execute(Event event)
{
    if (Group* group = bot->GetGroup())
    {
        uint32 currentRoles = sLFGMgr->GetRoles(bot->GetGUID());
        uint32 newRoles = GetRoles();
        // if (currentRoles == newRoles)
        //     return false;

        sLFGMgr->SetRoles(bot->GetGUID(), newRoles);

        sLFGMgr->UpdateRoleCheck(group->GetGUID(), bot->GetGUID(), newRoles);

        LOG_INFO("playerbots", "Bot {} {}:{} <{}>: LFG roles checked", bot->GetGUID().ToString().c_str(),
                 bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName().c_str());

        return true;
    }

    return false;
}

bool LfgAcceptAction::Execute(Event event)
{
    LfgState status = sLFGMgr->GetState(bot->GetGUID());
    if (status != LFG_STATE_PROPOSAL)
        return false;

    uint32 id = AI_VALUE(uint32, "lfg proposal");
    if (id)
    {
        // if (urand(0, 1 + 10 / sPlayerbotAIConfig->randomChangeMultiplier))
        //     return false;

        if (bot->IsInCombat() || bot->isDead())
        {
            LOG_INFO("playerbots", "Bot {} {}:{} <{}> is in combat and refuses LFG proposal {}",
                     bot->GetGUID().ToString().c_str(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->GetLevel(),
                     bot->GetName().c_str(), id);
            sLFGMgr->UpdateProposal(id, bot->GetGUID(), false);
            return true;
        }

        LOG_INFO("playerbots", "Bot {} {}:{} <{}> accepts LFG proposal {}", bot->GetGUID().ToString().c_str(),
                 bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName().c_str(), id);

        botAI->GetAiObjectContext()->GetValue<uint32>("lfg proposal")->Set(0);

        bot->ClearUnitState(UNIT_STATE_ALL_STATE);

        sLFGMgr->UpdateProposal(id, bot->GetGUID(), true);

        if (sRandomPlayerbotMgr->IsRandomBot(bot) && !bot->GetGroup())
        {
            sRandomPlayerbotMgr->Refresh(bot);
            botAI->ResetStrategies();
            // bot->TeleportToHomebind();
        }

        botAI->Reset();

        return true;
    }
    
    if (event.getPacket().empty())
        return false;

    WorldPacket p(event.getPacket());

    uint32 dungeon;
    uint8 state;
    p >> dungeon >> state >> id;

    botAI->GetAiObjectContext()->GetValue<uint32>("lfg proposal")->Set(id);

    return true;
}

bool LfgLeaveAction::Execute(Event event)
{
    // Don't leave if lfg strategy enabled
    // if (botAI->HasStrategy("lfg", BOT_STATE_NON_COMBAT))
    //    return false;

    // Don't leave if already invited / in dungeon
    if (sLFGMgr->GetState(bot->GetGUID()) > LFG_STATE_QUEUED)
        return false;

    sLFGMgr->LeaveLfg(bot->GetGUID());
    return true;
}

bool LfgLeaveAction::isUseful() { return true; }

bool LfgTeleportAction::Execute(Event event)
{
    bool out = false;

    WorldPacket p(event.getPacket());
    if (!p.empty())
    {
        p.rpos(0);
        p >> out;
    }

    bot->ClearUnitState(UNIT_STATE_ALL_STATE);

    sLFGMgr->TeleportPlayer(bot, out);

    return true;
}

bool LfgJoinAction::isUseful()
{
    if (!sPlayerbotAIConfig->randomBotJoinLfg)
    {
        // botAI->ChangeStrategy("-lfg", BOT_STATE_NON_COMBAT);
        return false;
    }

    if (bot->GetLevel() < 15)
        return false;

    if ((botAI->GetMaster() && !GET_PLAYERBOT_AI(botAI->GetMaster())) ||
        bot->GetGroup() && bot->GetGroup()->GetLeaderGUID() != bot->GetGUID())
    {
        // botAI->ChangeStrategy("-lfg", BOT_STATE_NON_COMBAT);
        return false;
    }

    if (bot->IsBeingTeleported())
        return false;

    if (bot->InBattleground())
        return false;

    if (bot->InBattlegroundQueue())
        return false;

    if (bot->isDead())
        return false;

    if (!sRandomPlayerbotMgr->IsRandomBot(bot))
        return false;

    Map* map = bot->GetMap();
    if (map && map->Instanceable())
        return false;

    LfgState state = sLFGMgr->GetState(bot->GetGUID());
    if (state != LFG_STATE_NONE)
        return false;

    return true;
}
