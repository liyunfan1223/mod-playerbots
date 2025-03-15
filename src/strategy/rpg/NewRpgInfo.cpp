#include "NewRpgInfo.h"
#include "Timer.h"

void NewRpgInfo::ChangeToGoGrind(WorldPosition pos)
{
    Reset();
    status = RPG_GO_GRIND;
    go_grind = GoGrind();
    go_grind.pos = pos;
}

void NewRpgInfo::ChangeToGoInnkeeper(WorldPosition pos)
{
    Reset();
    status = RPG_GO_INNKEEPER;
    go_innkeeper = GoInnkeeper();
    go_innkeeper.pos = pos;
}

void NewRpgInfo::ChangeToNearNpc()
{
    Reset();
    status = RPG_NEAR_NPC;
    near_npc = NearNpc();
}

void NewRpgInfo::ChangeToNearRandom()
{
    Reset();
    status = RPG_NEAR_RANDOM;
    near_random = NearRandom();
}

void NewRpgInfo::ChangeToDoQuest(uint32 questId, const Quest* quest)
{
    Reset();
    status = RPG_DO_QUEST;
    do_quest = DoQuest();
    do_quest.questId = questId;
    do_quest.quest = quest;
}

void NewRpgInfo::ChangeToRest()
{
    Reset();
    status = RPG_REST;
    rest = Rest();
}

void NewRpgInfo::ChangeToIdle()
{
    Reset();
    status = RPG_IDLE;
}

bool NewRpgInfo::CanChangeTo(NewRpgStatus status)
{
    return true;
}

void NewRpgInfo::Reset()
{
    *this = NewRpgInfo();
    startT = getMSTime();
}

void NewRpgInfo::SetMoveFarTo(WorldPosition pos)
{
    nearestMoveFarDis = FLT_MAX;
    stuckTs = 0;
    stuckAttempts = 0;
    moveFarPos = pos;
}

std::string NewRpgInfo::ToString()
{
    std::stringstream out;
    out << "Status: ";
    switch (status)
    {
        case RPG_GO_GRIND:
            out << "GO_GRIND";
            out << "\nGrindPos: " << go_grind.pos.GetMapId() << " " << go_grind.pos.GetPositionX() << " " << go_grind.pos.GetPositionY() << " " << go_grind.pos.GetPositionZ();
            out << "\nlastGoGrind: " << startT;
            break;
        case RPG_GO_INNKEEPER:
            out << "GO_INNKEEPER";
            out << "\nInnKeeperPos: " << go_innkeeper.pos.GetMapId() << " " << go_innkeeper.pos.GetPositionX() << " " << go_innkeeper.pos.GetPositionY() << " " << go_innkeeper.pos.GetPositionZ();
            out << "\nlastGoInnKeeper: " << startT;
            break;
        case RPG_NEAR_NPC:
            out << "NEAR_NPC";
            out << "\nnpcOrGoEntry: " << near_npc.npcOrGo.GetCounter();
            out << "\nlastNearNpc: " << startT;
            out << "\nlastReachNpcOrGo: " << near_npc.lastReach;
            break;
        case RPG_NEAR_RANDOM:
            out << "NEAR_RANDOM";
            out << "\nlastNearRandom: " << startT;
            break;
        case RPG_IDLE:
            out << "IDLE";
            break;
        case RPG_REST:
            out << "REST";
            out << "\nlastRest: " << startT;
            break;
        case RPG_DO_QUEST:
            out << "DO_QUEST";
            out << "\nquestId: " << do_quest.questId;
            out << "\nobjectiveIdx: " << do_quest.objectiveIdx;
            out << "\npoiPos: " << do_quest.pos.GetMapId() << " " << do_quest.pos.GetPositionX() << " " << do_quest.pos.GetPositionY() << " " << do_quest.pos.GetPositionZ();
            out << "\nlastReachPOI: " << do_quest.lastReachPOI;
            break;
        default:
            out << "UNKNOWN";
    }
    return out.str();
}