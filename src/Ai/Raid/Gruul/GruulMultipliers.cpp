/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GruulMultipliers.h"
#include "ChooseTargetActions.h"
#include "EncounterHelpers.h"
#include "GruulActions.h"
#include "GruulHelpers.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"

using namespace GruulHelpers;
using namespace EncounterHelpers;

// General

float GruulsLairDelayDpsCooldownsMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!IsDpsCooldownAction(bot, action))
        return 1.0f;

    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
    if (gruul && gruul->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT)
        return 0.0f;

    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    return blindeye && blindeye->GetHealthPct() > BLINDEYE_ENGAGED_HEALTH_PCT ? 0.0f : 1.0f;
}

// High King Maulgar <Lord of the Ogres>

float HighKingMaulgarControlTankActionsMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<TankAssistAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action))
    {
        return 1.0f;
    }

    return AI_VALUE2(Unit*, "find target", "high king maulgar") ? 0.0f : 1.0f;
}

float HighKingMaulgarRestrictTauntingMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    bool const isAoeThreat = IsAoeThreatAction(bot, action);
    if (!isAoeThreat && !IsTauntAction(bot, action))
        return 1.0f;

    // The main tank stays on Maulgar the whole time so it can do whatever.
    if (PlayerbotAI::IsMainTank(bot))
        return 1.0f;

    // Blindeye and Olm are tanked next to each other by separate tanks; until Blindeye is dead,
    // don't use AoE threat abilities.
    if (isAoeThreat && AI_VALUE2(Unit*, "find target", "blindeye the seer"))
        return 0.0f;

    // Kiggler is the only ogre for which taunting is a problem because he is the only one that is
    // both (1) tanked by a non-traditional-tank and (2) directed to be attacked by traditional
    // tanks (the Blindeye and Olm tanks after both are down).
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    if (!kiggler)
        return 1.0f;

    if (!GetKigglerMoonkinTank(botAI))
        return 1.0f;

    return AI_VALUE(Unit*, "current target") == kiggler ? 0.0f : 1.0f;
}

float HighKingMaulgarDisableDpsAssistMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<DpsAssistAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "high king maulgar") ? 0.0f : 1.0f;
}

float HighKingMaulgarAvoidWhirlwindMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<HighKingMaulgarRunAwayFromWhirlwindAction*>(action))
        return 1.0f;

    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    if (!maulgar || !maulgar->HasAura(Id(GruulSpells::SPELL_WHIRLWIND)))
        return 1.0f;

    if (PlayerbotAI::IsMainTank(bot))
        return 1.0f;

    return bot->GetExactDist2d(maulgar) < MAULGAR_WHIRLWIND_HOLD_DISTANCE ? 0.0f : 1.0f;
}

float HighKingMaulgarControlHunterActionsMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    bool const isMainTankMisdirect = dynamic_cast<CastMisdirectionOnMainTankAction*>(action);
    if (!isMainTankMisdirect && !dynamic_cast<CastArcaneShotAction*>(action))
        return 1.0f;

    // Krosh/Kiggler will be the last to die before Maulgar.
    // When only Maulgar is left, the standard Misdirection strategy is fine.
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    if (isMainTankMisdirect &&
        (krosh || AI_VALUE2(Unit*, "find target", "kiggler the crazed")))
    {
        return 0.0f;
    }

    // Arcane Shot removes Spell Shield, which the mage tank needs to survive.
    return krosh && action->GetTarget() == krosh ? 0.0f : 1.0f;
}

float HighKingMaulgarControlMageTankActionsMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_MAGE)
        return 1.0f;

    if (action->getThreatType() != Action::ActionThreatType::Aoe &&
        !dynamic_cast<CastIceBlockAction*>(action) &&
        !dynamic_cast<CastInvisibilityAction*>(action))
    {
        return 1.0f;
    }

    if (!AI_VALUE2(Unit*, "find target", "krosh firehand"))
        return 1.0f;

    return GetKroshMageTank(botAI) == bot ? 0.0f : 1.0f;
}

// Gruul the Dragonkiller

float GruulTheDragonkillerControlTankMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<AvoidAoeAction*>(action))
    {
        return 1.0f;
    }

    return AI_VALUE2(Unit*, "find target", "gruul the dragonkiller") ? 0.0f : 1.0f;
}

float GruulTheDragonkillerStaySpreadForShatterMultiplier::GetValueInEncounter(Action* action)
{
    if (!HasGroundSlam(bot))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    return dynamic_cast<GruulTheDragonkillerShatterSpreadAction*>(action) ||
        dynamic_cast<GruulTheDragonkillerGetOutOfCaveInAction*>(action) ? 1.0f : 0.0f;
}

// When near a cave in, ignore the ranged spread, as well as standard movement actions like reaching
// the target, with some exceptions for tanks (and full exception for the active tank on Gruul).
float GruulTheDragonkillerControlAvoidanceMultiplier::GetValueInEncounter(Action* action)
{
    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    bool const isReachTargetSpell = dynamic_cast<CastReachTargetSpellAction*>(action);

    if (PlayerbotAI::IsTank(bot) &&
        (isReachTargetSpell || dynamic_cast<ReachTargetAction*>(action)))
    {
        return 1.0f;
    }

    if (!isReachTargetSpell && !dynamic_cast<MovementAction*>(action))
        return 1.0f;

    if (dynamic_cast<GruulTheDragonkillerGetOutOfCaveInAction*>(action))
        return 1.0f;

    // The shatter spread multiplier takes over during Ground Slam (and allows the Cave In escape).
    if (HasGroundSlam(bot))
        return 1.0f;

    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
    if (!gruul)
        return 1.0f;

    if (gruul->GetVictim() == bot)
        return 1.0f;

    return IsNearCaveIn(botAI, CAVE_IN_CONTROL_RADIUS) ? 0.0f : 1.0f;
}

// MoveTo does not check speed, and thus even with a snare of -100% or more, it starts a spline
// and MoveDelay calculates distance / speed for IsWaitingForLastMove, which is infinite in that
// case and clamps to MaxWaitForMove (5s), blocking all movements for that duration.
float GruulTheDragonkillerHoldWhileSnaredMultiplier::GetValueInEncounter(Action* action)
{
    if (bot->GetSpeed(MOVE_RUN) > 0.0f)
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "gruul the dragonkiller") ? 0.0f : 1.0f;
}
