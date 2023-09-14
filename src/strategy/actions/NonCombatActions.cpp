/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "NonCombatActions.h"
#include "Event.h"
#include "Playerbots.h"

bool DrinkAction::Execute(Event event)
{
    if (bot->IsInCombat())
        return false;

    bool hasMana = AI_VALUE2(bool, "has mana", "self target");
    if (!hasMana)
        return false;

    if (sPlayerbotAIConfig->freeFood)
    {
        
        // if (bot->IsNonMeleeSpellCast(true))
        //     return false;

        bot->ClearUnitState(UNIT_STATE_CHASE);
        bot->ClearUnitState(UNIT_STATE_FOLLOW);

        if (bot->isMoving())
        {
            // bot->StopMoving();
            // botAI->SetNextCheckDelay(sPlayerbotAIConfig->globalCoolDown);
            return false;
        }
        bot->SetStandState(UNIT_STAND_STATE_SIT);
        botAI->InterruptSpell();

        //float hp = bot->GetHealthPercent();
        float mp = bot->GetPowerPct(POWER_MANA);
        float p = mp;
        float delay;

        if (!bot->InBattleground())
            delay = 27000.0f * (100 - p) / 100.0f;
        else
            delay = 20000.0f * (100 - p) / 100.0f;

        botAI->SetNextCheckDelay(delay);
        
        bot->AddAura(24707, bot);
        return true;
        // return botAI->CastSpell(24707, bot);
    }

    return UseItemAction::Execute(event);
}

bool DrinkAction::isUseful()
{
    return UseItemAction::isUseful() && AI_VALUE2(uint8, "mana", "self target") < 85;
}

bool DrinkAction::isPossible()
{
    return !bot->IsInCombat() && (sPlayerbotAIConfig->freeFood || UseItemAction::isPossible());
}

bool EatAction::Execute(Event event)
{
    if (bot->IsInCombat())
        return false;

    if (sPlayerbotAIConfig->freeFood)
    {
        // if (bot->IsNonMeleeSpellCast(true))
        //     return false;

        bot->ClearUnitState(UNIT_STATE_CHASE);
        bot->ClearUnitState(UNIT_STATE_FOLLOW);

        if (bot->isMoving())
        {
            // bot->StopMoving();
            // botAI->SetNextCheckDelay(sPlayerbotAIConfig->globalCoolDown);
            return false;
        }

        bot->SetStandState(UNIT_STAND_STATE_SIT);
        botAI->InterruptSpell();

        float hp = bot->GetHealthPct();
        //float mp = bot->HasMana() ? bot->GetPowerPercent() : 0.f;
        float p = hp;
        float delay;

        if (!bot->InBattleground())
            delay = 27000.0f * (100 - p) / 100.0f;
        else
            delay = 20000.0f * (100 - p) / 100.0f;

        botAI->SetNextCheckDelay(delay);
        
        bot->AddAura(24707, bot);
        return true;
    }

    return UseItemAction::Execute(event);
}

bool EatAction::isUseful()
{ 
    return UseItemAction::isUseful() && AI_VALUE2(uint8, "health", "self target") < 85;
}

bool EatAction::isPossible()
{
    return !bot->IsInCombat() && (sPlayerbotAIConfig->freeFood || UseItemAction::isPossible());
}
