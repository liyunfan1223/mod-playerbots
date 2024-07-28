/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_POSITIONVALUE_H
#define _PLAYERBOT_POSITIONVALUE_H

#include "TravelMgr.h"
#include "Value.h"

class PlayerbotAI;

class PositionInfo
{
public:
    PositionInfo() : valueSet(false), x(0), y(0), z(0), mapId(0) {}
    PositionInfo(PositionInfo const& other)
        : valueSet(other.valueSet), x(other.x), y(other.y), z(other.z), mapId(other.mapId)
    {
    }

    void Set(float newX, float newY, float newZ, uint32 newMapId)
    {
        x = newX;
        y = newY;
        z = newZ;
        mapId = newMapId;
        valueSet = true;
    }

    void Reset() { valueSet = false; }

    bool isSet() { return valueSet; }

    float x;
    float y;
    float z;
    bool valueSet;
    uint32 mapId;
};

typedef std::map<std::string, PositionInfo> PositionMap;

class PositionValue : public ManualSetValue<PositionMap&>
{
public:
    PositionValue(PlayerbotAI* botAI, std::string const name = "position");

    std::string const Save() override;
    bool Load(std::string const value) override;

private:
    PositionMap positions;
};

class CurrentPositionValue : public LogCalculatedValue<WorldPosition>
{
public:
    CurrentPositionValue(PlayerbotAI* botAI, std::string const name = "current position", uint32 checkInterval = 1)
        : LogCalculatedValue<WorldPosition>(botAI, name, checkInterval)
    {
        minChangeInterval = 60;
        logLength = 30;
    };

    bool EqualToLast(WorldPosition value) override
    {
        return value.fDist(lastValue) < sPlayerbotAIConfig->tooCloseDistance;
    }

    WorldPosition Calculate() override;
};

#endif
