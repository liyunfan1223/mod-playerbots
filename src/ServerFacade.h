/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_SERVERFACADE_H
#define _PLAYERBOT_SERVERFACADE_H

#include "Common.h"

class Player;
class Unit;
class WorldObject;
class WorldPacket;

class ServerFacade
{
public:
    ServerFacade(){};
    virtual ~ServerFacade(){};
    static ServerFacade* instance()
    {
        static ServerFacade instance;
        return &instance;
    }

public:
    float GetDistance2d(Unit* unit, WorldObject* wo);
    float GetDistance2d(Unit* unit, float x, float y);
    bool IsDistanceLessThan(float dist1, float dist2);
    bool IsDistanceGreaterThan(float dist1, float dist2);
    bool IsDistanceGreaterOrEqualThan(float dist1, float dist2);
    bool IsDistanceLessOrEqualThan(float dist1, float dist2);

    void SetFacingTo(Player* bot, WorldObject* wo, bool force = false);
    Unit* GetChaseTarget(Unit* target);

    void SendPacket(Player *player, WorldPacket* packet);
};

#define sServerFacade ServerFacade::instance()

#endif
