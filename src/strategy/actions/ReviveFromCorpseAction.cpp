/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ReviveFromCorpseAction.h"

#include "Event.h"
#include "FleeManager.h"
#include "GameGraveyard.h"
#include "MapMgr.h"
#include "PlayerbotFactory.h"
#include "Playerbots.h"
#include "RandomPlayerbotMgr.h"
#include "ServerFacade.h"

bool ReviveFromCorpseAction::Execute(Event event)
{
    Player* master = botAI->GetGroupMaster();
    Corpse* corpse = bot->GetCorpse();

    // follow master when master revives
    WorldPacket& p = event.getPacket();
    if (!p.empty() && p.GetOpcode() == CMSG_RECLAIM_CORPSE && master && !corpse && bot->IsAlive())
    {
        if (sServerFacade->IsDistanceLessThan(AI_VALUE2(float, "distance", "master target"),
                                              sPlayerbotAIConfig->farDistance))
        {
            if (!botAI->HasStrategy("follow", BOT_STATE_NON_COMBAT))
            {
                botAI->TellMasterNoFacing("Welcome back!");
                botAI->ChangeStrategy("+follow,-stay", BOT_STATE_NON_COMBAT);
                return true;
            }
        }
    }

    if (!corpse)
        return false;

    // if (corpse->GetGhostTime() + bot->GetCorpseReclaimDelay(corpse->GetType() == CORPSE_RESURRECTABLE_PVP) >
    // time(nullptr))
    //     return false;

    if (master)
    {
        if (!GET_PLAYERBOT_AI(master) && master->isDead() && master->GetCorpse() &&
            sServerFacade->IsDistanceLessThan(AI_VALUE2(float, "distance", "master target"),
                                              sPlayerbotAIConfig->farDistance))
            return false;
    }

    if (!botAI->HasRealPlayerMaster())
    {
        uint32 dCount = AI_VALUE(uint32, "death count");

        if (dCount >= 5)
        {
            return botAI->DoSpecificAction("spirit healer");
        }
    }

    LOG_DEBUG("playerbots", "Bot {} {}:{} <{}> revives at body", bot->GetGUID().ToString().c_str(),
              bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName().c_str());

    bot->GetMotionMaster()->Clear();
    bot->StopMoving();

    WorldPacket packet(CMSG_RECLAIM_CORPSE);
    packet << bot->GetGUID();
    bot->GetSession()->HandleReclaimCorpseOpcode(packet);

    return true;
}

bool FindCorpseAction::Execute(Event event)
{
    if (bot->InBattleground())
        return false;

    Player* master = botAI->GetGroupMaster();
    Corpse* corpse = bot->GetCorpse();
    if (!corpse)
        return false;

    // if (master)
    // {
    //     if (!GET_PLAYERBOT_AI(master) &&
    //         sServerFacade->IsDistanceLessThan(AI_VALUE2(float, "distance", "master target"),
    //         sPlayerbotAIConfig->farDistance)) return false;
    // }

    uint32 dCount = AI_VALUE(uint32, "death count");

    if (!botAI->HasRealPlayerMaster())
    {
        if (dCount >= 5)
        {
            // LOG_INFO("playerbots", "Bot {} {}:{} <{}>: died too many times, was revived and teleported",
            //     bot->GetGUID().ToString().c_str(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->GetLevel(),
            //     bot->GetName().c_str());
            context->GetValue<uint32>("death count")->Set(0);
            // sRandomPlayerbotMgr->RandomTeleportForLevel(bot);
            sRandomPlayerbotMgr->Revive(bot);
            return true;
        }
    }

    WorldPosition botPos(bot);
    WorldPosition corpsePos(corpse);
    WorldPosition moveToPos = corpsePos;
    WorldPosition masterPos(master);

    float reclaimDist = CORPSE_RECLAIM_RADIUS - 5.0f;
    float corpseDist = botPos.distance(corpsePos);
    int64 deadTime = time(nullptr) - corpse->GetGhostTime();

    bool moveToMaster = master && master != bot && masterPos.fDist(corpsePos) < reclaimDist;

    // Should we ressurect? If so, return false.
    if (corpseDist < reclaimDist)
    {
        if (moveToMaster)  // We are near master.
        {
            if (botPos.fDist(masterPos) < sPlayerbotAIConfig->spellDistance)
                return false;
        }
        else if (deadTime > 8 * MINUTE)  // We have walked too long already.
            return false;
        else
        {
            GuidVector units = AI_VALUE(GuidVector, "possible targets no los");

            if (botPos.getUnitsAggro(units, bot) == 0)  // There are no mobs near.
                return false;
        }
    }

    // If we are getting close move to a save ressurrection spot instead of just the corpse.
    if (corpseDist < sPlayerbotAIConfig->reactDistance)
    {
        if (moveToMaster)
            moveToPos = masterPos;
        else
        {
            FleeManager manager(bot, reclaimDist, 0.0, urand(0, 1), moveToPos);

            if (manager.isUseful())
            {
                float rx, ry, rz;
                if (manager.CalculateDestination(&rx, &ry, &rz))
                    moveToPos = WorldPosition(moveToPos.getMapId(), rx, ry, rz, 0.0);
                else if (!moveToPos.GetReachableRandomPointOnGround(bot, reclaimDist, urand(0, 1)))
                    moveToPos = corpsePos;
            }
        }
    }

    // Actual mobing part.
    bool moved = false;

    if (!botAI->AllowActivity(ALL_ACTIVITY))
    {
        uint32 delay = sServerFacade->GetDistance2d(bot, corpse) /
                       bot->GetSpeed(MOVE_RUN);        // Time a bot would take to travel to it's corpse.
        delay = std::min(delay, uint32(10 * MINUTE));  // Cap time to get to corpse at 10 minutes.

        if (deadTime > delay)
        {
            bot->GetMotionMaster()->Clear();
            bot->TeleportTo(moveToPos.getMapId(), moveToPos.getX(), moveToPos.getY(), moveToPos.getZ(), 0);
        }

        moved = true;
    }
    else
    {
        if (bot->isMoving())
            moved = true;
        else
        {
            if (deadTime < 10 * MINUTE && dCount < 5)  // Look for corpse up to 30 minutes.
            {
                moved =
                    MoveTo(moveToPos.getMapId(), moveToPos.getX(), moveToPos.getY(), moveToPos.getZ(), false, false);
            }

            if (!moved)
            {
                moved = botAI->DoSpecificAction("spirit healer");
            }
        }
    }

    return moved;
}

bool FindCorpseAction::isUseful()
{
    if (bot->InBattleground())
        return false;

    return bot->GetCorpse();
}

GraveyardStruct const* SpiritHealerAction::GetGrave(bool startZone)
{
    GraveyardStruct const* ClosestGrave = nullptr;
    GraveyardStruct const* NewGrave = nullptr;

    ClosestGrave = sGraveyard->GetClosestGraveyard(bot, bot->GetTeamId());

    if (!startZone && ClosestGrave)
        return ClosestGrave;

    if (botAI->HasStrategy("follow", BOT_STATE_NON_COMBAT) && botAI->GetGroupMaster() && botAI->GetGroupMaster() != bot)
    {
        Player* master = botAI->GetGroupMaster();
        if (master && master != bot)
        {
            ClosestGrave = sGraveyard->GetClosestGraveyard(master, bot->GetTeamId());

            if (ClosestGrave)
                return ClosestGrave;
        }
    }
    else if (startZone && AI_VALUE(uint8, "durability"))
    {
        TravelTarget* travelTarget = AI_VALUE(TravelTarget*, "travel target");

        if (travelTarget->getPosition())
        {
            WorldPosition travelPos = *travelTarget->getPosition();
            if (travelPos.GetMapId() != uint32(-1))
            {
                uint32 areaId = 0;
                uint32 zoneId = 0;
                sMapMgr->GetZoneAndAreaId(bot->GetPhaseMask(), zoneId, areaId, travelPos.getMapId(), travelPos.getX(),
                                          travelPos.getY(), travelPos.getZ());
                ClosestGrave = sGraveyard->GetClosestGraveyard(travelPos.getMapId(), travelPos.getX(), travelPos.getY(),
                                                               travelPos.getZ(), bot->GetTeamId(), areaId, zoneId,
                                                               bot->getClass() == CLASS_DEATH_KNIGHT);

                if (ClosestGrave)
                    return ClosestGrave;
            }
        }
    }

    std::vector<uint32> races;

    if (bot->GetTeamId() == TEAM_ALLIANCE)
        races = {RACE_HUMAN, RACE_DWARF, RACE_GNOME, RACE_NIGHTELF};
    else
        races = {RACE_ORC, RACE_TROLL, RACE_TAUREN, RACE_UNDEAD_PLAYER};

    float graveDistance = -1;

    WorldPosition botPos(bot);

    for (auto race : races)
    {
        for (uint32 cls = 0; cls < MAX_CLASSES; cls++)
        {
            PlayerInfo const* info = sObjectMgr->GetPlayerInfo(race, cls);
            if (!info)
                continue;

            uint32 areaId = 0;
            uint32 zoneId = 0;
            sMapMgr->GetZoneAndAreaId(bot->GetPhaseMask(), zoneId, areaId, info->mapId, info->positionX,
                                      info->positionY, info->positionZ);

            NewGrave = sGraveyard->GetClosestGraveyard(info->mapId, info->positionX, info->positionY, info->positionZ,
                                                       bot->GetTeamId(), areaId, zoneId, cls == CLASS_DEATH_KNIGHT);
            if (!NewGrave)
                continue;

            WorldPosition gravePos(NewGrave->Map, NewGrave->x, NewGrave->y, NewGrave->z);

            float newDist = botPos.fDist(gravePos);

            if (graveDistance < 0 || newDist < graveDistance)
            {
                ClosestGrave = NewGrave;
                graveDistance = newDist;
            }
        }
    }

    return ClosestGrave;
}

bool SpiritHealerAction::Execute(Event event)
{
    Corpse* corpse = bot->GetCorpse();
    if (!corpse)
    {
        botAI->TellError("I am not a spirit");
        return false;
    }

    uint32 dCount = AI_VALUE(uint32, "death count");
    int64 deadTime = time(nullptr) - corpse->GetGhostTime();

    GraveyardStruct const* ClosestGrave =
        GetGrave(dCount > 10 || deadTime > 15 * MINUTE || AI_VALUE(uint8, "durability") < 10);

    if (bot->GetDistance2d(ClosestGrave->x, ClosestGrave->y) < sPlayerbotAIConfig->sightDistance)
    {
        GuidVector npcs = AI_VALUE(GuidVector, "nearest npcs");
        for (GuidVector::iterator i = npcs.begin(); i != npcs.end(); i++)
        {
            Unit* unit = botAI->GetUnit(*i);
            if (unit && unit->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPIRITHEALER))
            {
                LOG_DEBUG("playerbots", "Bot {} {}:{} <{}> revives at spirit healer", bot->GetGUID().ToString().c_str(),
                          bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName());
                PlayerbotChatHandler ch(bot);
                bot->ResurrectPlayer(0.5f);
                bot->SpawnCorpseBones();
                context->GetValue<Unit*>("current target")->Set(nullptr);
                bot->SetTarget();
                botAI->TellMaster("Hello");

                if (dCount > 20)
                    context->GetValue<uint32>("death count")->Set(0);

                return true;
            }
        }
    }

    if (!ClosestGrave)
    {
        return false;
    }

    bool moved = false;

    if (bot->IsWithinLOS(ClosestGrave->x, ClosestGrave->y, ClosestGrave->z))
        moved = MoveNear(ClosestGrave->Map, ClosestGrave->x, ClosestGrave->y, ClosestGrave->z, 0.0);
    else
        moved = MoveTo(ClosestGrave->Map, ClosestGrave->x, ClosestGrave->y, ClosestGrave->z, false, false);

    if (moved)
        return true;

    if (!botAI->HasActivePlayerMaster())
    {
        context->GetValue<uint32>("death count")->Set(dCount + 1);
        return bot->TeleportTo(ClosestGrave->Map, ClosestGrave->x, ClosestGrave->y, ClosestGrave->z, 0.f);
    }

    LOG_INFO("playerbots", "Bot {} {}:{} <{}> can't find a spirit healer", bot->GetGUID().ToString().c_str(),
             bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName().c_str());

    botAI->TellError("Cannot find any spirit healer nearby");
    return false;
}

bool SpiritHealerAction::isUseful() { return bot->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST); }
