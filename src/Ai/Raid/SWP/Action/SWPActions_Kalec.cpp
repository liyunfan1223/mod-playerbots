/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPActions.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "PlayerbotTextMgr.h"
#include "SWPEncounter_Kalec.h"
#include "SWPShared.h"
#include <cmath>
#include <map>
#include <string>

using namespace SwpHelpers;
using namespace EncounterHelpers;

bool KalecgosAnnounceBossHealthAction::Execute(Event /*event*/)
{
    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    if (!kalecgos)
        return false;

    KalecgosEncounterState* foundState = FindKalecgosState(bot->GetInstanceId());
    if (!foundState)
        return false;

    KalecgosEncounterState& state = *foundState;
    std::string text;

    if (!IsInSpectralRealm(bot))
    {
        if (state.surfaceHealthAnnounced)
            return false;

        state.surfaceHealthAnnounced = true;

        text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "kalecgos_below_twenty_percent_health",
            "Kalecgos's health is at 20%!",
            {});
    }
    else
    {
        if (state.spectralHealthAnnounced)
            return false;

        Unit* sathrovarr = AI_VALUE2(Unit*, "find target", "sathrovarr the corruptor");
        if (!sathrovarr)
            return false;

        state.spectralHealthAnnounced = true;

        std::string const sathrovarrHealth = std::to_string(
            static_cast<uint32>(sathrovarr->GetHealthPct()));

        text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "sathrovarr_health_when_kalecgos_below_twenty_percent_health",
            "Sathrovarr's health is at %sathrovarrHealth%! "
            "Don't forget that we need to defeat them at about the same time!",
            std::map<std::string, std::string>{
                {"%sathrovarrHealth", sathrovarrHealth}});
    }

    return botAI->SayToRaid(text);
}

bool KalecgosSurfaceTankPositionDragonAction::Execute(Event event)
{
    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    if (!kalecgos)
        return false;

    if (AI_VALUE(Unit*, "current target") != kalecgos)
        return Attack(kalecgos);

    // This action also runs for a tank that does not have aggro on Kalecgos, so whether to move
    // backwards is kicked to GetStepToPosition(), which makes backwards movement contingent on
    // the bot being Kalecgos's victim.
    constexpr float arrivalDist = 3.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (GetStepToPosition(
            bot, KALECGOS_TANK_POSITION, arrivalDist, kalecgos, moveX, moveY, backwards))
    {
        return MoveTo(
            SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_COMBAT, true, backwards);
    }

    if (GetKalecgosDesignatedTank(bot) == bot && kalecgos->GetVictim() != bot)
        return botAI->DoSpecificAction("taunt spell", event, true);

    return false;
}

// FYI, there is a (Blizzlike) delay of 1s after a player clicks a rift before the next can click.
bool KalecgosEnterSpectralRiftAction::Execute(Event /*event*/)
{
    // Special conditions for tanks only
    if (PlayerbotAI::IsTank(bot) && !ShouldTankEnter())
        return false;

    GameObject* rift = botAI->GetGameObject(AI_VALUE(ObjectGuid, "kalecgos spectral rift"));
    if (!rift)
        return false;

    if (rift->IsAtInteractDistance(*bot, rift->GetInteractionDistance()))
    {
        rift->Use(bot);
        return true;
    }

    float const targetDist = rift->GetInteractionDistance() - 0.5f;
    float const angle = rift->GetAngle(bot);
    float const destX = rift->GetPositionX() + std::cos(angle) * targetDist;
    float const destY = rift->GetPositionY() + std::sin(angle) * targetDist;

    return MoveTo(
        SWP_MAP_ID, destX, destY, rift->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

bool KalecgosEnterSpectralRiftAction::ShouldTankEnter()
{
    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    if (!kalecgos)
        return false;

    // The current tank cannot enter a portal until the next tank takes over. If the designated
    // tank is still this bot, nobody has taken over yet.
    Player* surfaceTank = GetKalecgosDesignatedTank(bot);
    if (!surfaceTank || surfaceTank == bot)
        return false;

    Position const& position = KALECGOS_TANK_POSITION;
    if (surfaceTank->GetExactDist2d(position) > 3.0f || kalecgos->GetVictim() != surfaceTank)
        return false;

    return true;
}

bool KalecgosDisperseRangedAction::Execute(Event /*event*/)
{
    if (!_initialRangedPositionReached)
    {
        Position const& initialPos = KALECGOS_INITIAL_RANGED_POSITION;
        constexpr float initialRangedRadius = 10.0f;

        if (bot->GetExactDist2d(initialPos) <= initialRangedRadius)
        {
            _initialRangedPositionReached = true;
            return false;
        }

        // The point on the circle MoveInside() would head for, so the ranged still fan out
        float const angle = GetFollowAngle();
        Position const rimPosition(
            initialPos.GetPositionX() + std::cos(angle) * initialRangedRadius,
            initialPos.GetPositionY() + std::sin(angle) * initialRangedRadius,
            initialPos.GetPositionZ());

        constexpr float arrivalDist = 0.0f;
        float moveX;
        float moveY;
        bool backwards;
        if (!GetStepToPosition(bot, rimPosition, arrivalDist, nullptr, moveX, moveY, backwards))
            return false;

        return MoveTo(
            SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    if (Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos"))
    {
        constexpr float safeDistFromDragon = 20.0f;
        constexpr uint32 minInterval = 0;
        if (bot->GetExactDist2d(kalecgos) < safeDistFromDragon)
            return FleePosition(kalecgos->GetPosition(), safeDistFromDragon, minInterval);
    }

    constexpr float safeDistFromPlayer = 6.0f;
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer);
    if (!nearestPlayer)
        return false;

    return FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer);
}

bool KalecgosSathrovarrTankStandWithKalecAction::Execute(Event /*event*/)
{
    Unit* sathrovarr = AI_VALUE2(Unit*, "find target", "sathrovarr the corruptor");
    if (!sathrovarr)
        return false;

    constexpr float searchRadius = 20.0f;
    Creature* kalec = bot->FindNearestCreature(Id(SwpNpcs::NPC_KALECGOS_HUMANOID), searchRadius);
    if (!kalec || sathrovarr->GetVictim() != kalec)
        return false;

    constexpr float arrivalDist = 3.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, kalec->GetPosition(), arrivalDist, nullptr, moveX, moveY, backwards))
    {
        return false;
    }

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool KalecgosReturnToSpectralRealmGroundAction::Execute(Event /*event*/)
{
    bot->NearTeleportTo(
        bot->GetPositionX(), bot->GetPositionY(), SPECTRAL_REALM_Z, bot->GetOrientation());

    constexpr float zTolerance = 1.0f;
    return std::fabs(bot->GetPositionZ() - SPECTRAL_REALM_Z) <= zTolerance;
}
