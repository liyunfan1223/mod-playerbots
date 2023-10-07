/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "AttackersValue.h"
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
            float dist = creature->GetDistance(bot);
            if (!result) {
                minDistance = dist;
                result = creature;
            }
            // neglect if victim is main tank, or no victim (for untauntable target)
            if (threatMgr->getCurrentVictim()) {
                // float max_threat = threatMgr->GetThreat(threatMgr->getCurrentVictim()->getTarget());
                Unit* victim = threatMgr->getCurrentVictim()->getTarget();
                if (victim && victim->ToPlayer() && botAI->IsTank(victim->ToPlayer())) {
                    return;
                }
            }
            if (minDistance >= dist || badResult)
            {
                badResult = false;
                minDistance = dist;
                result = creature;
            }
        }

    protected:
        float minThreat;
        float minDistance;
        bool badResult = true;
};

Unit* TankTargetValue::Calculate()
{
    FindTargetForTankStrategy strategy(botAI);
    return FindTarget(&strategy);
}
