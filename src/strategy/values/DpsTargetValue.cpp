/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "DpsTargetValue.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

class FindLeastHpTargetStrategy : public FindTargetStrategy
{
    public:
        FindLeastHpTargetStrategy(PlayerbotAI* botAI) : FindTargetStrategy(botAI), minHealth(0) { }

        void CheckAttacker(Unit* attacker, ThreatMgr* threatMgr) override
        {
            if (Group* group = botAI->GetBot()->GetGroup())
            {
                ObjectGuid guid = group->GetTargetIcon(4);
                if (guid && attacker->GetGUID() == guid)
                    return;
            }
            if (!attacker->IsAlive()) {
                return;
            }
            if (!result || result->GetHealth() > attacker->GetHealth())
                result = attacker;
        }

    protected:
        float minHealth;
};

class FindMaxThreatGapTargetStrategy : public FindTargetStrategy
{
    public:
        FindMaxThreatGapTargetStrategy(PlayerbotAI* botAI) : FindTargetStrategy(botAI), minThreat(0) { }

        void CheckAttacker(Unit* attacker, ThreatMgr* threatMgr) override
        {
            if (Group* group = botAI->GetBot()->GetGroup())
            {
                ObjectGuid guid = group->GetTargetIcon(4);
                if (guid && attacker->GetGUID() == guid)
                    return;
            }
            if (!attacker->IsAlive()) {
                return;
            }
            Unit* victim = attacker->GetVictim();
            if (!result || CalcThreatGap(attacker, threatMgr) > CalcThreatGap(result, &result->GetThreatMgr()))
                result = attacker;
        }
        float CalcThreatGap(Unit* attacker, ThreatMgr* threatMgr) {
            Unit* victim = attacker->GetVictim();
            return threatMgr->GetThreat(victim) - threatMgr->GetThreat(attacker); 
        }

    protected:
        float minThreat;
};

class FindTargetSmartStrategy : public FindTargetStrategy
{
    public:
        FindTargetSmartStrategy(PlayerbotAI* botAI, float dps) : FindTargetStrategy(botAI), dps_(dps), targetExpectedLifeTime(1000000) { }

        void CheckAttacker(Unit* attacker, ThreatMgr* threatMgr) override
        {
            if (Group* group = botAI->GetBot()->GetGroup())
            {
                ObjectGuid guid = group->GetTargetIcon(4);
                if (guid && attacker->GetGUID() == guid)
                    return;
            }
            if (!attacker->IsAlive()) {
                return;
            }
            float expectedLifeTime = attacker->GetHealth() / dps_;
            // Unit* victim = attacker->GetVictim();
            if (!result || IsBetter(attacker, result)) {
                targetExpectedLifeTime = expectedLifeTime;
                result = attacker;
            }
        }
        bool IsBetter(Unit* new_unit, Unit* old_unit) {
            float new_time = new_unit->GetHealth() / dps_;
            float old_time = old_unit->GetHealth() / dps_;
            // [5-20] > (5-0] > (20-inf)
            if (GetIntervalLevel(new_unit) > GetIntervalLevel(old_unit)) {
                return true;
            }
            int32_t level = GetIntervalLevel(new_unit);
            if (level % 10 == 2 || level % 10 == 0) {
                return new_time < old_time;
            }
            // dont switch targets when all of them with low health
            Unit* currentTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
            if (currentTarget == new_unit) {
                return true;
            }
            if (currentTarget == old_unit) {
                return false;
            }
            return new_time > old_time;
        }
        int32_t GetIntervalLevel(Unit* unit) {
            float time = unit->GetHealth() / dps_;
            float dis = unit->GetDistance(botAI->GetBot());
            float attackRange = botAI->IsRanged(botAI->GetBot()) ? sPlayerbotAIConfig->spellDistance : sPlayerbotAIConfig->meleeDistance;
            attackRange += 5.0f;
            int level = dis < attackRange ? 10 : 0;
            if (time >= 5 && time <= 20) {
                return level + 2;
            }
            if (time < 5) {
                return level + 1;
            }
            return level;
        }

    protected:
        float dps_;
        float targetExpectedLifeTime;
};

Unit* DpsTargetValue::Calculate()
{
    Unit* rti = RtiTargetValue::Calculate();
    if (rti)
        return rti;

    // FindLeastHpTargetStrategy strategy(botAI);
    float dps = AI_VALUE(float, "expected group dps");
    FindTargetSmartStrategy strategy(botAI, dps);
    // FindMaxThreatGapTargetStrategy strategy(botAI);
    return TargetValue::FindTarget(&strategy);
}

class FindMaxHpTargetStrategy : public FindTargetStrategy
{
    public:
        FindMaxHpTargetStrategy(PlayerbotAI* botAI) : FindTargetStrategy(botAI), maxHealth(0) { }

        void CheckAttacker(Unit* attacker, ThreatMgr* threatMgr) override
        {
            if (Group* group = botAI->GetBot()->GetGroup())
            {
                ObjectGuid guid = group->GetTargetIcon(4);
                if (guid && attacker->GetGUID() == guid)
                    return;
            }

            if (!result || result->GetHealth() < attacker->GetHealth())
                result = attacker;
        }

    protected:
        float maxHealth;
};

Unit* DpsAoeTargetValue::Calculate()
{
    Unit* rti = RtiTargetValue::Calculate();
    if (rti)
        return rti;

    FindMaxHpTargetStrategy strategy(botAI);
    return TargetValue::FindTarget(&strategy);
}
