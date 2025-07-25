#ifndef _PLAYERBOT_NEWRPGINFO_H
#define _PLAYERBOT_NEWRPGINFO_H

#include "Define.h"
#include "ObjectGuid.h"
#include "ObjectMgr.h"
#include "QuestDef.h"
#include "Strategy.h"
#include "Timer.h"
#include "TravelMgr.h"

enum NewRpgStatus: int
{
    RPG_STATUS_START = 0,
    // Going to far away place
    RPG_GO_GRIND = 0,
    RPG_GO_INNKEEPER = 1,
    // Exploring nearby
    RPG_NEAR_RANDOM = 2,
    RPG_NEAR_NPC = 3,
    // Do Quest (based on quest status)
    RPG_DO_QUEST = 4,
    // Taking a break
    RPG_REST = 5,
    // Initial status
    RPG_IDLE = 6,
    RPG_STATUS_END = 7
};

using NewRpgStatusTransitionProb = std::vector<std::vector<int>>;

struct NewRpgInfo
{
    NewRpgInfo() {}
    
    // RPG_GO_GRIND
    struct GoGrind {
        GoGrind() = default;
        WorldPosition pos{};
    };
    // RPG_GO_INNKEEPER
    struct GoInnkeeper {
        GoInnkeeper() = default;
        WorldPosition pos{};
    };
    // RPG_NEAR_NPC
    struct NearNpc {
        NearNpc() = default;
        ObjectGuid npcOrGo{};
        uint32 lastReach{0};
    };
    // RPG_NEAR_RANDOM
    struct NearRandom {
        NearRandom() = default;
    };
    // NewRpgStatus::QUESTING
    struct DoQuest {
        const Quest* quest{nullptr};
        uint32 questId{0};
        int32 objectiveIdx{0};
        WorldPosition pos{};
        uint32 lastReachPOI{0};
    };
    // RPG_REST
    struct Rest {
        Rest() = default;
    };
    struct Idle {
    };
    NewRpgStatus status{RPG_IDLE};

    uint32 startT{0}; // start timestamp of the current status

    // MOVE_FAR
    float nearestMoveFarDis{FLT_MAX};
    uint32 stuckTs{0};
    uint32 stuckAttempts{0};
    WorldPosition moveFarPos;
    // END MOVE_FAR

    union {
        GoGrind go_grind;
        GoInnkeeper go_innkeeper;
        NearNpc near_npc;
        NearRandom near_random;
        DoQuest do_quest;
        Rest rest;
        DoQuest quest;
    };

    bool HasStatusPersisted(uint32 maxDuration) { return GetMSTimeDiffToNow(startT) > maxDuration; }
    void ChangeToGoGrind(WorldPosition pos);
    void ChangeToGoInnkeeper(WorldPosition pos);
    void ChangeToNearNpc();
    void ChangeToNearRandom();
    void ChangeToDoQuest(uint32 questId, const Quest* quest);
    void ChangeToRest();
    void ChangeToIdle();
    bool CanChangeTo(NewRpgStatus status);
    void Reset();
    void SetMoveFarTo(WorldPosition pos);
    std::string ToString();
};

struct NewRpgStatistic
{
    uint32 questAccepted{0};
    uint32 questCompleted{0};
    uint32 questAbandoned{0};
    uint32 questRewarded{0};
    uint32 questDropped{0};
    NewRpgStatistic operator+(const NewRpgStatistic& other) const
    {
        NewRpgStatistic result;
        result.questAccepted = this->questAccepted + other.questAccepted;
        result.questCompleted = this->questCompleted + other.questCompleted;
        result.questAbandoned = this->questAbandoned + other.questAbandoned;
        result.questRewarded = this->questRewarded + other.questRewarded;
        result.questDropped = this->questDropped + other.questDropped;
        return result;
    }
    NewRpgStatistic& operator+=(const NewRpgStatistic& other)
    {
        this->questAccepted += other.questAccepted;
        this->questCompleted += other.questCompleted;
        this->questAbandoned += other.questAbandoned;
        this->questRewarded += other.questRewarded;
        this->questDropped += other.questDropped;
        return *this;
    }
};

// not sure is it necessary but keep it for now
#define RPG_INFO(x, y) botAI->rpgInfo.x.y

#endif