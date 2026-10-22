/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "FoodActions.h"
#include "Event.h"
#include "Playerbots.h"

namespace
{
constexpr uint32 BG_WS_SPELL_WARSONG_FLAG = 23333;
constexpr uint32 BG_WS_SPELL_SILVERWING_FLAG = 23335;
constexpr uint32 BG_EY_NETHERSTORM_FLAG_SPELL = 34976;

// Graccu's Mince Meat Fruitcake lasts 20s and restores 5% health and mana every 2s.
constexpr uint32 SPELL_GRACCUS_MINCE_MEAT_FRUITCAKE = 25990;
constexpr uint32 RECOVERY_MAX_MS = 20 * IN_MILLISECONDS;

bool IsFullMana(Player* bot)
{
    return bot->GetPower(POWER_MANA) >= bot->GetMaxPower(POWER_MANA);
}

// The cheat aura restores both health and mana, so allow bots to eat/drink until both are restored.
uint32 FoodCheatRecoveryDurationMs(Player* bot)
{
    float missingShare = 1.0f - static_cast<float>(bot->GetHealth()) / bot->GetMaxHealth();

    if (uint32 const maxMana = bot->GetMaxPower(POWER_MANA))
    {
        float const manaMissing = 1.0f - static_cast<float>(bot->GetPower(POWER_MANA)) / maxMana;
        if (manaMissing > missingShare)
            missingShare = manaMissing;
    }

    return static_cast<uint32>(RECOVERY_MAX_MS * missingShare);
}

uint32 StartFoodCheatRecovery(Player* bot)
{
    bot->ClearUnitState(UNIT_STATE_CHASE);
    bot->ClearUnitState(UNIT_STATE_FOLLOW);

    if (bot->isMoving())
        bot->StopMoving();

    bot->SetStandState(UNIT_STAND_STATE_SIT);
    bot->CastStop();

    uint32 const durationMs = FoodCheatRecoveryDurationMs(bot);
    bot->AddAura(SPELL_GRACCUS_MINCE_MEAT_FRUITCAKE, bot);
    return durationMs;
}

bool IsDisallowedShapeshiftForm(Player* bot)
{
    if (bot->getClass() == CLASS_DRUID)
    {
        ShapeshiftForm form = bot->GetShapeshiftForm();
        return form == FORM_TRAVEL || form == FORM_AQUA || form == FORM_CAT || form == FORM_BEAR ||
            form == FORM_DIREBEAR || form == FORM_FLIGHT || form == FORM_FLIGHT_EPIC;
    }

    return bot->getClass() == CLASS_PRIEST && bot->GetShapeshiftForm() == FORM_SPIRITOFREDEMPTION;
}
} // end anonymous namespace

bool DrinkAction::Execute(Event event)
{
    if (botAI->HasCheat(BotCheatMask::food))
    {
        SetDuration(StartFoodCheatRecovery(bot));
        return true;
    }

    return UseItemAction::Execute(event);
}

bool DrinkAction::isUseful()
{
    if (!bot->GetMaxPower(POWER_MANA)) // no mana pool to refill
        return false;

    return !IsFullMana(bot) || (botAI->HasCheat(BotCheatMask::food) && !bot->IsFullHealth());
}

bool DrinkAction::isPossible()
{
    if (bot->IsInCombat() || bot->IsMounted() || IsDisallowedShapeshiftForm(bot))
        return false;

    if (bot->HasAura(BG_WS_SPELL_WARSONG_FLAG) || bot->HasAura(BG_WS_SPELL_SILVERWING_FLAG) ||
        bot->HasAura(BG_EY_NETHERSTORM_FLAG_SPELL))
    {
        return false;
    }

    return botAI->HasCheat(BotCheatMask::food) || UseItemAction::isPossible();
}

bool EatAction::Execute(Event event)
{
    if (botAI->HasCheat(BotCheatMask::food))
    {
        SetDuration(StartFoodCheatRecovery(bot));
        return true;
    }

    return UseItemAction::Execute(event);
}

bool EatAction::isUseful()
{
    return !bot->IsFullHealth() || (botAI->HasCheat(BotCheatMask::food) && !IsFullMana(bot));
}

bool EatAction::isPossible()
{
    if (bot->IsInCombat() || bot->IsMounted() || IsDisallowedShapeshiftForm(bot))
        return false;

    if (bot->HasAura(BG_WS_SPELL_WARSONG_FLAG) || bot->HasAura(BG_WS_SPELL_SILVERWING_FLAG) ||
        bot->HasAura(BG_EY_NETHERSTORM_FLAG_SPELL))
    {
        return false;
    }

    return botAI->HasCheat(BotCheatMask::food) || UseItemAction::isPossible();
}
