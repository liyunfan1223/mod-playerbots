/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MgTMultipliers.h"
#include "AiObjectContext.h"
#include "AttackersValue.h"
#include "ChooseTargetActions.h"
#include "FollowActions.h"
#include "GenericSpellActions.h"
#include "MgTActions.h"
#include "MgTShared.h"
#include "MovementActions.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"
#include "ShamanActions.h"

float MgTCrystalFocusMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<MgTKillCrystalAction*>(action))
        return 1.0f;

    bool const suppressed =
        dynamic_cast<DpsAssistAction*>(action) ||
        dynamic_cast<TankAssistAction*>(action) ||
        dynamic_cast<CastDebuffSpellOnAttackerAction*>(action) ||
        (action->getThreatType() == Action::ActionThreatType::Aoe && PlayerbotAI::IsDps(bot) &&
         !dynamic_cast<CastHealingSpellAction*>(action));

    if (!suppressed)
        return 1.0f;

    if (PlayerbotAI::IsHeal(bot))
        return 1.0f;

    if (AI_VALUE(ObjectGuid, "mgt crystal target").IsEmpty())
        return 1.0f;

    return 0.0f;
}

float MgTFocusOrderMultiplier::GetValue(Action* action)
{
    bool const picker =
        dynamic_cast<DpsAssistAction*>(action) ||
        dynamic_cast<DpsAoeAction*>(action) ||
        dynamic_cast<TankAssistAction*>(action);

    bool const drop = !picker && dynamic_cast<DropTargetAction*>(action) != nullptr;

    if (!picker && !drop)
        return 1.0f;

    if (!bot->IsInCombat())
        return 1.0f;

    if (PlayerbotAI::IsHeal(bot) || PlayerbotAI::IsTank(bot) || !PlayerbotAI::IsDps(bot))
        return 1.0f;

    if (!AI_VALUE(ObjectGuid, "mgt crystal target").IsEmpty())
        return 1.0f;

    ObjectGuid const order = MagistersTerrace::ResolveFocusOrder(botAI);
    if (order.IsEmpty())
        return 1.0f;

    if (!drop)
        return 0.0f;

    Unit* current = AI_VALUE(Unit*, "current target");
    if (!current || current->GetGUID() != order)
        return 1.0f;
    if (!current->IsAlive() || !AttackersValue::IsValidTarget(current, bot))
        return 1.0f;

    return 0.0f;
}

float MgTFocusBurstMultiplier::GetValue(Action* action)
{
    if (action->getThreatType() != Action::ActionThreatType::Aoe)
        return 1.0f;

    if (dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    if (PlayerbotAI::IsHeal(bot) || !PlayerbotAI::IsDps(bot))
        return 1.0f;

    switch (MagistersTerrace::ResolveFocusOrderOwner(botAI))
    {
        case MagistersTerrace::FocusOrder::Kael:
        case MagistersTerrace::FocusOrder::Delrissa:
            return 0.0f;
        default:
            return 1.0f;
    }
}

float MgTRoomLeashMultiplier::GetValue(Action* action)
{
    bool const isFlight =
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<FleeWithPetAction*>(action) ||
        dynamic_cast<RunAwayAction*>(action);

    if (!isFlight)
        return 1.0f;

    if (bot->GetPositionX() > MagistersTerrace::SELIN_SAFE_X + MagistersTerrace::SELIN_FLEE_SLACK)
        return 1.0f;

    if (!MagistersTerrace::GetSelin(bot))
        return 1.0f;

    return 0.0f;
}

float MgTDampeningFieldMultiplier::GetValue(Action* action)
{
    bool const isCompeting =
        dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<AvoidAoeAction*>(action);

    if (!isCompeting)
        return 1.0f;

    auto const& spots = AI_VALUE_REF(MagistersTerrace::EscapeSpots, "mgt dampening escape");
    if (spots.empty())
        return 1.0f;

    return 0.0f;
}

float MgTDelrissaTremorTotemMultiplier::GetValue(Action* action)
{
    bool const isCompeting =
        dynamic_cast<CastStrengthOfEarthTotemAction*>(action) ||
        dynamic_cast<CastStoneskinTotemAction*>(action) ||
        dynamic_cast<CastEarthbindTotemAction*>(action);

    if (!isCompeting)
        return 1.0f;

    if (!AI_VALUE(bool, "mgt delrissa tremor totem"))
        return 1.0f;

    return 0.0f;
}

float MgTFlameStrikeMultiplier::GetValue(Action* action)
{
    bool const isCompeting =
        dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<AvoidAoeAction*>(action);

    if (!isCompeting)
        return 1.0f;

    auto const& spots = AI_VALUE_REF(MagistersTerrace::EscapeSpots, "mgt flame strike escape");
    if (spots.empty())
        return 1.0f;

    return 0.0f;
}

float MgTGravityLapseMultiplier::GetValue(Action* action)
{
    bool const isCompeting =
        dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<ReachTargetAction*>(action) ||
        dynamic_cast<CastReachTargetSpellAction*>(action);

    if (!isCompeting)
        return 1.0f;

    if (!AI_VALUE(bool, "mgt gravity lapse"))
        return 1.0f;

    return 0.0f;
}

float MgTKaelUnattackableMultiplier::GetValue(Action* action)
{
    bool const suppressed =
        dynamic_cast<DpsAssistAction*>(action) ||
        dynamic_cast<TankAssistAction*>(action) ||
        dynamic_cast<CastDebuffSpellOnAttackerAction*>(action) ||
        (action->getThreatType() == Action::ActionThreatType::Aoe && PlayerbotAI::IsDps(bot) &&
         !dynamic_cast<CastHealingSpellAction*>(action));

    if (!suppressed)
        return 1.0f;

    if (PlayerbotAI::IsHeal(bot))
        return 1.0f;

    if (!AI_VALUE(bool, "mgt kael unattackable"))
        return 1.0f;

    return 0.0f;
}

float MgTPhoenixBurnMultiplier::GetValue(Action* action)
{
    bool const isCompeting =
        dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<AvoidAoeAction*>(action) ||
        dynamic_cast<ReachTargetAction*>(action) ||
        dynamic_cast<CastReachTargetSpellAction*>(action);

    if (!isCompeting)
        return 1.0f;

    MagistersTerrace::PhoenixRing const ring = AI_VALUE(MagistersTerrace::PhoenixRing, "mgt phoenix ring");
    if (ring != MagistersTerrace::PhoenixRing::None && dynamic_cast<SetBehindTargetAction*>(action))
        return 0.0f;

    auto const& spots = AI_VALUE_REF(MagistersTerrace::EscapeSpots, "mgt phoenix escape");
    if (!spots.empty())
        return 0.0f;

    if (ring == MagistersTerrace::PhoenixRing::Covers)
        return 0.0f;

    return 1.0f;
}
