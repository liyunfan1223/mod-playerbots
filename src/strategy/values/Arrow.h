/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ARROW_H
#define _PLAYERBOT_ARROW_H

#include "Formations.h"
#include "TravelMgr.h"

class Player;
class PlayerbotAI;

class UnitPosition
{
public:
    UnitPosition(float x, float y) : x(x), y(y) {}
    UnitPosition(UnitPosition const& other)
    {
        x = other.x;
        y = other.y;
    }

    float x, y;
};

class FormationUnit
{
public:
    FormationUnit(uint32 groupIndex, bool master) : groupIndex(groupIndex), master(master), position(0, 0) {}
    FormationUnit(FormationUnit const& other) : position(other.position.x, other.position.y)
    {
        groupIndex = other.groupIndex;
        master = other.master;
    }

    uint32 GetGroupIdex() { return groupIndex; }
    void SetLocation(UnitPosition pos) { position = pos; }
    void SetLocation(float x, float y)
    {
        position.x = x;
        position.y = y;
    }
    float GetX() { return position.x; }
    float GetY() { return position.y; }

private:
    uint32 groupIndex;
    bool master;
    UnitPosition position;
};

class UnitPlacer
{
public:
    UnitPlacer() {}

    virtual UnitPosition Place(FormationUnit* unit, uint32 index, uint32 count) = 0;
};

class FormationSlot
{
public:
    FormationSlot() {}
    virtual ~FormationSlot();

    void AddLast(FormationUnit* unit) { units.push_back(unit); }
    void InsertAtCenter(FormationUnit* unit) { units.insert(units.begin() + (units.size() + 1) / 2, unit); }
    void PlaceUnits(UnitPlacer* placer);
    void Move(float dx, float dy);
    uint32 Size() const { return units.size(); }

private:
    WorldLocation center;
    std::vector<FormationUnit*> units;
};

class MultiLineUnitPlacer : public UnitPlacer
{
public:
    MultiLineUnitPlacer(float orientation) : UnitPlacer(), orientation(orientation) {}

    UnitPosition Place(FormationUnit* unit, uint32 index, uint32 count) override;

private:
    float orientation;
};

class SingleLineUnitPlacer
{
public:
    SingleLineUnitPlacer(float orientation) : orientation(orientation) {}

    virtual UnitPosition Place(FormationUnit* unit, uint32 index, uint32 count);

private:
    float orientation;
};

class ArrowFormation : public MoveAheadFormation
{
public:
    ArrowFormation(PlayerbotAI* botAI)
        : MoveAheadFormation(botAI, "arrow"), built(false), masterUnit(nullptr), botUnit(nullptr)
    {
    }

    WorldLocation GetLocationInternal() override;

private:
    void Build();
    void FillSlotsExceptMaster();
    void AddMasterToSlot();
    FormationSlot* FindSlot(Player* member);

private:
    FormationSlot tanks;
    FormationSlot melee;
    FormationSlot ranged;
    FormationSlot healers;
    FormationUnit* masterUnit;
    FormationUnit* botUnit;
    bool built;
};

#endif
