/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPActions.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "PlayerbotTextMgr.h"
#include "SWPEncounter_Felmyst.h"
#include "SWPShared.h"
#include <cmath>
#include <limits>
#include <string>
#include <vector>

using namespace SwpHelpers;
using namespace EncounterHelpers;

bool FelmystMainTankPositionBossOnGroundAction::Execute(Event /*event*/)
{
    ClearFelmystDemonicVaporKiteState(bot);

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    if (AI_VALUE(Unit*, "current target") != felmyst)
        return Attack(felmyst);

    if (felmyst->GetVictim() != bot || bot->GetHealthPct() < 50.0f ||
        !bot->IsWithinMeleeRange(felmyst))
    {
        return false;
    }

    constexpr float arrivalDist = 3.0f;
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_COMBAT;
    if (IsWaitingForLastMove(priority))
        return false;

    float moveX;
    float moveY;
    bool backwards;
    if (!GetPathStepToPosition(
            bot, GetFelmystMainTankGroundPosition(bot), arrivalDist, felmyst, moveX, moveY,
            backwards))
    {
        return false;
    }

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        priority, true, backwards);
}

bool FelmystRangedStackInThreeGroupsAction::Execute(Event /*event*/)
{
    ClearFelmystDemonicVaporKiteState(bot);

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    Position position;
    if (!TryGetFelmystRangedPosition(bot, felmyst, position))
        return false;

    constexpr MovementPriority priority = MovementPriority::MOVEMENT_COMBAT;
    if (IsWaitingForLastMove(priority))
        return false;

    float moveX;
    float moveY;
    bool backwards;
    if (!GetPathStepToPosition(
            bot, position, FELMYST_RANGED_GROUP_RADIUS, nullptr, moveX, moveY, backwards))
    {
        return false;
    }

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        priority, true, false);
}

bool FelmystMeleeStackBehindBossAction::Execute(Event /*event*/)
{
    ClearFelmystDemonicVaporKiteState(bot);

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    Position position;
    if (!TryGetFelmystGroundStackPosition(bot, felmyst, FelmystGroundStack::Melee, position))
        return false;

    constexpr float arrivalDist = 0.25f;
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_COMBAT;
    if (IsWaitingForLastMove(priority))
        return false;

    float moveX;
    float moveY;
    bool backwards;
    if (!GetPathStepToPosition(bot, position, arrivalDist, nullptr, moveX, moveY, backwards))
        return false;

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        priority, true, false);
}

bool FelmystRemoveEncapsulateAction::Execute(Event /*event*/)
{
    uint32 const spellId = GetSelfImmunitySpell(bot);
    return spellId && botAI->CanCastSpell(spellId, bot) && botAI->CastSpell(spellId, bot);
}

bool FelmystRunAwayFromEncapsulatedPlayerAction::Execute(Event /*event*/)
{
    Player* encapsulateTarget = GetFelmystEncapsulateTarget(bot);
    if (!encapsulateTarget || encapsulateTarget == bot)
        return false;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    FelmystGroundStack const botStack = GetClosestFelmystGroundStack(bot, felmyst, bot);
    FelmystGroundStack const targetStack = GetClosestFelmystGroundStack(
        bot, felmyst, encapsulateTarget);

    if (botStack == FelmystGroundStack::None || targetStack == FelmystGroundStack::None ||
        botStack != targetStack)
    {
        return false;
    }

    auto const tryMoveToStack = [&](FelmystGroundStack stack)
    {
        Position position;
        if (!TryGetFelmystGroundStackPosition(bot, felmyst, stack, position))
            return false;

        constexpr MovementPriority priority = MovementPriority::MOVEMENT_FORCED;
        if (IsWaitingForLastMove(priority))
            return false;

        float moveX;
        float moveY;
        bool backwards;
        if (!GetPathStepToPosition(
                bot, position, FELMYST_RANGED_GROUP_RADIUS, nullptr, moveX, moveY, backwards))
        {
            return false;
        }

        return MoveTo(
            SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
            priority, true, false);
    };

    if (targetStack == FelmystGroundStack::Left || targetStack == FelmystGroundStack::Right)
    {
        if (tryMoveToStack(FelmystGroundStack::Melee))
            return true;

        return tryMoveToStack(targetStack == FelmystGroundStack::Left ?
            FelmystGroundStack::Right : FelmystGroundStack::Left);
    }

    Position leftPosition;
    Position rightPosition;
    if (!TryGetFelmystGroundStackPosition(
            bot, felmyst, FelmystGroundStack::Left, leftPosition) ||
        !TryGetFelmystGroundStackPosition(
            bot, felmyst, FelmystGroundStack::Right, rightPosition))
    {
        return false;
    }

    if (bot->GetExactDist2d(leftPosition) <= bot->GetExactDist2d(rightPosition))
    {
        if (tryMoveToStack(FelmystGroundStack::Left))
            return true;

        return tryMoveToStack(FelmystGroundStack::Right);
    }

    if (tryMoveToStack(FelmystGroundStack::Right))
        return true;

    return tryMoveToStack(FelmystGroundStack::Left);
}

bool FelmystMassDispelGasNovaAction::Execute(Event /*event*/)
{
    Player* gasNovaTarget = GetFelmystGasNovaDispelTarget(bot);
    return gasNovaTarget &&
        botAI->CanCastSpell(Id(SwpSpells::SPELL_MASS_DISPEL), gasNovaTarget) &&
        botAI->CastSpell(Id(SwpSpells::SPELL_MASS_DISPEL), gasNovaTarget);
}

bool FelmystAvoidDemonicVaporAction::Execute(Event /*event*/)
{
    Player* leader = GetFelmystFlightLeader(bot);
    if (!leader)
        return MoveAwayFromVapor(false);

    if (leader == bot)
    {
        if (MarkTargetWithDiamond(bot, leader))
            return true;

        if (leader->GetGUID() != _announcedFlightLeaderGuid)
        {
            _announcedFlightLeaderGuid = leader->GetGUID();
            AnnounceFlightLeader(leader);
        }

        return MoveAwayFromVapor(false);
    }

    constexpr float tooFarDistance = 20.0f;
    if (bot->GetExactDist2d(leader) > tooFarDistance && MoveAwayFromVapor(true))
        return true;

    return MoveToFlightLeader(leader);
}

bool FelmystAvoidDemonicVaporAction::MoveAwayFromVapor(bool unrestricted)
{
    std::vector<Creature*> const hazards = GetDemonicVaporHazards(bot);

    constexpr float hazardRadius = 13.0f;
    bool inDanger = false;
    for (Creature* hazard : hazards)
    {
        if (hazard && bot->GetExactDist2d(hazard) < hazardRadius)
        {
            inDanger = true;
            break;
        }
    }

    if (!inDanger)
        return false;

    constexpr float maxSearchRadius = 40.0f;
    constexpr float distanceStep = 1.0f;

    std::vector<float> angles;
    if (unrestricted)
    {
        for (int i = 0; i < 8; ++i)
            angles.push_back(static_cast<float>(i) * (2.0f * static_cast<float>(M_PI) / 8.0f));
    }
    else
    {
        angles.push_back(0.0f);
        angles.push_back(static_cast<float>(M_PI));
    }

    Position bestPos;
    float minMoveDistance = std::numeric_limits<float>::max();
    bool foundSafe = false;

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    float const botZ = bot->GetPositionZ();

    uint32 const stepCount = static_cast<uint32>(maxSearchRadius / distanceStep);
    for (uint32 step = 0; step <= stepCount; ++step)
    {
        float const distance = static_cast<float>(step) * distanceStep;
        for (float angle : angles)
        {
            float candidateX = botX + distance * std::cos(angle);
            float candidateY = botY + distance * std::sin(angle);

            bool isSafe = true;
            for (Creature* hazard : hazards)
            {
                if (hazard && hazard->GetExactDist2d(candidateX, candidateY) < hazardRadius)
                {
                    isSafe = false;
                    break;
                }
            }

            if (!isSafe)
                continue;

            float candidateZ = botZ;
            bot->GetMap()->CheckCollisionAndGetValidCoords(
                bot, botX, botY, botZ, candidateX, candidateY, candidateZ, false);

            float const moveDistance = bot->GetExactDist2d(candidateX, candidateY);
            if (moveDistance < minMoveDistance)
            {
                bestPos = Position(candidateX, candidateY, candidateZ);
                minMoveDistance = moveDistance;
                foundSafe = true;
            }
        }

        if (foundSafe)
            break;
    }

    if (!foundSafe)
        return false;

    bot->CastStop();
    return MoveTo(
        SWP_MAP_ID, bestPos.GetPositionX(), bestPos.GetPositionY(), bestPos.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool FelmystAvoidDemonicVaporAction::MoveToFlightLeader(Player* leader)
{
    constexpr float followDist = 5.0f;
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_COMBAT;
    if (IsWaitingForLastMove(priority))
        return false;

    float moveX;
    float moveY;
    bool backwards;
    if (!GetPathStepToPosition(
            bot, leader->GetPosition(), followDist, nullptr, moveX, moveY, backwards))
    {
        return false;
    }

    bot->CastStop();
    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        priority, true, false);
}

void FelmystAvoidDemonicVaporAction::AnnounceFlightLeader(Player* leader)
{
    std::string const text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "felmyst_flight_leader",
        "[NAME] is now the flight phase leader. Everybody needs to stack on [NAME] "
        "during the flight phase.",
        std::map<std::string, std::string>{{"[NAME]", leader->GetName()}});

    botAI->SayToRaid(text);
}

bool FelmystKiteDemonicVaporAction::Execute(Event /*event*/)
{
    Position destination;
    if (!TryGetFelmystDemonicVaporKiteDestination(bot, destination))
        return false;

    constexpr float arrivalDist = 0.5f;
    constexpr MovementPriority priority = MovementPriority::MOVEMENT_FORCED;
    if (IsWaitingForLastMove(priority))
        return false;

    float moveX;
    float moveY;
    bool backwards;
    if (!GetPathStepToPosition(bot, destination, arrivalDist, nullptr, moveX, moveY, backwards))
        return false;

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        priority, true, false);
}

bool FelmystMoveToSafeFogLaneAction::Execute(Event /*event*/)
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
    {
        _fogCrateStuckSampleMs = 0;
        return false;
    }

    FogOfCorruptionState fogState;
    bool const hasActiveFog = TryGetActiveFogOfCorruptionState(bot, felmyst, fogState);

    FogLane thirdPassLane = FogLane::None;
    bool const shouldRepositionAfterThirdPass = !hasActiveFog &&
        TryGetFelmystPostThirdPassWindow(felmyst, thirdPassLane);

    if (!hasActiveFog && !shouldRepositionAfterThirdPass)
    {
        _fogCrateStuckSampleMs = 0;
        return false;
    }

    Position destination;
    bool const foundDestination = shouldRepositionAfterThirdPass ?
        TryGetFelmystLandingApproachDestination(bot, thirdPassLane, felmyst, destination) :
        TryGetFelmystFogCrossingDestination(bot, fogState.lane, destination);

    if (!foundDestination)
    {
        _fogCrateStuckSampleMs = 0;
        return false;
    }

    LastMovement const& lastMove = AI_VALUE(LastMovement&, "last movement");
    if (Position(
            lastMove.lastMoveToX, lastMove.lastMoveToY,
            lastMove.lastMoveToZ).GetExactDist(destination) > FELMYST_LOCATION_MATCH_DISTANCE)
    {
        _fogCrateStuckSampleMs = 0;
    }
    else if (TryTeleportStuckBotOntoCrate(destination))
    {
        return true;
    }

    // Try ccing skeletons in place before first pass move
    if (bot->getClass() == CLASS_MAGE &&
        botAI->CanCastSpell("frost nova", bot) && botAI->CastSpell("frost nova", bot))
    {
        return true;
    }

    if (bot->getClass() == CLASS_HUNTER &&
        botAI->CanCastSpell("frost trap", bot) && botAI->CastSpell("frost trap", bot))
    {
        return true;
    }

    bot->CastStop();
    return MoveTo(
        SWP_MAP_ID, destination.GetPositionX(), destination.GetPositionY(),
        destination.GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

bool FelmystMoveToSafeFogLaneAction::TryTeleportStuckBotOntoCrate(
    Position const& destination)
{
    constexpr float collisionCheckDist = 2.0f;
    if (bot->GetExactDist2d(FOG_CRATE_STUCK_POSITION) > collisionCheckDist)
    {
        _fogCrateStuckSampleMs = 0;
        return false;
    }

    uint32 const now = getMSTime();
    float const distToDestination = bot->GetExactDist(destination);

    if (!_fogCrateStuckSampleMs || _fogCrateStuckDestination.GetExactDist(destination) >
        FELMYST_LOCATION_MATCH_DISTANCE)
    {
        _fogCrateStuckDestination = destination;
        _fogCrateStuckNearestDist = distToDestination;
        _fogCrateStuckSampleMs = now;
        return false;
    }

    constexpr float progressResetDist = 1.0f;
    if (distToDestination + progressResetDist < _fogCrateStuckNearestDist)
    {
        _fogCrateStuckNearestDist = distToDestination;
        _fogCrateStuckSampleMs = now;
        return false;
    }

    constexpr uint32 stuckTimeoutMs = 1000;
    if (getMSTimeDiff(_fogCrateStuckSampleMs, now) < stuckTimeoutMs)
        return false;

    Position const& onCratePosition = FOG_CRATE_TELEPORT_POSITION;

    _fogCrateStuckSampleMs = 0;
    bot->CastStop();
    bot->NearTeleportTo(
        onCratePosition.GetPositionX(), onCratePosition.GetPositionY(),
        onCratePosition.GetPositionZ(), bot->GetOrientation());
    return true;
}

bool FelmystMeleeClearTargetAction::Execute(Event /*event*/)
{
    bot->AttackStop();
    bot->InterruptSpell(CURRENT_MELEE_SPELL);
    bot->CastStop();
    context->GetValue<Unit*>("current target")->Set(nullptr);
    bot->SetSelection(ObjectGuid());
    return true;
}

bool FelmystKillCharmedPlayerAction::Execute(Event /*event*/)
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    Player* charmedPlayer = GetFelmystCharmedTarget(bot, felmyst);
    if (!charmedPlayer)
        return false;

    return AI_VALUE(Unit*, "current target") != charmedPlayer && Attack(charmedPlayer);
}

bool FelmystManageLandingDpsTimerAction::Execute(Event /*event*/)
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return false;

    uint32 const instanceId = felmyst->GetInstanceId();
    auto& state = FelmystState(instanceId);

    auto const clearTimers = [&state]
    {
        if (!state.landingDpsWaitStartMs && !state.landingTouchdownMs)
            return false;

        state.landingDpsWaitStartMs = 0;
        state.landingTouchdownMs = 0;
        return true;
    };

    if (felmyst->IsFlying())
    {
        if (!IsFelmystLanding(felmyst))
            return clearTimers();

        if (state.landingDpsWaitStartMs)
            return false;

        state.landingDpsWaitStartMs = getMSTime();
        state.landingTouchdownMs = 0;
        return true;
    }

    // Grounded
    if (!state.landingDpsWaitStartMs)
        return false;

    if (!state.landingTouchdownMs)
    {
        state.landingTouchdownMs = getMSTime();
        return true;
    }

    if (GetMSTimeDiffToNow(state.landingTouchdownMs) < FELMYST_GROUNDED_DPS_WAIT_MS)
        return false;

    return clearTimers();
}
