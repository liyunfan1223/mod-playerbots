/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RampMultipliers.h"
#include "EncounterHelpers.h"
#include "PaladinActions.h"
#include "Playerbots.h"
#include "RampActions.h"
#include "RampBossHelper.h"
#include "RampShared.h"
#include "ReachTargetActions.h"
#include "ShamanActions.h"

using namespace EncounterHelpers;
using namespace RampShared;

// Omor the Unscarred

float OmorTreacheryAuraFleeFromPlayersMultiplier::GetValue(Action* action)
{
    // Allow all Omor fight specific actions & any flee/runaway actions
    if (dynamic_cast<OmorRangedSpreadAction*>(action) ||
        dynamic_cast<OmorTreacheryAuraFleeFromPlayersAction*>(action) ||
        dynamic_cast<OmorTreacheryAuraFleeFromTankAction*>(action))
        return 1.0f;

    bool const isMovementSpell =
        dynamic_cast<CastReachTargetSpellAction*>(action) ||
        dynamic_cast<ReachTargetAction*>(action);

    // Allow non-movement based Actions
    if (!isMovementSpell && !dynamic_cast<MovementAction*>(action))
        return 1.0f;

    Unit* omor = AI_VALUE2(Unit*, "find target", "omor the unscarred");

    // If Omor isn't found, allow all actions
    if (!omor)
        return 1.0f;

    // Let non-melee bots do anything
    if (PlayerbotAI::IsMainTank(bot) ||
        PlayerbotAI::IsRanged(bot))
        return 1.0f;

    // Melee bots try to kill themselves more so need more logic
    // If melee bot has Aura - don't attack
    if (helper.HasTreacheryAura(bot))
        return 0.0f;

    // If tank exists and has Aura - don't attack
    Player* tank = GetGroupMainTank(bot);
    if (tank && helper.HasTreacheryAura(tank))
        return 0.0f;

    // Edge-case to try and prevent wipes if there is no main tank, or Omor is focusing a non-tank - don't attack
    Unit* omorVictim = omor->GetVictim();
    if (!omorVictim)
        return 1.0f;

    Player* victimPlayer = dynamic_cast<Player*>(omorVictim);
    if (victimPlayer && helper.HasTreacheryAura(victimPlayer))
        return 0.0f;

    // It should be safe to attack
    return 1.0f;
}

// Vazruden & Nazan

float NazanSetTremorTotemMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!dynamic_cast<CastStrengthOfEarthTotemAction*>(action) &&
        !dynamic_cast<CastStoneskinTotemAction*>(action) &&
        !dynamic_cast<CastStoneclawTotemAction*>(action) &&
        !dynamic_cast<CastEarthbindTotemAction*>(action))
    {
        return 1.0f;
    }

    Unit* nazan = AI_VALUE2(Unit*, "find target", "nazan");
    return nazan && !nazan->IsFlying() ? 0.0f : 1.0f;
}

float NazanSetFireResistanceTotemMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!dynamic_cast<CastCleansingTotemAction*>(action) &&
        !dynamic_cast<CastHealingStreamTotemAction*>(action) &&
        !dynamic_cast<CastManaSpringTotemAction*>(action) &&
        !dynamic_cast<CastManaTideTotemAction*>(action))
    {
        return 1.0f;
    }

    Unit* nazan = AI_VALUE2(Unit*, "find target", "nazan");
    return nazan && !nazan->IsFlying() ? 0.0f : 1.0f;
}

float NazanSetFireResistanceAuraMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_PALADIN)
        return 1.0f;

    if (!dynamic_cast<CastDevotionAuraAction*>(action) &&
        !dynamic_cast<CastConcentrationAuraAction*>(action) &&
        !dynamic_cast<CastShadowResistanceAuraAction*>(action) &&
        !dynamic_cast<CastRetributionAuraAction*>(action) &&
        !dynamic_cast<CastFrostResistanceAuraAction*>(action) &&
        !dynamic_cast<CastCrusaderAuraAction*>(action) &&
        !dynamic_cast<CastSanctityAuraAction*>(action))
    {
        return 1.0f;
    }

    Unit* nazan = AI_VALUE2(Unit*, "find target", "nazan");
    return nazan && !nazan->IsFlying() ? 0.0f : 1.0f;
}
