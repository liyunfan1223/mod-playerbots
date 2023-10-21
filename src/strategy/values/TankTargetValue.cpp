/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "AttackersValue.h"
#include "PlayerbotAIConfig.h"
#include "TankTargetValue.h"
#include "Playerbots.h"

class FindTargetForTankStrategy : public FindNonCcTargetStrategy
{
    public:
        FindTargetForTankStrategy(PlayerbotAI* botAI) : FindNonCcTargetStrategy(botAI), minThreat(0) { }

        void CheckAttacker(Unit* creature, ThreatMgr* threatMgr) override
        {
            if (!creature || !creature->IsAlive()) {
                return;
            }
            Player* bot = botAI->GetBot();
            float threat = threatMgr->GetThreat(bot);
            if (!result) {
                minThreat = threat;
                result = creature;
            }
            // neglect if victim is main tank, or no victim (for untauntable target)
            if (threatMgr->getCurrentVictim()) {
                // float max_threat = threatMgr->GetThreat(threatMgr->getCurrentVictim()->getTarget());
                Unit* victim = threatMgr->getCurrentVictim()->getTarget();
                if (victim && victim->ToPlayer() && botAI->IsMainTank(victim->ToPlayer())) {
                    return;
                }
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
        FindTankTargetSmartStrategy(PlayerbotAI* botAI) : FindTargetStrategy(botAI) { }

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
            if (!result || IsBetter(attacker, result)) {
                result = attacker;
            }
        }
        bool IsBetter(Unit* new_unit, Unit* old_unit) {
            Player* bot = botAI->GetBot();
            float new_threat = new_unit->GetThreatMgr().GetThreat(bot);
            float old_threat = old_unit->GetThreatMgr().GetThreat(bot);
            float new_dis = bot->GetDistance(new_unit);
            float old_dis = bot->GetDistance(old_unit);
            // hasAggro? -> withinMelee? -> threat
            if (GetIntervalLevel(new_unit) > GetIntervalLevel(old_unit)) {
                return true;
            }
            int32_t interval = GetIntervalLevel(new_unit);
            if (interval == 1) {
                return new_dis < old_dis;
            }
            return new_threat < old_threat;
        }
        int32_t GetIntervalLevel(Unit* unit) {
            if (!botAI->HasAggro(unit)) {
                return 1;
            }
            return 0;
        }
};

Unit* TankTargetValue::Calculate()
{
    // FindTargetForTankStrategy strategy(botAI);
    FindTankTargetSmartStrategy strategy(botAI);
    return FindTarget(&strategy);
}
