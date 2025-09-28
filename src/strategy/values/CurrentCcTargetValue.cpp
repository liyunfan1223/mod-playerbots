/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "CurrentCcTargetValue.h"

#include "Playerbots.h"

class FindCurrentCcTargetStrategy : public FindTargetStrategy
{
public:
    FindCurrentCcTargetStrategy(PlayerbotAI* botAI, std::string const spell) : FindTargetStrategy(botAI), spell(spell)
    {
    }

    void CheckAttacker(Unit* attacker, ThreatMgr* [[maybe_unused]] threatMgr) override
    {
        if (botAI->HasAura(spell, attacker))
            result = attacker;
    }

private:
    std::string const spell;
};

Unit* CurrentCcTargetValue::Calculate()
{
    FindCurrentCcTargetStrategy strategy(botAI, qualifier);
    return FindTarget(&strategy);
}