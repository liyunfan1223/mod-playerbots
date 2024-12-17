/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_NEWRPGSTRATEGY_H
#define _PLAYERBOT_NEWRPGSTRATEGY_H

#include <cstdint>
#include "Strategy.h"
#include "TravelMgr.h"

class PlayerbotAI;

enum class NewRpgStatus
{
    // Going to far away place
    GO_GRIND,
    GO_INNKEEPER,
    // Exploring nearby
    NEAR_RANDOM,
    NEAR_NPC,
    // Taking a break
    REST,
    // Initial status
    IDLE
};

struct NewRpgInfo
{
    NewRpgStatus status{NewRpgStatus::IDLE};
    // NewRpgStatus::GO_GRIND
    WorldPosition grindPos{};
    uint32 lastGoGrind{0};
    // NewRpgStatus::GO_INNKEEPER
    WorldPosition innKeeperPos{};
    uint32 lastGoInnKeeper{0};
    // NewRpgStatus::NEAR_NPC
    GuidPosition npcPos{};
    uint32 lastNearNpc{0};
    uint32 lastReachNpc{0};
    // NewRpgStatus::NEAR_RANDOM
    uint32 lastNearRandom{0};
    // NewRpgStatus::REST
    uint32 lastRest{0};

    std::string ToString()
    {
        std::stringstream out;
        out << "Status: ";
        switch (status)
        {
            case NewRpgStatus::GO_GRIND:
                out << "GO_GRIND";
                out << "\nGrindPos: " << grindPos.GetMapId() << " " << grindPos.GetPositionX() << " " << grindPos.GetPositionY() << " " << grindPos.GetPositionZ();
                out << "\nlastGoGrind: " << lastGoGrind;
                break;
            case NewRpgStatus::GO_INNKEEPER:
                out << "GO_INNKEEPER";
                out << "\nInnKeeperPos: " << innKeeperPos.GetMapId() << " " << innKeeperPos.GetPositionX() << " " << innKeeperPos.GetPositionY() << " " << innKeeperPos.GetPositionZ();
                out << "\nlastGoInnKeeper: " << lastGoInnKeeper;
                break;
            case NewRpgStatus::NEAR_NPC:
                out << "NEAR_NPC";
                out << "\nNpcPos: " << npcPos.GetMapId() << " " << npcPos.GetPositionX() << " " << npcPos.GetPositionY() << " " << npcPos.GetPositionZ();
                out << "\nlastNearNpc: " << lastNearNpc;
                out << "\nlastReachNpc: " << lastReachNpc;
                break;
            case NewRpgStatus::NEAR_RANDOM:
                out << "NEAR_RANDOM";
                out << "\nlastNearRandom: " << lastNearRandom;
                break;
            case NewRpgStatus::IDLE:
                out << "IDLE";
                break;
            case NewRpgStatus::REST:
                out << "REST";
                out << "\nlastRest: " << lastRest;
                break;
            default:
                out << "UNKNOWN";
        }
        return out.str();
    }
};

class NewRpgStrategy : public Strategy
{
public:
    NewRpgStrategy(PlayerbotAI* botAI);

    std::string const getName() override { return "new rpg"; }
    NextAction** getDefaultActions() override;
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
