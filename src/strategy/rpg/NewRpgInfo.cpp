#include "NewRpgInfo.h"
#include "Timer.h"

void NewRpgInfo::ChangeToGoGrind(WorldPosition pos)
{
    Reset();
    status = NewRpgStatus::GO_GRIND;
    go_grind.pos = pos;
}

void NewRpgInfo::ChangeToGoInnkeeper(WorldPosition pos)
{
    Reset();
    status = NewRpgStatus::GO_INNKEEPER;
    go_innkeeper.pos = pos;
}

void NewRpgInfo::ChangeToNearNpc()
{
    Reset();
    status = NewRpgStatus::NEAR_NPC;
}

void NewRpgInfo::ChangeToNearRandom()
{
    status = NewRpgStatus::NEAR_RANDOM;
}

void NewRpgInfo::ChangeToRest()
{
    status = NewRpgStatus::REST;
}

void NewRpgInfo::ChangeToIdle()
{
    status = NewRpgStatus::IDLE;
}


void NewRpgInfo::Reset()
{
    *this = NewRpgInfo();
    startT = getMSTime();
}

std::string NewRpgInfo::ToString()
{
    std::stringstream out;
    out << "Status: ";
    switch (status)
    {
        case NewRpgStatus::GO_GRIND:
            out << "GO_GRIND";
            out << "\nGrindPos: " << go_grind.pos.GetMapId() << " " << go_grind.pos.GetPositionX() << " " << go_grind.pos.GetPositionY() << " " << go_grind.pos.GetPositionZ();
            out << "\nlastGoGrind: " << startT;
            break;
        case NewRpgStatus::GO_INNKEEPER:
            out << "GO_INNKEEPER";
            out << "\nInnKeeperPos: " << go_innkeeper.pos.GetMapId() << " " << go_innkeeper.pos.GetPositionX() << " " << go_innkeeper.pos.GetPositionY() << " " << go_innkeeper.pos.GetPositionZ();
            out << "\nlastGoInnKeeper: " << startT;
            break;
        case NewRpgStatus::NEAR_NPC:
            out << "NEAR_NPC";
            out << "\nNpcPos: " << near_npc.pos.GetMapId() << " " << near_npc.pos.GetPositionX() << " " << near_npc.pos.GetPositionY() << " " << near_npc.pos.GetPositionZ();
            out << "\nlastNearNpc: " << startT;
            out << "\nlastReachNpc: " << near_npc.lastReach;
            break;
        case NewRpgStatus::NEAR_RANDOM:
            out << "NEAR_RANDOM";
            out << "\nlastNearRandom: " << startT;
            break;
        case NewRpgStatus::IDLE:
            out << "IDLE";
            break;
        case NewRpgStatus::REST:
            out << "REST";
            out << "\nlastRest: " << startT;
            break;
        default:
            out << "UNKNOWN";
    }
    return out.str();
}