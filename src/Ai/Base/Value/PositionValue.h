/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_POSITIONVALUE_H
#define PLAYERBOTS_POSITIONVALUE_H

#include "NamedObjectContext.h"
#include "TravelMgr.h"
#include "Value.h"

class PlayerbotAI;

class PositionInfo
{
public:
    PositionInfo() : x(0), y(0), z(0), mapId(0), valueSet(false) {}
    PositionInfo(float x, float y, float z, uint32 mapId, bool valueSet = true)
        : x(x), y(y), z(z), mapId(mapId), valueSet(valueSet)
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
    uint32 mapId;
    bool valueSet;
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
        return value.fDist(lastValue) < sPlayerbotAIConfig.TooCloseDistance;
    }

    WorldPosition Calculate() override;
};

class SinglePositionValue : public CalculatedValue<PositionInfo>, public Qualified
{
public:
    SinglePositionValue(PlayerbotAI* botAI, std::string name = "pos") : CalculatedValue(botAI, name), Qualified() {};
    virtual PositionInfo Calculate() override;
    virtual void Set(PositionInfo value) override;
    virtual void Reset() override;
};

#endif
