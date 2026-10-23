/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "Arrow.h"
#include "Group.h"
#include "Map.h"
#include "PlayerbotAI.h"

WorldLocation ArrowFormation::GetLocationInternal()
{
    if (!bot->GetGroup())
        return Formation::NullLocation;

    Build();

    uint32 tankLines = 1 + tanks.Size() / 6;
    uint32 meleeLines = 1 + melee.Size() / 6;
    uint32 rangedLines = 1 + ranged.Size() / 6;
    //@TODO Implement Healer Lines
    //uint32 healerLines = 1 + healers.Size() / 6;
    float offset = 0.f;

    Player* master = botAI->GetMaster();
    if (!botAI->IsSafe(master))
        return Formation::NullLocation;

    float orientation = master->GetOrientation();
    MultiLineUnitPlacer placer(orientation);

    tanks.PlaceUnits(&placer);
    tanks.Move(-cos(orientation) * offset, -sin(orientation) * offset);

    offset += tankLines * sPlayerbotAIConfig.FollowDistance + sPlayerbotAIConfig.TooCloseDistance / 2;
    melee.PlaceUnits(&placer);
    melee.Move(-cos(orientation) * offset, -sin(orientation) * offset);

    offset += meleeLines * sPlayerbotAIConfig.FollowDistance + sPlayerbotAIConfig.TooCloseDistance / 2;
    ranged.PlaceUnits(&placer);
    ranged.Move(-cos(orientation) * offset, -sin(orientation) * offset);

    offset += rangedLines * sPlayerbotAIConfig.FollowDistance;
    healers.PlaceUnits(&placer);
    healers.Move(-cos(orientation) * offset, -sin(orientation) * offset);

    if (!masterUnit || !botUnit)
        return Formation::NullLocation;

    float x = master->GetPositionX() - masterUnit->GetX() + botUnit->GetX();
    float y = master->GetPositionY() - masterUnit->GetY() + botUnit->GetY();
    float z = master->GetPositionZ() + master->GetHoverHeight();
    if (!master->GetMap()->CheckCollisionAndGetValidCoords(master, master->GetPositionX(), master->GetPositionY(),
                                                           master->GetPositionZ(), x, y, z))
    {
        x = master->GetPositionX() - masterUnit->GetX() + botUnit->GetX();
        y = master->GetPositionY() - masterUnit->GetY() + botUnit->GetY();
        z = master->GetPositionZ() + master->GetHoverHeight();
        master->UpdateAllowedPositionZ(x, y, z);
    }
    return WorldLocation(master->GetMapId(), x, y, z);
}

void ArrowFormation::Build()
{
    if (built)
        return;

    FillSlotsExceptMaster();
    AddMasterToSlot();

    built = true;
}

FormationSlot* ArrowFormation::FindSlot(Player* member)
{
    if (PlayerbotAI::IsTank(member))
        return &tanks;
    else if (PlayerbotAI::IsHeal(member))
        return &healers;
    else if (PlayerbotAI::IsRanged(member))
        return &ranged;
    else
        return &melee;
}

void ArrowFormation::FillSlotsExceptMaster()
{
    Group* group = bot->GetGroup();
    GroupReference* gref = group->GetFirstMember();
    uint32 index = 0;
    while (gref)
    {
        Player* member = gref->GetSource();
        if (botAI->IsSafe(member))
        {
            if (member == bot)
                FindSlot(member)->AddLast(botUnit = new FormationUnit(index, false));
            else if (member != botAI->GetMaster())
                FindSlot(member)->AddLast(new FormationUnit(index, false));
            ++index;
        }
        gref = gref->next();
    }
}

void ArrowFormation::AddMasterToSlot()
{
    Group* group = bot->GetGroup();
    GroupReference* gref = group->GetFirstMember();
    uint32 index = 0;
    while (gref)
    {
        Player* member = gref->GetSource();

        if (member == botAI->GetMaster())
        {
            FindSlot(member)->InsertAtCenter(masterUnit = new FormationUnit(index, true));
            break;
        }

        gref = gref->next();
        ++index;
    }
}

void FormationSlot::PlaceUnits(UnitPlacer* placer)
{
    uint32 index = 0;
    uint32 count = units.size();
    for (FormationUnit* unit : units)
    {
        unit->SetLocation(placer->Place(unit, index, count));
        ++index;
    }
}

UnitPosition MultiLineUnitPlacer::Place(FormationUnit* unit, uint32 index, uint32 count)
{
    SingleLineUnitPlacer placer(orientation);
    if (count <= 6)
        return placer.Place(unit, index, count);

    uint32 lineNo = index / 6;
    uint32 indexInLine = index % 6;
    uint32 lineSize = std::max(count - lineNo * 6, uint32(6));
    return placer.Place(unit, indexInLine, lineSize);
}

UnitPosition SingleLineUnitPlacer::Place(FormationUnit* /*unit*/, uint32 index, uint32 count)
{
    float angle = orientation - M_PI / 2.0f;
    float x = cos(angle) * sPlayerbotAIConfig.FollowDistance * ((float)index - (float)count / 2);
    float y = sin(angle) * sPlayerbotAIConfig.FollowDistance * ((float)index - (float)count / 2);
    return UnitPosition(x, y);
}

void FormationSlot::Move(float dx, float dy)
{
    for (FormationUnit* unit : units)
        unit->SetLocation(unit->GetX() + dx, unit->GetY() + dy);
}

FormationSlot::~FormationSlot()
{
    for (FormationUnit* unit : units)
        delete unit;

    units.clear();
}
