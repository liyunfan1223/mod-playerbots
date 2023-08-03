/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_TARGETVALUE_H
#define _PLAYERBOT_TARGETVALUE_H

#include "TravelMgr.h"
#include "Value.h"
#include "NamedObjectContext.h"

class PlayerbotAI;
class ThreatMgr;
class Unit;

class FindTargetStrategy
{
    public:
        FindTargetStrategy(PlayerbotAI* botAI) : result(nullptr), botAI(botAI) { }

        Unit* GetResult();
        virtual void  CheckAttacker(Unit* attacker, ThreatMgr* threatMgr) = 0;
        void GetPlayerCount(Unit* creature, uint32* tankCount, uint32* dpsCount);

    protected:
        Unit* result;
        PlayerbotAI* botAI;
        std::map<Unit*, uint32> tankCountCache;
        std::map<Unit*, uint32> dpsCountCache;
};

class FindNonCcTargetStrategy : public FindTargetStrategy
{
    public:
        FindNonCcTargetStrategy(PlayerbotAI* botAI) : FindTargetStrategy(botAI) { }

    protected:
        virtual bool IsCcTarget(Unit* attacker);
};

class TargetValue : public UnitCalculatedValue
{
	public:
        TargetValue(PlayerbotAI* botAI, std::string const name = "target", int checkInterval = 1) : UnitCalculatedValue(botAI, name, checkInterval) { }

    protected:
        Unit* FindTarget(FindTargetStrategy* strategy);
};

class RpgTargetValue : public ManualSetValue<GuidPosition>
{
    public:
        RpgTargetValue(PlayerbotAI* botAI, std::string const name = "rpg target") : ManualSetValue<GuidPosition>(botAI, GuidPosition(), name) { }
};

class TravelTargetValue : public ManualSetValue<TravelTarget*>
{
    public:
        TravelTargetValue(PlayerbotAI* botAI, std::string const name = "travel target") : ManualSetValue<TravelTarget*>(botAI, new TravelTarget(botAI), name) { }

        virtual ~TravelTargetValue() { delete value; }
};

class LastLongMoveValue : public CalculatedValue<WorldPosition>
{
    public:
        LastLongMoveValue(PlayerbotAI* botAI) : CalculatedValue<WorldPosition>(botAI, "last long move", 30 * 1000) { }

        WorldPosition Calculate() override;
};

class HomeBindValue : public CalculatedValue<WorldPosition>
{
    public:
        HomeBindValue(PlayerbotAI* botAI) : CalculatedValue<WorldPosition>(botAI, "home bind", 30 * 1000) { }

        WorldPosition Calculate() override;
};

class IgnoreRpgTargetValue : public ManualSetValue<GuidSet&>
{
    public:
        IgnoreRpgTargetValue(PlayerbotAI* botAI) : ManualSetValue<GuidSet&>(botAI, data, "ignore rpg targets") { }

    private:
        GuidSet data;
};

class TalkTargetValue : public ManualSetValue<ObjectGuid>
{
    public:
        TalkTargetValue(PlayerbotAI* botAI, std::string const name = "talk target") : ManualSetValue<ObjectGuid>(botAI, ObjectGuid::Empty, name) { }
};

class PullTargetValue : public ManualSetValue<ObjectGuid>
{
    public:
        PullTargetValue(PlayerbotAI* botAI, std::string const name = "pull target") : ManualSetValue<ObjectGuid>(botAI, ObjectGuid::Empty, name) { }
};

class FindTargetValue : public UnitCalculatedValue, public Qualified
{
public:
    FindTargetValue(PlayerbotAI* ai) : UnitCalculatedValue(ai, "find target", 2 * 1000) {}

public:
    Unit* Calculate();
};

class FindBossTargetStrategy : public FindTargetStrategy
    {
    public:
        FindBossTargetStrategy(PlayerbotAI* ai) : FindTargetStrategy(ai) {}
        virtual void CheckAttacker(Unit* attacker, ThreatMgr* threatManager);
    };

class BossTargetValue : public TargetValue, public Qualified
{
public:
    BossTargetValue(PlayerbotAI* ai) : TargetValue(ai, "boss target", 2 * 1000) {}

public:
    Unit* Calculate();
};
#endif
