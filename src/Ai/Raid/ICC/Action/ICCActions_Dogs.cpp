/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GenericSpellActions.h"
#include "ICCActions.h"
#include "ICCTriggers.h"
#include "Multiplier.h"
#include "Playerbots.h"

bool IccDogsTankPositionAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "stinky");
    if (!boss)
        boss = AI_VALUE2(Unit*, "find target", "precious");
    if (!boss)
        return false;

    if (PlayerbotAI::IsTank(bot))
    {
        Aura* aura = botAI->GetAura("mortal wound", bot, false, true);
        bool const hasMortalWound = aura && aura->GetStackAmount() >= 8;

        if (hasMortalWound)
        {
            if (bot->GetVictim() == boss)
                bot->AttackStop();

            return true;
        }

        // Tank without high mortal wound stacks: taunt boss if current tank has the debuff
        Unit* currentTarget = boss->GetVictim();
        if (currentTarget && currentTarget != bot)
        {
            Aura* victimAura = botAI->GetAura("mortal wound", currentTarget, false, true);
            if (victimAura && victimAura->GetStackAmount() >= 8)
                IccCastClassTaunt(bot, botAI,boss);
        }

        // Taunt nearby hostile adds not targeting a tank
        GuidVector const npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
        for (auto const& npc : npcs)
        {
            Unit* unit = botAI->GetUnit(npc);
            if (!unit || !unit->IsAlive())
                continue;

            if (unit == boss)
                continue;

            if (bot->GetDistance2d(unit) > 20.0f)
                continue;

            Unit* victim = unit->GetVictim();
            Player* victimPlayer = victim ? victim->ToPlayer() : nullptr;
            if (!victimPlayer || !PlayerbotAI::IsTank(victimPlayer))
            {
                IccCastClassTaunt(bot, botAI,unit);
                break;
            }
        }

        return false;
    }

    return false;
}
