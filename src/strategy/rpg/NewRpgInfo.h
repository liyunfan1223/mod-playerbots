#ifndef _PLAYERBOT_NEWRPGINFO_H
#define _PLAYERBOT_NEWRPGINFO_H

#include "ObjectMgr.h"
#include "QuestDef.h"
#include "Strategy.h"
#include "Timer.h"
#include "TravelMgr.h"

enum class NewRpgStatus
{
    // Going to far away place
    GO_GRIND,
    GO_INNKEEPER,
    // Exploring nearby
    NEAR_RANDOM,
    NEAR_NPC,
    // Do Quest (based on quest status and location)
    QUESTING,
    // Taking a break
    REST,
    // Initial status
    IDLE
};

struct NewRpgInfo
{
    NewRpgInfo() {}
    
    // NewRpgStatus::GO_GRIND
    struct GoGrind {
        GoGrind() = default;
        WorldPosition pos{};
    };
    // NewRpgStatus::GO_INNKEEPER
    struct GoInnkeeper {
        GoInnkeeper() = default;
        WorldPosition pos{};
    };
    // NewRpgStatus::NEAR_NPC
    struct NearNpc {
        NearNpc() = default;
        GuidPosition pos{};
        uint32 lastReach{0};
    };
    // NewRpgStatus::NEAR_RANDOM
    struct NearRandom {
        NearRandom() = default;
    };
    // NewRpgStatus::QUESTING
    struct Questing {
        Quest* quest;
    };
    // NewRpgStatus::REST
    struct Rest {
        Rest() = default;
    };
    struct Idle {
    };
    NewRpgStatus status{NewRpgStatus::IDLE};

    uint32 startT{0}; // start timestamp of the current status

    // MOVE_FAR
    float nearestMoveFarDis{FLT_MAX};
    uint32 stuckTs{0};
    uint32 stuckAttempts{0};
    // END MOVE_FAR

    union {
        GoGrind go_grind;
        GoInnkeeper go_innkeeper;
        NearNpc near_npc;
        NearRandom near_random;
        Rest rest;
        Questing questing;
    };

    bool HasStatusPersisted(uint32 maxDuration) { return startT + maxDuration < getMSTime(); }
    void ChangeToGoGrind(WorldPosition pos);
    void ChangeToGoInnkeeper(WorldPosition pos);
    void ChangeToNearNpc();
    void ChangeToNearRandom();
    void ChangeToRest();
    void ChangeToIdle();
    
    void Reset();
    std::string ToString();
};

#endif