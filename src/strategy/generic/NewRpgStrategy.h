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
    // Idling
    IDLE
};

struct NewRpgInfo
{
    NewRpgStatus status{NewRpgStatus::IDLE};
    WorldPosition grindPos{};
    uint32 lastGrind{0};
    uint32 lastNearNpc{0};
    std::string ToString()
    {
        std::stringstream out;
        out << "Status: ";
        switch (status)
        {
            case NewRpgStatus::GO_GRIND:
                out << "GO_GRIND";
                break;
            case NewRpgStatus::GO_INNKEEPER:
                out << "GO_INNKEEPER";
                break;
            case NewRpgStatus::NEAR_NPC:
                out << "NEAR_NPC";
                break;
            case NewRpgStatus::NEAR_RANDOM:
                out << "NEAR_RANDOM";
                break;
            case NewRpgStatus::IDLE:
                out << "IDLE";
                break;
        }
        out << "\nGrindPos: " << grindPos.GetMapId() << " " << grindPos.GetPositionX() << " " << grindPos.GetPositionY() << " " << grindPos.GetPositionZ();
        out << "\nLastGrind: " << lastGrind;
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
