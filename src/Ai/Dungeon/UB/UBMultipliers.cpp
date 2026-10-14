/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UBMultipliers.h"
#include "AttackAction.h"
#include "ChooseTargetActions.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"
#include "UBActions.h"
#include "UBShared.h"

using namespace UnderbogHungarfen;

float HungarfenFoulSporesMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) && !dynamic_cast<CastReachTargetSpellAction*>(action))
        return 1.0f;

    if (dynamic_cast<UBRetreatFromFoulSporesAction*>(action) || dynamic_cast<UBVacateSporeCloudAction*>(action) ||
        dynamic_cast<AttackAction*>(action))
        return 1.0f;

    Unit* boss = AI_VALUE2(Unit*, "find target", "hungarfen");
    if (!boss || !boss->HasAura(SPELL_FOUL_SPORES))
        return 1.0f;

    return 0.0f;
}

float HungarfenMushroomIgnoreMultiplier::GetValue(Action* action)
{
    bool const aoe = action->getThreatType() == Action::ActionThreatType::Aoe;
    if (!aoe && !dynamic_cast<AttackAnythingAction*>(action))
        return 1.0f;

    if (aoe && (dynamic_cast<CastHealingSpellAction*>(action) || !PlayerbotAI::IsDps(bot)))
        return 1.0f;

    auto const& mushrooms = AI_VALUE_REF(GuidVector, "ub mushrooms");
    if (!AnyMushroomAlive(bot, mushrooms))
        return 1.0f;

    if (!aoe)
        return IsMushroom(AI_VALUE(Unit*, "grind target")) ? 0.0f : 1.0f;

    return 0.0f;
}

float UnderbatFacingMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    auto const& attackers = AI_VALUE_REF(GuidVector, "attackers");
    return AnyUnderbatInLashRange(bot, attackers) ? 0.0f : 1.0f;
}
