/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_NEARESTNPCSVALUE_H
#define PLAYERBOTS_NEARESTNPCSVALUE_H

#include "NearestUnitsValue.h"
#include "PlayerbotAIConfig.h"

class PlayerbotAI;

class NearestNpcsValue : public NearestUnitsValue
{
public:
    NearestNpcsValue(PlayerbotAI* botAI, float range = sPlayerbotAIConfig.SightDistance)
        : NearestUnitsValue(botAI, "nearest npcs", range)
    {
    }

protected:
    void FindUnits(std::list<Unit*>& targets) override;
    bool AcceptUnit(Unit* unit) override;
};

class NearestHostileNpcsValue : public NearestUnitsValue
{
public:
    NearestHostileNpcsValue(PlayerbotAI* botAI, float range = sPlayerbotAIConfig.SightDistance)
        : NearestUnitsValue(botAI, "nearest hostile npcs", range)
    {
    }

protected:
    void FindUnits(std::list<Unit*>& targets) override;
    bool AcceptUnit(Unit* unit) override;
};

class NearestVehiclesValue : public NearestUnitsValue
{
public:
    NearestVehiclesValue(PlayerbotAI* botAI, float range = sPlayerbotAIConfig.SightDistance)
        : NearestUnitsValue(botAI, "nearest vehicles", range)
    {
    }

protected:
    void FindUnits(std::list<Unit*>& targets) override;
    bool AcceptUnit(Unit* unit) override;
};

class NearestTriggersValue : public NearestUnitsValue
{
public:
    NearestTriggersValue(PlayerbotAI* botAI, float range = sPlayerbotAIConfig.SightDistance)
        : NearestUnitsValue(botAI, "nearest triggers", range)
    {
    }

protected:
    void FindUnits(std::list<Unit*>& targets) override;
    bool AcceptUnit(Unit* unit) override;
};

class NearestTotemsValue : public NearestUnitsValue
{
public:
    NearestTotemsValue(PlayerbotAI* botAI, float range = 30.0f)
        : NearestUnitsValue(botAI, "nearest totems", range, true)
    {
    }

protected:
    void FindUnits(std::list<Unit*>& targets) override;
    bool AcceptUnit(Unit* unit) override;
};

#endif
