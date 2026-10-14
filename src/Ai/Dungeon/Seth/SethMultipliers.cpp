/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SethMultipliers.h"
#include "EncounterHelpers.h"
#include "FollowActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"
#include "SethActions.h"
#include "SethShared.h"
#include "ShamanActions.h"

using namespace SethShared;
using namespace EncounterHelpers;

float SethekkProphetSetTremorTotemMultiplier::GetValue(Action* action)
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

    return AI_VALUE2(Unit*, "find target", "sethekk prophet") ? 0.0f : 1.0f;
}

float AnzuControlSpellCastingWithSpellBombMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<CastSpellAction*>(action))
        return 1.0f;

    if (bot->getPowerType() != POWER_MANA || PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!bot->HasAura(Id(SethSpells::SPELL_SPELL_BOMB)))
        return 1.0f;

    if (PlayerbotAI::IsDps(bot))
        return 0.0f;

    // For healer
    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank)
        return 1.0f;

    return mainTank->GetHealthPct() > 50.0f ? 0.0f : 1.0f;
}

float TalonKingIkissDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!dynamic_cast<CastHeroismAction*>(action) &&
        !dynamic_cast<CastBloodlustAction*>(action))
    {
        return 1.0f;
    }

    Unit* ikiss = AI_VALUE2(Unit*, "find target", "talon king ikiss");
    if (!ikiss)
        return 1.0f;

    return ikiss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT ? 0.0f : 1.0f;
}

float TalonKingIkissControlMovementMultiplier::GetValue(Action* action)
{
    bool const isAlwaysDisabled =
        dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action);

    if (!isAlwaysDisabled &&
        !dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<TankFaceAction*>(action) ||
        dynamic_cast<SetBehindTargetAction*>(action) ||
        dynamic_cast<TalonKingIkissLosArcaneExplosionAction*>(action))
    {
        return 1.0f;
    }

    Unit* ikiss = AI_VALUE2(Unit*, "find target", "talon king ikiss");
    if (!ikiss)
        return 1.0f;

    if (isAlwaysDisabled)
        return 0.0f;

    return ikiss->HasAura(Id(SethSpells::SPELL_ARCANE_BUBBLE)) ? 0.0f : 1.0f;
}
