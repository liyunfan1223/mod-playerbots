/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TARGETVALUE_H
#define PLAYERBOTS_TARGETVALUE_H

#include "NamedObjectContext.h"
#include "TravelMgr.h"
#include "Value.h"

class PlayerbotAI;
class ThreatManager;
class Unit;
struct Position;
enum class TargetValueExclusionType : uint8;

GuidSet GatherStrategyTargetExclusions(PlayerbotAI* botAI, TargetValueExclusionType type);
bool HasEnoughAoeTargets(PlayerbotAI* botAI, Position const& center, float range, uint32 minCount);

class FindTargetStrategy
{
public:
    FindTargetStrategy(PlayerbotAI* botAI) : result(nullptr), botAI(botAI) {}

    Unit* GetResult();
    virtual void CheckAttacker(Unit* attacker, ThreatManager* threatMgr) = 0;
    void GetPlayerCount(Unit* creature, uint32* tankCount, uint32* dpsCount);
    bool IsHighPriority(Unit* attacker);

protected:
    Unit* result;
    PlayerbotAI* botAI;
    std::map<Unit*, uint32> tankCountCache;
    std::map<Unit*, uint32> dpsCountCache;
    bool foundHighPriority = false;
};

class FindNonCcTargetStrategy : public FindTargetStrategy
{
public:
    FindNonCcTargetStrategy(PlayerbotAI* botAI) : FindTargetStrategy(botAI) {}

protected:
    virtual bool IsCcTarget(Unit* attacker);
};

class TargetValue : public UnitCalculatedValue
{
public:
    TargetValue(PlayerbotAI* botAI, std::string const name = "target", int checkInterval = 1)
        : UnitCalculatedValue(botAI, name, checkInterval)
    {
    }

protected:
    Unit* FindTarget(FindTargetStrategy* strategy, GuidSet const& exclusions = {});
};

class RpgTargetValue : public ManualSetValue<GuidPosition>
{
public:
    RpgTargetValue(PlayerbotAI* botAI, std::string const name = "rpg target")
        : ManualSetValue<GuidPosition>(botAI, GuidPosition(), name)
    {
    }
};

class TravelTargetValue : public ManualSetValue<TravelTarget*>
{
public:
    TravelTargetValue(PlayerbotAI* botAI, std::string const name = "travel target")
        : ManualSetValue<TravelTarget*>(botAI, new TravelTarget(botAI), name)
    {
    }

    virtual ~TravelTargetValue() { delete value; }
};

class LastLongMoveValue : public CalculatedValue<WorldPosition>
{
public:
    LastLongMoveValue(PlayerbotAI* botAI)
        : CalculatedValue<WorldPosition>(botAI, "last long move", 30 * IN_MILLISECONDS) {}

    WorldPosition Calculate() override;
};

class HomeBindValue : public CalculatedValue<WorldPosition>
{
public:
    HomeBindValue(PlayerbotAI* botAI) : CalculatedValue<WorldPosition>(botAI, "home bind", 30 * IN_MILLISECONDS) {}

    WorldPosition Calculate() override;
};

class IgnoreRpgTargetValue : public ManualSetValue<GuidSet&>
{
public:
    IgnoreRpgTargetValue(PlayerbotAI* botAI) : ManualSetValue<GuidSet&>(botAI, data, "ignore rpg targets") {}

private:
    GuidSet data;
};

class TalkTargetValue : public ManualSetValue<ObjectGuid>
{
public:
    TalkTargetValue(PlayerbotAI* botAI, std::string const name = "talk target")
        : ManualSetValue<ObjectGuid>(botAI, ObjectGuid::Empty, name)
    {
    }
};

class PullTargetValue : public ManualSetValue<ObjectGuid>
{
public:
    PullTargetValue(PlayerbotAI* botAI, std::string const name = "pull target")
        : ManualSetValue<ObjectGuid>(botAI, ObjectGuid::Empty, name)
    {
    }
};

class PullStrategyTargetValue : public ManualSetValue<ObjectGuid>
{
public:
    PullStrategyTargetValue(PlayerbotAI* botAI, std::string const name = "pull strategy target")
        : ManualSetValue<ObjectGuid>(botAI, ObjectGuid::Empty, name)
    {
    }
};

class FindTargetValue : public UnitCalculatedValue, public Qualified
{
public:
    FindTargetValue(PlayerbotAI* botAI) : UnitCalculatedValue(botAI, "find target", /*2 * IN_MILLISECONDS*/ 1) {}

public:
    Unit* Calculate();
};

class FindBossTargetStrategy : public FindTargetStrategy
{
public:
    FindBossTargetStrategy(PlayerbotAI* botAI) : FindTargetStrategy(botAI) {}
    virtual void CheckAttacker(Unit* attacker, ThreatManager* threatManager);
};

class BossTargetValue : public TargetValue, public Qualified
{
public:
    BossTargetValue(PlayerbotAI* botAI) : TargetValue(botAI, "boss target", 2 * IN_MILLISECONDS) {}

public:
    Unit* Calculate();
};

class FocusHealTargetValue : public ManualSetValue<std::list<ObjectGuid>>
{
public:
    FocusHealTargetValue(PlayerbotAI* botAI) : ManualSetValue<std::list<ObjectGuid>>(botAI, {}, "focus heal targets") {}
};

#endif
