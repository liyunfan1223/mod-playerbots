/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "TankTargetValue.h"

#include "AttackersValue.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

class FindTargetForTankStrategy : public FindNonCcTargetStrategy
{
public:
    FindTargetForTankStrategy(PlayerbotAI* botAI) : FindNonCcTargetStrategy(botAI), minThreat(0) {}

    /*void CheckAttacker(Unit* creature, ThreatMgr* threatMgr) override
    {
        if (!creature || !creature->IsAlive())
        {
            return;
        }
        Player* bot = botAI->GetBot();
        float threat = threatMgr->GetThreat(bot);
        if (!result)
        {
            minThreat = threat;
            result = creature;
        }
        // neglect if victim is main tank, or no victim (for untauntable target)
        if (threatMgr->getCurrentVictim())
        {
            // float max_threat = threatMgr->GetThreat(threatMgr->getCurrentVictim()->getTarget());
            Unit* victim = threatMgr->getCurrentVictim()->getTarget();
            if (victim && victim->ToPlayer() && botAI->IsMainTank(victim->ToPlayer()))
            {
                return;
            }
        }
        if (minThreat >= threat)
        {
            minThreat = threat;
            result = creature;
        }
    }*/

    void CheckAttacker(Unit* creature, ThreatMgr* threatMgr) override
    {
        // [Crash fix] Filter out anything that is not ready/valid
        if (!creature || !creature->IsAlive() || !creature->IsInWorld() || creature->IsDuringRemoveFromWorld())
            return;
    
        if (!threatMgr)
            return;
    
        Player* bot = botAI->GetBot();
        if (!bot)
            return;
    
        float threat = threatMgr->GetThreat(bot);
    
        if (!result || !result->IsAlive() || !result->IsInWorld() || result->IsDuringRemoveFromWorld())
        {
            // [Crash fix] If the previous target has become invalid, restart cleanly
            minThreat = threat;
            result = creature;
        }
    
        // Neglect si la victime actuelle est le MT (ou s'il n'y a pas de victime)
        if (HostileReference* cv = threatMgr->getCurrentVictim())
        {
            Unit* victim = cv->getTarget();
            if (victim && victim->ToPlayer() && botAI->IsMainTank(victim->ToPlayer()))
                return;
        }
    
        if (minThreat >= threat)
        {
            minThreat = threat;
            result = creature;
        }
    }

protected:
    float minThreat;
};

class FindTankTargetSmartStrategy : public FindTargetStrategy
{
public:
    FindTankTargetSmartStrategy(PlayerbotAI* botAI) : FindTargetStrategy(botAI) {}

    /*void CheckAttacker(Unit* attacker, ThreatMgr* threatMgr) override
    {
        if (Group* group = botAI->GetBot()->GetGroup())
        {
            ObjectGuid guid = group->GetTargetIcon(4);
            if (guid && attacker->GetGUID() == guid)
                return;
        }
        if (!attacker->IsAlive())
        {
            return;
        }
        if (!result || IsBetter(attacker, result))
        {
            result = attacker;
        }
    }*/
	void CheckAttacker(Unit* attacker, ThreatMgr* /*threatMgr*/) override
    {
        // [Crash fix] Protect against null/out-of-world/being-removed units
        if (!attacker || !attacker->IsAlive() || !attacker->IsInWorld() || attacker->IsDuringRemoveFromWorld())
            return;
    
        if (Player* me = botAI->GetBot())
        {
            if (Group* group = me->GetGroup())
            {
                ObjectGuid guid = group->GetTargetIcon(4);
                if (guid && attacker->GetGUID() == guid)
                    return;
            }
        }
    
        // [Crash fix] If 'result' has become invalid, forget it
        if (result && (!result->IsAlive() || !result->IsInWorld() || result->IsDuringRemoveFromWorld()))
            result = nullptr;
    
        if (!result || IsBetter(attacker, result))
            result = attacker;
    }

    /*bool IsBetter(Unit* new_unit, Unit* old_unit)
    {
        Player* bot = botAI->GetBot();
        // if group has multiple tanks, main tank just focus on the current target
        Unit* currentTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
        if (currentTarget && botAI->IsMainTank(bot) && botAI->GetGroupTankNum(bot) > 1)
        {
            if (old_unit == currentTarget)
                return false;
            if (new_unit == currentTarget)
                return true;
        }
        float new_threat = new_unit->GetThreatMgr().GetThreat(bot);
        float old_threat = old_unit->GetThreatMgr().GetThreat(bot);
        float new_dis = bot->GetDistance(new_unit);
        float old_dis = bot->GetDistance(old_unit);
        // hasAggro? -> withinMelee? -> threat
        if (GetIntervalLevel(new_unit) != GetIntervalLevel(old_unit))
        {
            return GetIntervalLevel(new_unit) > GetIntervalLevel(old_unit);
        }
        int32_t interval = GetIntervalLevel(new_unit);
        if (interval == 2)
        {
            return new_dis < old_dis;
        }
        return new_threat < old_threat;
    }*/
	bool IsBetter(Unit* new_unit, Unit* old_unit)
    {
        // [Crash fix] If either one is invalid, decide straight away
        if (!new_unit || !new_unit->IsAlive() || !new_unit->IsInWorld() || new_unit->IsDuringRemoveFromWorld())
            return false;
        if (!old_unit || !old_unit->IsAlive() || !old_unit->IsInWorld() || old_unit->IsDuringRemoveFromWorld())
            return true;
    
        Player* bot = botAI->GetBot();
        if (!bot)
            return false;
    
        // if multiple tanks, logically focus on the current target
        Unit* currentTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
        if (currentTarget && botAI->IsMainTank(bot) && botAI->GetGroupTankNum(bot) > 1)
        {
            if (old_unit == currentTarget)
                return false;
            if (new_unit == currentTarget)
                return true;
        }
    
        float new_threat = new_unit->GetThreatMgr().GetThreat(bot);
        float old_threat = old_unit->GetThreatMgr().GetThreat(bot);
        float new_dis = bot->GetDistance(new_unit);
        float old_dis = bot->GetDistance(old_unit);
    
        // hasAggro? -> withinMelee? -> threat
        int nl = GetIntervalLevel(new_unit);
        int ol = GetIntervalLevel(old_unit);
        if (nl != ol)
            return nl > ol;
    
        if (nl == 2)
            return new_dis < old_dis;
    
        return new_threat < old_threat;
    }

    /*int32_t GetIntervalLevel(Unit* unit)
    {
        if (!botAI->HasAggro(unit))
        {
            return 2;
        }
        if (botAI->GetBot()->IsWithinMeleeRange(unit))
        {
            return 1;
        }
        return 0;
    }*/
	int32_t GetIntervalLevel(Unit* unit)
    {
        // [Crash fix] Basic guards
        if (!unit || !unit->IsAlive() || !unit->IsInWorld() || unit->IsDuringRemoveFromWorld())
            return 0;
    
        if (!botAI->HasAggro(unit))
            return 2;
    
        if (Player* bot = botAI->GetBot())
        {
            if (bot->IsWithinMeleeRange(unit))
                return 1;
        }
        return 0;
    }
};

Unit* TankTargetValue::Calculate()
{
    // [Note] Using the "smart" strategy below. Guards have been added in CheckAttacker/IsBetter.
    FindTankTargetSmartStrategy strategy(botAI);
    return FindTarget(&strategy);
}
