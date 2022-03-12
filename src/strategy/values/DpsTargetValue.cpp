/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "DpsTargetValue.h"
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

            if (!result || result->GetHealth() > attacker->GetHealth())
                result = attacker;
        }

    protected:
        float minHealth;
};

Unit* DpsTargetValue::Calculate()
{
    Unit* rti = RtiTargetValue::Calculate();
    if (rti)
        return rti;

    FindLeastHpTargetStrategy strategy(botAI);
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
