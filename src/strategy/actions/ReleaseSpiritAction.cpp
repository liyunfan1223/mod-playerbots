/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ReleaseSpiritAction.h"

#include "Event.h"
#include "GameGraveyard.h"
#include "NearestNpcsValue.h"
#include "ObjectDefines.h"
#include "ObjectGuid.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "Corpse.h"

// ReleaseSpiritAction implementation
bool ReleaseSpiritAction::Execute(Event event)
{
    if (bot->IsAlive())
    {
        botAI->TellMasterNoFacing("I am not dead, will wait here");
        botAI->ChangeStrategy("-follow,+stay", BOT_STATE_NON_COMBAT);
        return false;
    }

    if (bot->GetCorpse() && bot->HasPlayerFlag(PLAYER_FLAGS_GHOST))
    {
        botAI->TellMasterNoFacing("I am already a spirit");
        return false;
    }

    const WorldPacket& packet = event.getPacket();
    const std::string message = !packet.empty() && packet.GetOpcode() == CMSG_REPOP_REQUEST 
                                ? "Releasing..." 
                                : "Meet me at the graveyard";
    botAI->TellMasterNoFacing(message);

    IncrementDeathCount();
    LogRelease("released");

    WorldPacket releasePacket(CMSG_REPOP_REQUEST);
    releasePacket << uint8(0);
    bot->GetSession()->HandleRepopRequestOpcode(releasePacket);

    return true;
}

void ReleaseSpiritAction::IncrementDeathCount() const
{
    Player* master = botAI->GetMaster();
    if (!master || GET_PLAYERBOT_AI(master))
    {
        uint32 deathCount = AI_VALUE(uint32, "death count");
        context->GetValue<uint32>("death count")->Set(deathCount + 1);
    }
}

void ReleaseSpiritAction::LogRelease(const std::string& releaseMsg, bool isAutoRelease) const
{
    const std::string teamPrefix = bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H";

    LOG_DEBUG("playerbots", "Bot {} {}:{} <{}> {}",
        bot->GetGUID().ToString().c_str(),
        teamPrefix,
        bot->GetLevel(),
        bot->GetName().c_str(),
        releaseMsg.c_str());
}

// AutoReleaseSpiritAction implementation
bool AutoReleaseSpiritAction::Execute(Event event)
{
    IncrementDeathCount();
    LogRelease("auto released", true);

    WorldPacket packet(CMSG_REPOP_REQUEST);
    packet << uint8(0);
    bot->GetSession()->HandleRepopRequestOpcode(packet);

    LogRelease("releases spirit", true);
    
    if (bot->InBattleground())
    {
        return HandleBattlegroundSpiritHealer();
    }

    botAI->SetNextCheckDelay(1000);
    return true;
}

bool AutoReleaseSpiritAction::isUseful()
{
    if (!bot->isDead() || bot->InArena())
        return false;

    if (bot->InBattleground())
        return ShouldDelayBattlegroundRelease();

    if (bot->HasPlayerFlag(PLAYER_FLAGS_GHOST))
        return false;

    return ShouldAutoRelease();
}

bool AutoReleaseSpiritAction::HandleBattlegroundSpiritHealer()
{
    constexpr uint32_t RESURRECT_DELAY = 15;
    const time_t now = time(nullptr);
    
    if ((now - m_bgGossipTime < RESURRECT_DELAY) && 
        bot->HasAura(SPELL_WAITING_FOR_RESURRECT))
    {
        return false;
    }

    float bgRange = 2000.0f;
    GuidVector npcs = NearestNpcsValue(botAI, bgRange);
    Unit* spiritHealer = nullptr;

    for (const auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsFriendlyTo(bot) && unit->IsSpiritService())
        {
            spiritHealer = unit;
            break;
        }
    }

    if (!spiritHealer)
        return false;

    if (bot->GetDistance(spiritHealer) >= INTERACTION_DISTANCE)
    {
        // Bot needs to actually click spirit-healer in BG to get res timer going
        // and in IOC it's not within clicking range when they res in own base

        // Teleport to nearest friendly Spirit Healer when not currently in range of one.
        bot->TeleportTo(bot->GetMapId(), spiritHealer->GetPositionX(), spiritHealer->GetPositionY(), spiritHealer->GetPositionZ(), 0.f);
        RESET_AI_VALUE(bool, "combat::self target");
        RESET_AI_VALUE(WorldPosition, "current position");
    }
    else if (!botAI->IsRealPlayer())
    {
        m_bgGossipTime = now;
        WorldPacket packet(CMSG_GOSSIP_HELLO);
        packet << spiritHealer->GetGUID();
        bot->GetSession()->HandleGossipHelloOpcode(packet);
    }

    return true;
}

bool AutoReleaseSpiritAction::ShouldAutoRelease() const
{
    if (!bot->GetGroup())
        return true;

    Player* groupMaster = botAI->GetGroupMaster();
    if (!groupMaster || groupMaster == bot)
        return true;

    if (!botAI->HasActivePlayerMaster())
        return true;

    if (botAI->HasActivePlayerMaster() && 
        groupMaster->GetMapId() == bot->GetMapId() &&
        bot->GetMap() && 
        (bot->GetMap()->IsRaid() || bot->GetMap()->IsDungeon()))
    {
        return false;
    }

    return sServerFacade->IsDistanceGreaterThan(
        AI_VALUE2(float, "distance", "master target"),
        sPlayerbotAIConfig->sightDistance);
}

bool AutoReleaseSpiritAction::ShouldDelayBattlegroundRelease() const
{
    // The below delays release to spirit with 6 seconds.
    // This prevents currently casted (ranged) spells to be re-directed to the died bot's ghost.
    const int32_t botId = bot->GetGUID().GetRawValue();

    // If the bot already is a spirit, erase release time and return true
    if (bot->HasPlayerFlag(PLAYER_FLAGS_GHOST))
    {
        m_botReleaseTimes.erase(botId);
        return true;
    }

    // Delay release to spirit.
    const time_t now = time(nullptr);
    constexpr time_t RELEASE_DELAY = 6;

    auto& lastReleaseTime = m_botReleaseTimes[botId];
    if (lastReleaseTime == 0)
        lastReleaseTime = now;

    if (now - lastReleaseTime < RELEASE_DELAY)
        return false;

    m_botReleaseTimes.erase(botId);
    return true;
}

bool RepopAction::Execute(Event event)
{
    const GraveyardStruct* graveyard = GetGrave(
        AI_VALUE(uint32, "death count") > 10 || 
        CalculateDeadTime() > 30 * MINUTE
    );

    if (!graveyard)
        return false;

    PerformGraveyardTeleport(graveyard);
    return true;
}

bool RepopAction::isUseful()
{
    return !bot->InBattleground();
}

int64 RepopAction::CalculateDeadTime() const
{
    if (Corpse* corpse = bot->GetCorpse())
        return time(nullptr) - corpse->GetGhostTime();
    
    return bot->isDead() ? 0 : 60 * MINUTE;
}

void RepopAction::PerformGraveyardTeleport(const GraveyardStruct* graveyard) const
{
    bot->TeleportTo(graveyard->Map, graveyard->x, graveyard->y, graveyard->z, 0.f);
    RESET_AI_VALUE(bool, "combat::self target");
    RESET_AI_VALUE(WorldPosition, "current position");
}
